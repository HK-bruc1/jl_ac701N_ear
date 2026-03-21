/**
 * @file tuya_ble_type.h
 * @brief This is tuya_ble_type file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_TYPE_H__
#define __TUYA_BLE_TYPE_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#ifndef NULL
#define NULL                         0
#endif

#if defined(__CC_ARM)
#pragma anon_unions
#elif defined(__ICCARM__)
#pragma language = extended
#elif defined(__GNUC__)
/* anonymous unions are enabled by default */
#endif

#if defined ( __CC_ARM )

#ifndef __TUYA_BLE_ASM
#define __TUYA_BLE_ASM               __asm
#endif

#ifndef __TUYA_BLE_INLINE
#define __TUYA_BLE_INLINE            __inline
#endif

#ifndef __TUYA_BLE_WEAK
#define __TUYA_BLE_WEAK              __weak
#endif

#ifndef __TUYA_BLE_ALIGN
#define __TUYA_BLE_ALIGN(n)          __align(n)
#endif

#ifndef __TUYA_BLE_PACKED
#define __TUYA_BLE_PACKED            __packed
#endif

#define TUYA_BLE_GET_SP()            __current_sp()

#elif defined ( __ICCARM__ )

#ifndef __TUYA_BLE_ASM
#define __TUYA_BLE_ASM               __asm
#endif

#ifndef __TUYA_BLE_INLINE
#define __TUYA_BLE_INLINE            inline
#endif

#ifndef __TUYA_BLE_WEAK
#define __TUYA_BLE_WEAK              __weak
#endif

#ifndef __TUYA_BLE_ALIGN
#define TUYA_BLE_STRING_PRAGMA(x) _Pragma(#x)
#define __TUYA_BLE_ALIGN(n) STRING_PRAGMA(data_alignment = n)
#endif

#ifndef __TUYA_BLE_PACKED
#define __TUYA_BLE_PACKED            __packed
#endif

#define TUYA_BLE_GET_SP()            __get_SP()

#elif defined   ( __GNUC__ )

#ifndef __TUYA_BLE_ASM
#define __TUYA_BLE_ASM               __asm
#endif

#ifndef __TUYA_BLE_INLINE
#define __TUYA_BLE_INLINE            inline
#endif

#ifndef __TUYA_BLE_WEAK
#define __TUYA_BLE_WEAK              __attribute__((weak))
#endif

#ifndef __TUYA_BLE_ALIGN
#define __TUYA_BLE_ALIGN(n)          __attribute__((aligned(n)))
#endif

#ifndef __TUYA_BLE_PACKED
#define __TUYA_BLE_PACKED           __attribute__((packed))
#endif

#define TUYA_BLE_GET_SP()           tuya_ble_gcc_current_sp()

// STATIC inline unsigned int tuya_ble_gcc_current_sp(VOID_T)
// {
// register unsigned sp __asm("sp");
// return sp;
// }

#endif

#define TUYA_BLE_EVT_BASE                       0x00
#define TUYA_BLE_CB_EVT_BASE                    0x40
#define H_ID_LEN                                19
#define TUYA_BLE_PRODUCT_ID_DEFAULT_LEN         8
//#define PRODUCT_KEY_LEN                         TUYA_BLE_PRODUCT_ID_OR_KEY_LEN
#define DEVICE_ID_LEN                           16
#define DEVICE_ID_LEN_MAX                       20
#define AUTH_KEY_LEN                            32
#define LOGIN_KEY_LEN                           6
#define ECC_SECRET_KEY_LEN                      32
#define DEVICE_VIRTUAL_ID_LEN                   22
#define SECRET_KEY_LEN                          16
#define PAIR_RANDOM_LEN                         6
#define MAC_LEN                                 6
#define MAC_STRING_LEN                          12
#define BEACON_KEY_LEN                          16
#define LOGIN_KEY_V2_LEN                        16
#define TUYA_BLE_PRODUCT_ID_MAX_LEN             16
#define TUYA_BLE_ADV_LOCAL_NAME_MAX_SPACE_LEN   14

/** @defgroup TUY_BLE_DEVICE_COMMUNICATION_ABILITY tuya ble device communication ability
 * @{
 */
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE                           0x0000
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_REGISTER_FROM_BLE             0x0001
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_MESH                          0x0002
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_24G                      0x0004
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_5G                       0x0008
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_ZIGBEE                        0x0010
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_NB                            0x0020
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_LONG_RANGE                    0x0400
/** End of TUY_BLE_DEVICE_COMMUNICATION_ABILITY
  * @}
  */

#define TUYA_BLE_LOG_LEVEL_ERROR                                            1U
#define TUYA_BLE_LOG_LEVEL_WARNING                                          2U
#define TUYA_BLE_LOG_LEVEL_INFO                                             3U
#define TUYA_BLE_LOG_LEVEL_DEBUG                                            4U

#define TUYA_APP_LOG_LEVEL_ERROR                                            1U
#define TUYA_APP_LOG_LEVEL_WARNING                                          2U
#define TUYA_APP_LOG_LEVEL_INFO                                             3U
#define TUYA_APP_LOG_LEVEL_DEBUG                                            4U

/** @defgroup TUYA_BLE_SECURE_CONNECTION_TYPE tuya ble secure connection type
 * @{
 */
#define TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY                            0x00
#define TUYA_BLE_SECURE_CONNECTION_WITH_ECC                                 0x01
#define TUYA_BLE_SECURE_CONNECTION_WTIH_PASSTHROUGH                         0x02
#define TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_DEVCIE_ID_20               0x03 //Discarded in agreements 4.0 and later
#define TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION        0x04
#define TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_FOR_QR_CODE                0x05
#define TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_V2                         0x06
#define TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION_V2     0x07
#define TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_FOR_QR_CODE_V2             0x08
/** End of TUYA_BLE_SECURE_CONNECTION_TYPE
  * @}
  */

#if ENABLE_BLUETOOTH_BREDR
/** @defgroup TUY_BLE_DEVICE_BR_EDR_ABILITY tuya ble device br-edr ability
 * @{
 */
#define TUYA_BLE_DEVICE_BR_EDR_ABILITY_DEFAULT                              0x00
#define TUYA_BLE_DEVICE_BR_EDR_ABILITY_CTKD                                 0x01

/** End of TUY_BLE_DEVICE_BR_EDR_ABILITY
  * @}
  */
#endif

typedef enum {
    TUYA_BLE_SUCCESS  = 0x00,
    TUYA_BLE_ERR_INTERNAL,
    TUYA_BLE_ERR_NOT_FOUND,
    TUYA_BLE_ERR_NO_EVENT,
    TUYA_BLE_ERR_NO_MEM,
    TUYA_BLE_ERR_INVALID_ADDR, // Invalid pointer supplied
    TUYA_BLE_ERR_INVALID_PARAM, // Invalid parameter(s) supplied.
    TUYA_BLE_ERR_INVALID_STATE, // Invalid state to perform operation.
    TUYA_BLE_ERR_INVALID_LENGTH,
    TUYA_BLE_ERR_DATA_SIZE,
    TUYA_BLE_ERR_TIMEOUT,
    TUYA_BLE_ERR_BUSY,
    TUYA_BLE_ERR_COMMON,
    TUYA_BLE_ERR_RESOURCES,
    TUYA_BLE_ERR_UNKNOWN, // other ble sdk errors
} tuya_ble_status_t;

