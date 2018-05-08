#include "connectionsettings.h"
#include "ui_connectionsettings.h"

connectionSettings::connectionSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::connectionSettings)
{
    ui->setupUi(this);
    /*setting default values to ui*/
    ui->ipAddressEdit->setText(connection_settings.ipAddress);
    ui->responeTimeoutSpinBox->setValue(connection_settings.responseTime);
    ui->serverPortEdit->setText(QString::number(connection_settings.port));
    ui->parityComboBox->setCurrentIndex(1);
    ui->baudRateComboBox->setCurrentText(QString::number(connection_settings.baud));
    ui->dataBitsComboBox->setCurrentText(QString::number(connection_settings.dataBits));
    ui->stopBitsComboBox->setCurrentText(QString::number(connection_settings.stopBits));
    ui->numberofRetiresSpinBox->setValue(connection_settings.numberOfRetries);
    //saving settings
    connect(ui->saveButton,&QPushButton::clicked,[this](){
        connection_settings.ipAddress=ui->ipAddressEdit->text();
        connection_settings.responseTime=ui->responeTimeoutSpinBox->value();
        connection_settings.port=ui->serverPortEdit->text().toInt();
        connection_settings.connectType=ui->connectionTypeComboBox->currentIndex();
        connection_settings.parity=ui->parityComboBox->currentIndex();
        if (connection_settings.parity > 0){
            connection_settings.parity++;
        }
        connection_settings.baud=ui->baudRateComboBox->currentText().toInt();
        connection_settings.dataBits=ui->dataBitsComboBox->currentText().toInt();
        connection_settings.stopBits=ui->stopBitsComboBox->currentText().toInt();
        connection_settings.numberOfRetries=ui->numberofRetiresSpinBox->value();
        hide();
    });
    connect(ui->cancelButton,QPushButton::clicked,[this](){hide();});
    connect(ui->connectionTypeComboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&connectionSettings::showConnetionTypeSettings);
    showConnetionTypeSettings();
}

void connectionSettings::showConnetionTypeSettings(){
    //swiching between connection type settings
    if(ui->connectionTypeComboBox->currentIndex()==Tcp){
        ui->TCPIPSettings->show();
        ui->serialSettings->hide();
    }else{
        ui->TCPIPSettings->hide();
        ui->serialSettings->show();
    }
}

settings connectionSettings::get_settings(){
    return connection_settings;
}

connectionSettings::~connectionSettings()
{
    delete ui;
}
