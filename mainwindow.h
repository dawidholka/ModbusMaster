#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QModbusClient>
#include <QModbusTcpClient>
#include <QDateTime>
#include "connectionsettings.h"

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
    void state_changed();
    void send_button_clicked();
    void update_table_data();
    void read_reply();

private:
    connectionSettings *connectionSettingsDialog;
    Ui::MainWindow *ui;
    QModbusClient *modbusClient;
    QModbusReply *modbusReply;
};

#endif // MAINWINDOW_H
