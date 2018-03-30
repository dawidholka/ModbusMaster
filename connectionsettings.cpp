#include "connectionsettings.h"
#include "ui_connectionsettings.h"

connectionSettings::connectionSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::connectionSettings)
{
    ui->setupUi(this);
    ui->ipAddressEdit->setText(connection_settings.ipAddress);
    ui->connectionTimeoutEdit->setText(QString::number(connection_settings.responseTime));
    ui->serverPortEdit->setText(QString::number(connection_settings.port));
    connect(ui->saveButton,&QPushButton::clicked,[this](){
       connection_settings.ipAddress=ui->ipAddressEdit->text();
       connection_settings.responseTime=ui->connectionTimeoutEdit->text().toInt();
       connection_settings.port=ui->serverPortEdit->text().toInt();
       connection_settings.connectType=ui->connectionTypeBox->currentIndex();
       qDebug() << "Connect type" << connection_settings.connectType;
       hide();
    });
    connect(ui->cancelButton,QPushButton::clicked,[this](){hide();});
    //TODO dodac wykrywanie bledow
}

settings connectionSettings::get_settings(){
    return connection_settings;
}

connectionSettings::~connectionSettings()
{
    delete ui;
}