typedef enum {
    TUYA_BLE_PRODUCT_ID_TYPE_PID,
    TUYA_BLE_PRODUCT_ID_TYPE_PRODUCT_KEY,
} tuya_ble_product_id_type_t;

typedef enum {
    TUYA_BLE_ADDRESS_TYPE_PUBLIC, // public address
    TUYA_BLE_ADDRESS_TYPE_RANDOM, // random address
} tuya_ble_addr_type_t;

#if ENABLE_BLUETOOTH_BREDR
/*
 * current br-edr connect status
 *@note
 * */
typedef enum {
    BR_EDR_UNCONNECT = 0,
    BR_EDR_CONNECTING,
    BR_EDR_CONNECTED,
    BR_EDR_UNKNOW_STATUS
} tuya_ble_br_edr_connect_status_t;

#endif

typedef enum {
    TUYA_BLE_EVT_MTU_DATA_RECEIVE = TUYA_BLE_EVT_BASE,  // 0
    TUYA_BLE_EVT_DEVICE_INFO_UPDATE,
    TUYA_BLE_EVT_DP_DATA_REPORTED,
    TUYA_BLE_EVT_DP_DATA_WITH_TIME_REPORTED,
    TUYA_BLE_EVT_DP_DATA_WITH_TIME_STRING_REPORTED,
    TUYA_BLE_EVT_DP_DATA_WITH_FLAG_REPORTED,    // 5
    TUYA_BLE_EVT_DP_DATA_WITH_FLAG_AND_TIME_REPORTED,
    TUYA_BLE_EVT_DP_DATA_WITH_FLAG_AND_TIME_STRING_REPORTED,
    TUYA_BLE_EVT_DP_DATA_SEND,
    TUYA_BLE_EVT_DP_DATA_WITH_TIME_SEND,
    TUYA_BLE_EVT_DEVICE_UNBIND, // 10
    TUYA_BLE_EVT_FACTORY_RESET,
    TUYA_BLE_EVT_OTA_RESPONSE,
    TUYA_BLE_EVT_BULK_DATA_RESPONSE,
    TUYA_BLE_EVT_DATA_PASSTHROUGH,
    TUYA_BLE_EVT_SPEECH_CONTROL,
    TUYA_BLE_EVT_SPEECH_RAW_DATA_REPORT,
    TUYA_BLE_EVT_SPEECH_RESULT_WRITE_RSP,
    TUYA_BLE_EVT_SPEECH_CLOCK_SETTING_RSP,
    TUYA_BLE_EVT_SPEECH_TOKEN_READ_RSP,
    TUYA_BLE_EVT_SPEECH_TOKEN_REPORT,   // 20
    TUYA_BLE_EVT_SPEECH_TOKEN_WRITE_RSP,
    TUYA_BLE_EVT_SPEECH_COMMON_SETTING_RSP,
    TUYA_BLE_EVT_GPT_CONTROL,
    TUYA_BLE_EVT_GPT_RAW_DATA_REPORT,
    TUYA_BLE_EVT_GPT_RESULT_WRITE_RSP,  // 25
    TUYA_BLE_EVT_GPT_RAW_DATA_WRITE_RSP,
    TUYA_BLE_EVT_PRODUCTION_TEST_RESPONSE,
    TUYA_BLE_EVT_UART_CMD,
    TUYA_BLE_EVT_BLE_CMD,
    TUYA_BLE_EVT_NET_CONFIG_RESPONSE,   // 30
    TUYA_BLE_EVT_CUSTOM,
    TUYA_BLE_EVT_CONNECT_STATUS_UPDATE,
    TUYA_BLE_EVT_UNBOUND_RESPONSE,
    TUYA_BLE_EVT_ANOMALY_UNBOUND_RESPONSE,
    TUYA_BLE_EVT_DEVICE_RESET_RESPONSE,
    TUYA_BLE_EVT_TIME_REQ,
    TUYA_BLE_EVT_EXTEND_TIME_REQ,
    TUYA_BLE_EVT_GATT_SEND_DATA,
    TUYA_BLE_EVT_CONNECTING_REQUEST,
    TUYA_BLE_EVT_WEATHER_DATA_REQ,      // 40
    TUYA_BLE_EVT_LINK_STATUS_UPDATE,
    TUYA_BLE_EVT_SCENE_DATA_REQ,
    TUYA_BLE_EVT_REMOTER_PROXY_AUTH,
    TUYA_BLE_EVT_REMOTER_GROUP_SET,
    TUYA_BLE_EVT_REMOTER_GROUP_DELETE,  // 45
    TUYA_BLE_EVT_REMOTER_GROUP_GET,
    TUYA_BLE_EVT_DP_DATA_WITH_SRC_TYPE_SEND,
    TUYA_BLE_EVT_DP_DATA_WITH_SRC_TYPE_AND_TIME_SEND,
    TUYA_BLE_EVT_ACCESSORY_INFO_REPORT,
} tuya_ble_evt_t;

/*
 * dp data report mode
 *@note
 * */
typedef enum {
    DP_SEND_FOR_CLOUD_PANEL = 0,   // The mobile app uploads the received dp data to the cloud and also sends it to the panel for display.
    DP_SEND_FOR_CLOUD,             // The mobile app will only upload the received dp data to the cloud.
    DP_SEND_FOR_PANEL,             // The mobile app will only send the received dp data to the panel display.
    DP_SEND_FOR_NONE,              // Neither uploaded to the cloud nor sent to the panel display.
} tuya_ble_dp_data_send_mode_t;

typedef enum {
    DP_SEND_TYPE_ACTIVE = 0,       // The device actively sends dp data.
    DP_SEND_TYPE_PASSIVE,          // The device passively sends dp data. For example, in order to answer the dp query command of the mobile app. Currently only applicable to WIFI+BLE combo devices.
} tuya_ble_dp_data_send_type_t;

typedef enum {
    DP_SEND_WITH_RESPONSE = 0,    //  Need a mobile app to answer.
    DP_SEND_WITHOUT_RESPONSE,     //  No need for mobile app to answer.
} tuya_ble_dp_data_send_ack_t;

typedef enum {
    DEVICE_INFO_TYPE_PID,
    DEVICE_INFO_TYPE_PRODUCT_KEY,
    DEVICE_INFO_TYPE_LOGIN_KEY,
    DEVICE_INFO_TYPE_BOUND,
    DEVICE_INFO_TYPE_BEACON_KEY,
} tuya_ble_device_info_type_t;

typedef enum {
    DP_TIME_TYPE_MS_STRING = 0,
    DP_TIME_TYPE_UNIX_TIMESTAMP,
} tuya_ble_dp_data_send_time_type_t;

/*
 * Data source type.
 * */
typedef enum {
    DATA_SOURCE_TYPE_MAIN_EQUIPMENT,
    DATA_SOURCE_TYPE_ACCESSORY_EQUIPMENT
} tuya_ble_data_source_type_t;

/*
 * ota data
 * */
typedef enum {
    TUYA_BLE_OTA_REQ,
    TUYA_BLE_OTA_FILE_INFO,
    TUYA_BLE_OTA_FILE_OFFSET_REQ,
    TUYA_BLE_OTA_DATA,
    TUYA_BLE_OTA_END,
    TUYA_BLE_OTA_PREPARE_NOTIFICATION,
#if defined(TUYA_BLE_OTA_SIGNATURE_ENABLE) && (TUYA_BLE_OTA_SIGNATURE_ENABLE == 1)
    TUYA_BLE_OTA_SIGNATURE_DATA,
    TUYA_BLE_OTA_SIGNATURE_KEY_UPDATE,
#endif
    TUYA_BLE_OTA_UNKONWN,
} tuya_ble_ota_data_type_t;

