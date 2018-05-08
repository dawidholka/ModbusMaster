#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
TODO
Wyswietlanie w różnych formatach HEX,DEC
Popraw odpowiedzi PDU itp
*/

enum modbus_functions{
    read_coil_status,
    read_input_status,
    read_holding_registers,
    read_input_registers,
    force_multiple_coils,
    preset_multiple_registers,
    preset_single_register,
    force_single_coil
};

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
    connect(ui->functionCodeComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&MainWindow::update_table_data);
    connect(ui->formatComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&MainWindow::update_number_format);
    currentSystem=(number_systems)ui->formatComboBox->currentIndex();
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setShowGrid(true);
    update_table_data();
    statusBar()->showMessage("Disconnected",0);
}

void MainWindow::update_number_format(){
    //TODO changing format numbers
}

void MainWindow::update_table_data(){
    // jesli ilosc danych przekracza dostepny zakres
    if((ui->startingAddressSpinBox->value()+ui->numberOfCoilsSpinBox->value())>10000){
        ui->numberOfCoilsSpinBox->setValue(10000-ui->startingAddressSpinBox->value());
    }
    int max_per_line;
    int max_plus;
    if((ui->functionCodeComboBox->currentIndex()<read_holding_registers)||(ui->functionCodeComboBox->currentIndex()==force_multiple_coils)){
        max_per_line=16;
        max_plus=15;
    }else{
        max_per_line=10;
        max_plus=9;
    }
    int rowcount = 0;
    QStringList tableHeader;
    QStringList tableVerticalHeader;
    if(ui->numberOfCoilsSpinBox->value()<max_per_line){
       ui->tableWidget->setColumnCount(ui->numberOfCoilsSpinBox->value());
    }else{
        ui->tableWidget->setColumnCount(max_per_line);
    }
    for(int i=0;i<max_per_line;i++){
        tableHeader<<"+"+QString::number(i);
    }
    // Disable editing in read functions
    switch(ui->functionCodeComboBox->currentIndex()){
        case force_multiple_coils:
        case preset_multiple_registers:
            ui->tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked);
        break;
        default:
            ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        break;
    }
            int start = ui->startingAddressSpinBox->value();
            int end = ui->startingAddressSpinBox->value()+ui->numberOfCoilsSpinBox->value();
                for(int i=start;i<end;i+=max_per_line){
                    if(i+max_per_line>=end){
                        tableVerticalHeader << QString::number(i)+"-"+QString::number(end-1);
                    }else{
                        tableVerticalHeader << QString::number(i)+"-"+QString::number(i+max_plus);
                    }
                    rowcount++;
                }
    //custom this bottom
    ui->tableWidget->setRowCount(rowcount);
    ui->tableWidget->setHorizontalHeaderLabels(tableHeader);
    ui->tableWidget->setVerticalHeaderLabels(tableVerticalHeader);
}

QModbusDataUnit MainWindow::make_request() const{
    QModbusDataUnit::RegisterType register_type;
    switch(ui->functionCodeComboBox->currentIndex()){
        case force_multiple_coils:
        case read_coil_status:
            register_type = QModbusDataUnit::Coils;
        break;
        case read_input_status:
            register_type = QModbusDataUnit::DiscreteInputs;
        break;
        case preset_multiple_registers:
        case read_holding_registers:
            register_type = QModbusDataUnit::HoldingRegisters;
        break;
        case read_input_registers:
            register_type = QModbusDataUnit::InputRegisters;
        break;
    }
    return QModbusDataUnit(register_type, ui->startingAddressSpinBox->value(), ui->numberOfCoilsSpinBox->value());
}



void MainWindow::readReady(){
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()
                                      +" - Received PDU: "
                                      +(reply->rawResult().data().toHex())
                );
        qDebug()<<reply->rawResult().data();
        const QModbusDataUnit unit = reply->result();
        QVector<quint16> data(unit.values());
        for(uint i=0;i<unit.valueCount();i++){
            ui->tableWidget->setItem(0,i,new QTableWidgetItem(QString::number(data.value(i))));
        }
/*
        for (uint i = 0; i < unit.valueCount(); i++) {
            QString value = QString::number(unit.value(i),
                                       (unit.registerType()<=QModbusDataUnit::Coils ? 10: 16));
            ui->tableWidget->setItem(0,i, new QTableWidgetItem(value));
            // TEST


            const QString entry = tr("Address: %1, Value: %2").arg(unit.startAddress() + i)
                                     .arg(QString::number(unit.value(i),
                                          unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
        }*/
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16), 5000);
    }

    reply->deleteLater();
}

