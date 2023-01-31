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

GroveAI::GroveAI(TwoWire &wire, uint8_t address)
{
    _wire_com = &wire;

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
    _wire_com->begin();

    uint8_t buf[2];

    read(FEATURE_SYSTEM, CMD_SYS_READ_ID, NULL, 0, buf, CMD_SYS_ID_LENGTH);
    _system.id = buf[0] << 8 | buf[1];

    // if (GROVE_AI_CAMERA_ID != _system.id)
    // {
    //     return false;
    // }

    read(FEATURE_SYSTEM, CMD_SYS_READ_VERSION, NULL, 0, buf, CMD_SYS_VERSION_LENGTH);
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
    write(FEATURE_ALGO, CMD_ALGO_INOVKE, NULL, 0);

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
    read(FEATURE_ALGO, CMD_ALGO_READ_RET_LEN, NULL, 0, buf, CMD_ALGO_READ_RET_LEN_LENGTH);
    len = buf[0] << 8 | buf[1];

    return len;
}

void GroveAI::get_result(uint16_t index, uint8_t *buff, uint8_t len)
{
    uint8_t data[2] = {0};
    data[0] = (index << 8) & 0xFF;
    data[1] = index & 0XFF;
    read(FEATURE_ALGO, CMD_ALGO_READ_RET, data, 2, buff, len);

    return;
}

// void GroveAI::get_result(uint16_t index, uint16_t *buff, uint8_t len)
//{
//     uint8_t data[2] = {0};
//     data[0] = (index << 8) & 0xFF;
//     data[1] = index & 0XFF;
//     read(FEATURE_ALGO, CMD_ALGO_READ_RET, data, 2, buff, len);
//
//     return;
// }

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
    delay(5);
}

void GroveAI::read(uint8_t feature, uint8_t cmd, uint8_t *param, uint8_t param_len, uint8_t *buf, uint16_t len)
{
    uint8_t i = 0;
    uint8_t c = 0;
    // while (digitalRead(_signal_pin) == 0)
    // {
    // }
    delay(5);
    // while (digitalRead(_signal_pin) == 0)
    // {
    // }

    _wire_com->beginTransmission(_slave_addr);
    _wire_com->write(feature);
    _wire_com->write(cmd);
    for (int j = 0; j < param_len; j++)
    {
        _wire_com->write(param[j]);
    }
    _wire_com->endTransmission();

    // while (digitalRead(_signal_pin) == 0)
    // {
    // }
    delay(5);
    // while (digitalRead(_signal_pin) == 0)
    // {
    // }

    _wire_com->requestFrom(_slave_addr, len);

    while ((_wire_com->available()) && (i < len)) // slave may send less than requested
    {
        c = _wire_com->read(); // receive a byte as character
        *buf = c;
        buf++;
        i++;
    }
}

void GroveAI::write(uint8_t feature, uint8_t cmd, uint8_t *buf, uint16_t len)
{
    uint32_t tick = millis();
    // while (digitalRead(_signal_pin) == 0)
    // {
    // }
    delay(5);
    // while (digitalRead(_signal_pin) == 0)
    // {
    // }
    _wire_com->beginTransmission(_slave_addr);
    _wire_com->write(feature);
    _wire_com->write(cmd);
    for (uint16_t i = 0; i < len; i++)
    {
        _wire_com->write(buf[i]);
    }
    _wire_com->endTransmission();
}
