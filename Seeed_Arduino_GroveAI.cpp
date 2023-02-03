/***
 * Seeed_Arduino_GroveAI.cpp
 * Description: A drive for Seeed Grove AI Family.
 * 2022 Copyright (c) Seeed Technology Inc.  All right reserved.
 * Author: Hongtai Liu(lht856@foxmail.com)
 * 2022-4-24
 * Copyright (C) 2020  Seeed Technology Co.,Ltd.
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

#include "Seeed_Arduino_GroveAI.h"

// CRC8 x^8+x^5+x^4+x^0
static const uint8_t CRC8Table[] = {
    0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
    157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
    35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
    190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
    70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
    219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
    101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
    248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
    140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
    17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
    175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
    50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
    202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
    87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
    233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
    116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53};

GroveAI::GroveAI(TwoWire &wire, uint8_t address, int32_t signal_pin, bool crc_enable)
{
    _wire_com = &wire;

    _crc_enable = crc_enable;
    _signal_pin = signal_pin;
    _slave_addr = address;
    _algo.model = MODEL_MAX;
    _algo.algo = ALGO_MAX;
    _algo.confidence = 50;
    _algo.iou = 45;
}
GroveAI::~GroveAI()
{
}

bool GroveAI::begin(ALGO_INDEX_T algo, MODEL_INDEX_T model, uint8_t confidence)
{
    bool status = true;
    _wire_com->begin();

    if (_signal_pin != -1)
    {
        pinMode(_signal_pin, INPUT);
    }

    uint8_t buf[2];

    status = read(FEATURE_SYSTEM, CMD_SYS_READ_ID, NULL, 0, buf, CMD_SYS_ID_LENGTH);
    _system.id = buf[0] << 8 | buf[1];

    // if (!status || (GROVE_AI_CAMERA_ID != _system.id && VISION_AI_CAMERA_ID != _system.id))
    // {
    //     return false;
    // }

    status = read(FEATURE_SYSTEM, CMD_SYS_READ_VERSION, NULL, 0, buf, CMD_SYS_VERSION_LENGTH);
    _system.version = buf[0] << 8 | buf[1];

    _algo.algo = get_algo();
    _algo.model = get_model();
    _algo.confidence = get_confidence();

    if (_algo.algo != algo || _algo.model != model || _algo.confidence != confidence)
    {
        if (algo != set_algo(algo))
        {
            return false;
        }
        if (model != set_model(model))
        {
            return false;
        }
        if (confidence != set_confidence(confidence))
        {
            return false;
        }
        if (!config_save())
        {
            return false;
        }
        reset();
    }

    return true;
}

uint16_t GroveAI::version()
{

    return _system.version;
}

ALGO_INDEX_T GroveAI::algo()
{
    return (ALGO_INDEX_T)_algo.algo;
}
MODEL_INDEX_T GroveAI::model()
{
    return (MODEL_INDEX_T)_algo.model;
}
uint8_t GroveAI::confidence()
{
    return _algo.confidence;
}

CMD_STATE_T GroveAI::state()
{
    uint8_t buf;

    read(FEATURE_SYSTEM, CMD_SYS_READ_STATE, NULL, 0, &buf, CMD_SYS_STATE_LENGTH);
    _system.state = (CMD_STATE_T)buf;

    return _system.state;
}

uint16_t GroveAI::id()
{
    return _system.id;
}

bool GroveAI::invoke()
{
    write(FEATURE_ALGO, CMD_ALGO_INVOKE, NULL, 0);

    CMD_STATE_T ret = CMD_STATE_RUNNING;
    while (1)
    {
        ret = state();
        if (ret == CMD_STATE_RUNNING)
        {
            return true;
        }
        else if (ret == CMD_STATE_ERROR)
        {
            return false;
        }
    }
}

ALGO_INDEX_T GroveAI::set_algo(ALGO_INDEX_T algo)
{
    uint8_t data = (uint8_t)algo;
    if (_algo.algo != algo)
    {
        write(FEATURE_ALGO, CMD_ALGO_WRITE_ALGO, &data, CMD_ALGO_ALGO_LENGTH);
        data = 0;

        read(FEATURE_ALGO, CMD_ALGO_READ_ALGO, NULL, 0, &data, CMD_ALGO_ALGO_LENGTH);
        if (data == algo)
        {
            _algo.algo = algo;
        }
    }
    return (ALGO_INDEX_T)_algo.algo;
}

MODEL_INDEX_T GroveAI::set_model(MODEL_INDEX_T model)
{
    uint8_t data = (uint8_t)model;
    if (_algo.model != model)
    {
        write(FEATURE_ALGO, CMD_ALGO_WRITE_MODEL, &data, CMD_ALGO_MODEL_LENGTH);
        data = 0;

        read(FEATURE_ALGO, CMD_ALGO_READ_MODEL, NULL, 0, &data, CMD_ALGO_MODEL_LENGTH);
        if (data == model)
        {
            _algo.model = model;
        }
    }
    return (MODEL_INDEX_T)_algo.model;
}

uint8_t GroveAI::set_confidence(uint8_t confidence)
{
    uint8_t data = confidence;
    if (_algo.confidence != confidence)
    {
        write(FEATURE_ALGO, CMD_ALGO_WRITE_CONFIDENCE, &data, CMD_ALGO_CONFIDENCE_LENGTH);
        data = 0;

        read(FEATURE_ALGO, CMD_ALGO_READ_CONFIDENCE, NULL, 0, &data, CMD_ALGO_CONFIDENCE_LENGTH);
        if (data == confidence)
        {
            _algo.confidence = confidence;
        }
    }
    return _algo.confidence;
}

uint8_t GroveAI::set_iou(uint8_t iou)
{
    uint8_t data = iou;
    if (_algo.iou != iou)
    {
        write(FEATURE_ALGO, CMD_ALGO_WRITE_CONFIDENCE, &iou, CMD_ALGO_CONFIDENCE_LENGTH);
        data = 0;

        read(FEATURE_ALGO, CMD_ALGO_READ_CONFIDENCE, NULL, 0, &data, CMD_ALGO_CONFIDENCE_LENGTH);
        if (data == iou)
        {
            _algo.iou = iou;
        }
    }
    return _algo.iou;
}

ALGO_INDEX_T GroveAI::get_algo()
{
    uint8_t algo = 0;

    read(FEATURE_ALGO, CMD_ALGO_READ_ALGO, NULL, 0, &algo, CMD_ALGO_ALGO_LENGTH);

    _algo.algo = algo;

    return (ALGO_INDEX_T)_algo.algo;
}

MODEL_INDEX_T GroveAI::get_model()
{
    uint8_t model = 0;
    read(FEATURE_ALGO, CMD_ALGO_READ_MODEL, NULL, 0, &model, CMD_ALGO_MODEL_LENGTH);

    _algo.model = model;

    return (MODEL_INDEX_T)_algo.model;
}

uint8_t GroveAI::get_confidence()
{
    uint8_t confidence = 0;
    read(FEATURE_ALGO, CMD_ALGO_READ_CONFIDENCE, NULL, 0, &confidence, CMD_ALGO_CONFIDENCE_LENGTH);
    _algo.confidence = confidence;

    return _algo.confidence;
}

uint8_t GroveAI::get_iou()
{
    uint8_t iou = 0;
    read(FEATURE_ALGO, CMD_ALGO_READ_IOU, NULL, 0, &iou, CMD_ALGO_IOU_LENGTH);
    _algo.iou = iou;

    return _algo.iou;
}

uint16_t GroveAI::get_result_len()
{
    uint8_t buf[2];
    uint16_t len = 0;
    if (!read(FEATURE_ALGO, CMD_ALGO_READ_RET_LEN, NULL, 0, buf, CMD_ALGO_READ_RET_LEN_LENGTH))
    {
        return 0;
    }
    len = buf[0] << 8 | buf[1];

    return len;
}

bool GroveAI::get_result(uint16_t index, uint8_t *buff, uint8_t len)
{
    uint8_t data[2] = {0};
    data[0] = (index << 8) & 0xFF;
    data[1] = index & 0XFF;

    return read(FEATURE_ALGO, CMD_ALGO_READ_RET, data, 2, buff, len);
}

bool GroveAI::config_save()
{
    write(FEATURE_ALGO, CMD_ALGO_CONFIG_SAVE, NULL, 0);

    CMD_STATE_T ret = CMD_STATE_RUNNING;
    while (1)
    {
        ret = state();
        if (ret == CMD_STATE_IDLE)
        {
            return true;
        }
        else if (ret == CMD_STATE_ERROR)
        {
            return false;
        }
    }
}

bool GroveAI::config_clear()
{
    write(FEATURE_ALGO, CMD_ALGO_CONFIG_CLEAR, NULL, 0);

    CMD_STATE_T ret = state();
    while (1)
    {
        ret = state();
        if (ret == CMD_STATE_IDLE)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

void GroveAI::reset()
{
    write(FEATURE_SYSTEM, CMD_SYS_RESET, NULL, 0);
}

bool GroveAI::read(uint8_t feature, uint8_t cmd, uint8_t *param, uint8_t param_len, uint8_t *buf, uint16_t len)
{
    uint8_t i = 0;
    uint8_t c = 0;
    if (_signal_pin != -1)
    {
        while (digitalRead(_signal_pin) == LOW)
        {
        }
        delay(1);
        while (digitalRead(_signal_pin) == LOW)
        {
        }
    }
    else
    {
        delay(10);
    }
    _wire_com->beginTransmission(_slave_addr);
    _wire_com->write(feature);
    _wire_com->write(cmd);
    for (int j = 0; j < param_len; j++)
    {
        _wire_com->write(param[j]);
    }
    _wire_com->endTransmission();

    if (_signal_pin != -1)
    {
        while (digitalRead(_signal_pin) == LOW)
        {
        }
        delay(1);
        while (digitalRead(_signal_pin) == LOW)
        {
        }
    }
    else
    {
        delay(10);
    }

    if (_crc_enable)
    {
        uint8_t crc8 = 0;
        _wire_com->requestFrom(_slave_addr, len + 1);
        while ((_wire_com->available()) && (i < len)) // slave may send less than requested
        {
            c = _wire_com->read(); // receive a byte as character
            crc8 = CRC8Table[crc8 ^ c];
            *buf = c;
            buf++;
            i++;
        }

        c = _wire_com->read();
        return c == crc8;
    }
    else
    {
        _wire_com->requestFrom(_slave_addr, len);
        while ((_wire_com->available()) && (i < len)) // slave may send less than requested
        {
            c = _wire_com->read(); // receive a byte as character
            *buf = c;
            buf++;
            i++;
        }
    }
    return true;
}

void GroveAI::write(uint8_t feature, uint8_t cmd, uint8_t *buf, uint16_t len)
{
    uint32_t tick = millis();

    if (_signal_pin != -1)
    {
        while (digitalRead(_signal_pin) == LOW)
        {
        }
        delay(1);
        while (digitalRead(_signal_pin) == LOW)
        {
        }
    }
    else
    {
        delay(10);
    }

    _wire_com->beginTransmission(_slave_addr);
    _wire_com->write(feature);
    _wire_com->write(cmd);
    for (uint16_t i = 0; i < len; i++)
    {
        _wire_com->write(buf[i]);
    }

    if (_crc_enable)
    {
        uint8_t crc8 = 0;
        crc8 = CRC8Table[crc8 ^ feature];
        crc8 = CRC8Table[crc8 ^ cmd];
        for (uint16_t i = 0; i < len; i++)
        {
            crc8 = CRC8Table[crc8 ^ buf[i]];
        }
        _wire_com->write(crc8);
    }

    _wire_com->endTransmission();
}