void MainWindow::write_request(){
    /* Making write request
     * QModbusReply *QModbusClient::sendWriteRequest(const QModbusDataUnit &write, int serverAddress)
     * Sends a request to modify the contents of the data pointed by write. Returns a valid QModbusReply if no error ouccred, otherwhise return nullptr.
     * 	QModbusDataUnit(RegisterType type, int address, quint16 size) <- przygotowanie ModbusDataUnit
     * Trzeba jescze wypełnić DataUnit danymi
     * TODO Add handling hex numbers
     */
    qDebug() << "Write request";
    // Setting register type, starting addres, size to DataUnit
    QModbusDataUnit write_data_unit(make_request());
    // Getting data from table to ModbusDataUnit
    // Data must be in QVector<quint16>
    QVector<quint16> coils(ui->numberOfCoilsSpinBox->value());
    int rows = ui->tableWidget->rowCount();
    int columns = ui->tableWidget->columnCount();
    int k = 0;
    for(int i=0;i<rows;i++){
        for(int j=0;j<columns;j++){
            coils[k]=ui->tableWidget->item(i,j)->text().toUInt();
            ++k;
        }
    }
    for(uint i=0;i<write_data_unit.valueCount();i++){
        write_data_unit.setValue(i, coils.value(i));
    }
    if (auto *reply = modbusClient->sendWriteRequest(write_data_unit, ui->slaveIDSpinBox->value())) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);
                } else if (reply->error() != QModbusDevice::NoError) {
                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }else{
                    qDebug() << "sukces!";
                    ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()
                                                  +" - Received PDU: "
                                                  +(reply->rawResult().data().toHex())
                            );
                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()
                                          +" - Received PDU: "+(QString::number(reply->serverAddress()))+(reply->rawResult().data().toHex()));
            reply->deleteLater();
        }
    } else {
        statusBar()->showMessage(tr("Write error: ") + modbusClient->errorString(), 5000);
    }
}

void MainWindow::send_button_clicked(){
    if(!modbusClient){
        ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+" - Device is not connected");
        return;
    }
    if(ui->functionCodeComboBox->currentIndex()<force_multiple_coils){
        if (auto *reply = modbusClient->sendReadRequest(make_request(), ui->slaveIDSpinBox->value())) {
            if (!reply->isFinished())
                connect(reply, &QModbusReply::finished, this, &MainWindow::readReady);
            else
                delete reply; // broadcast replies return immediately
        } else {
            statusBar()->showMessage(tr("Read error: ") + modbusClient->errorString(), 5000);
        }
    }else{
        write_request();
    }
}

void MainWindow::connect_button_clicked(){
    if (modbusClient) {
        modbusClient->disconnectDevice();
        delete modbusClient;
        modbusClient = nullptr;
    }
    if(connectionSettingsDialog->get_settings().connectType==Serial){
        modbusClient = new QModbusRtuSerialMaster(this);
        if(!modbusClient){
            ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()
                                          +" - Could not create Modbus client.");
            return;
        }
        modbusClient->setConnectionParameter(QModbusDevice::SerialPortNameParameter,connectionSettingsDialog->get_settings().serialPortName);
        modbusClient->setConnectionParameter(QModbusDevice::SerialParityParameter,connectionSettingsDialog->get_settings().parity);
        modbusClient->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,connectionSettingsDialog->get_settings().baud);
        modbusClient->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,connectionSettingsDialog->get_settings().dataBits);
        modbusClient->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,connectionSettingsDialog->get_settings().stopBits);
        ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+" - Serial unsuportted yet");
    }else if(connectionSettingsDialog->get_settings().connectType==Tcp){
        modbusClient = new QModbusTcpClient(this);
        if(!modbusClient){
            ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()
                                          +" - Could not create Modbus client.");
            return;
        }
        modbusClient->setConnectionParameter(QModbusDevice::NetworkAddressParameter,connectionSettingsDialog->get_settings().ipAddress);
        modbusClient->setConnectionParameter(QModbusDevice::NetworkPortParameter,connectionSettingsDialog->get_settings().port);
        modbusClient->setTimeout(connectionSettingsDialog->get_settings().responseTime);
    }
    modbusClient->setNumberOfRetries(3);
    connect(modbusClient,&QModbusClient::errorOccurred,this,&MainWindow::error);
    connect(modbusClient,&QModbusClient::stateChanged,this,&MainWindow::connection_state_changed);
    modbusClient->connectDevice();
}

void MainWindow::error(){
    ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+" - "+modbusClient->errorString());
}

void MainWindow::connection_state_changed(){
    if(modbusClient->state()==QModbusDevice::ConnectedState){
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        statusBar()->showMessage("Connected",0);
        if(connectionSettingsDialog->get_settings().connectType==Tcp){
            ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+" - Connected to "+connectionSettingsDialog->get_settings().ipAddress+" on port "+QString::number(connectionSettingsDialog->get_settings().port)+".");
        }else{
            ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+" - Connected to "+connectionSettingsDialog->get_settings().serialPortName+".");
        }
    }else if(modbusClient->state()==QModbusDevice::ConnectingState){
        statusBar()->showMessage("Connecting",0);
    }else if(modbusClient->state()==QModbusDevice::UnconnectedState){
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
        ui->logsArea->appendPlainText(QDateTime::currentDateTime().toString()+" - Connection closed.");
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
    disconnect_button_clicked();
    delete ui;
}