/*
 * accessory ota data
 * */
typedef enum {
    TUYA_BLE_ACCESSORY_OTA_REQ,
    TUYA_BLE_ACCESSORY_OTA_FILE_INFO,
    TUYA_BLE_ACCESSORY_OTA_FILE_OFFSET_REQ,
    TUYA_BLE_ACCESSORY_OTA_DATA,
    TUYA_BLE_ACCESSORY_OTA_END,
    TUYA_BLE_ACCESSORY_OTA_UNKONWN,
} tuya_ble_accesory_ota_data_type_t;

typedef enum {
    /**
     * Idle state: No update available, no upgrade in progress.
     */
    TUYA_BLE_SINGLE_BANK_OTA_STATUS_IDLE = 0,//Idle state: No update available, no upgrade in progress.

    /**
     * Preparation state: An update is available, preparing to jump to BOOT for update.
     */
    TUYA_BLE_SINGLE_BANK_OTA_STATUS_PREPARING,

    /**
     * Ready state: Successfully jumped to BOOT, waiting for the update process to start.
     */
    TUYA_BLE_SINGLE_BANK_OTA_STATUS_READY,

    /**
     * Transmitting state: The update process has started, data is being transmitted.
     */
    TUYA_BLE_SINGLE_BANK_OTA_STATUS_TRANSMITTING,

    /**
     * Success state: The update package has been successfully transmitted, ready to jump to the program area.
     */
    TUYA_BLE_SINGLE_BANK_OTA_STATUS_SUCCESS,

    /**
     * Failure state: An error occurred during the update process.
     */
    TUYA_BLE_SINGLE_BANK_OTA_STATUS_FAILURE,
} tuya_ble_single_bank_ota_status_t;

typedef enum {
    MD5_CRYPT_LOOP_STEP_INIT = 0,
    MD5_CRYPT_LOOP_STEP_UPDATE,
    MD5_CRYPT_LOOP_STEP_FINISH,
} ENUM_MD5_CRYPE_LOOP_STEP;

/*
 * file data
 * */
typedef enum {
    TUYA_BLE_FILE_INFO,
    TUYA_BLE_FILE_OFFSET_REQ,
    TUYA_BLE_FILE_DATA,
    TUYA_BLE_FILE_END,
    TUYA_BLE_FILE_UNKONWN,
} tuya_ble_file_data_type_t;

typedef enum {
    TUYA_BLE_BULK_DATA_EVT_READ_INFO,
    TUYA_BLE_BULK_DATA_EVT_READ_BLOCK,
    TUYA_BLE_BULK_DATA_EVT_SEND_DATA,
    TUYA_BLE_BULK_DATA_EVT_ERASE,
    TUYA_BLE_BULK_DATA_EVT_SYNC_REQ,
    TUYA_BLE_BULK_DATA_EVT_UNKONWN,
} tuya_ble_bulk_data_evt_type_t;

typedef enum {
    TUYA_BLE_CONNECTED,
    TUYA_BLE_DISCONNECTED,
} tuya_ble_connect_status_change_t;

/*
 * tuya ble call back event type.
 * */
typedef enum {
    TUYA_BLE_CB_EVT_CONNECT_STATUS = TUYA_BLE_CB_EVT_BASE,      //0x40  // 64
    TUYA_BLE_CB_EVT_DP_WRITE,          // old version
    TUYA_BLE_CB_EVT_DP_QUERY,
    TUYA_BLE_CB_EVT_DP_DATA_RECEIVED,  // new version
    TUYA_BLE_CB_EVT_OTA_DATA,
    TUYA_BLE_CB_EVT_BULK_DATA,      // 0x45     69
    TUYA_BLE_CB_EVT_NETWORK_INFO,
    TUYA_BLE_CB_EVT_WIFI_SSID,
    TUYA_BLE_CB_EVT_TIME_STAMP,
    TUYA_BLE_CB_EVT_TIME_NORMAL,
    TUYA_BLE_CB_EVT_APP_LOCAL_TIME_NORMAL,  // 0x50     74
    TUYA_BLE_CB_EVT_TIME_STAMP_WITH_DST,
    TUYA_BLE_CB_EVT_DATA_PASSTHROUGH,
    TUYA_BLE_CB_EVT_SPEECH_CONTROL,
    TUYA_BLE_CB_EVT_SPEECH_RAW_DATA_REPORT_RSP,
    TUYA_BLE_CB_EVT_SPEECH_RESULT_WRITE,
    TUYA_BLE_CB_EVT_SPEECH_CLOCK_SETTING,
    TUYA_BLE_CB_EVT_SPEECH_TOKEN_READ,
    TUYA_BLE_CB_EVT_SPEECH_TOKEN_REPORT_RSP,
    TUYA_BLE_CB_EVT_SPEECH_TOKEN_WRITE,
    TUYA_BLE_CB_EVT_SPEECH_COMMON_SETTING,
    TUYA_BLE_CB_EVT_GPT_CONTROL,
    TUYA_BLE_CB_EVT_GPT_RAW_DATA_REPORT_RSP,
    TUYA_BLE_CB_EVT_GPT_RESULT_WRITE,
    TUYA_BLE_CB_EVT_GPT_RAW_DATA_WRITE,
    TUYA_BLE_CB_EVT_DP_DATA_REPORT_RESPONSE,
    TUYA_BLE_CB_EVT_DP_DATA_WTTH_TIME_REPORT_RESPONSE,
    TUYA_BLE_CB_EVT_DP_DATA_WITH_FLAG_REPORT_RESPONSE,
    TUYA_BLE_CB_EVT_DP_DATA_WITH_FLAG_AND_TIME_REPORT_RESPONSE,
    TUYA_BLE_CB_EVT_DP_DATA_SEND_RESPONSE,               // new version
    TUYA_BLE_CB_EVT_DP_DATA_WITH_TIME_SEND_RESPONSE,     // new version
    TUYA_BLE_CB_EVT_UNBOUND,
    TUYA_BLE_CB_EVT_ANOMALY_UNBOUND,
    TUYA_BLE_CB_EVT_DEVICE_RESET,
    TUYA_BLE_CB_EVT_UPDATE_LOGIN_KEY_VID,
    TUYA_BLE_CB_EVT_UNBIND_RESET_RESPONSE,               // Notify the application of the result of the local reset
    TUYA_BLE_CB_EVT_WEATHER_DATA_REQ_RESPONSE,             // received request weather data app response
    TUYA_BLE_CB_EVT_WEATHER_DATA_RECEIVED,                  // received app sync weather data
    TUYA_BLE_CB_EVT_SCENE_REQ_RESPONSE,                     // received request scene data or control app response
    TUYA_BLE_CB_EVT_SCENE_DATA_RECEIVED,                  // received app sync iot scene list data
    TUYA_BLE_CB_EVT_SCENE_CTRL_RESULT_RECEIVED,          // received app sync iot scene control result
    TUYA_BLE_CB_EVT_APP_PASSTHROUGH_DATA,
    TUYA_BLE_CB_EVT_REMOTER_PROXY_AUTH_RESP,
    TUYA_BLE_CB_EVT_REMOTER_GROUP_SET,
    TUYA_BLE_CB_EVT_REMOTER_GROUP_DELETE,
    TUYA_BLE_CB_EVT_REMOTER_GROUP_GET,
    TUYA_BLE_CB_EVT_QUERY_EXT_MODULE_DEV_INFO,          // received app query ext module request
    TUYA_BLE_CB_EVT_EXT_MODULE_ACTIVE_INFO_RECEIVED,    // received app sync ext module active data
    TUYA_BLE_CB_EVT_FILE_DATA,
    TUYA_BLE_CB_EVT_WITH_SRC_TYPE_DP_QUERY,//receives a DP data query with data source type.
    TUYA_BLE_CB_EVT_WITH_SRC_TYPE_DP_DATA_RECEIVED,//receives a DP data with data source type .
    TUYA_BLE_CB_EVT_DP_DATA_WITH_SRC_TYPE_SEND_RESPONSE,//receives a DP data with data source type send response.
    TUYA_BLE_CB_EVT_DP_DATA_WITH_SRC_TYPE_AND_TIME_SEND_RESPONSE,//receives a DP data with data source type and time send response.
    TUYA_BLE_CB_EVT_ACCESSORY_INFO_READING,// received from the APP to read the accessory connection information.
    TUYA_BLE_CB_EVT_ACCESSORY_INFO_REPORT_RESPONSE,// receive an accessory information report response.
    TUYA_BLE_CB_EVT_ACCESSORY_ACTIVE_INFO_RECEIVED,// receive an accessory active information.
    TUYA_BLE_CB_EVT_ACCESSORY_OTA_DATA,// receive an accessory ota data.
    TUYA_BLE_CB_EVT_GROUP_DP_DATA_RECEIVED,
} tuya_ble_cb_evt_t;

