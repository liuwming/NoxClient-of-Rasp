#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "./nox/noxClient.h"

#include <QDebug>
#include <QMessageBox>

prod_name_model data_base_model[] = {
        {"易来 皑月LED吸顶灯 480","yilai.light.ceiling1"},
        {"易来 荷枫LED吸顶灯 430","yilai.light.ceiling2"},
        {"易来 荷枫LED吸顶灯 Pro","yilai.light.ceiling3"},
};

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Factory Test Tool for NOX of Yeelight");

    NoxClient *pNox = new NoxClient("127.0.0.1");
    QThread *pThread = new QThread();
    pNox->moveToThread(pThread);

    connect(pThread, SIGNAL(started()), pNox, SLOT(beginToWork()));
    connect(pNox, SIGNAL(sendInfo(QString)), this, SLOT(onShowInfo(QString)));
    connect(this, SIGNAL(sig_cmd(int, QString)), pNox, SLOT(onRecvCmd(int, QString)));
    pThread->start();

    m_cursor = ui->textEdit->textCursor();
    ui->pushButtonStop->setVisible(false);

    m_iTimer = startTimer(500);
    isWifiTest = false;
    isModelTest = false;
    isReceiving = false;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onShowInfo(QString str)
{
    if ("1" == str) {
        isWifiTest = false;
        ui->pushButtonStartWifi->setText("启动WIFI测试");
        ui->pushButtonStartWifi->setStyleSheet("");
        ui->pushButtonStartWifi->setEnabled(true);
        ui->pushButtonStartModel->setEnabled(true);
        ui->textEdit->setTextColor(QColor("red"));
        return;
    } else if ("2" == str) {
        isModelTest = false;
        ui->pushButtonStartModel->setText("启动MODEL测试");
        ui->pushButtonStartModel->setStyleSheet("");
        ui->pushButtonStartModel->setEnabled(true);
        ui->pushButtonStartWifi->setEnabled(true);
        ui->textEdit->setTextColor(QColor("blue"));
        return;
    } else if (str.contains("waiting on")) {
        isReceiving = true;
        return;
    }

    // ui->textEdit->setTextColor(QColor("royalblue"));
    ui->textEdit->append(str);
    m_cursor.movePosition(QTextCursor::End);
    ui->textEdit->setTextCursor(m_cursor);
    isReceiving = false;
}

void MainWindow::on_pushButtonMac_clicked()
{
    QString strMac = ui->lineEditMac->text();
    if (strMac.isEmpty()) {
        ui->textEdit->append(QString("请输入遥控器MAC地址:"));
        return;
    }

    sig_cmd(CMD_SET_MAC, strMac);
}

void MainWindow::on_pushButtonModel_clicked()
{
    QString strMod = ui->lineEditModel->text();
    if (strMod.isEmpty()) {
        ui->textEdit->append("======================================================");
        ui->textEdit->append(QString("请输入目标产品model号,格式为：yilai.light.ceilingxx"));
        ui->textEdit->append(QString("参考当前已有产品对应表"));
        for(int i = 0; i < sizeof(data_base_model)/sizeof(prod_name_model); i++){
            ui->textEdit->append(QString().sprintf("%35s %30s",data_base_model[i].name, data_base_model[i].model));
        }
        ui->textEdit->append("======================================================");
        return;
    }

    sig_cmd(CMD_SET_MODEL, strMod);
}

void MainWindow::on_pushButtonClose_clicked()
{
    this->close();
}

void MainWindow::on_pushButtonStartWifi_clicked()
{
    ui->pushButtonStartWifi->setText("WIFI测试中...");
    ui->textEdit->setTextColor("");
    isWifiTest = true;
    ui->pushButtonStartWifi->setDisabled(true);
    ui->pushButtonStartModel->setDisabled(true);
    sig_cmd(CMD_START_WIFI_TEST, "");
}

void MainWindow::on_pushButtonStartModel_clicked()
{
    ui->pushButtonStartModel->setText("MODEL测试中...");
    ui->textEdit->setTextColor("");
    isModelTest = true;
    ui->pushButtonStartModel->setDisabled(true);
    ui->pushButtonStartWifi->setDisabled(true);
    sig_cmd(CMD_START_MODEL_TEST, "");
}

void MainWindow::on_pushButtonStop_clicked()
{
    sig_cmd(CMD_STOP_TEST, "");
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    static int iCounter = 0;
    static int iNum = 0;
    if (event->timerId() == m_iTimer) {
        ++iCounter;
        if (iCounter > 60000) {
            iCounter = 0;
        }

        if (iCounter % 2) {
            if (isWifiTest) {
                ui->pushButtonStartWifi->setStyleSheet("background-color:lightskyblue");
            }
            if (isModelTest) {
                ui->pushButtonStartModel->setStyleSheet("background-color:lightskyblue");
            }
        } else {
            if (isWifiTest) {
                ui->pushButtonStartWifi->setStyleSheet("");
            }
            if (isModelTest) {
                ui->pushButtonStartModel->setStyleSheet("");
            }
        }

        if (isReceiving) {
            ++iNum;
            qDebug() << "isReceiving : " << iNum;;
            if (iNum > 25) {

                if (1 == QMessageBox::warning(this, tr("提示"), tr("服务器响应超时，请重新测试!!!"), tr("否"), tr("是"))) {
                    this->deleteLater();
                }
            }
        } else {
            iNum = 0;
        }
    }
}
