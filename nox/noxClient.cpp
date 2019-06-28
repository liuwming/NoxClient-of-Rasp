#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <QDebug>
#include <QString>
#include <QTime>
#include <sys/stat.h>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>

#include "noxClient.h"

static int iModelCounter = 0;
static int iWifiCounter = 0;
static rt_cmd_result_t final_res;
// static rt_cmd_result_t *result = &final_res;
static char log_file[100] ; //= "rssi_test.log";
static char log_file_model_test[100];// = "model_test.log";

int  g_test_type = TEST_TYPE_WIFI;
int  g_is_exit_factory_wifi_pass = 0;  // default 0: exit factory.   1: do not exit
bool g_is_set_submode = false;
int  g_submode = -1;

char cmd_name[][50] = {
        {"启动"},//0
        {"结束"},
        {"状态"},
        {"结果"},
        {"设置参数"},
        {"获取参数"},//5
        {"真退工厂"},
        {"假退工厂"},
        {"升级"},
        {""},
        {""},//10
        {""},
        {"添加遥控器"},
        {""},
        {""},
        {""},//15
        {"设置submodel"},
        {""}
};

NoxClient::NoxClient(char *pIp) : QObject()
{
    m_pIP = pIp;
    //m_strDesktopDir = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();

    QString strDirPath =  QCoreApplication::applicationDirPath();
    m_strDesktopDir  = strDirPath.left( strDirPath.size() - 3) + "output/";
    //qDebug() << "dir:: " << m_strDesktopDir;
    char *ch;
    QByteArray ba = m_strDesktopDir.toLatin1();
    ch = ba.data();
    char *str1 = "rssi_test.log";
    char *str2 = "model_test.log";
    sprintf(log_file, "%s%s", ch, str1);
    sprintf(log_file_model_test, "%s%s", ch, str2);

    qDebug() << "wifi  dir:: " << log_file;
    qDebug() << "model dir:: " << log_file_model_test;

}

NoxClient::~NoxClient()
{

}

int NoxClient::valid_mac(char *s)
{
    int len = strlen(s);
    if (len != 17) {
        sendInfo(QString().sprintf("error : mac length %d out of range (%s)\n", len, s));
        return 0;
    }

    int i;
    for (i = 0; i < 17; i++) {
        s[i] = tolower(s[i]);
        if ((i+1) % 3) {
            if (s[i] > 'f' ||
                            s[i] < '0' ||
                            (s[i] > '9' && s[i] < 'a')) {
                return 0;
            }
        } else {
            if (s[i] != ':')
                return 0;
        }
    }

    return 1;
}

int valid_model(char * s)
{
    int len = strlen(s);
    if (len < 20) {
        printf("[长度错误] 当前model:%s 长度:%d \n", s, len);
        return 0;
    }

    int i;
    for (i = 0; i < 17; i++) {
        s[i] = tolower(s[i]);
    }

    return 1;
}

int valid_key(char * s)
{
    int i;
    if(strlen(s) != 24)
        return 0;

    for(i = 0; i < 24; i++)
    {
        if (s[i] < '0' ||
                        s[i] > 'f' ||
                        (s[i] > '9' && s[i] < 'A') ||
                        (s[i] > 'F' && s[i] < 'a')
                        ) {
            return 0;
        }
    }

    return 1;
}

static char beacon_mac[32];
static char beacon_key[32];
static char target_model[32];


int NoxClient::set_beacon(QString strMac)
{
    char cmd[128] = {0};
    char buf[32] = {0};

    // scanf("%s", buf);
    memcpy(buf, strMac.toLocal8Bit(), strMac.size());
    qDebug() << buf;
    if (!valid_mac(buf)) {
        return -1;
    }
    strcpy(beacon_mac, buf);

    sprintf(buf, "202122232425262728292a2b");
    if (!valid_key(buf)) {
        return -1;
    }
    strcpy(beacon_key, buf);

    sprintf(cmd, "echo %s > " beacon_info_fn, beacon_mac);
    system(cmd);
    sprintf(cmd, "echo %s >> " beacon_info_fn, beacon_key);
    system(cmd);

    return 0;
}