/*
 * current connect status
 *@note
 * */
typedef enum {
    UNBONDING_UNCONN = 0,
    UNBONDING_CONN,
    BONDING_UNCONN,
    BONDING_CONN,
    BONDING_UNAUTH_CONN,
    UNBONDING_UNAUTH_CONN,
    UNKNOW_STATUS
} tuya_ble_connect_status_t;

/*
 * current BLE connect source type.
 *@note
 * */
typedef enum {
    UNKNOW_TYPES = 0,
    MOBILE_APP,
    GATEWAY,
    APPLET,
    LOCK_FITTING,
    IOS_APP
} tuya_ble_connect_source_type_t;

/*
 * current link status
 *@note
 * */
typedef enum {
    TY_LINK_DISCONNECTED = 0,
    TY_LINK_CONNECTED,
    TY_LINK_ENCRYPTED_REQUEST,
    TY_LINK_ENCRYPTED,
    TY_LINK_UNKNOW_STATUS
} tuya_ble_link_status_t;

/*
 * current connect status
 *@note
 * */
typedef enum {
    TUYA_BLE_AUTH_STATUS_PHASE_NONE = 0,
    TUYA_BLE_AUTH_STATUS_PHASE_1,
    TUYA_BLE_AUTH_STATUS_PHASE_2,
    TUYA_BLE_AUTH_STATUS_PHASE_3,
} tuya_ble_auth_status_t;

typedef enum {
    RESET_TYPE_UNBIND,
    RESET_TYPE_FACTORY_RESET,
} tuya_ble_reset_type_t;

typedef enum {
    TUYA_BLE_TIMER_SINGLE_SHOT,
    TUYA_BLE_TIMER_REPEATED,
} tuya_ble_timer_mode;

/**@brief   Private data type, Generally stored at external security chip, such as cert / private key.
 *
 */
typedef enum {
    PRIVATE_DATA_ECC_KEY    = 1,
    PRIVATE_DATA_DEV_CERT   = 10,
    PRIVATE_DATA_TUYA_AUTH_TOKEN,
} tuya_ble_private_data_type;

/**@brief   Product test common data type, Generally storage by application, such as shorturl.
 *
 */
typedef enum {
    COMMON_DATA_SHORTURL = 1,
} tuya_ble_common_data_type;

/**@brief   Product test special data type, Generally storage by application, such as xo calibration.
 *
 */
typedef enum {
    SPECIAL_DATA_XO_CALIBRATION = 1,
} tuya_ble_special_data_type;

/** @defgroup TUYA_BLE_PROD_OEM_TYPE tuya ble prod oem type
 * @{
 */
#define TUYA_BLE_PROD_OEM_TYPE_NONE       ( 0 )
#define TUYA_BLE_PROD_OEM_TYPE_0_5        ( 1 )
#define TUYA_BLE_PROD_OEM_TYPE_1_0        ( 2 )
#define TUYA_BLE_PROD_OEM_TYPE_1_5        ( 3 )
#define TUYA_BLE_PROD_OEM_TYPE_2_0        ( 4 )
/** End of TUYA_BLE_PROD_OEM_TYPE
  * @}
  */

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
typedef struct {
    tuya_ble_addr_type_t addr_type;
    UINT8_T addr[6];
} tuya_ble_gap_addr_t;

#if ENABLE_BLUETOOTH_BREDR
/*
 * current br-edr connect status
 *@note
 * */
typedef struct {
    UINT8_T mac[6];
    UINT8_T dev_ability;  /**< default value is TUYA_BLE_DEVICE_BR_EDR_ABILITY_DEFAULT,if support ctkd ,the value is TUYA_BLE_DEVICE_BR_EDR_ABILITY_DEFAULT|TUYA_BLE_DEVICE_BR_EDR_ABILITY_CTKD */
    UINT8_T is_paired; /**< 0 - unpaired, 1 - paired. */
    tuya_ble_br_edr_connect_status_t connect_status;
    UINT8_T name_len;
    UINT8_T name[32];
} tuya_ble_br_edr_data_info_t;

#endif

typedef struct {
    UINT8_T use_ext_license_key; //If use the license key stored by the SDK,initialized to 0, Otherwise 1.
    UINT8_T device_id_len;       //if ==20,Compressed into 16
    UINT8_T device_id[DEVICE_ID_LEN_MAX];
    UINT8_T auth_key[AUTH_KEY_LEN];
    tuya_ble_gap_addr_t mac_addr;
    UINT8_T mac_addr_string[MAC_STRING_LEN];

    tuya_ble_product_id_type_t p_type;
    UINT8_T product_id_len;
    UINT8_T product_id[TUYA_BLE_PRODUCT_ID_MAX_LEN];
    UINT8_T adv_local_name_len;
    UINT8_T adv_local_name[TUYA_BLE_ADV_LOCAL_NAME_MAX_SPACE_LEN]; //Only supported when TUYA_BLE_PROTOCOL_VERSION_HIGN >= 4.
    UINT32_T firmware_version; //0x00010102 : v1.1.2
    UINT32_T hardware_version;

    UINT8_T device_vid[DEVICE_VIRTUAL_ID_LEN];
    UINT8_T login_key[LOGIN_KEY_LEN];
    UINT8_T beacon_key[BEACON_KEY_LEN];
    UINT8_T bound_flag;

    UINT8_T reserve_1;
    UINT8_T reserve_2;
} tuya_ble_device_param_t;

typedef struct {
    UINT16_T min_conn_interval; // Range: 0x0006 to 0x0C80, Time = N * 1.25 msec, Time Range: 7.5 msec to 4 seconds.
    UINT16_T max_conn_interval; // Range: 0x0006 to 0x0C80, Time = N * 1.25 msec, Time Range: 7.5 msec to 4 seconds.
    UINT16_T slave_latency; // Range: 0x0000 to 0x01F3
    UINT16_T conn_sup_timeout; // Range: 0x000A to 0x0C80, Time = N * 10 msec, Time Range: 100 msec to 32 seconds
} tuya_ble_gap_conn_param_t;

