/***
 * Seeed_Arduino_GroveAI.h
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

#ifndef SEEED_ARDUINO_GROVEAI_H
#define SEEED_ARDUINO_GROVEAI_H

#include <Arduino.h>
#include <Wire.h>

#include <ArduinoJson.h>
#include "lwRingBuffer.hpp"

#include <stdint.h>

/* Exported macros ---------------------------------------------------------*/
#define GROVE_AI_ADDRESS (0x62)

#define PL_HEAD_LEN    6
#if defined(ARDUINO_ARCH_AVR)
#define MAX_PL_LEN     32 - PL_HEAD_LEN
#elif defined(ARDUINO_ARCH_ESP32)
#define MAX_PL_LEN     128 - PL_HEAD_LEN
#else 
#define MAX_PL_LEN     256 - PL_HEAD_LEN
#endif

#define INTERVAL_DELAY 6
#define MAX_RESP_DELAY 1000

#define FEATURE_TRANSPORT 0x10
#define FEATURE_TRANSPORT_CMD_READ 0x01
#define FEATURE_TRANSPORT_CMD_WRITE 0x02
#define FEATURE_TRANSPORT_CMD_AVAILABLE 0x03
#define FEATURE_TRANSPORT_CMD_START 0x04
#define FEATURE_TRANSPORT_CMD_STOP 0x05
#define FEATURE_TRANSPORT_CMD_RESET 0x06

#define AT_CMD_BEGIN          "AT+"
#define AT_CMD_END            "\r\n"

#define AT_CMD_RO_HELP        "HELP"
#define AT_CMD_RO_NAME        "NAME"
#define AT_CMD_RO_VERSION     "VER"
#define AT_CMD_RO_ID          "ID"
#define AT_CMD_RO_STATE       "STATE"
#define AT_CMD_RO_ALGOS       "ALGOS"
#define AT_CMD_RO_MODELS      "MODELS"
#define AT_CMD_RO_SENSORS     "SENSORS"

#define AT_CMD_RW_ALGO        "ALGO"
#define AT_CMD_RW_MODEL       "MODEL"
#define AT_CMD_RW_SENSOR      "SENSOR"
#define AT_CMD_RW_TIOU        "TIOU"
#define AT_CMD_RW_TSCORE      "TSCORE"
#define AT_CMD_RW_WIFI        "WIFI"
#define AT_CMD_RW_MQTTSERVER  "MQTTSERVER"
#define AT_CMD_RW_MQTTPUBSUB  "MQTTPUBSUB"

#define AT_CMD_CTL_RESET      "RST"
#define AT_CMD_CTL_BREAK      "BREAK"
#define AT_CMD_CTL_YIELD      "YIELD"
#define AT_CMD_CTL_LED        "LED"
#define AT_CMD_CTL_SAMPLE     "SAMPLE"
#define AT_CMD_CTL_INVOKE     "INVOKE"
#define AT_CMD_CTL_ACTION     "ACTION"

typedef enum cmd_errcode : uint8_t
{
    CMD_OK = 0,
    CMD_AGAIN,
    CMD_ELOG,
    CMD_ETIMEOUT,
    CMD_EIO,
    CMD_EINVAL,
    CMD_ENOMEM,
    CMD_EBUSY,
    CMD_ENOTSUP,
    CMD_EPERM,
    CMD_EUNKNOW,

    CMD_NO_RESPONSE = 0x80
} cmd_errcode_t;

typedef enum resp_type : uint8_t
{
    RESP_TYPE_OPERATION = 0,
    RESP_TYPE_EVENT,
    RESP_TYPE_LOGGING
} resp_type_e;

typedef struct device_info
{
    // char name[32];
    char version[32];
    uint32_t id;
} device_info_t;

typedef struct model_info
{
    uint8_t algo_id;
    uint8_t model_id;
    uint8_t sensor_id;
    uint8_t tiou;
    uint8_t tscore;
} model_info_t;

typedef enum result_type
{
    RESULT_TYPE_BOX = 0,
    RESULT_TYPE_POINT,
    RESULT_TYPE_CLASS
} result_type_e;

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    uint8_t  score;
    uint8_t  target;
} box_result_t;

typedef struct 
{
    uint16_t x;
    uint16_t y;
    uint8_t  score;
    uint8_t  target;
} point_result_t;

typedef struct 
{
    uint16_t score;
    uint16_t target;
} class_result_t;

typedef struct ai_result
{
    result_type_e _type;
    uint8_t _num;
    union
    {
        box_result_t _box;
        point_result_t _point;
        class_result_t _class;
    } _result[8];
} ai_result_t;


class GroveAI_V2
{
private:
    device_info_t _sys;
    model_info_t _algo;

    TwoWire *_wire_com;
    uint8_t _slave_addr;

    char *_RespStr;
    lwRingBuffer *_RingBuf;

public:
    GroveAI_V2(TwoWire &wire = Wire, uint8_t address = GROVE_AI_ADDRESS, uint32_t size = 1024) 
    {
        _wire_com = &wire;
        _slave_addr = address;
        _RespStr = new char[size];
        _RingBuf = new lwRingBuffer(size);
    }
    ~GroveAI_V2() 
    {
        delete[] _RespStr;
        delete _RingBuf;
    }
    bool begin();

    char* version() { return _sys.version; }
    uint32_t id() { return _sys.id; }

    uint8_t algo() { return _algo.algo_id; }
    uint8_t model() { return _algo.model_id; }
    uint8_t tiou() { return _algo.tiou; }
    uint8_t tscore() { return _algo.tscore; }

    bool invoke(uint32_t cnt = 1);
    void reset();
    bool get_result(ai_result_t* data);

protected:
    int32_t write(uint8_t *data, int32_t length);
    void write(uint8_t data) { 
        write(&data, 1); 
    }
    int32_t read(uint8_t *data, int32_t length);
    uint8_t read() { 
        uint8_t data;
        read(&data, 1);
        return data;
    }
    int32_t available();
    bool get_line(char *line, uint16_t len);

    bool write_cmd(uint8_t feature, uint8_t cmd, uint16_t len = 0);
    cmd_errcode_t get_code(const char *cmd);
    cmd_errcode_t get_result_data(const char *cmd, ai_result_t* data);
    cmd_errcode_t get_info_data(const char *cmd, uint8_t* data);

    cmd_errcode_t set_tiou(uint8_t tiou);
    cmd_errcode_t set_tscore(uint8_t tscore);
    cmd_errcode_t get_tiou();
    cmd_errcode_t get_tscore();

    cmd_errcode_t get_algo();
    cmd_errcode_t get_model();

    bool get_device_info();
};

#endif
