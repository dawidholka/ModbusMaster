#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    modbusClient = nullptr;
    connectionSettingsDialog = new connectionSettings(this);
    ui->setupUi(this);
    ui->actionConnection_Settings->setEnabled(true);
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    connect(ui->actionConnection_Settings,&QAction::triggered,connectionSettingsDialog,&QDialog::show);
    connect(ui->actionConnect, &QAction::triggered,this, &MainWindow::connect_button_clicked);
    connect(ui->actionDisconnect,&QAction::triggered,this,&MainWindow::disconnect_button_clicked);
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
