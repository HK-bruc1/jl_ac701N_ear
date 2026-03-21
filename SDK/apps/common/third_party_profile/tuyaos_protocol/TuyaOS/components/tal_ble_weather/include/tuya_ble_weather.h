/**
 * @file tuya_ble_weather.h
 * @brief This is tuya_ble_weather file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_BLE_WEATHER_H__
#define __TUYA_BLE_WEATHER_H__

#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
/**@brief  Total number of weather parameters currently supported. */
#define SUPPORT_WEATHER_KEY_TYPE_MAX_NUMS    ( 25 )

/**@brief    Weather key type. */
typedef enum {
    WKT_TEMP        = (1 << 0),     /**< temperature. */
    WKT_THIHG       = (1 << 1),     /**< high temperature. */
    WKT_TLOW        = (1 << 2),     /**< low temperature. */
    WKT_HUMIDITY    = (1 << 3),     /**< humidity. */
    WKT_CONDITION   = (1 << 4),     /**< weather condition. */
    WKT_PRESSURE    = (1 << 5),     /**< pressure. */
    WKT_REALFEEL    = (1 << 6),     /**< sendible temperature. */
    WKT_UVI         = (1 << 7),     /**< uvi. */
    WKT_SUNRISE     = (1 << 8),     /**< sunrise. */
    WKT_SUNSET      = (1 << 9),     /**< sunset. */
    WKT_UNIX        = (1 << 10),    /**< unix time, Use with sunrise and sunset. */
    WKT_LOCAL       = (1 << 11),    /**< local time, Use with sunrise and sunset. */
    WKT_WINDSPEED   = (1 << 12),    /**< wind speed. */
    WKT_WINDDIR     = (1 << 13),    /**< wind direction. */
    WKT_WINDLEVEL   = (1 << 14),    /**< wind speed scale/level. */
    WKT_AQI         = (1 << 15),    /**< aqi. */
    WKT_TIPS        = (1 << 16),    /**< tips. */
    WKT_RANK        = (1 << 17),    /**< Detailed AQI status and national ranking. */
    WKT_PM10        = (1 << 18),    /**< pm10. */
    WKT_PM25        = (1 << 19),    /**< pm2.5. */
    WKT_O3          = (1 << 20),    /**< o3. */
    WKT_NO2         = (1 << 21),    /**< no2. */
    WKT_CO          = (1 << 22),    /**< co. */
    WKT_SO2         = (1 << 23),    /**< so2. */
    WKT_CONDITIONNUM = (1 << 24),   /**< weather condition mapping id. */

    WKT_COMBINE_BITMAP_MAXVAL = (1 << SUPPORT_WEATHER_KEY_TYPE_MAX_NUMS),
} tuya_ble_weather_key_type_t;

/**@brief    Weather value type. */
typedef enum {
    WVT_INTEGER = 0,
    WVT_STRING,
} tuya_ble_weather_value_type_t;

/**@brief    Weather location type. */
typedef enum {
    WLT_PAIR_NETWORK_LOCATION = 1,  /**< Pair network location. */
    WLT_APP_CURRENT_LOCATION,       /**< Mobile phone current location. */
    WLT_CUSTOM_LOCATION,            /**< The current sdk version unsupport. */

    WLT_MAX,
} tuya_ble_weather_location_type_t;

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
/**@brief   Weather data object structure. */
#pragma pack(1)
typedef struct {
    UINT8_T                         n_day;      /**< which day. */
    tuya_ble_weather_key_type_t     key_type;   /**< weather key type. */
    tuya_ble_weather_value_type_t   val_type;   /**< weather value type. */
    UINT8_T                         value_len;  /**< weather value length. */
    CHAR_T                          vaule[];    /**< weather values. */
} tuya_ble_wd_object_t;
#pragma pack()

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/

/**
 * @brief Function for request weather data. Default request current location of mobile phone.
 *
 * @note Example1: request temperatur & humidity and two days, called: tuya_ble_feature_weather_data_request((WKT_TEMP | WKT_HUMIDITY), 2);
 *       Example2: request sunrise data&time and ont day, called: tuya_ble_feature_weather_data_request((WKT_SUNRISE | WKT_LOCAL), 1);
 *       Example3: request highest temperature and seven days, called: tuya_ble_feature_weather_data_request((WKT_THIHG), 7);
 *       In addition, request sunrise/sunset must be matched with WKT_UNIX/WKT_LOCAL, otherwise the received data is unix time
 *
 * @param[in] combine_type: Request combine types. For details, see the enum of tuya_ble_weather_key_type_t
 * @param[in] n_days: Request forecast days, rang from [1-7]; 1-means only today, 2-means today+tomorrow ...
 *
 * @return TUYA_BLE_SUCCESS on success. Others on error, please refer to tuya_ble_status_t
 */
tuya_ble_status_t tuya_ble_feature_weather_data_request(UINT32_T combine_type, UINT8_T n_days);

/**
 * @brief Function for request weather data with appoint location.
 *
 * @param[in] location_type: Request location types. For details, see the enum of tuya_ble_weather_location_type_t.
 * @param[in] combine_type: Request combine types. For details, see the enum of tuya_ble_weather_key_type_t
 * @param[in] n_days: Request forecast days, rang from [1-7]; 1-means only today, 2-means today+tomorrow ...
 *
 * @return TUYA_BLE_SUCCESS on success. Others on error, please refer to tuya_ble_status_t
 */
tuya_ble_status_t tuya_ble_feature_weather_data_request_with_location(tuya_ble_weather_location_type_t location_type, UINT32_T combine_type, UINT8_T n_days);

/**
 * @brief Function for convert weather enum type to string
 *
 * @note For example input convert type=WKT_TEMP, output key "w.temp"
 *
 * @param[in] type: Convert type
 * @param[out] key: The ponit of weather key string
 *
 * @return TUYA_BLE_SUCCESS on success. Others on error, please refer to tuya_ble_status_t
 */
tuya_ble_status_t tuya_ble_feature_weather_key_enum_type_to_string(tuya_ble_weather_key_type_t type, CHAR_T *key);

/**
 * @brief Function for handler weather data request event
 *
 * @note Internal use of tuya ble sdk
 *
 * @param[in] evt: For details, see the struct of tuya_ble_evt_param_t
 *
 */
VOID_T tuya_ble_handle_weather_data_request_evt(tuya_ble_evt_param_t *evt);

/**
 * @brief Function for handler weather data request response/ack
 *
 * @note Internal use of tuya ble sdk
 *
 * @param[in] recv_data: The point of recvived response data
 * @param[in] recv_len: The numbers of data
 *
 */
VOID_T tuya_ble_handle_weather_data_request_response(UINT8_T *recv_data, UINT16_T recv_len);

/**
 * @brief Function for handler weather data received
 *
 * @note Internal use of tuya ble sdk
 *
 * @param[in] recv_data: The point of recvived weather data
 * @param[in] recv_len: The numbers of data
 *
 */
VOID_T tuya_ble_handle_weather_data_received(UINT8_T *recv_data, UINT16_T recv_len);


#ifdef __cplusplus
}
#endif

#endif /* __TUYA_BLE_WEATHER_H__ */