int NoxClient::set_target_model(QString strModel)
{
    char cmd[128] = {0};
    char buf[32] = {0};
    int i_len = strModel.size();

    memcpy(buf, strModel.toLocal8Bit(), i_len);
    // scanf("%s", buf);
    if (!valid_model(buf)) {
        return -1;
    }
    strcpy(target_model, buf);
    target_model[i_len] = '\0';

    sprintf(cmd, "echo %s > " model_info_fn, target_model);
    system(cmd);

    return 0;
}

int NoxClient::load_beacon()
{
    FILE *fp;
    char buf[64];

    fp = fopen(beacon_info_fn, "r");
    if (!fp) {
        sendInfo(QString("未发现遥控器配置文件"));
        return -1;
    }

    fscanf(fp, "%s", buf);
    if (!valid_mac(buf)) {
        sendInfo(QString().sprintf("遥控器配置文件包含非法MAC地址 %s", buf));
        goto err;
    }
    strcpy(beacon_mac, buf);

    fscanf(fp, "%s", buf);
    if (!valid_key(buf)) {
        sendInfo(QString().sprintf("遥控器配置文件包含非法key %s", buf));
        goto err;
    }
    strcpy(beacon_key, buf);

    sendInfo(QString().sprintf("发现遥控器配置信息，MAC地址-%s, key-%s\n", beacon_mac, beacon_key));
    fclose(fp);
    return 0;

err:
    fclose(fp);
    return -1;
}

int NoxClient::load_target_model()
{
    FILE *fp;
    char buf[64];

    fp = fopen(model_info_fn, "r");
    if (!fp) {
        sendInfo(QString("未发现目标model配置文件"));
        return -1;
    }

    fscanf(fp, "%s", buf);
    if (!valid_model(buf)) {
        sendInfo(QString().sprintf("目标model配置文件格式不正确 %s\n", buf));
        goto err;
    }
    strcpy(target_model, buf);

    sendInfo(QString().sprintf("发现目标model配置信息，%s\n", target_model));

    fclose(fp);
    return 0;

err:
    fclose(fp);
    return -1;
}


