/**
 * @file tal_util.c
 * @brief This is tal_util file
 * @version 1.0
 * @date 2021-09-10
 *
 * @copyright Copyright 2021-2023 Tuya Inc. All Rights Reserved.
 *
 */

#include "string.h"
#include "tal_util.h"

/***********************************************************************
 ********************* constant ( macro and enum ) *********************
 **********************************************************************/


/***********************************************************************
 ********************* struct ******************************************
 **********************************************************************/


/***********************************************************************
 ********************* variable ****************************************
 **********************************************************************/


/***********************************************************************
 ********************* function ****************************************
 **********************************************************************/




UINT8_T tal_util_check_sum8(UINT8_T *buf, UINT32_T size)
{
    UINT8_T sum = 0;
    for (UINT32_T idx = 0; idx < size; idx++) {
        sum += buf[idx];
    }
    return sum;
}

UINT16_T tal_util_check_sum16(UINT8_T *buf, UINT32_T size)
{
    UINT16_T sum = 0;
    for (UINT32_T idx = 0; idx < size; idx++) {
        sum += buf[idx];
    }
    return sum;
}

UINT8_T tal_util_crc8(UINT8_T *buf, UINT32_T size)
{
    const UINT8_T *data = buf;
    UINT32_T crc = 0;
    int i, j;
    for (j = size; j; j--, data++) {
        crc ^= (*data << 8);
        for (i = 8; i; i--) {
            if (crc & 0x8000) {
                crc ^= (0x1070 << 3);
            }
            crc <<= 1;
        }
    }
    return (UINT8_T)(crc >> 8);
}

UINT16_T tal_util_crc16(UINT8_T *buf, UINT32_T size, UINT16_T *p_crc)
{
    UINT16_T poly[2] = {0, 0xa001}; //0x8005 ---- 0xa001
    UINT16_T crc;
    INT_T i, j;

    crc = (p_crc == NULL) ? 0xFFFF : *p_crc;

    for (j = size; j > 0; j--) {
        UINT8_T ds = *buf++;
        for (i = 0; i < 8; i++) {
            crc = (crc >> 1) ^ poly[(crc ^ ds) & 1];
            ds = ds >> 1;
        }
    }
    return crc;
}

UINT32_T tal_util_crc32(UINT8_T *buf, UINT32_T size, UINT32_T *p_crc)
{
    UINT32_T crc = (p_crc == NULL) ? 0xFFFFFFFF : ~(*p_crc);
    for (UINT32_T i = 0; i < size; i++) {
        crc = crc ^ buf[i];
        for (UINT32_T j = 8; j > 0; j--) {
            crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
        }
    }
    return ~crc;
}

UINT32_T tal_util_xor(UINT8_T *buf1, UINT8_T *buf2, UINT32_T size, UINT8_T *out_buf)
{
    for (UINT32_T i = 0; i < size; i++) {
        out_buf[i] = buf1[i] ^ buf2[i];
    }
    return 0;
}

UINT32_T tal_util_intarray2int(UINT8_T *intArray, UINT32_T startIdx, UINT32_T size)
{
    if (startIdx >= size) {
        return (UINT32_T) - 1;
    }

    UINT32_T num = 0;
    for (UINT32_T idx = startIdx; idx < startIdx + size; idx++) {
        num = (num * 10) + intArray[idx];
    }
    return num;
}

UINT32_T tal_util_int2intarray(UINT32_T num, UINT8_T *intArray, UINT32_T size)
{
    UINT32_T idx = 0;
    UINT32_T tmp = 0;

    tmp = num;
    do {
        tmp /= 10;
        idx++;
    } while (tmp != 0);

    if (size < idx) {
        return 0;
    }

    tmp = num;
    for (idx = 0; tmp != 0; idx++) {
        intArray[idx] = tmp % 10;
        tmp /= 10;
    }

    tal_util_reverse_byte(intArray, idx);

    return idx;
}

