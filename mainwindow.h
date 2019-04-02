#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTextCursor>
#include <QTimerEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void timerEvent(QTimerEvent *);

signals:
    void sig_cmd(int cmd, QString strInfo);  // iCmd: 1: set mac   2: set model 3:start test

private slots:
    void onShowInfo(QString);
    void on_pushButtonMac_clicked();
    void on_pushButtonModel_clicked();
    void on_pushButtonClose_clicked();
    void on_pushButtonStartWifi_clicked();
    void on_pushButtonStartModel_clicked();

    void on_pushButtonStop_clicked();

private:
    Ui::MainWindow *ui;
    QTextCursor m_cursor;
    int m_iTimer;      // 定时器
    bool isWifiTest;
    bool isModelTest;
    bool isReceiving;

};

#endif // MAINWINDOW_H