void NoxClient::print_test_status(rtt_handle_t hdl, rt_cmd_result_t* result)
{
    int i;
    int l;
    char buf[4096];
    beacon_cmd_param_t beacon_cmd;
    submodel_cmd_param_t submodel_cmd;
    int pass;
    int add_beacon = 0;
    int set_submodel = 0;

    mapResInfo mapInfo;
    QStringList strlist;

    int passed_dev[MAX_TEST_DEV];
    bzero(passed_dev, MAX_TEST_DEV * sizeof(int));
    memset(buf, 0, sizeof buf);

    if (result->type != RT_CMD_STATUS) {
        iWifiCounter = 0;
        l = sprintf(buf, "\r\n共测试[%d]台设备,结果如下：\n", (int)result->dev_count );
        l += sprintf(buf + l, "%s\n", "----------------------------------------------------------------------------------");
        sendInfo("1");
    } else {
        l = sprintf(buf, "[wifi : %d] ------------%s------------\n" , ++iWifiCounter, result->type == RT_CMD_STATUS ?  "status" : "result");
    }

    //l += sprintf(buf + l, "-model-firmware_ver-brs-brf-avg_rssi-min_rssi-max_rssi-packet_send-packet_recv\n");
    for (i = 0; i < (int)result->dev_count; i++) {
        //l += sprintf(buf + l, "+Device: Status[%s] MAC[%s] IP[%s]\n", res(state), res(mac), res(ip));
        strlist.clear();
        if (result->type == RT_CMD_RESULT) {
            pass = 1;

            if (res(avg_rssi) < RSSI_THRESH_AVG) {
                sendInfo(QString().sprintf("error : %s 平均信号强度(=%d)过低", res(mac), res(avg_rssi)));
                pass = 0;
            }

            //if (res(min_rssi) < RSSI_THRESH_MIN) {
            //    printf("error : %s 最低信号强度(=%d)过低\n", res(mac), res(min_rssi));
            //    pass = 0;
            //}

            if (res(packet_recv) < MIN_PACKAGE) {  //  result->body.status[i].packet_recv
                sendInfo(QString().sprintf("error : %s 收到的数据包数量(=%d)太低", res(mac), res(packet_recv)));
                pass = 0;
            }

            else if ((100 * res(packet_recv)) / res(packet_send) < (100 - PACKAGE_LOSS_THRESH)) {
                sendInfo(QString().sprintf("error : %s 丢包率(%d/%d)太高", res(mac), res(packet_send)-res(packet_recv), res(packet_send)));
                pass = 0;
            }

            if (pass) {
                sendInfo(QString("测试通过"));
                // #if EXIT_FACTORY_WIFI_PASS
                //                control_dev(hdl, result->body.status[i].mac, 0);
                // #else
                //                sendInfo(QString("测试通过后不退工厂模式"));
                // #endif

                if (!g_is_exit_factory_wifi_pass) {
                    control_dev(hdl, result->body.status[i].mac, 0);
                } else {
                    sendInfo(QString("\r\n测试通过后不退工厂模式"));
                }

                l += sprintf(buf + l, "测试结果[%s],设备状态[%s] MAC[%s] IP[%s]\n", "成功", res(state), res(mac), res(ip));
                sendInfo(QString().sprintf("测试结果[%s],设备状态[%s] MAC[%s] IP[%s]", "成功", res(state), res(mac), res(ip)));
                passed_dev[i] = 1;
                add_beacon = 1;
                set_submodel = 1;

                strlist << res(model) << res(fw_ver) << QString::number(res(brs)) << QString::number(res(brf))
                        << QString::number(res(avg_rssi)) << QString::number(res(min_rssi)) << QString::number(res(max_rssi))
                        << QString::number(res(packet_send)) << QString::number(res(packet_recv)) << res(state) << res(ip);

                strlist.append("success");
                mapInfo.insert(res(mac), strlist);
                sendResInfo(mapInfo); // to do
                continue; // TO DO
            } else {
                sendInfo(QString("测试失败\n"));
                l += sprintf(buf + l, "测试结果[%s],设备状态[%s] MAC[%s] IP[%s]\n", "失败", res(state), res(mac), res(ip));
                sendInfo(QString().sprintf("测试结果[%s],设备状态[%s] MAC[%s] IP[%s]", "失败", res(state), res(mac), res(ip)));
                passed_dev[i] = 0;

                strlist << res(model) << res(fw_ver) << QString::number(res(brs)) << QString::number(res(brf))
                        << QString::number(res(avg_rssi)) << QString::number(res(min_rssi)) << QString::number(res(max_rssi))
                        << QString::number(res(packet_send)) << QString::number(res(packet_recv)) << res(state) << res(ip);

                strlist.append("fail");
                mapInfo.insert(res(mac), strlist);
                sendResInfo(mapInfo); // to do
                continue; // TO DO
                control_dev(hdl, result->body.status[i].mac, 1); // to do
            }
        }

        if (strcmp(res(state), STATE_DEV_CONN) == 0) {
            l += sprintf(buf + l, "\t%s", res(model));
            l += sprintf(buf + l, "\t%s", res(fw_ver));
            l += sprintf(buf + l, "\t%d", res(brs));
            l += sprintf(buf + l, "\t%d", res(brf));
            l += sprintf(buf + l, "\t%d", res(avg_rssi));
            l += sprintf(buf + l, "\t%d", res(min_rssi));
            l += sprintf(buf + l, "\t%d", res(max_rssi));
            l += sprintf(buf + l, "\t%d", res(packet_send));
            l += sprintf(buf + l, "\t%d", res(packet_recv));

            strlist << res(model) << res(fw_ver) << QString::number(res(brs)) << QString::number(res(brf))
                    << QString::number(res(avg_rssi)) << QString::number(res(min_rssi)) << QString::number(res(max_rssi))
                    << QString::number(res(packet_send)) << QString::number(res(packet_recv)) << res(state) << res(ip);
        }
        l += sprintf(buf + l, "\n");
        mapInfo.insert(res(mac), strlist);

        if (result->type != RT_CMD_RESULT) {
            sendResInfo(mapInfo);
        }
    }

    if (result->type != RT_CMD_STATUS) {
        l += sprintf(buf + l, "%s\n", "----------------------------------------------------------------------------------");
    }

    sendInfo(QString().sprintf("%s", buf));

#ifdef LOG_RESULT
    if (result->type == RT_CMD_RESULT) {
        FILE *fp;
        fp = fopen(log_file, "a");
        if (!fp)
            sendInfo(QString("Error: log file open failure, will lose some test results\n"));

        fwrite(buf, strlen(buf), 1, fp);
        fclose(fp);
    }
#endif

    if (set_submodel && g_is_set_submode && g_submode >= 0) {
        sendInfo(QString("设置 submodel = ").append(QString::number(g_submode)));
        for (i = 0; i < (int)result->dev_count; i++) {
            if (passed_dev[i]) {
                strcpy(submodel_cmd.mac_dev, result->body.status[i].mac);
                submodel_cmd.sudmodel_val = g_submode;
                control_dev(hdl, (char *)&submodel_cmd, 7);
            }
        }
    }


    if (add_beacon) {
        for (i = 0; i < (int)result->dev_count; i++) {
            if (passed_dev[i]) {
                strcpy(beacon_cmd.mac_dev, result->body.status[i].mac);
                strcpy(beacon_cmd.mac_beacon, beacon_mac);
                strcpy(beacon_cmd.key, beacon_key);
                control_dev(hdl, (char *)&beacon_cmd, 3);

                // strcpy(beacon_cmd.mac_beacon, "F8:24:41:D0:7F:27");//第二个
                // control_dev(hdl, (char *)&beacon_cmd, 3);
                // strcpy(beacon_cmd.mac_beacon, "F8:24:41:D0:7F:28");//第三个
                // control_dev(hdl, (char *)&beacon_cmd, 3);
            }
        }
    }
}


