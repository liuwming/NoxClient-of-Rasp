#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    qRegisterMetaType<mapResInfo>("mapResInfo");

    connect(pThread, SIGNAL(started()), pNox, SLOT(beginToWork()));
    connect(pNox, SIGNAL(sendInfo(QString)), this, SLOT(onShowInfo(QString)));
    connect(pNox, SIGNAL(sendResInfo(mapResInfo)), this, SLOT(onShowResInfo(mapResInfo)));
    connect(this, SIGNAL(sig_cmd(int, QString)), pNox, SLOT(onRecvCmd(int, QString)));
    pThread->start();

    m_cursor = ui->textEdit->textCursor();
    ui->pushButtonStop->setVisible(false);

    m_iTimer = startTimer(500);
    isWifiTest = false;
    isModelTest = false;
    isReceiving = false;

    ui->tableWidget->setColumnCount(13);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(10, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(11, QHeaderView::ResizeToContents);
    // ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

    QStringList header;
    header << "MAC地址" << "Model号" << "固件版本" << "老化开始" << "老化完成" << "平均信号强度" << "最小信号强度"
           << "最大信号强度" << "发包个数" << "收包个数" <<"设备状态" << "IP地址" <<  "是否合格";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//禁止修改
    // ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选中的方式
    // ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection); // 可以选中单个
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onShowResInfo(mapResInfo mapInfo)
{
    if (mapInfo.isEmpty()) {
        return;
    }

    ui->tableWidget->setRowCount(0);
    ui->tableWidget->clearContents();

    QStringList  strlist;
    mapResInfo::const_iterator it;
    for (it = mapInfo.constBegin(); it != mapInfo.constEnd(); ++it) {
        qDebug() << it.key() << ":" << it.value();

        strlist = it.value();
        if (strlist.size() < 2) {
            continue;
        }

        int iTmp = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(iTmp);//增加一行
        for (int i = 0; i < strlist.size(); i++) {
            ui->tableWidget->setItem(iTmp, 0, new QTableWidgetItem(it.key()));

            if (strlist.last() == "fail") {
                QTableWidgetItem *item = new QTableWidgetItem("不合格");
                item->setBackgroundColor(Qt::red);
                item->setTextColor(Qt::white);
                item->setFont(QFont( "Times", 12, QFont::Bold));
                ui->tableWidget->setItem(iTmp, 12, item);
                ui->tableWidget->item(iTmp, 12)->setTextAlignment(Qt::AlignCenter);
                strlist.removeLast();
            }
            if (strlist.last() == "success") {
                QTableWidgetItem *itemS = new QTableWidgetItem("合格");
                itemS->setBackgroundColor(Qt::darkGreen);
                itemS->setTextColor(Qt::white);
                itemS->setFont( QFont("Times", 12, QFont::Bold));
                ui->tableWidget->setItem(iTmp, 12, itemS);
                ui->tableWidget->item(iTmp, 12)->setTextAlignment(Qt::AlignCenter);
                strlist.removeLast();
            }

            ui->tableWidget->setItem(iTmp, i + 1, new QTableWidgetItem(strlist.value(i)));
        }
    }

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
    } else if (str.contains("Failed to")) {
        ui->pushButtonStartModel->setText("启动MODEL测试");
        ui->pushButtonStartModel->setEnabled(true);
        isWifiTest = false;
        ui->pushButtonStartWifi->setText("启动WIFI测试");
        isModelTest = false;
        ui->pushButtonStartWifi->setEnabled(true);
    }

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
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->clearContents();
    ui->textEdit->setTextColor("");
    isWifiTest = true;
    ui->pushButtonStartWifi->setDisabled(true);
    ui->pushButtonStartModel->setDisabled(true);
    sig_cmd(CMD_START_WIFI_TEST, "");
}

void MainWindow::on_pushButtonStartModel_clicked()
{
    ui->pushButtonStartModel->setText("MODEL测试中...");
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->clearContents();
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

                if (1 == QMessageBox::warning(this, tr("提示"), tr("服务器响应超时，请重新测试!!!"), tr("取消"), tr("确定"))) {
                    this->deleteLater();
                }
            }
        } else {
            iNum = 0;
        }
    }
}
