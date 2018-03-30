#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    modbusReply = nullptr;
    modbusClient = nullptr;
    connectionSettingsDialog = new connectionSettings(this);
    ui->setupUi(this);
    ui->actionConnection_Settings->setEnabled(true);
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    connect(ui->clearLogsButton,&QPushButton::clicked,ui->logsArea,&QPlainTextEdit::clear);
    connect(ui->actionConnection_Settings,&QAction::triggered,connectionSettingsDialog,&QDialog::show);
    connect(ui->actionConnect, &QAction::triggered,this, &MainWindow::connect_button_clicked);
    connect(ui->actionDisconnect,&QAction::triggered,this,&MainWindow::disconnect_button_clicked);
    connect(ui->sendFrameButton,&QPushButton::clicked,this,&MainWindow::send_button_clicked);
    connect(ui->startingAddressSpinBox,QOverload<int>::of(&QSpinBox::valueChanged),this,&MainWindow::update_table_data);
    connect(ui->numberOfCoilsSpinBox,QOverload<int>::of(&QSpinBox::valueChanged),this,&MainWindow::update_table_data);
}

void MainWindow::update_table_data(){
    int rowcount = 0;
    QStringList tableHeader;
    QStringList tableVerticalHeader;
    if(ui->numberOfCoilsSpinBox->value()<10){
       ui->tableWidget->setColumnCount(ui->numberOfCoilsSpinBox->value());
    }else{
        ui->tableWidget->setColumnCount(10);
    }
    for(int i=0;i<ui->numberOfCoilsSpinBox->value();i++){
        tableHeader<<"+"+QString::number(i);
    }
    switch(ui->functionCodeComboBox->currentIndex()){
        case 1:
            int start = ui->startingAddressSpinBox->value()+40000;
            int end = ui->startingAddressSpinBox->value()+40000+ui->numberOfCoilsSpinBox->value()-1;
                qDebug() << "start"<<start;
                qDebug() << "end"<<end;
            if((end-start)<=10){
                tableVerticalHeader << QString::number(start)+"-"+QString::number(end);
                rowcount++;
            }else{
                for(int i=start;i<end;i+=10){
                    if(i+10>=end){
                        tableVerticalHeader << QString::number(i)+"-"+QString::number(end);
                    }else{
                        tableVerticalHeader << QString::number(i)+"-"+QString::number(i+9);
                    }
                    rowcount++;
                }
            }
            //tableVerticalHeader<<
        break;
    }
    ui->tableWidget->setRowCount(rowcount);
    ui->tableWidget->setHorizontalHeaderLabels(tableHeader);
    ui->tableWidget->setVerticalHeaderLabels(tableVerticalHeader);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setShowGrid(true);
    //ui->tableWidget->setItem(0, 1, new QTableWidgetItem("Hello"));
}

void MainWindow::send_button_clicked(){
    qDebug() << "send button clicked";
    if (!modbusClient){
        ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+" - Not connected");
        return;
    }
    update_table_data();
    QModbusDataUnit readRequest;
    readRequest.setRegisterType(QModbusDataUnit::HoldingRegisters);
    readRequest.setStartAddress(ui->startingAddressSpinBox->value());
    readRequest.setValueCount(ui->numberOfCoilsSpinBox->value());
    modbusReply = modbusClient->sendReadRequest(readRequest,ui->slaveIDSpinBox->value());
    connect(modbusReply, &QModbusReply::finished, this, &MainWindow::read_reply);
}

void MainWindow::read_reply(){
    if(!modbusReply){
        return;
    }
    if(modbusReply->error()==QModbusDevice::NoError){
        qDebug() << "No error!";
        QModbusDataUnit data = modbusReply->result();
        for(uint i=0;i<data.valueCount();i++){
            ui->tableWidget->setItem(0,i, new QTableWidgetItem(QString::number(data.value(i))));
        }
        ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+
                                      " Read register from "+QString::number(data.startAddress())+
                                      " for "+QString::number(data.valueCount()));
    }else if(modbusReply->error()==QModbusDevice::ProtocolError){
        ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+
                                      "Read respone error: "+modbusReply->errorString()+
                                      "(Modbus exception: 0x"+modbusReply->rawResult().exceptionCode()+").");
    }else{
        ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+
                                      "RRead response error: "+modbusReply->errorString()+
                                      "(code: 0x"+modbusReply->error()+").");
    }
    delete modbusReply;
    modbusReply=nullptr;
}

void MainWindow::connect_button_clicked(){
    if (modbusClient) {
        modbusClient->disconnectDevice();
        delete modbusClient;
        modbusClient = nullptr;
    }
    if(connectionSettingsDialog->get_settings().connectType==Serial){
        //modbusClient new Qmodbusserialclient(this);
        ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+" - Serial unsuportted yet");
        return;
    }else if(connectionSettingsDialog->get_settings().connectType==Tcp){
        modbusClient = new QModbusTcpClient(this);
        if(!modbusClient){
            statusBar()->showMessage(tr("Could not create Modbus client."), 5000);
            return;
        }
        modbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter,connectionSettingsDialog->get_settings().ipAddress);
        modbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter,connectionSettingsDialog->get_settings().port);
    }
    modbusClient->setTimeout(1000);
    modbusClient->setNumberOfRetries(3);
    connect(modbusClient,&QModbusClient::errorOccurred,this,&MainWindow::error);
    connect(modbusClient,&QModbusClient::stateChanged,this,&MainWindow::state_changed);
    modbusClient->connectDevice();
}

void MainWindow::error(){
    ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+" - "+modbusClient->errorString());
}

void MainWindow::state_changed(){
    if(modbusClient->state()==QModbusDevice::ConnectedState){
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        statusBar()->showMessage("Connected",0);
        ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+" - Connected");
    }else if(modbusClient->state()==QModbusDevice::ConnectingState){
        statusBar()->showMessage("Connecting",0);
    }else if(modbusClient->state()==QModbusDevice::UnconnectedState){
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
        //jesli blad polaczenia to nich wypisze blad tutaj
        statusBar()->showMessage("Disconnected",0);
    }
}

void MainWindow::disconnect_button_clicked(){
    if (modbusClient) {
        modbusClient->disconnectDevice();
        delete modbusClient;
        modbusClient = nullptr;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
