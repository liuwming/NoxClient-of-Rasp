#ifndef RTT_INTERFACE_H
#define RTT_INTERFACE_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

#define MAX_TEST_DEV  16

#define rt_cmd_interface "/tmp/rt_cmd_interface"

#define RTT_PROTO_UNIX_SOCKET  0
#define RTT_PROTO_UDP          1
#define UDP_PORT  43210

typedef void* rtt_handle_t;

typedef enum {
    RT_CMD_START, // 0
    RT_CMD_STOP,
    RT_CMD_STATUS,
    RT_CMD_RESULT,
    RT_CMD_SET_PARAM,
    RT_CMD_GET_PARAM, // 5
    RT_CMD_CTRL_DEV,
    RT_CMD_FAIL_DEV,
    RT_CMD_OTA_DEV,
    RT_CMD_QUIT,
    RT_CMD_OPEN_SBOX, // 10
    RT_CMD_CLOSE_SBOX,
    RT_CMD_ADD_BEACON,
    RT_CMD_START_AUDIO_TEST,
    RT_CMD_START_BLE_TEST,
    RT_CMD_GET_AUDIO_TEST, // 15
    RT_CMD_PRINT_LABEL, /* rtt_server -> rtt client */
    RT_CMD_MAX,
    RT_CMD_INF=0xffffffff,
} rt_cmd_type_t;

typedef enum {
    RT_ERR_OK,
    RT_ERR_BUSY,
    RT_ERR_INVALID,
    RT_ERR_CLEAR_DNS,
    RT_ERR_START_DNS,
    RT_ERR_START_HOSTAPD,
    RT_ERR_NOMEM,
    RT_ERR_FAILURE,
    RT_ERR_MAX,
    RT_ERR_INF=0xffffffff,
} rt_error_t;

typedef struct rt_param_field {
    int       prsnt;            /* 0: not present, 1: present */
    int       val;
} rt_param_field_t;

typedef struct rt_param {
    rt_param_field_t      duration;
    rt_param_field_t      sample_rate;
    rt_param_field_t      thresh_avg_rssi;
    rt_param_field_t      thresh_min_rssi;
    rt_param_field_t      thresh_max_rssi;
    rt_param_field_t      thresh_packet_loss;
} rt_param_t;

typedef struct beacon_cmd{
    char           mac_dev[32];
    char           mac_beacon[32];
    char           key[32];
} beacon_cmd_param_t;

typedef struct cmd {
    rt_cmd_type_t          type;
    union {
        rt_param_t         cmd_param;
        beacon_cmd_param_t beacon;
        char               mac[32];
    } body;
} rt_cmd_t;


    typedef enum {
        AUDIO_TEST_IDLE,
        AUDIO_TEST_UNKN,
        AUDIO_TEST_PASS,
        AUDIO_TEST_FAIL,
    }audio_test_result_t;

    typedef enum {
        HEALTH_STATUS_UNKN,
        HEALTH_STATUS_PASS,
        HEALTH_STATUS_FAIL,
    }health_status_t;

typedef struct rt_status {
    int                 rssi_calc_cnt;
    int                 avg_rssi;
    int                 min_rssi;
    int                 max_rssi;
    uint64_t            did;
    unsigned            packet_send;
    unsigned            packet_recv;
    char                fw_ver[16];
    char                mac[32];
    char                ip[16];
    char                state[32];
    char                model[32];
    int                 brs;
    int                 brf;
    int                 audio_test_result; // audio_test_result_t
    int                 health_status; // health_status_t
    int                 fcc;
} rt_status_t;

typedef struct rt_audio_res {
    char                mac[32];
    char                result[32];
} rt_audio_res_t;

typedef struct rt_label_info {
    char                mac[32];
    char                did[32];
} rt_label_info_t;

typedef struct cmd_result {
    rt_cmd_type_t        type;
    rt_error_t           ret_code;
    uint32_t             dev_count;
    union {
        rt_param_t       cmd_param;
        rt_audio_res_t   audio_res;
        rt_status_t      status[MAX_TEST_DEV];
        rt_label_info_t  label_info;
    } body;
} rt_cmd_result_t;

#define STATE_ALLOC_IP     "Acquiring IP"
#define STATE_IP_ALLOCED   "IP Allocated"
#define STATE_PROBE        "Probing Device"
#define STATE_HS           "Hand-shaking with Device"
#define STATE_DEV_CONN     "Device Connected"
#define STATE_PROBE_FAIL   "Probing Failed"


extern rtt_handle_t rtt_connect(int timeout, int* err, int proto, char* ip);
extern int rtt_discon(rtt_handle_t hdl);
extern int rtt_send(rtt_handle_t, rt_cmd_t* cmd);
extern int rtt_recv(rtt_handle_t, rt_cmd_result_t*, rt_cmd_type_t);
extern rt_status_t* rtt_get_status(rt_cmd_result_t*, int idx);

#ifdef __cplusplus
}
#endif

#endif
