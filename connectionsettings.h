#ifndef CONNECTIONSETTINGS_H
#define CONNECTIONSETTINGS_H

#include <QDialog>
#include <QSerialPort>
#include <QString>
#include <QDebug>

namespace Ui {
class connectionSettings;
}

enum connectionType{
    Tcp,
    Serial
};

struct settings {
    //Struct with default values
    //int parity = QSerialPort::EvenParity;
    //int baud = QSerialPort::Baud19200;
    //int dataBits = QSerialPort::Data8;
    //int stopBits = QSerialPort::OneStop;
    int responseTime = 1000;
    QString ipAddress = QStringLiteral("127.0.0.1");
    int port = 502;
    int connectType= Tcp;
};

class connectionSettings : public QDialog
{
    Q_OBJECT

public:
    explicit connectionSettings(QWidget *parent = 0);
    ~connectionSettings();
    settings get_settings();

private:
    settings connection_settings;
    Ui::connectionSettings *ui;
};

#endif // CONNECTIONSETTINGS_H
