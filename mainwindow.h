#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QModbusClient>
#include <QModbusRtuSerialMaster>
#include <QModbusTcpClient>
#include <QDateTime>
#include <QBitArray>
#include "connectionsettings.h"

enum number_systems{
    decimal,
    hexadecimal
};

namespace Ui {
class MainWindow;
class connectionSetings;
}

class connectionSettings;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void connect_button_clicked();
    void disconnect_button_clicked();
    void error();
    void connection_state_changed();
    void send_button_clicked();
    void update_table_data();
    QModbusDataUnit make_request() const;
    void readReady();
    void write_request();
    void update_number_format();

private:
    connectionSettings *connectionSettingsDialog;
    Ui::MainWindow *ui;
    QModbusClient *modbusClient;
    QModbusReply *modbusReply;
    QBitArray *coils;
    QVector<quint16> *data;
    number_systems currentSystem;

};

#endif // MAINWINDOW_H