void NoxClient::print_test_modle_status(rtt_handle_t hdl, rt_cmd_result_t* result)
{
    int i;
    int l;
    char buf[4096];
    int pass;
    mapResInfo mapInfo;
    QStringList strlist;

    int passed_dev[MAX_TEST_DEV];
    bzero(passed_dev, MAX_TEST_DEV * sizeof(int));

    memset(buf, 0, sizeof buf);

    switch ((int)result->type){
    case RT_CMD_STATUS:
        l = sprintf(buf, "[model %d] ------------%s------------\n" , ++iModelCounter, result->type == RT_CMD_STATUS ? "status" : "result");
        //        sendInfo(QString().sprintf("[model %d] ------------%s------------" , iModelCounter, result->type == RT_CMD_STATUS ? "status" : "result"));
        break;
    case RT_CMD_RESULT:
        l = sprintf(buf, "\r\n共测试[%d]台设备,测试结果：\n",(int)result->dev_count);
        l += sprintf(buf + l, "%s\n\n", "----------------------------------------------------------------------------------");
        iModelCounter = 0;

        sendInfo("exitModle");
        sendInfo(QString().sprintf("[Model] 共测试[%d]台设备,测试结果：",(int)result->dev_count));
        break;
    }
    //l += sprintf(buf + l, "-model-firmware_ver-brs-brf-avg_rssi-min_rssi-max_rssi-packet_send-packet_recv\n");
    for (i = 0; i < (int)result->dev_count; i++) {
        //l += sprintf(buf + l, "+Device: Status[%s] MAC[%s] IP[%s]\n", res(state), res(mac), res(ip));
        strlist.clear();
        if (result->type == RT_CMD_RESULT) {
            pass = 0;

            if(res(model) != NULL && strcmp(target_model, res(model)) == 0){
                pass = 1;
            }

            if (pass) {
                l += sprintf(buf + l, "[%s]------MAC[%s] model[%s]\n", "成功",res(mac),res(model));
                sendInfo(QString().sprintf("[%s]------MAC[%s] model[%s]", "成功",res(mac),res(model)));
                passed_dev[i] = 1;
                strlist << res(model) << "success";
                mapInfo.insert(res(mac), strlist);
                sendResInfo(mapInfo);
                continue;  // to do
                control_dev(hdl, result->body.status[i].mac, 1);
            } else {
                l += sprintf(buf + l, "[%s]------MAC[%s] model[%s] 设备状态[%s] \n", "失败", res(mac), res(model), res(state));
                sendInfo(QString().sprintf("[%s]------MAC[%s] model[%s] 设备状态[%s]", "失败", res(mac), res(model), res(state)));
                passed_dev[i] = 0;
                strlist << res(model) << "fail";
                mapInfo.insert(res(mac), strlist);
                sendResInfo(mapInfo);
                continue;  // to do
                control_dev(hdl, result->body.status[i].mac, 1);
            }

        } else {
            if (strcmp(res(state), STATE_DEV_CONN) == 0) {
                l += sprintf(buf + l, "\t%s", res(model));
                l += sprintf(buf + l, "\t%s", res(fw_ver));
                l += sprintf(buf + l, "\t%d", res(brs));
                l += sprintf(buf + l, "\t%d", res(brf));
                l += sprintf(buf + l, "\t%d", res(avg_rssi));
                l += sprintf(buf + l, "\t%d", res(min_rssi));
                l += sprintf(buf + l, "\t%d", res(max_rssi));
                l += sprintf(buf + l, "\t%d", res(packet_send));
                l += sprintf(buf + l, "\t%d", res(packet_recv));
            }

            strlist << res(model) << res(fw_ver) << QString::number(res(brs)) << QString::number(res(brf))
                    << QString::number(res(avg_rssi)) << QString::number(res(min_rssi)) << QString::number(res(max_rssi))
                    << QString::number(res(packet_send)) << QString::number(res(packet_recv)) << res(state) << res(ip);

            l += sprintf(buf + l, "\n");

            mapInfo.insert(res(mac), strlist);
            sendResInfo(mapInfo); // to do
        }
    }

    switch ((int)result->type){
    case RT_CMD_STATUS:
        break;
    case RT_CMD_RESULT:
        l += sprintf(buf + l, "\n%s\n", "----------------------------------------------------------------------------------");
        break;
    }

    sendInfo(QString().sprintf("%s\n", buf));

#ifdef LOG_RESULT
    if (result->type == RT_CMD_RESULT) {
        FILE *fp;
        fp = fopen(log_file_model_test, "a");
        if (!fp)
            sendInfo(QString("Error: log file open failure, will lose some test results\n"));

        fwrite(buf, strlen(buf), 1, fp);
        fclose(fp);
    }
#endif
}

