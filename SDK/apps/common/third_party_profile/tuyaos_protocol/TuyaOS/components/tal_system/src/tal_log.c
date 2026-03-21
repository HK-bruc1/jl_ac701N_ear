/**
 * @file tal_log.c
 * @brief This is tal_log file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

/* #include "system/includes.h" */
#include "system/includes.h"
/* #include <stdio.h> */
/* #include <string.h> */
/* #include <ctype.h> */
#include "tal_rtc.h"
#include "tal_utc.h"
#include "tal_system.h"
#include "tal_memory.h"
#include "tal_log.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/
#define LOG_OUTPUT_MAX      6
#define LOG_LEVEL_MAX       5
#define MODULE_NAME_MAX     8
#define INVALID_MD_INDEX    0xFF
#define DEF_OUTPUT_NAME     "ty"
#define LOG_USE_COLOR

/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/
typedef struct {
    TAL_LOG_OUTPUT_CB fn;
    TAL_LOG_LEVEL_E level;
    CHAR_T name[MODULE_NAME_MAX];
} log_t;

struct {
    TAL_LOG_LEVEL_E g_level;
    UINT32_T len;
    CHAR_T  *buff;
    log_t log[LOG_OUTPUT_MAX];
} log_mgr;

/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/
STATIC CONST PCHAR_T sLevelStr[] = { "E", "W", "N", "I", "D", "T"};
#ifdef LOG_USE_COLOR
STATIC CONST PCHAR_T level_colors[] = {
    "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




UINT8_T __find_module(CONST CHAR_T *name)
{
    UINT8_T i;

    for (i = 0; i < LOG_OUTPUT_MAX; i++) {
        if (!memcmp(name, log_mgr.log[i].name, strlen(name))) {
            return i;
        }
    }

    return INVALID_MD_INDEX;
}

UINT8_T __find_free_module(CONST CHAR_T *name)
{
    UINT8_T i;

    for (i = 0; i < LOG_OUTPUT_MAX; i++) {
        if (!log_mgr.log[i].fn) {
            return i;
        }
    }

    return INVALID_MD_INDEX;
}

OPERATE_RET __v_printf(va_list ap, UINT8_T index, LOG_LEVEL level, const char *file, INT_T line, const char *fmt, ...)
{
    INT_T cnt = 0;
    log_t *log = &log_mgr.log[index];

    if (level > log->level) {
        return OPRT_BASE_LOG_MNG_PRINT_LOG_LEVEL_HIGHER;
    }

    const CHAR_T *pfile = (strrchr(file, (INT_T)'/') ? (strrchr(file, (INT_T)'/') + 1) : file);
    pfile = (strrchr(pfile, (INT_T)'\\') ? (strrchr(pfile, (INT_T)'\\') + 1) : pfile);

    TIME_T timestamp = tal_utc_get_local_time();
    tal_utc_date_t date = {0};
    tal_utc_timestamp2date(timestamp, &date, false);

#ifdef LOG_USE_COLOR
    cnt = snprintf(log_mgr.buff, log_mgr.len, "[[%02d-%02d %02d:%02d:%02d %s %s %s\x1b[0m][%s:%d] ",
                   date.month, date.day, date.hour, date.min, date.sec, level_colors[level], log->name, sLevelStr[level], pfile, line);
    if (cnt <= 0) {
        return -1;
    }
#else
    cnt = snprintf(log_mgr.buff, log_mgr.len, "[%02d-%02d %02d:%02d:%02d %s %s][%s:%d] ",
                   date.month, date.day, date.hour, date.min, date.sec, log->name, sLevelStr[level], pfile, line);
    if (cnt <= 0) {
        return OPRT_BASE_LOG_MNG_LOG_SEQ_FILE_FULL;
    }
#endif

    INT_T len = cnt;
    cnt = vsnprintf(log_mgr.buff + len, log_mgr.len - len, fmt, ap);
    if (cnt <= 0) {
        return OPRT_BASE_LOG_MNG_LOG_SEQ_FILE_FULL;
    }
    len += cnt;
    if (len > (INT_T)(log_mgr.len - 3)) {
        len = log_mgr.len - 3;
    }
    log_mgr.buff[len]     = '\r';
    log_mgr.buff[len + 1] = '\n';
    log_mgr.buff[len + 2] = 0;

    log->fn(log_mgr.buff);

    return OPRT_OK;
}

OPERATE_RET __v_raw_printf(va_list ap, UINT8_T index, const char *fmt, ...)
{
    INT_T cnt = 0;
    log_t *log = &log_mgr.log[index];

    cnt = vsnprintf(log_mgr.buff, log_mgr.len, fmt, ap);
    if (cnt <= 0) {
        return OPRT_BASE_LOG_MNG_FORMAT_STRING_FAILED;
    }
    log->fn(log_mgr.buff);

    return OPRT_OK;
}

OPERATE_RET tal_log_add_output_term(CONST CHAR_T *name, CONST TAL_LOG_OUTPUT_CB term)
{
    UINT8_T index;

    if (term == NULL || name == NULL ||
        strlen(name) >= MODULE_NAME_MAX) {
        return OPRT_INVALID_PARM;
    }

    index = __find_module(name);
    if (index == INVALID_MD_INDEX) {
        index = __find_free_module(name);
        if (index != INVALID_MD_INDEX) {
            memcpy(log_mgr.log[index].name, name, strlen(name));
        } else {
            return OPRT_BASE_LOG_MNG_LOG_SEQ_CREATE_FAIL;
        }
    }
    log_mgr.log[index].level = TAL_LOG_LEVEL_TRACE;
    log_mgr.log[index].fn = term;

    return OPRT_OK;
}

VOID_T tal_log_del_output_term(CONST CHAR_T *name)
{
    UINT8_T index;

    if (name == NULL ||
        strlen(name) >= MODULE_NAME_MAX) {
        return;
    }
    index = __find_module(name);
    if (index == INVALID_MD_INDEX) {
        return;
    }

    log_mgr.log[index].fn = NULL;
    log_mgr.log[index].level = TAL_LOG_LEVEL_TRACE;
    memset(log_mgr.log[index].name, 0, MODULE_NAME_MAX);
}

OPERATE_RET tal_log_set_manage_attr(CONST TAL_LOG_LEVEL_E level)
{
    log_mgr.g_level = level;
    return OPRT_OK;
}

OPERATE_RET tal_log_get_log_manage_attr(TAL_LOG_LEVEL_E *level)
{
    *level = log_mgr.g_level;
    return OPRT_OK;
}

VOID_T tal_log_release_manager(VOID_T)
{
}

OPERATE_RET tal_log_set_manage_ms_info(BOOL_T if_ms_level)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tal_log_add_module_level(CONST PCHAR_T module_name, CONST TAL_LOG_LEVEL_E level)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tal_log_delete_module_level(CONST PCHAR_T module_name)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tal_log_set_module_level(CONST PCHAR_T module_name, TAL_LOG_LEVEL_E level)
{
    UINT8_T index;

    if (level > LOG_LEVEL_MAX || module_name == NULL ||
        strlen(module_name) >= MODULE_NAME_MAX) {
        return OPRT_INVALID_PARM;
    }

    index = __find_module(module_name);
    if (index == INVALID_MD_INDEX) {
        return OPRT_BASE_LOG_MNG_DONOT_FOUND_MODULE;
    }

    log_mgr.log[index].level = level;

    return OPRT_OK;
}

OPERATE_RET tal_log_get_module_level(CONST PCHAR_T module_name, TAL_LOG_LEVEL_E *level)
{
    UINT8_T index;

    if (level == NULL || module_name == NULL ||
        strlen(module_name) >= MODULE_NAME_MAX) {
        return OPRT_INVALID_PARM;
    }

    index = __find_module(module_name);
    if (index == INVALID_MD_INDEX) {
        return OPRT_BASE_LOG_MNG_DONOT_FOUND_MODULE;
    }

    *level = log_mgr.log[index].level;

    return OPRT_OK;
}

OPERATE_RET tal_log_print_raw(CONST PCHAR_T pFmt, ...)
{
    UINT8_T i;
    OPERATE_RET ret = 0;

//    TAL_ENTER_CRITICAL();
    va_list ap;
    va_start(ap, pFmt);
    for (i = 0; i < LOG_OUTPUT_MAX && log_mgr.log[i].fn; i++) {
        ret = __v_raw_printf(ap, i, pFmt);
        if (ret != 0) {
            break;
        }
    }
    va_end(ap);
//    TAL_EXIT_CRITICAL();

    return OPRT_OK;
}

VOID_T tal_log_hex_dump(CONST TAL_LOG_LEVEL_E level, CONST CHAR_T *file, CONST INT_T line, CONST CHAR_T *title, UINT8_T width, UINT8_T *buf, UINT16_T size)
{

#if 1
    printf("tuya log:%s", title);
    put_buf(buf, size);
    return;

#else
    INT_T i = 0;
#if (DISABLE_LOG_CHAR_DISPLAY == 0)
    INT_T j = 0;
    INT_T num = 0;
    INT_T display_width = 0;
#endif

    if (width > 16) {
        width = 16;
    }

    tal_log_print(level, file, line, "%s %d:", title, size);

    for (i = 0; i < size; i++) {
        tal_log_print_raw("%02x ", buf[i] & 0xFF);

        if (((i + 1) % width == 0) || ((i + 1) == size)) {
#if (DISABLE_LOG_CHAR_DISPLAY == 0)
            tal_log_print_raw("        ");

            display_width = width;
            if (((i + 1) == size)) {
                display_width = size - width * num;
                for (j = 0; j < (width - display_width) * 3; j++) {
                    tal_log_print_raw(" ");
                }
            }

            for (j = 0; j < display_width; j++) {
                UINT8_T ch = buf[width * num + j] & 0xFF;
                if ((ch < 128) && isprint(ch)) {
                    tal_log_print_raw("%c", ch);
                } else {
                    tal_log_print_raw(".");
                }
            }

            num++;
#endif

            tal_log_print_raw("\r\n");
        }
    }

    tal_log_print_raw("\r\n");
#endif
}

OPERATE_RET tal_log_module_print(CONST CHAR_T *name, CONST TAL_LOG_LEVEL_E level, CONST CHAR_T *file, CONST INT_T line, CONST CHAR_T *fmt, ...)
{
    UINT8_T index;
    OPERATE_RET ret = 0;

    if (level > log_mgr.g_level) {
        return OPRT_INVALID_PARM;
    }

//    TAL_ENTER_CRITICAL();
    index = __find_module(name);
    if (index == INVALID_MD_INDEX) {
        ret = OPRT_BASE_LOG_MNG_DONOT_FOUND_MODULE;
        goto ext;
    }
    va_list ap;
    va_start(ap, fmt);
    ret = __v_printf(ap, index, level, file, line, fmt);
    va_end(ap);
ext:
//    TAL_EXIT_CRITICAL();

    return ret;
}

OPERATE_RET tal_log_print(CONST TAL_LOG_LEVEL_E level, CONST CHAR_T *file, CONST INT_T line, CONST CHAR_T *fmt, ...)
{
    UINT8_T i;
    OPERATE_RET ret = 0;

    if (level > log_mgr.g_level) {
        return OPRT_INVALID_PARM;
    }

#if 1
    int len = 0;

    const CHAR_T *pfile = (strrchr(file, (INT_T)'/') ? (strrchr(file, (INT_T)'/') + 1) : file);
    pfile = (strrchr(pfile, (INT_T)'\\') ? (strrchr(pfile, (INT_T)'\\') + 1) : pfile);

    printf("tuya log:[%s:%d]", pfile, line);

    va_list ap;
    va_start(ap, fmt);
    len = print(NULL, NULL, fmt, ap);
    va_end(ap);
    return 0;
#else

//    TAL_ENTER_CRITICAL();
    va_list ap;
    va_start(ap, fmt);

    for (i = 0; i < LOG_OUTPUT_MAX && log_mgr.log[i].fn; i++) {
        ret = __v_printf(ap, i, level, file, line, fmt);
        if (ret != 0) {
            break;
        }
    }
    va_end(ap);
//    TAL_EXIT_CRITICAL();

    return ret;
#endif
}

OPERATE_RET tal_log_create_manage_and_init(CONST TAL_LOG_LEVEL_E level, CONST INT_T buf_len, CONST TAL_LOG_OUTPUT_CB output)
{
    OPERATE_RET ret;

    if (level > LOG_LEVEL_MAX || output == NULL || buf_len == 0) {
        return OPRT_INVALID_PARM;
    }
    log_mgr.buff = (CHAR_T *)tal_malloc(buf_len);
    if (log_mgr.buff == NULL) {
        return OPRT_MALLOC_FAILED;
    }

    ret = tal_log_add_output_term(DEF_OUTPUT_NAME, output);
    if (ret != OPRT_OK) {
        return ret;
    }
    log_mgr.len = buf_len;
    log_mgr.g_level = level;

    return OPRT_OK;
}