VOID_T tal_util_device_id_20_to_16(UINT8_T *in, UINT8_T *out)
{
    UINT8_T i, j;
    UINT8_T temp[4];

    for (i = 0; i < 5; i++) {
        for (j = i * 4; j < (i * 4 + 4); j++) {
            if ((in[j] >= 0x30) && (in[j] <= 0x39)) {
                temp[j - i * 4] = in[j] - 0x30;
            } else if ((in[j] >= 0x41) && (in[j] <= 0x5A)) {
                temp[j - i * 4] = in[j] - 0x41 + 36;
            } else if ((in[j] >= 0x61) && (in[j] <= 0x7A)) {
                temp[j - i * 4] = in[j] - 0x61 + 10;
            } else {
            }
        }

        out[i * 3] = temp[0] & 0x3F;
        out[i * 3] <<= 2;
        out[i * 3] |= ((temp[1] >> 4) & 0x03);

        out[i * 3 + 1] = temp[1] & 0x0F;
        out[i * 3 + 1] <<= 4;
        out[i * 3 + 1] |= ((temp[2] >> 2) & 0x0F);

        out[i * 3 + 2] = temp[2] & 0x03;
        out[i * 3 + 2] <<= 6;
        out[i * 3 + 2] |= temp[3] & 0x3F;
    }

    out[15] = 0xFF;
}

VOID_T tal_util_device_id_16_to_20(UINT8_T *in, UINT8_T *out)
{
    UINT8_T i, j;
    UINT8_T temp[4];

    for (i = 0; i < 5; i++) {
        j = i * 3;
        temp[j - i * 3] = (in[j] >> 2) & 0x3F;
        temp[j - i * 3 + 1] = in[j] & 0x03;
        temp[j - i * 3 + 1] <<= 4;
        temp[j - i * 3 + 1] |= (in[j + 1] >> 4) & 0x0F;
        temp[j - i * 3 + 2] = (in[j + 1] & 0x0F) << 2;
        temp[j - i * 3 + 2] |= ((in[j + 2] & 0xC0) >> 6) & 0x03;
        temp[j - i * 3 + 3] = in[j + 2] & 0x3F;

        for (j = i * 4; j < (i * 4 + 4); j++) {
            if (temp[j - i * 4] <= 9) {
                out[j] = temp[j - i * 4] + 0x30;
            } else if ((temp[j - i * 4] >= 10) && (temp[j - i * 4] <= 35)) {
                out[j] = temp[j - i * 4] + 87;
            } else if ((temp[j - i * 4] >= 36) && (temp[j - i * 4] <= 61)) {
                out[j] = temp[j - i * 4] + 29;
            } else {
            }
        }
    }
}

UINT32_T tal_util_reverse_byte(void *buf, UINT32_T size)
{
    UINT8_T *p_tmp = buf;
    UINT8_T  tmp;
    for (UINT32_T idx = 0; idx < size / 2; idx++) {
        tmp = *(p_tmp + idx);
        *(p_tmp + idx) = *(p_tmp + size - 1 - idx);
        *(p_tmp + size - 1 - idx) = tmp;
    }
    return 0;
}

UINT32_T tal_util_count_one_in_num(UINT32_T num)
{
    num = (num & 0x55555555) + ((num >> 1) & 0x55555555);
    num = (num & 0x33333333) + ((num >> 2) & 0x33333333);
    num = (num & 0x0f0f0f0f) + ((num >> 4) & 0x0f0f0f0f);
    num = (num & 0x00ff00ff) + ((num >> 8) & 0x00ff00ff);
    num = (num & 0x0000ffff) + ((num >> 16) & 0x0000ffff);

    return num;
}