void NoxClient::test_stop(rtt_handle_t hdl)
{
    rt_cmd_result_t res;
    rt_cmd_t cmd;
    int rc;

    cmd.type = RT_CMD_STOP;

    rtt_send(hdl, &cmd);
    sendInfo(QString("waiting on the response from the server"));
    rc = rtt_recv(hdl, &res, RT_CMD_STOP);

    if (rc < 0 || (rc > 0 && res.ret_code != RT_ERR_OK)) {
        sendInfo(QString().sprintf("Failed to exe command %d.\n", res.ret_code));
        return;
    } else {
        sendInfo(QString("waiting on the response from the server"));
        rc = rtt_recv(hdl, &res, RT_CMD_STOP);
        if (rc <= 0) return;
        print_test_status(hdl, &res);
    }

}

void NoxClient::run_test(rtt_handle_t hdl)
{
    rt_cmd_result_t res;
    rt_cmd_t cmd;
    int rc;

    mapResInfo mapInfo;
    QStringList strlist;

    cmd.type = RT_CMD_CLOSE_SBOX;
    sendInfo(QString("close sbox SEND command"));

    rtt_send(hdl, &cmd);

    sendInfo(QString("waiting on the response from the server"));
    rc = rtt_recv(hdl, &res, RT_CMD_CLOSE_SBOX);
    sendInfo(QString("received data from server"));

    if (rc < 0 || (rc > 0 && res.ret_code != RT_ERR_OK)) {
        sendInfo(QString("Failed to exe command: %1 ").arg(res.ret_code));
        return;
    }

    cmd.type = RT_CMD_START;
    rtt_send(hdl, &cmd);
    sendInfo(QString("Start SEND Command"));

    sendInfo(QString("waiting on the response from the server"));
    rc = rtt_recv(hdl, &res, RT_CMD_START);
    sendInfo(QString("Received Command: %1").arg(res.ret_code));

    if (rc < 0 || (rc > 0 && res.ret_code != RT_ERR_OK)) {
        sendInfo(QString().sprintf("Failed to exe command: %d", res.ret_code));
        return;
    }

    sendInfo(QString("Command Begin."));

    while (1) {
        rc = rtt_recv(hdl, &res, RT_CMD_MAX); /* receive all messages from daemon */

        if (rc <= 0) {
            break;
        }

        mapInfo.clear();

        if (res.type == RT_CMD_CLOSE_SBOX) {
            qDebug() << "RT_CMD_CLOSE_SBOX";
            sendInfo("exitWifi"); // to do
            break;
        }

        if (res.type == RT_CMD_SET_SUBMODEL) {
            rt_cmd_result_t *result = &res;
            for (int i = 0; i < (int)result->dev_count; i++) {
                // QString strMac(res(mac));
                QString strModel(res(submodel));
                qDebug() << "get_submode val: " << strModel;

                if (!strModel.isEmpty()) {
                    strlist.clear();
                    // sendInfo(strMac + "  :  " + strModel);
                    strlist << strModel << "submodel";
                    mapInfo.insert(res(mac), strlist);
                    sendResInfo(mapInfo); // to do
                }
            }
            //break;
        }

        if (res.type == RT_CMD_STATUS) {
            switch (g_test_type){
            case TEST_TYPE_WIFI:
                print_test_status(hdl, &res);
                break;
            case TEST_TYPE_MODEL:
                print_test_modle_status(hdl,&res);
                break;
            default:
                break;
            }
        }

        if (res.type == RT_CMD_RESULT) {
            qDebug() << "g_test_type value: " << g_test_type;
            sendInfo(QString("Test Result: "));
            switch (g_test_type){
            case TEST_TYPE_WIFI:
                print_test_status(hdl, &res);
                break;
            case TEST_TYPE_MODEL:
                print_test_modle_status(hdl,&res);
                break;
            default:
                break;
            }
            memcpy(&final_res, &res, sizeof(res));
            // break;
        }
    }
}

