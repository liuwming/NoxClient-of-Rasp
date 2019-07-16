#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QTimer>

extern int g_is_exit_factory_wifi_pass;
extern int g_submode;
extern int g_version;
int isDigitStr(QString str);
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
    this->setWindowTitle("NOX Client Of Yeelight");

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

    m_iTimer = startTimer(500);
    isWifiTest = false;
    isModelTest = false;
    isReceiving = false;

    ui->comboBox->setStyleSheet("QComboBox{font:13px;color:darkblack;height: 50px;}" );
    ui->tableWidget->setColumnCount(14);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch); //自适应行高

    QStringList header;
    header << "MAC地址" << "Model号" << "固件版本" << "老化次数" << "老化时间" << "平均信号强度" << "最小信号强度"
           << "最大信号强度" << "发包个数" << "收包个数" <<"设备状态" << "IP地址" << "Submodel" <<  "测试结果";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    //ui->tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{font:13px}");

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

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(10, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(11, QHeaderView::ResizeToContents);

    QStringList strlist;
    mapResInfo::const_iterator it;
    for (it = mapInfo.constBegin(); it != mapInfo.constEnd(); ++it) {

        strlist = it.value();
        if (strlist.size() < 2) {
            continue;
        }
        if (strlist.last() != "submodel") {
            ui->tableWidget->setRowCount(0);
            ui->tableWidget->clearContents();
        }
    }

    for (it = mapInfo.constBegin(); it != mapInfo.constEnd(); ++it) {
        qDebug() << it.key() << ":" << it.value();

        strlist = it.value();
        if (strlist.size() < 2) {
            continue;
        }

        int iTmp = ui->tableWidget->rowCount();

        if (strlist.last() == "fail") {
            ui->tableWidget->insertRow(iTmp);//增加一行
            QTableWidgetItem *item = new QTableWidgetItem("不合格");
            item->setBackgroundColor(Qt::red);
            item->setTextColor(Qt::white);
            item->setFont(QFont("Times", 12, QFont::Bold));
            ui->tableWidget->setItem(iTmp, 13, item);
            ui->tableWidget->item(iTmp, 13)->setTextAlignment(Qt::AlignCenter);
            strlist.removeLast();

            QTableWidgetItem *itemMac = new QTableWidgetItem(it.key());
            itemMac->setBackgroundColor(Qt::red);
            itemMac->setTextColor(Qt::white);
            itemMac->setFont(QFont("Times", 13, QFont::Bold));
            ui->tableWidget->setItem(iTmp, 0, itemMac);

            QTableWidgetItem *itemSubModel = new QTableWidgetItem("");
            itemSubModel->setBackgroundColor(Qt::red);
            itemSubModel->setTextColor(Qt::white);
            itemSubModel->setFont(QFont("Times", 13, QFont::Bold));
            ui->tableWidget->setItem(iTmp, 12, itemSubModel);

            for (int i = 0; i < strlist.size(); i++) {
                QTableWidgetItem *item = new QTableWidgetItem(strlist.value(i));
                item->setBackgroundColor(Qt::red);
                item->setTextColor(Qt::white);
                item->setFont(QFont("Times", 12, QFont::Bold));
                ui->tableWidget->setItem(iTmp, i + 1, item);
            }

            continue;
        } else if (strlist.last() == "success") {
            ui->tableWidget->insertRow(iTmp);//增加一行
            QTableWidgetItem *itemS = new QTableWidgetItem("合格");
            itemS->setBackgroundColor(Qt::darkGreen);
            itemS->setTextColor(Qt::white);
            itemS->setFont(QFont("Times", 13, QFont::Bold));
            ui->tableWidget->setItem(iTmp, 13, itemS);
            ui->tableWidget->item(iTmp, 13)->setTextAlignment(Qt::AlignCenter);

            strlist.removeLast();

            QTableWidgetItem *itemMac = new QTableWidgetItem(it.key());
            itemMac->setBackgroundColor(Qt::darkGreen);
            itemMac->setTextColor(Qt::white);
            itemMac->setFont(QFont("Times", 13, QFont::Bold));
            ui->tableWidget->setItem(iTmp, 0, itemMac);

            QTableWidgetItem *itemSubModel = new QTableWidgetItem("");
            itemSubModel->setBackgroundColor(Qt::darkGreen);
            itemSubModel->setTextColor(Qt::white);
            itemSubModel->setFont(QFont("Times", 13, QFont::Bold));
            ui->tableWidget->setItem(iTmp, 12, itemSubModel);

            for (int i = 0; i < strlist.size(); i++) {
                QTableWidgetItem *item = new QTableWidgetItem(strlist.value(i));
                item->setBackgroundColor(Qt::darkGreen);
                item->setTextColor(Qt::white);
                item->setFont(QFont("Times", 13, QFont::Bold));
                ui->tableWidget->setItem(iTmp, i + 1, item);

                if (g_version> 0) {
                    QString strVer =  strlist.value(1);
                    strVer.remove("\"");
                    if (strVer.toInt() != g_version) {
                        QTableWidgetItem *item = new QTableWidgetItem(strlist.value(1));
                        item->setBackgroundColor(Qt::red);
                        item->setTextColor(Qt::white);
                        item->setFont(QFont("Times", 12, QFont::Bold));
                        ui->tableWidget->setItem(iTmp, 2, item);
                    }
                }
            }

            if (g_submode > 0) {
                for (int i = 0; i < iTmp + 1; i++) {
                    QTableWidgetItem *preItem= new QTableWidgetItem("");
                    preItem->setBackgroundColor(Qt::red);
                    ui->tableWidget->setItem(i, 12, preItem);
                }
            }

            continue;

        } else if (strlist.last() == "submodel") {
            int row = ui->tableWidget->rowCount();

            QString strSubmodel = strlist.first();

            strSubmodel.remove("\"");
            qDebug() << "row:" << row;

            for (int i = 0; i < row; i++) {
                QString itemName = ui->tableWidget->item(i, 0)->text();//获取某一格内容
                qDebug() << "item:" << itemName << "submodel:" << it.key();
                if (itemName == it.key()) {
                    QTableWidgetItem *tmp = new QTableWidgetItem(strSubmodel);

                    if (strSubmodel.toInt() == g_submode) {
                        tmp->setBackgroundColor(Qt::darkGreen);
                        QTableWidgetItem *itemS = new QTableWidgetItem("合格");
                        itemS->setBackgroundColor(Qt::darkGreen);
                        itemS->setTextColor(Qt::white);
                        itemS->setFont(QFont("Times", 13, QFont::Bold));
                        ui->tableWidget->setItem(i, 13, itemS);
                        ui->tableWidget->item(i, 13)->setTextAlignment(Qt::AlignCenter);
                    } else {
                        tmp->setBackgroundColor(Qt::red);
                        QTableWidgetItem *item = new QTableWidgetItem("不合格");
                        item->setBackgroundColor(Qt::red);
                        item->setTextColor(Qt::white);
                        item->setFont(QFont("Times", 12, QFont::Bold));
                        ui->tableWidget->setItem(i, 13, item);
                        ui->tableWidget->item(i, 13)->setTextAlignment(Qt::AlignCenter);
                    }

                    tmp->setTextColor(Qt::white);
                    tmp->setFont(QFont("Times", 13, QFont::Bold));
                    ui->tableWidget->setItem(i, 12, tmp);
                }
            }

            //qDebug() << "submodel " << g_submode << "mac" << it.key();

            // QTimer::singleShot(5000, this, SLOT(onTimeOut()));
            continue;
        } else {
            ui->tableWidget->insertRow(iTmp);//增加一行
            for (int i = 0; i < strlist.size(); i++) {
                ui->tableWidget->setItem(iTmp, 0, new QTableWidgetItem(it.key()));
                ui->tableWidget->setItem(iTmp, i + 1, new QTableWidgetItem(strlist.value(i)));

                QString strVer =  strlist.value(1);
                strVer.remove("\"");
                if (g_version > 0 && strVer.toInt() != g_version) {

                    QTableWidgetItem *item = new QTableWidgetItem(strlist.value(1));
                    item->setBackgroundColor(Qt::red);
                    item->setTextColor(Qt::white);
                    item->setFont(QFont("Times", 12, QFont::Bold));
                    ui->tableWidget->setItem(iTmp, 2, item);
                }
            }
        }
    }
}

