#ifndef NOXCLIENT_H
#define NOXCLIENT_H

#include "../lib/rtt_interface.h"
#include <QThread>
#include <QMap>

#define CLNT_VERION            "1.7"
#define RSSI_THRESH_AVG        -50
#define RSSI_THRESH_MIN        -65
#define PACKAGE_LOSS_THRESH     75
#define MIN_PACKAGE             8    /*minimum package received from device*/

#define beacon_info_fn          ".beacon_info"
#define model_info_fn           "target_model_config"
#define res(field) result->body.status[i].field

// #define EXIT_FACTORY_WIFI_PASS  1

//#define LOG_RESULT

typedef struct {
    char name[50];
    char model[32];
} prod_name_model;

typedef QMap<QString, QStringList> mapResInfo;

enum TEST_TYPE {
    TEST_TYPE_WIFI  = 1,
    TEST_TYPE_MODEL = 2
};

enum CMD_TYPE {
    CMD_SET_MAC = 1,
    CMD_SET_MODEL = 2,
    CMD_START_WIFI_TEST = 3,
    CMD_START_MODEL_TEST = 4,
    CMD_SET_SUBMODEL = 5,
    CMD_VERIFY_VER = 6,
    CMD_EXIT_TEST
};

extern int g_test_type;

class NoxClient : public QObject
{
    Q_OBJECT

public:
    explicit NoxClient(char *);
    ~NoxClient();
signals:
    void sendInfo(QString);
    void sendResInfo(mapResInfo);

private slots:
    void onRecvCmd(int cmd, QString strInfo);
    void beginToWork();

private:
    rtt_handle_t m_hdl;

    void run_test(rtt_handle_t hdl);

    int load_beacon();
    int load_target_model();
    int set_beacon(QString);

    int valid_mac(char *s);
    int set_target_model(QString);
    int nox_client(char *ptr_ip);
    void parse_result_contrl_dev(rt_cmd_result_t * res, rt_cmd_t * cmd, int rc);
    void control_dev(rtt_handle_t hdl, char* body, int result);
    void print_test_status(rtt_handle_t hdl, rt_cmd_result_t* result);
    void print_test_modle_status(rtt_handle_t hdl, rt_cmd_result_t* result);
    void test_stop(rtt_handle_t hdl);

    char *m_pIP;
    QThread *m_show_thread;
    QString m_strDesktopDir;

};


#endif