void NoxClient::parse_result_contrl_dev(rt_cmd_result_t * res, rt_cmd_t * cmd, int rc)
{
    if (rc < 0 || (rc > 0 && res->ret_code != RT_ERR_OK)) {
        sendInfo(QString().sprintf("MAC[%s] 执行指令[%s]失败，错误码：%d.",cmd->body.mac, cmd_name[cmd->type], res->ret_code));
        if(cmd->type == RT_CMD_ADD_BEACON){
            sendInfo(QString().sprintf("遥控器[%s-%s]", cmd->body.beacon.mac_beacon, cmd->body.beacon.key));
        }
        return;
    } else {
        sendInfo(QString().sprintf("MAC[%s] 执行指令[%s]成功.", cmd->body.mac, cmd_name[cmd->type]));
        if (cmd->type == RT_CMD_ADD_BEACON) {
            sendInfo(QString().sprintf("遥控器[%s-%s]", cmd->body.beacon.mac_beacon, cmd->body.beacon.key));
        } else if(cmd->type == RT_CMD_SET_SUBMODEL) {
            sendInfo(QString().sprintf("设置SUB_MODEL[VAL = %d]", cmd->body.sudmodel.sudmodel_val));
        }
    }
}

void NoxClient::control_dev(rtt_handle_t hdl, char* body, int result)
{
    rt_cmd_result_t res;
    rt_cmd_t cmd;
    int rc;

    // for command types 0, 1, 2
    char * mac = body;

    qDebug() << "control_dev: " << result;
    if (result == 0) {
        cmd.type = RT_CMD_CTRL_DEV;
        strcpy(cmd.body.mac, mac);
        rtt_send(hdl, &cmd);
        sendInfo(QString("waiting on the response from the server"));
        rc = rtt_recv(hdl, &res, RT_CMD_CTRL_DEV);
    } else if (result == 1) {
        strcpy(cmd.body.mac, mac);
        cmd.type = RT_CMD_FAIL_DEV;
        rtt_send(hdl, &cmd);
        sendInfo(QString("waiting on the response from the server"));
        rc = rtt_recv(hdl, &res, RT_CMD_FAIL_DEV);
    } else if (result == 2){
        strcpy(cmd.body.mac, mac);
        cmd.type = RT_CMD_OTA_DEV;
        rtt_send(hdl, &cmd);
        sendInfo(QString("waiting on the response from the server"));
        rc = rtt_recv(hdl, &res, RT_CMD_OTA_DEV);
    } else if (result == 4) {
        strcpy(cmd.body.mac, mac);
        cmd.type = RT_CMD_START_AUDIO_TEST;
        rtt_send(hdl, &cmd);
        sendInfo(QString("waiting on the response from the server"));
        rc = rtt_recv(hdl, &res, RT_CMD_START_AUDIO_TEST);
    } else if (result == 5) {
        strcpy(cmd.body.mac, mac);
        cmd.type = RT_CMD_START_BLE_TEST;
        rtt_send(hdl, &cmd);
        sendInfo(QString("waiting on the response from the server"));
        rc = rtt_recv(hdl, &res, RT_CMD_START_BLE_TEST);
    } else if (result == 6) {   /* special handling for get audio result */
        strcpy(cmd.body.mac, mac);
        cmd.type = RT_CMD_GET_AUDIO_TEST;
        rtt_send(hdl, &cmd);
        sendInfo(QString("waiting on the response from the server"));
        rc = rtt_recv(hdl, &res, RT_CMD_GET_AUDIO_TEST);

        if (rc < 0 || (rc > 0 && res.ret_code != RT_ERR_OK)) {
            sendInfo("1");
            sendInfo(QString().sprintf("Failed to exe command %d.\n", res.ret_code));
            return;
        } else {
            sendInfo(QString().sprintf("command execute successfully.\n"));
        }

        sendInfo(QString("waiting on the response from the server"));
        rc = rtt_recv(hdl, &res, RT_CMD_GET_AUDIO_TEST);

        if (rc < 0) {
            sendInfo(QString().sprintf("Failed to get result\n"));
            return;
        } else {
            sendInfo(QString().sprintf("audio test result for [%s] is %s\n",
                                       res.body.audio_res.mac,
                                       res.body.audio_res.result));
        }
        return;
    } else if (result == 7) {   /* set submodel for get audio result */
        cmd.type = RT_CMD_SET_SUBMODEL;
        memcpy(&cmd.body, body, sizeof(submodel_cmd_param_t));
        rtt_send(hdl, &cmd);
        sendInfo(QString("waiting on the response from the server"));
        rc = rtt_recv(hdl, &res, RT_CMD_SET_SUBMODEL);

    } else {
        cmd.type = RT_CMD_ADD_BEACON;
        memcpy(&cmd.body, body, sizeof(beacon_cmd_param_t));
        // printf("%d: add beacon %s-%s for %s\r\n", cmd.type, cmd.body.beacon.mac_beacon, cmd.body.beacon.key, cmd.body.beacon.mac_dev);
        // printf("send add beacon command\r\n");
        rtt_send(hdl, &cmd);
        // printf("recv response...\r\n");
        sendInfo(QString("waiting on the response from the server"));
        rc = rtt_recv(hdl, &res, RT_CMD_ADD_BEACON);
    }

    parse_result_contrl_dev(&res, &cmd, rc);
}