//void MainWindow::onTimeOut()
//{
//    qDebug() << "5s time out";
//    sig_cmd(CMD_EXIT_TEST, "");
//}

void MainWindow::onShowInfo(QString str)
{
    if ("1" == str) {
        ui->textEdit->setTextColor(QColor("blue"));
        return;
    } else if ("exitWifi" == str) {
        isWifiTest = false;
        ui->pushButtonStartWifi->setText("启动WIFI测试");
        ui->pushButtonStartWifi->setStyleSheet("");
        ui->pushButtonStartWifi->setEnabled(true);
        ui->comboBox->setEnabled(true);
        ui->comboBox->setStyleSheet("QComboBox{font:13px;color:darkblack;height: 50px;}" );
        ui->pushButtonStartModel->setEnabled(true);
        ui->textEdit->setTextColor(QColor("blue"));
        return;
    } else if ("exitModle" == str) {
        isModelTest = false;
        ui->pushButtonStartModel->setText("启动MODEL测试");
        ui->pushButtonStartModel->setStyleSheet("");
        ui->pushButtonStartModel->setEnabled(true);
        ui->pushButtonStartWifi->setEnabled(true);
        ui->comboBox->setEnabled(true);
        ui->comboBox->setStyleSheet("QComboBox{font:13px;color:darkblack;height: 50px;}" );
        ui->textEdit->setTextColor(QColor("blue"));
        return;
    } else if (str.contains("waiting on")) {
        isReceiving = true;
        return;
    } else if (str.contains("Failed to")) {
        ui->pushButtonStartModel->setText("启动MODEL测试");
        ui->pushButtonStartModel->setEnabled(true);
        ui->comboBox->setEnabled(true);
        ui->comboBox->setStyleSheet("QComboBox{font:13px;color:darkblack;height: 50px;}" );
        isWifiTest = false;
        ui->pushButtonStartWifi->setText("启动WIFI测试");
        isModelTest = false;
        ui->textEdit->setTextColor(QColor("red"));
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
    ui->comboBox->setDisabled(true);
    ui->comboBox->setStyleSheet("QComboBox{font:13px;color:gray;height: 50px;}" );
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
    ui->comboBox->setDisabled(true);
    ui->comboBox->setStyleSheet("QComboBox{font:13px;color:gray;height: 50px;}" );
    ui->pushButtonStartWifi->setDisabled(true);
    sig_cmd(CMD_START_MODEL_TEST, "");
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
            if (iNum > 100) { // 50s
                if (1 == QMessageBox::warning(this, tr("提示"), tr("服务器响应超时，请重新测试!!!"), tr("取消"), tr("确定"))) {
                    this->deleteLater();
                }
            }
        } else {
            iNum = 0;
        }
    }
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    g_is_exit_factory_wifi_pass = index;
}