typedef struct {
    UINT8_T *p_data;             // Used when the length of mtu data is greater than 20.
    UINT8_T data[20];            // Used when the length of mtu data is less than or equal to 20, In order to improve communication efficiency in BLE4.0 devices.
    UINT16_T len;
} tuya_ble_mtu_data_receive_t;

typedef struct {
    tuya_ble_device_info_type_t type;
    UINT8_T len;
    UINT8_T data[32];
} tuya_ble_device_info_data_t;

typedef struct {
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_dp_data_reported_t;

typedef struct {
    UINT32_T timestamp;
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_dp_data_with_time_reported_t;

typedef struct {
    UINT8_T time_string[13 + 1];
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_dp_data_with_time_string_reported_t;

typedef struct {
    UINT16_T sn;
    tuya_ble_dp_data_send_mode_t mode;
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_dp_data_with_flag_reported_t;

typedef struct {
    UINT16_T sn;
    tuya_ble_dp_data_send_mode_t mode;
    UINT32_T timestamp;
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_dp_data_with_flag_and_time_reported_t;

typedef struct {
    UINT16_T sn;
    tuya_ble_dp_data_send_mode_t mode;
    UINT8_T time_string[13 + 1];
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_dp_data_with_flag_and_time_string_reported_t;

typedef struct {
    UINT32_T sn;
    tuya_ble_dp_data_send_type_t type;
    tuya_ble_dp_data_send_mode_t mode;
    tuya_ble_dp_data_send_ack_t ack;
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_dp_data_send_data_t;

typedef struct {
    uint32_t sn;
    tuya_ble_data_source_type_t src_type;
    tuya_ble_dp_data_send_type_t type;
    tuya_ble_dp_data_send_mode_t mode;
    tuya_ble_dp_data_send_ack_t ack;
    uint8_t *p_data;
    uint8_t *p_add_info;
    uint16_t data_len;
    uint8_t add_info_len;
} tuya_ble_dp_data_with_src_type_send_data_t;

typedef struct {
    UINT32_T sn;
    tuya_ble_dp_data_send_type_t type;
    tuya_ble_dp_data_send_mode_t mode;
    tuya_ble_dp_data_send_ack_t ack;
    tuya_ble_dp_data_send_time_type_t time_type;
    UINT8_T time_data[13 + 1];
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_dp_data_with_time_send_data_t;

typedef struct {
    uint32_t sn;
    tuya_ble_dp_data_send_type_t type;
    tuya_ble_dp_data_send_mode_t mode;
    tuya_ble_dp_data_send_ack_t ack;
    tuya_ble_dp_data_send_time_type_t time_type;
    uint8_t time_data[13 + 1];
    tuya_ble_data_source_type_t src_type;
    uint8_t add_info_len;
    uint8_t *p_add_info;
    uint8_t *p_data;
    uint16_t data_len;
} tuya_ble_dp_data_with_src_type_and_time_send_data_t;

typedef struct {
    UINT8_T reserve;
} tuya_ble_device_unbind_t;

typedef struct {
    UINT8_T reserve;
} tuya_ble_factory_reset_t;

typedef struct {
    tuya_ble_ota_data_type_t type;
    UINT16_T data_len;
    UINT8_T *p_data;
} tuya_ble_ota_response_t;

typedef struct {
    tuya_ble_file_data_type_t type;
    uint16_t data_len;
    uint8_t *p_data;
} tuya_ble_file_response_t;

typedef struct {
    UINT16_T block_number;
} tuya_ble_bulk_data_evt_read_block_req_t;

typedef struct {
    UINT16_T block_number;
} tuya_ble_bulk_data_evt_send_data_req_t;

typedef struct {
    tuya_ble_bulk_data_evt_type_t evt;
    UINT8_T bulk_type;
    union {
        tuya_ble_bulk_data_evt_read_block_req_t block_data_req_data;
        tuya_ble_bulk_data_evt_send_data_req_t  send_data_req_data;
    } params;
} tuya_ble_bulk_data_request_t;

typedef struct {
    UINT8_T status;
    UINT8_T flag;
    UINT32_T bulk_data_length;
    UINT32_T bulk_data_crc;
    UINT16_T block_data_length;
} tuya_ble_bulk_data_evt_read_info_res_t;

typedef struct {
    UINT8_T status;
    UINT16_T block_number;
    UINT16_T block_data_length;
    UINT16_T block_data_crc16;
    UINT16_T max_packet_data_length;
} tuya_ble_bulk_data_evt_read_block_res_t;

typedef struct {
    UINT16_T current_block_number;
    UINT16_T current_block_length;
    UINT8_T  *p_current_block_data;
} tuya_ble_bulk_data_evt_send_data_res_t;

typedef struct {
    UINT8_T status;
} tuya_ble_bulk_data_evt_erase_res_t;

typedef struct {
    UINT8_T  request_bulkdata_count;
    UINT8_T *p_request_bulkdata_info;
} tuya_ble_bulk_data_evt_sync_req_t;

typedef struct {
    tuya_ble_bulk_data_evt_type_t evt;
    UINT8_T bulk_type;
    union {
        tuya_ble_bulk_data_evt_read_info_res_t bulk_info_res_data;
        tuya_ble_bulk_data_evt_read_block_res_t block_res_data;
        tuya_ble_bulk_data_evt_send_data_res_t  send_res_data;
        tuya_ble_bulk_data_evt_erase_res_t erase_res_data;
        tuya_ble_bulk_data_evt_sync_req_t sync_req;
    } params;
} tuya_ble_bulk_data_response_t;

typedef struct {
    UINT16_T data_len;
    UINT8_T *p_data;
} tuya_ble_passthrough_data_t;

typedef struct {
    UINT16_T data_len;
    UINT8_T *p_data;
} tuya_ble_speech_data_t;

typedef struct {
    UINT8_T channel;
    UINT16_T data_len;
    UINT8_T *p_data;
} tuya_ble_production_test_response_data_t;

typedef struct {
    UINT32_T cmd;
    UINT16_T data_len;
    UINT8_T *p_data;
} tuya_ble_uart_cmd_t;

typedef struct {
    UINT32_T cmd;
    UINT16_T data_len;
    UINT8_T *p_data;
} tuya_ble_ble_cmd_t;

typedef struct {
    INT16_T result_code;
} tuya_ble_net_config_response_t;

typedef struct {
    UINT8_T cmd;
} tuya_ble_connecting_request_data_t;

typedef struct {
    INT32_T evt_id;
    VOID_T *data;
    VOID_T(*custom_event_handler)(INT32_T evt_id, VOID_T *data);
} tuya_ble_custom_evt_t;

typedef struct {
    UINT8_T result_code;
} tuya_ble_ubound_response_t;

typedef struct {
    UINT8_T result_code;
} tuya_ble_anomaly_ubound_response_t;

typedef struct {
    UINT8_T result_code;
} tuya_ble_device_reset_response_t;

typedef struct {
    // 0-13-byte millisecond string[from cloud]
    // 1 - normal time format[from cloud]
    // 2 - normal time format[from app local]
    UINT8_T time_type;
} tuya_ble_time_req_data_t;

typedef struct {
    UINT8_T type;
    UINT8_T mac[6];
    UINT8_T id;
    UINT8_T s1[2];
} tuya_ble_remoter_proxy_auth_data_unit_t;

typedef struct {
    UINT8_T num;
    UINT8_T *p_data;
} tuya_ble_remoter_proxy_auth_data_t;

typedef struct {
    UINT8_T status;
} tuya_ble_remoter_group_set_data_rsp_t;

typedef struct {
    UINT8_T status;
} tuya_ble_remoter_group_delete_data_rsp_t;

typedef struct {
    UINT8_T mac[6];
    UINT8_T id;
} tuya_ble_remoter_group_get_data_rsp_t;

typedef struct {
    UINT8_T n_years_dst;
} tuya_ble_extend_time_req_data_t;

typedef struct {
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_weather_req_data_t;

typedef struct {
    UINT8_T link_app_status;
} tuya_ble_link_status_update_data_t;

typedef struct {
    uint8_t *p_data;
    uint16_t data_len;
} tuya_ble_scene_req_data_t;

typedef struct {
    uint8_t *p_data;
    uint16_t data_len;
} tuya_ble_query_em_info_response_t;

typedef struct {
    uint8_t *attach_data;
    uint8_t attach_len;
} tuya_ble_accessory_info_reading_data_t;

typedef struct {
    uint8_t *active_info_data;
    uint8_t active_info_len;
} tuya_ble_accessory_active_info_data_t;

typedef struct {
    uint8_t *p_data;
    uint8_t data_len;
} tuya_ble_accessory_info_report_data_t;

typedef struct {
    uint8_t status;
} tuya_ble_accessory_info_report_response_t;

typedef struct {
    tuya_ble_evt_t  event;
    VOID_T(*event_handler)(VOID_T *evt);
} tuya_ble_evt_hdr_t;

/*
 * tuya ble sdk evt parameters union
 * */
typedef struct {
    tuya_ble_evt_hdr_t hdr;
    union {
        tuya_ble_mtu_data_receive_t  mtu_data;
        tuya_ble_device_info_data_t  device_info_data;
        tuya_ble_dp_data_reported_t  reported_data;
        tuya_ble_dp_data_with_time_reported_t       reported_with_time_data;
        tuya_ble_dp_data_with_time_string_reported_t reported_with_time_string_data;
        tuya_ble_dp_data_with_flag_reported_t        flag_reported_data;
        tuya_ble_dp_data_with_flag_and_time_reported_t       flag_reported_with_time_data;
        tuya_ble_dp_data_with_flag_and_time_string_reported_t flag_reported_with_time_string_data;
        tuya_ble_dp_data_send_data_t dp_send_data;
        tuya_ble_dp_data_with_time_send_data_t dp_with_time_send_data;
        tuya_ble_device_unbind_t   device_unbind_data;
        tuya_ble_factory_reset_t   factory_reset_data;
        tuya_ble_ota_response_t ota_response_data;
        tuya_ble_bulk_data_response_t bulk_res_data;
        tuya_ble_passthrough_data_t passthrough_data;
        tuya_ble_speech_data_t speech_data;
        tuya_ble_production_test_response_data_t prod_test_res_data;
        tuya_ble_uart_cmd_t uart_cmd_data;
        tuya_ble_ble_cmd_t    ble_cmd_data;
        tuya_ble_net_config_response_t net_config_response_data;
        tuya_ble_custom_evt_t custom_evt;
        tuya_ble_connect_status_change_t connect_change_evt;
        tuya_ble_ubound_response_t ubound_res_data;
        tuya_ble_anomaly_ubound_response_t anomaly_ubound_res_data;
        tuya_ble_device_reset_response_t device_reset_res_data;
        tuya_ble_time_req_data_t  time_req_data;
        tuya_ble_remoter_proxy_auth_data_t remoter_proxy_auth_data;
        tuya_ble_remoter_group_set_data_rsp_t remoter_group_set_data;
        tuya_ble_remoter_group_delete_data_rsp_t remoter_group_delete_data;
        tuya_ble_remoter_group_get_data_rsp_t remoter_group_get_data;
        tuya_ble_extend_time_req_data_t extend_time_req_data;
        tuya_ble_connecting_request_data_t connecting_request_data;
        tuya_ble_weather_req_data_t weather_req_data;
        tuya_ble_link_status_update_data_t link_update_data;
        tuya_ble_dp_data_with_src_type_send_data_t dp_with_src_type_send_data;
        tuya_ble_dp_data_with_src_type_and_time_send_data_t dp_with_src_type_and_time_send_data;
        tuya_ble_accessory_info_report_data_t accessory_info_report_data;
    };
} tuya_ble_evt_param_t;

/*
 * dp data  buffer:  (Dp_id,Dp_type,Dp_len,Dp_data),(Dp_id,Dp_type,Dp_len,Dp_data),....
 * */
typedef struct {
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_dp_write_data_t;

/*
 * query dp point data,if data_len is 0,means query all dp point data,otherwise query the dp point in p_data buffer.
 * */
typedef struct {
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_dp_query_data_t;

/*
 * query dp point data,if data_len is 0,means query all dp point data,otherwise query the dp point in p_data buffer.
 * ADD_INFO-Data source supplemental information. When the data source is the subject device, there is no supplementary information; however, when the data source is an accessory, this field holds the ID_INFO of the accessory.
 * */
typedef struct {
    uint8_t *p_add_info;
    uint8_t *p_data;
    uint16_t data_len;
    uint8_t add_info_len;
    tuya_ble_data_source_type_t src_type;
} tuya_ble_dp_query_data_with_src_type_t;

/*
* dp data  buffer(Dp_len:2):  (Dp_id,Dp_type,Dp_len,Dp_data),(Dp_id,Dp_type,Dp_len,Dp_data),....
 * */
typedef struct {
    UINT32_T sn;
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_dp_data_received_data_t;

typedef struct {
    UINT8_T *p_data;
    UINT16_T data_len;
} tuya_ble_group_dp_data_received_t;

typedef struct {
    uint32_t sn;
    uint8_t *p_data; //dp-point
    uint8_t *p_add_info;
    uint16_t data_len;
    uint8_t add_info_len;
    tuya_ble_data_source_type_t src_type;
} tuya_ble_dp_data_with_src_type_received_data_t;

typedef struct {
    tuya_ble_ota_data_type_t type;
    UINT16_T data_len;
    UINT8_T *p_data;
} tuya_ble_ota_data_t;

typedef struct {
    uint8_t *p_data;
    uint16_t data_len;
    tuya_ble_accesory_ota_data_type_t type;
} tuya_ble_accessory_ota_data_t;

typedef struct {
    tuya_ble_file_data_type_t type;
    uint16_t data_len;
    uint8_t *p_data;
} tuya_ble_file_data_t;

/*
 * network data,unformatted json data,for example " {"wifi_ssid":"tuya","password":"12345678","token":"xxxxxxxxxx"} "
 * */
typedef struct {
    UINT16_T data_len; //include '\0'
    UINT8_T *p_data;
} tuya_ble_network_data_t;

/*
 * wifi ssid data,unformatted json data,for example " {"wifi_ssid":"tuya","password":"12345678"} "
 * */
typedef struct {
    UINT16_T data_len; //include '\0'
    UINT8_T *p_data;
} tuya_ble_wifi_ssid_data_t;

/*
 * uninx timestamp
 * */
typedef struct {
    UINT8_T timestamp_string[14];
    INT16_T  time_zone;   //actual time zone Multiply by 100.
} tuya_ble_timestamp_data_t;

/*
 * normal time formatted
 * */
typedef struct {
    UINT16_T nYear;
    UINT8_T nMonth;
    UINT8_T nDay;
    UINT8_T nHour;
    UINT8_T nMin;
    UINT8_T nSec;
    UINT8_T DayIndex; /* 0 = Sunday */
    INT16_T time_zone;   //actual time zone Multiply by 100.
} tuya_ble_time_noraml_data_t;

typedef struct {
    UINT8_T type;
    UINT8_T mac[6];
    UINT8_T id;
    UINT8_T status;
} tuya_ble_remoter_proxy_auth_data_rsp_unit_t;

typedef struct {
    UINT8_T num;
    UINT8_T *p_data;
} tuya_ble_remoter_proxy_auth_data_rsp_t;

typedef struct {
    UINT8_T mac[6];
    UINT8_T id;
} tuya_ble_remoter_group_set_data_t;

typedef struct {
    UINT8_T mac[6];
    UINT8_T id;
} tuya_ble_remoter_group_delete_data_t;

typedef struct {
    UINT8_T mac[6];
} tuya_ble_remoter_group_get_data_t;
/*
 * normal time formatted
 * */
typedef struct {
    UINT32_T timestamp;
    INT16_T  time_zone;   /**< actual time zone Multiply by 100. */
    UINT8_T n_years_dst;  /**< how many years of daylight saving time. */
    UINT8_T *p_data;      /**< the point of dst data. */
    UINT16_T data_len;      /**< dst data length. */
} tuya_ble_timestamp_with_dst_data_t;

/*
 *
 * */
typedef struct {
    UINT8_T status;
} tuya_ble_dp_data_report_response_t;

/*
 *
 * */
typedef struct {
    UINT8_T status;
} tuya_ble_dp_data_with_time_report_response_t;

/*
 *
 * */
typedef struct {
    UINT16_T sn;
    tuya_ble_dp_data_send_mode_t mode;
    UINT8_T status;
} tuya_ble_dp_data_with_flag_report_response_t;

/*
 *
 * */
typedef struct {
    UINT16_T sn;
    tuya_ble_dp_data_send_mode_t mode;
    UINT8_T status;
} tuya_ble_dp_data_with_flag_and_time_report_response_t;

/*
 *
 * */
typedef struct {
    UINT32_T sn;
    tuya_ble_dp_data_send_type_t type;
    tuya_ble_dp_data_send_mode_t mode;
    tuya_ble_dp_data_send_ack_t ack;
    UINT8_T status;  // 0 - succeed, 1- failed.
} tuya_ble_dp_data_send_response_data_t;

/*
 *
 * */
typedef struct {
    UINT32_T sn;
    tuya_ble_dp_data_send_type_t type;
    tuya_ble_dp_data_send_mode_t mode;
    tuya_ble_dp_data_send_ack_t ack;
    UINT8_T status; // 0 - succeed, 1- failed.
} tuya_ble_dp_data_with_time_send_response_data_t;

/*
 *
 * */
typedef struct {
    uint32_t sn;
    tuya_ble_data_source_type_t src_type;
    tuya_ble_dp_data_send_type_t type;
    tuya_ble_dp_data_send_mode_t mode;
    tuya_ble_dp_data_send_ack_t ack;
    uint8_t status;  // 0 - succeed, 1- failed.
    uint8_t add_info_len;
    uint8_t *p_add_info;
} tuya_ble_dp_data_with_src_type_send_response_data_t;

/*
 *
 * */
typedef struct {
    uint32_t sn;
    tuya_ble_data_source_type_t src_type;
    tuya_ble_dp_data_send_type_t type;
    tuya_ble_dp_data_send_mode_t mode;
    tuya_ble_dp_data_send_ack_t ack;
    uint8_t status; // 0 - succeed, 1- failed.
    uint8_t add_info_len;
    uint8_t *p_add_info;
} tuya_ble_dp_data_with_src_type_and_time_send_response_data_t;

/*
 *
 * */
typedef struct {
    UINT8_T data;
} tuya_ble_unbound_data_t;

/*
 *
 * */
typedef struct {
    UINT8_T data;
} tuya_ble_anomaly_unbound_data_t;

/*
 *
 * */
typedef struct {
    UINT8_T data;
} tuya_ble_device_reset_data_t;

/*
 *
 * */
typedef struct {
    UINT8_T status;
} tuya_ble_weather_data_req_response_t;

/**
 * weather data  (key_len,key,vaule_type,value_len,value),(key_len,key,vaule_type,value_len,value),....
 *
 */
typedef struct {
    UINT16_T object_count;     /**< weather data object counts. */
    UINT8_T location;        /**< location. */
    UINT8_T *p_data;        /**< weather data. */
    UINT16_T data_len;        /**< weather data length. */
} tuya_ble_weather_data_received_data_t;

/*
 * received scene request response data
 * */
typedef struct {
    uint16_t scene_cmd;        /**< scene cmd. 1- scene data. 2-scene control */
    uint8_t status;            /**< response status, 0-success 1-failed. */
    uint32_t err_code;        /**< err code. */
} tuya_ble_scene_req_response_t;

/**
 * received scene list data
 *
 */
typedef struct {
    uint8_t status;            /**< status, 0-success 1-failed. */
    uint32_t err_code;        /**< err code. */
    bool need_update;        /**< need update. */
    uint32_t check_code;    /**< scene data check code, used crc32. */

    uint8_t *p_data;        /**< scene data. */
    uint16_t data_len;        /**< scene data length. */
} tuya_ble_scene_data_received_data_t;

/**
 * received scene control result
 *
 */
typedef struct {
    uint8_t status;            /**< status, 0-success 1-failed. */
    uint32_t err_code;        /**< err code. */
    uint8_t scene_id_len;    /**< scene id length. */
    uint8_t *p_scene_id;    /**< scene id. */
} tuya_ble_scene_ctrl_received_data_t;

/**
 * received ext module active info
 *
 */
typedef struct {
    uint8_t *p_data;        /**< ext module active data. */
    uint16_t data_len;        /**< active data length. */
} tuya_ble_ext_module_active_data_t;

/*
 *
 * */
#pragma pack(1)
typedef struct {
    uint16_t type;
    uint32_t data_len;
    uint8_t *p_data;
} tuya_ble_app_passthrough_data_t;
#pragma pack()

/*
 *
 * */
typedef struct {
    UINT8_T login_key_len;
    UINT8_T vid_len;
    UINT8_T beacon_key_len;
    UINT8_T login_key[LOGIN_KEY_LEN];
    UINT8_T vid[DEVICE_VIRTUAL_ID_LEN];
    UINT8_T beacon_key[BEACON_KEY_LEN];
} tuya_ble_login_key_vid_data_t;

/*
 *
 * */
typedef struct {
    tuya_ble_reset_type_t type;
    UINT8_T status;     //0-succeed,1-failed.
} tuya_ble_unbind_reset_response_data_t;

/*
 * tuya ble sdk callback parameters union
 * */
typedef struct {
    tuya_ble_cb_evt_t evt;
    union {
        tuya_ble_connect_status_t connect_status;
        tuya_ble_dp_write_data_t  dp_write_data;
        tuya_ble_dp_query_data_t  dp_query_data;
        tuya_ble_dp_data_received_data_t dp_received_data;
        tuya_ble_ota_data_t       ota_data;
        tuya_ble_bulk_data_request_t  bulk_req_data;
        tuya_ble_network_data_t   network_data;
        tuya_ble_wifi_ssid_data_t wifi_info_data;
        tuya_ble_timestamp_data_t timestamp_data;
        tuya_ble_time_noraml_data_t time_normal_data;
        tuya_ble_remoter_proxy_auth_data_rsp_t remoter_proxy_auth_data_rsp;
        tuya_ble_remoter_group_set_data_t remoter_group_set_data;
        tuya_ble_remoter_group_delete_data_t remoter_group_delete_data;
        tuya_ble_remoter_group_get_data_t remoter_group_get_data;
        tuya_ble_timestamp_with_dst_data_t timestamp_with_dst_data;
        tuya_ble_passthrough_data_t ble_passthrough_data;
        tuya_ble_speech_data_t ble_speech_data;
        tuya_ble_dp_data_report_response_t dp_response_data;
        tuya_ble_dp_data_with_time_report_response_t dp_with_time_response_data;
        tuya_ble_dp_data_with_flag_report_response_t dp_with_flag_response_data;
        tuya_ble_dp_data_with_flag_and_time_report_response_t dp_with_flag_and_time_response_data;
        tuya_ble_dp_data_send_response_data_t dp_send_response_data;
        tuya_ble_dp_data_with_time_send_response_data_t dp_with_time_send_response_data;
        tuya_ble_unbound_data_t unbound_data;
        tuya_ble_anomaly_unbound_data_t anomaly_unbound_data;
        tuya_ble_device_reset_data_t device_reset_data;
        tuya_ble_login_key_vid_data_t device_login_key_vid_data;
        tuya_ble_unbind_reset_response_data_t reset_response_data;
        tuya_ble_weather_data_req_response_t weather_req_response_data;
        tuya_ble_weather_data_received_data_t weather_received_data;
        tuya_ble_scene_req_response_t scene_req_response_data;
        tuya_ble_scene_data_received_data_t scene_data_received_data;
        tuya_ble_scene_ctrl_received_data_t scene_ctrl_received_data;
        tuya_ble_app_passthrough_data_t app_passthrough_data;
        tuya_ble_ext_module_active_data_t ext_module_active_data;
        tuya_ble_file_data_t file_data;
        tuya_ble_dp_query_data_with_src_type_t dp_query_data_with_src_type;
        tuya_ble_dp_data_with_src_type_received_data_t dp_data_with_src_type_received_data;
        tuya_ble_dp_data_with_src_type_send_response_data_t dp_with_src_type_send_response_data;
        tuya_ble_dp_data_with_src_type_and_time_send_response_data_t dp_with_src_type_and_time_send_response_data;
        tuya_ble_accessory_info_reading_data_t accessory_info_reading_data;
        tuya_ble_accessory_info_report_response_t accessory_info_report_response_data;
        tuya_ble_accessory_active_info_data_t accessory_active_info_data;
        tuya_ble_accessory_ota_data_t accessory_ota_data;
        tuya_ble_group_dp_data_received_t group_dp_data_received;
    };
} tuya_ble_cb_evt_param_t;

/* TIMER related */
typedef VOID_T *tuya_ble_timer_t;

typedef VOID_T(*tuya_ble_timer_handler_t)(VOID_T *);

typedef VOID_T(*tuya_ble_callback_t)(tuya_ble_cb_evt_param_t *param);

#if ENABLE_BLUETOOTH_BREDR

typedef struct {
    UINT32_T  crc;
    UINT32_T  settings_version;
    UINT8_T   h_id[H_ID_LEN];
    UINT8_T   device_id[DEVICE_ID_LEN];
    UINT8_T   mac[MAC_LEN];
    UINT8_T   auth_key[AUTH_KEY_LEN];
    UINT8_T   mac_string[MAC_LEN * 2];
    UINT8_T   pid_type;
    UINT8_T   pid_len;
    UINT8_T   factory_pid[TUYA_BLE_PRODUCT_ID_MAX_LEN];
    UINT8_T   bt_mac_len;
    UINT8_T   bt_mac[MAC_LEN];
    UINT8_T   bt_mac_string[MAC_LEN * 2];
    UINT8_T   res[91];
} tuya_ble_auth_settings_t;

#else

typedef struct {
    UINT32_T  crc;
    UINT32_T  settings_version;
    UINT8_T   h_id[H_ID_LEN];
    UINT8_T   device_id[DEVICE_ID_LEN];
    UINT8_T   mac[MAC_LEN];
    UINT8_T   auth_key[AUTH_KEY_LEN];
    UINT8_T   mac_string[MAC_LEN * 2];
    UINT8_T   pid_type;
    UINT8_T   pid_len;
    UINT8_T   factory_pid[TUYA_BLE_PRODUCT_ID_MAX_LEN];
    UINT8_T   country_code;
    UINT32_T  tx_dBm;
    UINT8_T   res[105];
} tuya_ble_auth_settings_t;

#endif

typedef struct {
    UINT32_T  crc;
    UINT32_T  settings_version;
    tuya_ble_product_id_type_t pid_type;
    UINT8_T   pid_len;
    UINT8_T   common_pid[TUYA_BLE_PRODUCT_ID_MAX_LEN];
    UINT8_T   login_key[LOGIN_KEY_LEN];
    UINT8_T   ecc_secret_key[ECC_SECRET_KEY_LEN];
    UINT8_T   device_virtual_id[DEVICE_VIRTUAL_ID_LEN];
    UINT8_T   user_rand[PAIR_RANDOM_LEN];
    UINT8_T   bound_flag;
    UINT8_T   factory_test_flag;
    UINT8_T   server_cert_pub_key[64];
    UINT8_T   beacon_key[BEACON_KEY_LEN];
    UINT8_T   login_key_v2[LOGIN_KEY_V2_LEN];
    UINT8_T   secret_key[SECRET_KEY_LEN];
    UINT8_T   protocol_v2_enable;
    UINT8_T   ota_status;
    UINT8_T   mac[MAC_LEN];
    UINT16_T  node_id;
    UINT8_T   res[5];
} tuya_ble_sys_settings_t;

typedef struct {
    tuya_ble_auth_settings_t auth_settings;
    tuya_ble_sys_settings_t  sys_settings;
    tuya_ble_product_id_type_t pid_type;
    UINT8_T pid_len;
    UINT8_T pid[TUYA_BLE_PRODUCT_ID_MAX_LEN];
    UINT8_T adv_local_name_len;
    UINT8_T adv_local_name[TUYA_BLE_ADV_LOCAL_NAME_MAX_SPACE_LEN];
} tuya_ble_parameters_settings_t;

typedef struct {
    tuya_ble_product_id_type_t pid_type;
    UINT8_T   pid_len;
    UINT8_T   pid[TUYA_BLE_PRODUCT_ID_MAX_LEN];
    UINT8_T   h_id[H_ID_LEN];
    UINT8_T   device_id[DEVICE_ID_LEN];
    UINT8_T   mac[MAC_LEN];
    UINT8_T   auth_key[AUTH_KEY_LEN];
    UINT8_T   mac_string[MAC_LEN * 2];

    UINT8_T   bt_mac_len;
    UINT8_T   bt_mac[MAC_LEN];
    UINT8_T   bt_mac_string[MAC_LEN * 2];
} tuya_ble_factory_id_data_t;

typedef struct {
    UINT8_T  *p_firmware_name;
    UINT8_T  *p_firmware_version;
    UINT32_T firmware_version;
    UINT8_T  *p_hardware_version;
    UINT32_T hardware_version;

    UINT8_T  *p_sdk_version;
    UINT8_T  *p_kernel_version;
} tal_common_info_t;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_TYPE_H__ */

