/***
 * Seeed_Arduino_GroveAI.cpp
 * Description: A drive for Seeed Grove AI Family.
 * 2023 Copyright (c) Seeed Technology Inc.  All right reserved.
 * Author: Jiaxuan Weng(jiaxuan.weng@outlook.com)
 * 2023-11-24
 * Copyright (C) 2023  Seeed Technology Co.,Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Seeed_Arduino_GroveAI_V2.h"

bool GroveAI_V2::begin()
{
    Serial.printf("GroveAI_V2 begin\n");
    _wire_com->begin();
    _wire_com->setClock(400000);
    // write_cmd(FEATURE_TRANSPORT, FEATURE_TRANSPORT_CMD_START);
    delay(1000);
    if (!get_device_info()) return false;
    return true;
}

bool GroveAI_V2::invoke(uint32_t cnt)
{
    char buf[32] = {0};
    sprintf(buf, AT_CMD_BEGIN AT_CMD_CTL_INVOKE "=%d,1" AT_CMD_END, cnt);
    return (strlen(buf) == this->write((uint8_t *)buf, strlen(buf)));
}

void GroveAI_V2::reset()
{
    char buf[16] = {0};
    sprintf(buf, AT_CMD_BEGIN AT_CMD_CTL_RESET AT_CMD_END);
    this->write((uint8_t *)buf, strlen(buf));
}

bool GroveAI_V2::get_result(ai_result_t* data)
{
    if (!data) {
        return false;
    }
    if (get_result_data(AT_CMD_CTL_INVOKE, data) == CMD_OK) {
        return true;
    }
    return false;
}


int32_t GroveAI_V2::write(uint8_t *data, int32_t length)
{
    Serial.printf("write %d bytes to 0x%02x\n", length, _slave_addr);
    int32_t pl_len = 0;
    for (int32_t cnt = 0; cnt < length; cnt += pl_len)
    {
        pl_len = (length - cnt > MAX_PL_LEN) ? MAX_PL_LEN : length - cnt;
        delay(INTERVAL_DELAY);
        _wire_com->beginTransmission(_slave_addr);
        _wire_com->write(FEATURE_TRANSPORT);
        _wire_com->write(FEATURE_TRANSPORT_CMD_WRITE);
        _wire_com->write(pl_len >> 8);
        _wire_com->write(pl_len & 0xFF);
        _wire_com->write(data + cnt, pl_len);
        if (_wire_com->endTransmission() == 0) {
            Serial.printf("write %d bytes: %s\n", length, data);
        } else {
            Serial.printf("write failed\n");
        }
    }
    return length;
}

int32_t GroveAI_V2::read(uint8_t *data, int32_t length)
{
    Serial.printf("read %d bytes from 0x%02x\n", length, _slave_addr);
    int32_t pl_len = 0;
    for (int32_t cnt = 0; cnt < length; cnt += pl_len)
    {
        pl_len = (length - cnt > MAX_PL_LEN) ? MAX_PL_LEN : length - cnt;
        if (write_cmd(FEATURE_TRANSPORT, FEATURE_TRANSPORT_CMD_READ, pl_len)) {
            delay(INTERVAL_DELAY);
            _wire_com->requestFrom(_slave_addr, pl_len);
            _wire_com->readBytes(data + cnt, pl_len);
            Serial.printf("read %d bytes: %s\n", length, data);
        } else {
            Serial.printf("read failed\n");
        }
    }
    return length;
}

int32_t GroveAI_V2::available()
{
    uint8_t buf[2] = {0};
    if (write_cmd(FEATURE_TRANSPORT, FEATURE_TRANSPORT_CMD_AVAILABLE, 0))
    {
        delay(INTERVAL_DELAY);
        _wire_com->requestFrom(_slave_addr, 2);
        _wire_com->readBytes(buf, 2);
    }
    return (buf[0] << 8) | buf[1];
}

bool GroveAI_V2::get_line(char *line, uint16_t len)
{
    uint32_t t = millis();
    uint8_t buf[256] = {0};
    int32_t cnt = 0;
    while (millis() - t < MAX_RESP_DELAY)
    {
        cnt = available();
        if (cnt > 0) {
            Serial.printf("available: %d\n", cnt);
            // cnt = cnt > sizeof(buf) ? sizeof(buf) : cnt;
            this->read(buf, cnt);
            for (uint16_t i = 0; i < cnt; i++) {
                *_RingBuf << buf[i];
            }
        }
        cnt = _RingBuf->extract('\n', line, len - 1);
        if (cnt > 0) {
            line[cnt] = '\0';
            Serial.printf("get line: %s", line);
            return true;
        }
    }
    return false;
}

bool GroveAI_V2::write_cmd(uint8_t feature, uint8_t cmd, uint16_t len)
{
    delay(INTERVAL_DELAY);
    // Serial.printf("write cmd: 0x%02x\n", cmd);
    _wire_com->beginTransmission(_slave_addr);
    _wire_com->write(feature);
    _wire_com->write(cmd);
    _wire_com->write(len >> 8);
    _wire_com->write(len & 0xFF);
    _wire_com->write(0); // TODO checksum
    _wire_com->write(0);
    return _wire_com->endTransmission() == 0;
}

cmd_errcode_t GroveAI_V2::get_code(const char *cmd)
{
    StaticJsonDocument<256> doc;
    DeserializationError error;
    uint8_t cnt = 0;
    while (cnt++ < 3) {
        if (!get_line(_RespStr, sizeof(_RespStr))) {
            continue;
        }
        error = deserializeJson(doc, _RespStr);
        if (error) {
            continue;
        }
        if (strstr(doc["name"], cmd) != NULL) {
            return (cmd_errcode_t)(doc["code"]);
        }
    }
    return CMD_NO_RESPONSE;
}

cmd_errcode_t GroveAI_V2::get_result_data(const char *cmd, ai_result_t* data)
{
    StaticJsonDocument<1024> doc;
    DeserializationError error;
    uint8_t cnt = 0;
    while (cnt++ < 3) {
        if (!get_line(_RespStr, sizeof(_RespStr))) {
            continue;
        }
        error = deserializeJson(doc, _RespStr);
        if (error) {
            continue;
        }
        if (strstr(doc["name"], cmd) == NULL) {
            continue;
        }
        if ((resp_type_e)(doc["type"]) != RESP_TYPE_EVENT) {
            continue;
        }
        if ((cmd_errcode_t)(doc["code"]) != CMD_OK) {
            return (cmd_errcode_t)(doc["code"]);
        }
        if (doc["data"].containsKey("boxes")) {
            data->_num = doc["data"]["boxes"].size();
            for (uint8_t i = 0; i < data->_num; i++) {
                data->_type = RESULT_TYPE_BOX;
                data->_result[i]._box.x = doc["data"]["boxes"][i][0];
                data->_result[i]._box.y = doc["data"]["boxes"][i][1];
                data->_result[i]._box.w = doc["data"]["boxes"][i][2];
                data->_result[i]._box.h = doc["data"]["boxes"][i][3];
                data->_result[i]._box.score = doc["data"]["boxes"][i][4];
                data->_result[i]._box.target = doc["data"]["boxes"][i][5];
            }
        } else if (doc["data"].containsKey("points")) {
            data->_num = doc["data"]["points"].size();
            for (uint8_t i = 0; i < data->_num; i++) {
                data->_type = RESULT_TYPE_POINT;
                data->_result[i]._point.x = doc["data"]["points"][i][0];
                data->_result[i]._point.y = doc["data"]["points"][i][1];
                data->_result[i]._point.score = doc["data"]["points"][i][2];
                data->_result[i]._point.target = doc["data"]["points"][i][3];
            }
        } else if (doc["data"].containsKey("classes")) {
            data->_num = doc["data"]["classes"].size();
            for (uint8_t i = 0; i < data->_num; i++) {
                data->_type = RESULT_TYPE_CLASS;
                data->_result[i]._class.score = doc["data"]["classes"][i];
                data->_result[i]._class.target = doc["data"]["classes"][i];
            }
        }
        return CMD_OK;
    }
    return CMD_NO_RESPONSE;
}

cmd_errcode_t GroveAI_V2::get_info_data(const char *cmd, uint8_t* data)
{
    Serial.println("get info data");
    StaticJsonDocument<256> doc;
    DeserializationError error;
    uint8_t cnt = 0;

    while (cnt++ < 3) {
        if (!get_line(_RespStr, sizeof(_RespStr))) {
            continue;
        }
        error = deserializeJson(doc, _RespStr);
        if (error) {
            continue;
        }
        if (strstr(doc["name"], cmd) == NULL) {
            continue;
        }
        if ((resp_type_e)(doc["type"]) != RESP_TYPE_OPERATION) {
            continue;
        }
        if ((cmd_errcode_t)(doc["code"]) != CMD_OK) {
            return (cmd_errcode_t)(doc["code"]);
        }
        *data = doc["data"];
        return CMD_OK;
    }
    return CMD_NO_RESPONSE;
}


cmd_errcode_t GroveAI_V2::set_tiou(uint8_t tiou)
{
    if (tiou > 100) {
        tiou = 100;
    }
    char buf[16] = {0};
    sprintf(buf, AT_CMD_BEGIN AT_CMD_RW_TIOU "=%d" AT_CMD_END, tiou);
    this->write((uint8_t *)buf, strlen(buf));

    return get_code(AT_CMD_RW_TIOU);
}

cmd_errcode_t GroveAI_V2::set_tscore(uint8_t tscore)
{
    if (tscore > 100) {
        tscore = 100;
    }
    char buf[16] = {0};
    sprintf(buf, AT_CMD_BEGIN AT_CMD_RW_TSCORE "=%d" AT_CMD_END, tscore);
    this->write((uint8_t *)buf, strlen(buf));

    return get_code(AT_CMD_RW_TIOU);
}

cmd_errcode_t GroveAI_V2::get_tiou()
{
    char buf[16] = {0};
    sprintf(buf, AT_CMD_BEGIN AT_CMD_RW_TIOU "?" AT_CMD_END);
    this->write((uint8_t *)buf, strlen(buf));

    return get_info_data(AT_CMD_RW_TIOU, &(_algo.tiou));
}

cmd_errcode_t GroveAI_V2::get_tscore()
{
    char buf[16] = {0};
    sprintf(buf, AT_CMD_BEGIN AT_CMD_RW_TSCORE "?" AT_CMD_END);
    this->write((uint8_t *)buf, strlen(buf));

    return get_info_data(AT_CMD_RW_TSCORE, &(_algo.tscore));;
}

cmd_errcode_t GroveAI_V2::get_algo()
{
    char buf[16] = {0};
    sprintf(buf, AT_CMD_BEGIN AT_CMD_RW_ALGO "?" AT_CMD_END);
    this->write((uint8_t *)buf, strlen(buf));
    return CMD_OK;
}

cmd_errcode_t GroveAI_V2::get_model()
{
    char buf[16] = {0};
    sprintf(buf, AT_CMD_BEGIN AT_CMD_RW_MODEL "?" AT_CMD_END);
    this->write((uint8_t *)buf, strlen(buf));
    return CMD_OK;
}

bool GroveAI_V2::get_device_info()
{
    char buf[16] = {0};
    sprintf(buf, AT_CMD_BEGIN AT_CMD_RO_ID "?" AT_CMD_END);
    Serial.printf("write dat: %s", buf);
    this->write((uint8_t *)buf, strlen(buf));
    Serial.println("waiting...");
    uint8_t cnt = 0;
    while (cnt++ < 5) {
        if (!get_line(_RespStr, sizeof(_RespStr))) {
            continue;
        }
    }
    Serial.println(_RespStr);

    // if (get_tiou() != CMD_OK) {
    //     return false;
    // }
    // if (get_info_data(AT_CMD_RO_ID, &(_algo.tiou)) != CMD_OK) {
    //     return false;
    // }
    // if (get_tscore() != CMD_OK) {
    //     return false;
    // }
    return true;
}