void MainWindow::on_pushButtonSubModel_clicked()
{
    QString StrNum = ui->lineEditSubModel->text();

    if (ui->lineEditSubModel->text().isEmpty()) {
        ui->textEdit->append(QString("请输入正确的submodel值!"));
        return;
    }

    if (0 == isDigitStr(StrNum) && (StrNum.toInt() <= 999 && StrNum.toInt() >= 0)) {  // digit
        // ui->textEdit->append(QString().sprintf("正在设置submodel = %s ...\r\n", StrNum));
        sig_cmd(CMD_SET_SUBMODEL, StrNum);
        ui->lineEditSubModel->setStyleSheet("font:14px;color:mediumblue");
        //ui->lineEditSubModel->setStyleSheet("background-color:palegoldenrod; font:13px;color:green");
        ui->lineEditSubModel->setDisabled(true);
    } else {
        ui->textEdit->append(QString("请输入正确的submodel值!"));
    }
}

int isDigitStr(QString str)
{
    QByteArray ba = str.toLatin1(); //QString 转换为 char*
    const char *s = ba.data();

    while (*s && *s>='0' && *s<='9') s++;

    if (*s) { // 不是纯数字
        return -1;
    } else {  // 纯数字
        return 0;
    }
}

void MainWindow::on_pushButtonVersion_clicked()
{
    QString StrVer = ui->lineEditVersion->text();

    if (StrVer.isEmpty()) {
        ui->textEdit->append(QString("请输入有效的版本号!"));
        return;
    }

    if (0 == isDigitStr(StrVer) && (StrVer.toInt() <= 999 && StrVer.toInt() >= 0)) {  // digit
        sig_cmd(CMD_VERIFY_VER, StrVer);
        ui->lineEditVersion->setStyleSheet("font:14px;color:mediumblue");
        ui->lineEditVersion->setDisabled(true);
    } else {
        ui->textEdit->append(QString("请输入有效的版本号!"));
    }

}