BOOL_T tal_util_buffer_value_is_all_x(CONST UINT8_T *buf, UINT32_T size, UINT8_T x_value)
{
    for (UINT32_T idx = 0; idx < size; idx++) {
        if (buf[idx] != x_value) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL_T tal_util_is_word_aligned(VOID_T CONST *p)
{
    return (((UINT32_T)p & 0x03) == 0);
}

INT32_T tal_util_search_symbol_index(UINT8_T *buf, UINT32_T size, UINT8_T symbol, UINT8_T index[])
{
    INT32_T i;
    UINT8_T index_buf[64] = {0};
    UINT8_T symbol_cnt = 0;

    if (buf == NULL || size == 0 || index == NULL) {
        return symbol_cnt;
    }

    for (i = 0; i < size; i++) {
        if (buf[i] == symbol) {
            index_buf[symbol_cnt] = i;
            symbol_cnt += 1;

            if (symbol_cnt >= sizeof(index_buf)) {
                /* error, too many symbols */
                break;
            }
        }
    }

    if (symbol_cnt != 0) {
        memcpy(index, index_buf, symbol_cnt);
    }

    return symbol_cnt;
}

static UINT8_T base64_decode(const UINT8_T *input, UINT16_T inlen, UINT8_T *output, UINT16_T *outlen)
{
    const char *base64_tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    UINT8_T reverse_tbl[256] = {0};
    UINT32_T off = 0;
    UINT32_T i = 0;

    if (NULL == input) {
        return 1;
    }

    if (inlen == 0) {
        inlen = strlen((char *)input);
    }

    if (inlen == 0 || (inlen % 4 != 0)) {
        return 1;
    }

    for (i = 0; i < 64; i++) {
        reverse_tbl[base64_tbl[i]] = i;
    }

    for (i = 0; i < inlen - 4; i += 4) {
        output[off++] = (reverse_tbl[input[i]] << 2) | ((reverse_tbl[input[i + 1]] >> 4) & 0xFF);
        output[off++] = (reverse_tbl[input[i + 1]] << 4) | ((reverse_tbl[input[i + 2]] >> 2) & 0xFF);
        output[off++] = (reverse_tbl[input[i + 2]] << 6) | ((reverse_tbl[input[i + 3]]) & 0xFF);
    }

    if (input[i + 2] == '=') {
        output[off++] = (reverse_tbl[input[i]] << 2) | ((reverse_tbl[input[i + 1]] >> 4) & 0xFF);
    } else if (input[i + 3] == '=') {
        output[off++] = (reverse_tbl[input[i]] << 2) | ((reverse_tbl[input[i + 1]] >> 4) & 0xFF);
        output[off++] = (reverse_tbl[input[i + 1]] << 4) | ((reverse_tbl[input[i + 2]] >> 2) & 0xFF);
    } else {
        output[off++] = (reverse_tbl[input[i]] << 2) | ((reverse_tbl[input[i + 1]] >> 4) & 0xFF);
        output[off++] = (reverse_tbl[input[i + 1]] << 4) | ((reverse_tbl[input[i + 2]] >> 2) & 0xFF);
        output[off++] = (reverse_tbl[input[i + 2]] << 6) | ((reverse_tbl[input[i + 3]]) & 0xFF);
    }

    if (NULL != outlen) {
        *outlen = off;
    }

    return 0;
}

static INT32_T delchar(UINT8_T *s, UINT16_T len, UINT8_T match_char)
{
    INT32_T i, j;
    INT32_T counter = 0;

    for (i = 0; i < len; i++) {
        if (s[i] == match_char) {
            counter++;

            for (j = i; j < len; j++) {
                s[j] = s[j + 1];
                i--;
            }
        }
    }
    return counter;
}

INT32_T tal_util_ecc_key_pem2hex(const UINT8_T *pem, UINT8_T *key, UINT16_T *key_len)
{
    UINT8_T buf1[256] = {0};
    UINT8_T buf2[256] = {0};
    UINT16_T inlen = 0;
    UINT16_T i = 0;
    UINT16_T len, len1 = 0, len2 = 0;

    if (NULL == pem) {
        return 0;
    }

    inlen = strlen((void *)pem);

    if (inlen > 256) {
        return 0;
    }

    //head
    if ((pem[0] != '-') || (pem[1] != '-') || (pem[2] != '-') || (pem[3] != '-') || (pem[4] != '-')) {
        return 0;
    }

    //tail
    if ((pem[inlen - 1] != '-') || (pem[inlen - 2] != '-') || (pem[inlen - 3] != '-') || (pem[inlen - 4] != '-') || (pem[inlen - 5] != '-')) {
        return 0;
    }

    //find head end
    for (i = 5; i < inlen - 5; i++) {
        if (pem[i] == '-') {
            if ((pem[i + 1] != '-') || (pem[i + 2] != '-') || (pem[i + 3] != '-') || (pem[i + 4] != '-')) {
                return 0;
            }

            len1 = i + 5;
            break;
        }
    }

    //remove head
    memcpy(buf1, pem + len1, inlen - len1);

    //find tail
    for (i = 0; i < inlen - len1; i++) {
        if (buf1[i] == '-') {
            if ((buf1[i + 1] != '-') || (buf1[i + 2] != '-') || (buf1[i + 3] != '-') || (buf1[i + 4] != '-')) {
                return 0;
            }

            len2 = i;
            break;
        }
    }
    len = len2;

    //remove \n
    len1 = delchar(buf1, len, '\n');
    len = len - len1;
    //remove \r
    len1 = delchar(buf1, len, '\r');
    len = len - len1;

    //decode
    base64_decode((UINT8_T *)buf1, len, (UINT8_T *)buf2, (UINT16_T *)&len2);

    //next is asn.1 decode
    if (buf2[0] != 0x30) { //0x30
        return 0;
    }

    len1 = buf2[1]; //0x30 0x41
    if ((len1 + 2) != len2) {
        return 0;
    }

    if (buf2[2] != 0x02) { //0x30 0x41 0x20 0x01 0x00
        return 0;
    }

    if (buf2[5] != 0x30) {
        return 0;
    }

    len1 = buf2[6];

    if (buf2[7 + len1] != 0x04) { //0x04
        return 0;
    }

//    len2 = buf2[8+len1]; //0x04 0x27

    if (buf2[9 + len1] != 0x30) { //0x30
        return 0;
    }

//    len = buf2[10+len1]; //0x25

    if (buf2[11 + len1] != 0x02) { //0x02
        return 0;
    }

    if (buf2[14 + len1] != 0x04) { //0x04
        return 0;
    }

    *key_len = buf2[15 + len1];
    memcpy(key, &buf2[16 + len1], *key_len);

    return 1;
}

INT32_T tal_util_ecc_sign_secp256r1_extract_raw_from_der(const UINT8_T *der, UINT8_T *raw_rs)
{
    /* extract r + s from der */
    int pos = 0;
    UINT8_T raw[64] = {0};

    if (der == NULL || raw_rs == NULL) {
        return 0;
    }

    if (der[3] != 0x20) {
        memcpy(raw, &der[5], 32);
        pos = 5 + 32;
    } else {
        memcpy(raw, &der[4], 32);
        pos = 4 + 32;
    }

    // 37
    if (der[pos + 1] != 0x20) {
        memcpy(&raw[32], &der[pos + 3], 32);
    } else {
        memcpy(&raw[32], &der[pos + 2], 32);
    }

    memcpy(raw_rs, raw, SIZEOF(raw));
    return 1;
}

UINT32_T tal_util_shell_sort(INT_T *buf, INT_T size)
{
    INT_T i;
    INT_T j;
    INT_T temp;
    INT_T gap;  //Step size
    for (gap = size / 2; gap >= 1; gap /= 2) {
        for (i = 0 + gap; i < size; i += gap) {
            temp = buf[i];
            j = i - gap;
            while (j >= 0 && buf[j] > temp) {
                buf[j + gap] = buf[j];
                j -= gap;
            }
            buf[j + gap] = temp;
        }
    }
    return 0;
}

UINT8_T tal_util_str_hexchar2int(UINT8_T hexChar)
{
    switch (hexChar) {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
    case 'A':
        return 10;
    case 'b':
    case 'B':
        return 11;
    case 'c':
    case 'C':
        return 12;
    case 'd':
    case 'D':
        return 13;
    case 'e':
    case 'E':
        return 14;
    case 'f':
    case 'F':
        return 15;
    default:
        return (UINT8_T) - 1;
    }
}

UINT8_T tal_util_str_int2hexchar(BOOL_T isHEX, UINT8_T intNum)
{
    switch (intNum) {
    case 0:
        return '0';
    case 1:
        return '1';
    case 2:
        return '2';
    case 3:
        return '3';
    case 4:
        return '4';
    case 5:
        return '5';
    case 6:
        return '6';
    case 7:
        return '7';
    case 8:
        return '8';
    case 9:
        return '9';
    case 10:
        return (isHEX ? 'A' : 'a');
    case 11:
        return (isHEX ? 'B' : 'b');
    case 12:
        return (isHEX ? 'C' : 'c');
    case 13:
        return (isHEX ? 'D' : 'd');
    case 14:
        return (isHEX ? 'E' : 'e');
    case 15:
        return (isHEX ? 'F' : 'f');
    default:
        return (UINT8_T) - 1;
    }
}

UINT32_T tal_util_str_hexstr2int(UINT8_T *hexStr, UINT32_T size, UINT32_T *num)
{
    *num = 0;
    for (UINT32_T idx = 0; idx < size; idx++) {
        UINT8_T tmp = tal_util_str_hexchar2int(hexStr[idx]);
        if (tmp == (UINT8_T) - 1) {
            return 1;
        }

        (*num) = (*num) << 4;
        (*num) += tmp;
    }
    return 0;
}

UINT32_T tal_util_str_int2hexstr(BOOL_T isHEX, UINT32_T num, UINT8_T *hexStr, UINT32_T size)
{
    UINT32_T idx = 0;
    hexStr[idx++] = tal_util_str_int2hexchar(isHEX, num % 16);
    num = num / 16;

    while (num >= 16) {
        hexStr[idx++] = tal_util_str_int2hexchar(isHEX, num % 16);
        num = num / 16;
    }

    hexStr[idx++] = tal_util_str_int2hexchar(isHEX, num);

    if (idx < size) {
        memset(&hexStr[idx], '0', size - idx);
    }

    tal_util_reverse_byte(hexStr, size);

    return idx;
}

UINT32_T tal_util_str_intstr2int(UINT8_T *intStr, UINT32_T size, UINT32_T *num)
{
    *num = 0;
    for (UINT32_T idx = 0; idx < size; idx++) {
        UINT8_T tmp = tal_util_str_hexchar2int(intStr[idx]);
        if (tmp == (UINT8_T) - 1 || tmp > 9) {
            return 1;
        }

        (*num) = (*num) * 10;
        (*num) += tmp;
    }
    return 0;
}

UINT32_T tal_util_str_intstr2int_with_negative(CHAR_T *intStr, UINT32_T size, INT32_T *num)
{
    INT32_T cal_num = 0;
    BOOL_T is_negative = FALSE;

    if (intStr == NULL || size == 0 || num == NULL) {
        return 1;
    }

    for (UINT32_T idx = 0; idx < size; idx++) {
        UINT8_T tmp = tal_util_str_hexchar2int(intStr[idx]);

        if ((idx == 0) && (intStr[idx] == 0x2D)) {
            is_negative = TRUE;
            continue;
        } else if (tmp == (UINT8_T) - 1 || tmp > 9) {
            return 1;
        }

        cal_num *= 10;
        cal_num += tmp;
    }

    if (is_negative) {
        cal_num = (-1 * cal_num);
    }

    *num = cal_num;
    return 0;
}

UINT32_T tal_util_str_int2intstr(UINT32_T num, UINT8_T *intStr, UINT32_T size)
{
    UINT32_T idx = 0;
    UINT32_T tmp = 0;

    tmp = num;
    do {
        tmp /= 10;
        idx++;
    } while (tmp != 0);

    if (size < idx) {
        return 0;
    }

    tmp = num;
    for (idx = 0; tmp != 0; idx++) {
        intStr[idx] = tal_util_str_int2hexchar(true, tmp % 10);
        tmp /= 10;
    }

    tal_util_reverse_byte(intStr, idx);

    return idx;
}

UINT32_T tal_util_str_hexstr2hexarray(UINT8_T *hexStr, UINT32_T size, UINT8_T *hexArray)
{
    UINT8_T hex_num = 0;
    for (UINT32_T idx = 0; idx < size; idx++) {
        UINT8_T tmp = tal_util_str_hexchar2int(hexStr[idx]);
        if (tmp == (UINT8_T) - 1) {
            return 1;
        }

        hex_num <<= 4;
        hex_num |= tmp;

        if ((idx & 1) == 1) {
            hexArray[idx >> 1] = hex_num;
            hex_num = 0;
        }
    }
    return 0;
}

UINT32_T tal_util_str_hexarray2hexstr(BOOL_T isHEX, UINT8_T *hexArray, UINT32_T size, UINT8_T *hexStr)
{
    UINT32_T idx;
    for (idx = 0; idx < size; idx++) {
        UINT8_T high = hexArray[idx] >> 4;
        UINT8_T low  = hexArray[idx] & 0x0F;

        hexStr[idx * 2] = tal_util_str_int2hexchar(isHEX, high);
        hexStr[idx * 2 + 1] = tal_util_str_int2hexchar(isHEX, low);
    }
//    hexstr[idx*2] = '0'; //To prevent the length of the hexStr array from being insufficient, it can be added if needed
    return 0;
}

STATIC INLINE INT32_T find_char(UINT8_T *buf, UINT16_T len, UINT8_T data_byte)
{
    for (UINT16_T i = 0; i < len; i++) {
        if (buf[i] == data_byte) {
            return i;
        }
    }
    return -1;
}

UINT8_T tal_util_get_value_by_key(UINT8_T *input_buf, UINT16_T input_len, UINT8_T *key, UINT16_T key_len, UINT8_T *value_data, UINT16_T *value_len)
{
    if ((input_len == 0) || (key_len == 0) || (input_len < key_len) || (NULL == value_data)) {
        return 2;
    }

    UINT16_T i = 0;
    UINT8_T temp[2];
    temp[0] = input_buf[0];
    temp[1] = input_buf[input_len - 1];
    input_buf[0] = ','; //easy for search: ,xx:xx,xx:xx,
    input_buf[input_len - 1] = ',';

    while (i < input_len) {
        UINT16_T base = i;
        INT16_T index1 = find_char(&input_buf[base], input_len, ',');
        if (index1 == -1) {
            return 0;
        }
        INT16_T index2 = find_char(&input_buf[base + index1], input_len - index1, ':');
        if (index2 == -1) {
            return 0;
        }
        INT16_T index3 = find_char(&input_buf[base + index1 + index2], input_len - index1 - index2, ',');
        if (index3 == -1) {
            return 0;
        }

        INT16_T find_key_pos = base + index1 + 1;
        INT16_T find_key_len = index2 - 1;
        INT16_T find_value_pos = base + index1 + index2 + 1;
        INT16_T find_value_len = index3 - 1;

        if (find_key_len >= 2 &&//delete "\"...\""
            input_buf[find_key_pos] == '"' &&
            input_buf[find_key_pos + find_key_len - 1] == '"') {
            find_key_pos++;
            find_key_len -= 2;
        }
        if (find_value_len >= 2 &&//delete
            input_buf[find_value_pos] == '"' &&
            input_buf[find_value_pos + find_value_len - 1] == '"') {
            find_value_pos++;
            find_value_len -= 2;
        }

        if (find_key_len != key_len) {
            i = base + index3 + index2 + index1;
            continue;
        }

        UINT8_T find = 1;
        for (UINT16_T j = 0; j < find_key_len; j++) {
            if (input_buf[find_key_pos + j] != key[j]) {
                find = 0;
                i = base + index3 + index2 + index1;
                break;
            }
        }
        if (find) {
            for (UINT16_T k = 0; k < find_value_len; k++) {
                value_data[k] = input_buf[find_value_pos + k];
            }
            input_buf[0] = temp[0];
            input_buf[input_len - 1] = temp[1];
            *value_len = find_value_len;
            return 1;
        }
    }
    input_buf[0] = temp[0];
    input_buf[input_len - 1] = temp[1];
    return 0;
}

UINT8_T tal_util_get_value_by_key_to_int(UINT8_T *input_buf, UINT16_T input_len, UINT8_T *key, UINT8_T key_len, UINT32_T *result)
{
    UINT8_T value_data[20];
    UINT16_T value_len = 0;
    UINT8_T ret, ok = 0;

    ret = tal_util_get_value_by_key(input_buf, input_len, key, key_len, value_data, &value_len);
    if (ret == 1) {
        if (0 == tal_util_str_intstr2int(value_data, value_len, result)) {
            ok = 1;
        }
    }
    return ok;
}

UINT8_T tal_util_get_value_by_key_to_hex(UINT8_T *input_buf, UINT16_T input_len, UINT8_T *key, UINT8_T key_len, UINT32_T *result)
{
    UINT8_T value_data[20];
    UINT16_T value_len = 0;
    UINT8_T ret, ok = 0;

    ret = tal_util_get_value_by_key(input_buf, input_len, key, key_len, value_data, &value_len);
    if (ret == 1) {
        if (0 == tal_util_str_hexstr2int(value_data, value_len, result)) {
            ok = 1;
        }
    }
    return ok;
}

UINT8_T tal_util_get_value_by_key_to_bool(UINT8_T *input_buf, UINT16_T input_len, UINT8_T *key, UINT8_T key_len, UINT32_T *result)
{
    UINT8_T value_data[20];
    UINT16_T value_len = 0;
    UINT8_T ret, ok = 0;

    ret = tal_util_get_value_by_key(input_buf, input_len, key, key_len, value_data, &value_len);
    if (ret == 1) {
        if (0 == memcmp(value_data, "false", 5)) {
            *result = 0;
            ok = 1;
        } else if (0 == memcmp(value_data, "true", 4)) {
            *result = 1;
            ok = 1;
        }
    }

    return ok;
}

OPERATE_RET tal_util_adv_report_parse(UINT8_T type, UINT8_T *input_buf, UINT16_T input_len, UINT8_T **output_buf, UINT8_T *output_len)
{
    UINT8_T index = 0;
    UINT8_T *p_data;
    p_data = input_buf;
    while (index < input_len) {
        UINT8_T field_length = p_data[index];
        UINT8_T field_type   = p_data[index + 1];

        if (input_len - index <= field_length) {
            return OPRT_NOT_FOUND;
        }
        if (field_type == type) {
            *output_buf = &(p_data[index + 2]);
            *output_len = field_length - 1;
            return OPRT_OK;
        }
        index += field_length + 1;
    }
    return OPRT_NOT_FOUND;
}