void NoxClient::beginToWork()
{
    qDebug() << "begin nox client!!";

    m_hdl = NULL;
    sendInfo(QString("=================== Tool client version : v%1  ===================").arg(CLNT_VERION));
    int err;

    if (load_beacon()) {
        sendInfo(QString("未发现遥控器信息，如wifi测试需手动输入"));
    }

    if (load_target_model()) {
        sendInfo(QString("未发现model信息，如model测试需手动输入"));
    }

    setbuf(stdout, NULL);

    m_hdl = rtt_connect(5, &err, RTT_PROTO_UDP, m_pIP);
    qDebug() << "return connect : " << m_hdl << m_pIP;
    if (!m_hdl) {
        sendInfo(QString("连不上测试服务器 %1").arg(err));
    }

    sendInfo("======================================================");
}


void NoxClient::onRecvCmd(int cmd, QString strInfo)
{
    qDebug() << "cmd:" << cmd;
    switch (cmd) {
    case CMD_SET_MAC:

        if (-1 ==  set_beacon(strInfo)) {
            sendInfo(QString("MAC地址格式不正确"));
        } else {
            sendInfo(QString("MAC设置成功"));
        }
        break;
    case CMD_SET_MODEL:

        if (-1 == set_target_model(strInfo)) {
            sendInfo(QString("model格式不正确"));
        } else {
            sendInfo(QString("model设置成功"));
        }
        break;
    case CMD_START_WIFI_TEST:
        g_test_type = TEST_TYPE_WIFI;
        sendInfo(QString("Start WIFI TEST"));
        run_test(m_hdl);
        break;
    case CMD_START_MODEL_TEST:
        g_test_type = TEST_TYPE_MODEL;
        sendInfo(QString("Start MODEL TEST"));
        run_test(m_hdl);
        break;
    case CMD_SET_SUBMODEL:
        sendInfo(QString("submodel = ").append(QString::number(strInfo.toInt())));
        g_submode = strInfo.toInt();
        g_is_set_submode = true;
        break;
    default:
        break;
    }

}


void Delay_MSec(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}
