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
    0,   94,  188, 226, 97,  63,  221, 131, 194, 156, 126, 32,  163, 253, 31,
    65,  157, 195, 33,  127, 252, 162, 64,  30,  95,  1,   227, 189, 62,  96,
    130, 220, 35,  125, 159, 193, 66,  28,  254, 160, 225, 191, 93,  3,   128,
    222, 60,  98,  190, 224, 2,   92,  223, 129, 99,  61,  124, 34,  192, 158,
    29,  67,  161, 255, 70,  24,  250, 164, 39,  121, 155, 197, 132, 218, 56,
    102, 229, 187, 89,  7,   219, 133, 103, 57,  186, 228, 6,   88,  25,  71,
    165, 251, 120, 38,  196, 154, 101, 59,  217, 135, 4,   90,  184, 230, 167,
    249, 27,  69,  198, 152, 122, 36,  248, 166, 68,  26,  153, 199, 37,  123,
    58,  100, 134, 216, 91,  5,   231, 185, 140, 210, 48,  110, 237, 179, 81,
    15,  78,  16,  242, 172, 47,  113, 147, 205, 17,  79,  173, 243, 112, 46,
    204, 146, 211, 141, 111, 49,  178, 236, 14,  80,  175, 241, 19,  77,  206,
    144, 114, 44,  109, 51,  209, 143, 12,  82,  176, 238, 50,  108, 142, 208,
    83,  13,  239, 177, 240, 174, 76,  18,  145, 207, 45,  115, 202, 148, 118,
    40,  171, 245, 23,  73,  8,   86,  180, 234, 105, 55,  213, 139, 87,  9,
    235, 181, 54,  104, 138, 212, 149, 203, 41,  119, 244, 170, 72,  22,  233,
    183, 85,  11,  136, 214, 52,  106, 43,  117, 151, 201, 74,  20,  246, 168,
    116, 42,  200, 150, 21,  75,  169, 247, 182, 232, 10,  84,  215, 137, 107,
    53};

GroveAI::GroveAI(TwoWire &wire, uint8_t address, int32_t signal_pin,
                 bool crc_enable)
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
GroveAI::~GroveAI() {}

bool GroveAI::begin(ALGO_INDEX_T algo, MODEL_INDEX_T model, uint8_t confidence)
{
    bool status = true;
    _wire_com->begin();

    if (_signal_pin != -1)
    {
        pinMode(_signal_pin, INPUT);
    }

    uint8_t buf[2];

    status =
        read(FEATURE_SYSTEM, CMD_SYS_READ_ID, NULL, 0, buf, CMD_SYS_ID_LENGTH);
    _system.id = buf[0] << 8 | buf[1];

    // if (!status || (GROVE_AI_CAMERA_ID != _system.id && VISION_AI_CAMERA_ID
    // != _system.id))
    // {
    //     return false;
    // }

    status = read(FEATURE_SYSTEM, CMD_SYS_READ_VERSION, NULL, 0, buf,
                  CMD_SYS_VERSION_LENGTH);
    _system.version = buf[0] << 8 | buf[1];

    _algo.algo = get_algo();
    _algo.model = get_model();
    _algo.confidence = get_confidence();

    if (_algo.algo != algo || _algo.model != model ||
        _algo.confidence != confidence)
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

uint16_t GroveAI::version() { return _system.version; }

ALGO_INDEX_T GroveAI::algo() { return (ALGO_INDEX_T)_algo.algo; }
MODEL_INDEX_T GroveAI::model() { return (MODEL_INDEX_T)_algo.model; }
uint8_t GroveAI::confidence() { return _algo.confidence; }

CMD_STATE_T GroveAI::state()
{
    uint8_t buf;

    read(FEATURE_SYSTEM, CMD_SYS_READ_STATE, NULL, 0, &buf,
         CMD_SYS_STATE_LENGTH);
    _system.state = (CMD_STATE_T)buf;

    return _system.state;
}

uint16_t GroveAI::id() { return _system.id; }

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

        read(FEATURE_ALGO, CMD_ALGO_READ_ALGO, NULL, 0, &data,
             CMD_ALGO_ALGO_LENGTH);
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

        read(FEATURE_ALGO, CMD_ALGO_READ_MODEL, NULL, 0, &data,
             CMD_ALGO_MODEL_LENGTH);
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
        write(FEATURE_ALGO, CMD_ALGO_WRITE_CONFIDENCE, &data,
              CMD_ALGO_CONFIDENCE_LENGTH);
        data = 0;

        read(FEATURE_ALGO, CMD_ALGO_READ_CONFIDENCE, NULL, 0, &data,
             CMD_ALGO_CONFIDENCE_LENGTH);
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
        write(FEATURE_ALGO, CMD_ALGO_WRITE_CONFIDENCE, &iou,
              CMD_ALGO_CONFIDENCE_LENGTH);
        data = 0;

        read(FEATURE_ALGO, CMD_ALGO_READ_CONFIDENCE, NULL, 0, &data,
             CMD_ALGO_CONFIDENCE_LENGTH);
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

    read(FEATURE_ALGO, CMD_ALGO_READ_ALGO, NULL, 0, &algo,
         CMD_ALGO_ALGO_LENGTH);

    _algo.algo = algo;

    return (ALGO_INDEX_T)_algo.algo;
}

MODEL_INDEX_T GroveAI::get_model()
{
    uint8_t model = 0;
    read(FEATURE_ALGO, CMD_ALGO_READ_MODEL, NULL, 0, &model,
         CMD_ALGO_MODEL_LENGTH);

    _algo.model = model;

    return (MODEL_INDEX_T)_algo.model;
}

uint8_t GroveAI::get_confidence()
{
    uint8_t confidence = 0;
    read(FEATURE_ALGO, CMD_ALGO_READ_CONFIDENCE, NULL, 0, &confidence,
         CMD_ALGO_CONFIDENCE_LENGTH);
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
    if (!read(FEATURE_ALGO, CMD_ALGO_READ_RET_LEN, NULL, 0, buf,
              CMD_ALGO_READ_RET_LEN_LENGTH))
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

void GroveAI::reset() { write(FEATURE_SYSTEM, CMD_SYS_RESET, NULL, 0); }

bool GroveAI::read(uint8_t feature, uint8_t cmd, uint8_t *param,
                   uint8_t param_len, uint8_t *buf, uint16_t len)
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
        delay(40);
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
        delay(40);
    }

    if (_crc_enable)
    {
        uint8_t crc8 = 0;
        _wire_com->requestFrom(_slave_addr, len + 1);
        while ((_wire_com->available()) &&
               (i < len)) // slave may send less than requested
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
        while ((_wire_com->available()) &&
               (i < len)) // slave may send less than requested
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
        delay(40);
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

void WEI::start_repeat(void)
{
    pinMode(_pinSCL, OUTPUT);
    delayMicroseconds(20);
    digitalWrite(_pinSCL, HIGH);
    delayMicroseconds(20);
    digitalWrite(_pinSCL, LOW);
    delayMicroseconds(20);
    Wire.begin();
}

void WEI::write_reg(uint8_t addr, uint8_t reg, uint8_t data)
{
    _wire_com->beginTransmission(addr);
    _wire_com->write(reg);
    _wire_com->write(data);
    _wire_com->endTransmission();
    delay(40);
}

void WEI::read_reg(uint8_t addr, uint8_t reg, uint8_t *data)
{
    uint32_t time = millis();
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission();
    start_repeat();
    Wire.requestFrom(addr, 1);
    while (!Wire.available() && millis() - time < 1000)
        ;
    *data = Wire.read();
    delay(40);
}

void WEI::sys_read(uint32_t reg, uint32_t *data)
{
    uint8_t status = 0xFF;
    for (uint8_t i = 0; i < 4; i++)
    {
        write_reg(HIMAX_SYSTEM_ADDRESS, 0x00 + i, (reg >> (8 * i)) & 0xFF);
    }
    write_reg(HIMAX_SYSTEM_ADDRESS, 0x0C,
              0x00); // 0x0C: 0x00: read, 0x01: write
    while (status != 0x00)
    {
        read_reg(HIMAX_SYSTEM_ADDRESS, 0x0F, &status);
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        read_reg(HIMAX_SYSTEM_ADDRESS, 0x08 + i, (uint8_t *)data + i);
    }
}

void WEI::sys_write(uint32_t reg, uint32_t data)
{
    uint8_t status = 0xFF;
    uint32_t time = millis();
    for (uint8_t i = 0; i < 4; i++)
    {
        write_reg(HIMAX_SYSTEM_ADDRESS, 0x00 + i, (reg >> (8 * i)) & 0xFF);
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        write_reg(HIMAX_SYSTEM_ADDRESS, 0x04 + i, (data >> (8 * i)) & 0xFF);
    }
    write_reg(HIMAX_SYSTEM_ADDRESS, 0x0C,
              0x01); // 0x0C: 0x00: read, 0x01: write
    while (status != 0x00 && millis() - time < 3000)
    {
        read_reg(HIMAX_SYSTEM_ADDRESS, 0x0F, &status);
    }
}

uint16_t WEI::flash_crc16(const uint8_t *data, uint32_t len)
{
    uint32_t crc16 = 0xFFFF;
    uint32_t temp = 0;
    if (len == 0) return crc16;
    do
    {
        for (int i = 0, temp = (uint32_t)0xff & *data++; i < 8; i++, temp >>= 1)
        {
            if ((crc16 & 0x0001) ^ (temp & 0x0001))
                crc16 = (crc16 >> 1) ^ 0x8408;
            else
                crc16 = (crc16 >> 1);
        }
    } while (--len);
    return (uint16_t)(crc16);
}

void WEI::flash_write(const uint8_t *data, uint32_t len, bool crc)
{
    uint16_t crc16 = 0;
    _wire_com->beginTransmission(HIMAX_FLASH_ADDRESS);
    _wire_com->write(data, len);
    if (crc)
    {
        crc16 = flash_crc16(data, len);
        _wire_com->write((uint8_t *)&crc16, 2);
    }
    _wire_com->endTransmission();
    delay(40);
}

void WEI::flash_read(uint8_t *data, uint32_t len)
{
    uint32_t time = millis();
    _wire_com->requestFrom(HIMAX_FLASH_ADDRESS, len);
    while (!_wire_com->available() && millis() - time < 3000)
        ;
    for (uint32_t i = 0; i < len; i++)
    {
        data[i] = _wire_com->read();
    }
    delay(40);
}

uint32_t WEI::ID(void)
{
    uint32_t id = 0;
    sys_read(0xB00000FC, &id);
    return id;
}

const uint8_t loader_config[] = {
    0x42, 0x4C, 0x70, 0x02, 0x20, 0x00, 0x00, 0x03, 0xDE, 0xE1, 0x61, 0xED,
    0x76, 0xF6, 0x04, 0x06, 0x0E, 0x2B, 0xE4, 0xE5, 0xC4, 0x1E, 0x7C, 0x82,
    0xEF, 0xCF, 0x21, 0x2E, 0x99, 0xB4, 0xB1, 0x80, 0x1C, 0x25, 0xF9, 0x96,
    0x17, 0xE6, 0x89, 0x22, 0x4E, 0x44, 0x97, 0x11, 0xBE, 0x26, 0x3D, 0xC2,
    0x62, 0x0F, 0x38, 0xDD, 0x74, 0x9D, 0x9F, 0xD4, 0xC1, 0x55, 0x35, 0xD5,
    0xFE, 0xF5, 0x8B, 0xE9, 0xB5, 0xC0, 0x0A, 0x93, 0xFD, 0x0B, 0xF0, 0x29,
    0xC2, 0xF7, 0xA7, 0x9F, 0xC8, 0x28, 0x1F, 0xA2, 0xF5, 0x70, 0xBD, 0xC8,
    0x49, 0x93, 0x3D, 0x07, 0x9D, 0x05, 0xFE, 0xE7, 0x27, 0x8D, 0x0A, 0xED,
    0xAE, 0x04, 0x0B, 0xEF, 0x60, 0x7D, 0x63, 0x42, 0xAC, 0xE3, 0x7D, 0x3C,
    0x17, 0x7D, 0xFB, 0xC6, 0xE0, 0x72, 0xD6, 0x6E, 0x3F, 0x41, 0x46, 0xB5,
    0xC2, 0xDB, 0x22, 0x37, 0x01, 0xA6, 0x19, 0x40, 0x84, 0x1D, 0xD1, 0x89,
    0xF2, 0x23, 0x7E, 0x03, 0x3D, 0xB6, 0x0F, 0x35, 0x39, 0x60, 0x7C, 0xC7,
    0x19, 0x6D, 0xEF, 0x04, 0x2A, 0xB5, 0xF6, 0x6C, 0x3A, 0x63, 0xFD, 0x71,
    0x53, 0x55, 0x7A, 0x48, 0x2B, 0x25, 0x17, 0x0E, 0x2B, 0x8D, 0x95, 0x22,
    0xBC, 0x23, 0xBA, 0x9D, 0xF8, 0x8F, 0x35, 0x83, 0xC5, 0x19, 0xEE, 0xF0,
    0x6A, 0xE3, 0x4F, 0xC9, 0x4B, 0xA3, 0x13, 0x67, 0x2D, 0xCF, 0x46, 0x3E,
    0x07, 0xCE, 0x2B, 0x5D, 0x53, 0xAF, 0x8E, 0x12, 0x91, 0x2D, 0x72, 0x3A,
    0xF0, 0x54, 0xDD, 0x36, 0xAB, 0x7D, 0x5C, 0x36, 0xCC, 0xE3, 0x03, 0xDA,
    0x9D, 0xD2, 0x99, 0xC3, 0x7D, 0xD1, 0xB4, 0x9E, 0x89, 0x10, 0x99, 0x4A,
    0x04, 0x69, 0x54, 0xC9, 0x7C, 0xF5, 0xB9, 0x16, 0x09, 0x3D, 0x35, 0xAC,
    0x85, 0xAC, 0xC6, 0x3C, 0x21, 0x7B, 0x8F, 0xC4, 0xF5, 0x6B, 0x67, 0x2A,
    0x71, 0xEE, 0xDE, 0x12, 0x4C, 0xFD, 0x44, 0xDF, 0xE4, 0x3B, 0xFD, 0xF4,
    0x00, 0x01, 0x00, 0x01, 0xE0, 0x38, 0x91, 0xA1, 0x8D, 0x3E, 0xC2, 0x3E,
    0x6C, 0xF9, 0xE5, 0xCB, 0x14, 0xEA, 0x3F, 0x09, 0x74, 0x2A, 0xB1, 0xCC,
    0x55, 0x87, 0xD9, 0xD5, 0x58, 0x39, 0xA1, 0xD7, 0xB7, 0x36, 0x9D, 0x5B,
    0x3E, 0xDA, 0xE1, 0x91, 0xA8, 0x86, 0xDA, 0x7F, 0xD8, 0xC5, 0x1C, 0xBA,
    0x74, 0x18, 0xD3, 0x78, 0xA5, 0x8C, 0x24, 0x13, 0xA9, 0x63, 0x92, 0xB3,
    0xCB, 0x82, 0x01, 0x73, 0xE2, 0xD4, 0x0F, 0x1B, 0xC1, 0xAE, 0xB7, 0x96,
    0x1B, 0x2C, 0x23, 0xAD, 0xD1, 0x88, 0x5C, 0x2D, 0x4A, 0xEB, 0xC2, 0x46,
    0xA1, 0x6C, 0x12, 0xD6, 0xE5, 0x8E, 0x6B, 0x7B, 0xDB, 0xF7, 0xF8, 0x67,
    0x3D, 0x9F, 0xE6, 0xEC, 0x3B, 0x72, 0x75, 0x7C, 0x77, 0x34, 0xEA, 0x36,
    0xA7, 0xFA, 0x1D, 0x84, 0x8C, 0xB2, 0xD5, 0x49, 0x43, 0x94, 0xC7, 0xB0,
    0xEB, 0x04, 0x02, 0x13, 0x74, 0x5B, 0x47, 0xD6, 0x1A, 0xBB, 0x82, 0x24,
    0xE4, 0xED, 0x16, 0xFC, 0x6D, 0xDA, 0x8C, 0xCE, 0x77, 0xA1, 0xE0, 0xA5,
    0xB3, 0x29, 0xE0, 0xF5, 0x33, 0x84, 0xDC, 0x35, 0x32, 0xC7, 0xD0, 0xC0,
    0x76, 0xD9, 0x7C, 0x41, 0x73, 0xD6, 0xE9, 0xD8, 0xE0, 0xA5, 0x40, 0x9F,
    0x89, 0xA8, 0x63, 0xBE, 0x6F, 0x7B, 0x72, 0x01, 0x47, 0xE9, 0xD6, 0x46,
    0x43, 0x72, 0xCB, 0xC5, 0xF1, 0xF2, 0xCB, 0x0C, 0x1D, 0x67, 0x9F, 0x9C,
    0xAB, 0xDC, 0x3F, 0x4E, 0xFB, 0x0F, 0x69, 0x9C, 0x60, 0x08, 0xC5, 0x61,
    0xF6, 0x34, 0xA4, 0x2C, 0xB9, 0xE4, 0x5E, 0x7E, 0x69, 0x93, 0x46, 0x36,
    0xA1, 0xDF, 0x75, 0xD7, 0x84, 0x26, 0x5F, 0x41, 0xF9, 0xD3, 0xF0, 0xCD,
    0x56, 0xBB, 0xE7, 0x64, 0x7C, 0xDD, 0x86, 0x3D, 0x60, 0xC8, 0x44, 0xF3,
    0xD7, 0x85, 0x22, 0xA6, 0x3D, 0x25, 0xD4, 0xFF, 0xBA, 0xE1, 0x6C, 0xC2,
    0x2B, 0x89, 0xF5, 0x3E, 0x81, 0x04, 0xB4, 0xA1, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,
    0x80, 0x54, 0x59, 0x50, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x84, 0x79, 0x79, 0x66, 0x01, 0x00, 0x01, 0x00,
    0x80, 0xF0, 0xFA, 0x02, 0x00, 0x84, 0xD7, 0x17, 0x03, 0x10, 0x61, 0x38,
    0x8D, 0x08, 0xB6, 0xC0, 0x16, 0x32, 0x00, 0x00, 0x80, 0x10, 0x00, 0xD0,
    0x66, 0xC0, 0x66, 0x00, 0x00, 0x2A, 0xC0, 0x00, 0xFE, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x00, 0x00, 0x10, 0x0E, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0x80, 0xF0, 0xFA, 0x02, 0x00, 0x84, 0xD7, 0x17, 0x03, 0x10, 0x61, 0x38,
    0x8D, 0x08, 0xB6, 0xC0, 0x16, 0x32, 0x00, 0x00, 0x80, 0x10, 0x00, 0xD0,
    0x66, 0xC0, 0x66, 0x00, 0x00, 0x2A, 0xC0, 0x00, 0xFE, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

void WEI::enter_bootloader()
{
    uint32_t data;
    sys_read(0xB0000010, &data);
    sys_write(0xB0000010, 0x00000008);
    sys_read(0xB0000010, &data);

    sys_read(0xB0000520, &data);
    sys_write(0xB0000520, 0x00000002);
    sys_read(0xB0000520, &data);

    sys_read(0xB0000070, &data);
    sys_write(0xB0000070, 0x00000001);
    sys_read(0xB0000070, &data);

    sys_read(0xB0000020, &data);
    sys_write(0xB0000020, 0x80000002);
    // sys_read(0xB0000020, &data);
    delay(1000);
}

void WEI::load_config()
{
    const uint8_t cmd_start[] = {0x51, 0x02, 0x10, 0x00, 0x12, 0x00, 0x00,
                                 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x10,
                                 0x02, 0x00, 0xFC, 0x02, 0x00, 0x00};
    uint8_t cmd_flash[WEI_FALSH_PAGE_SIZE + 4] = {0x51, 0x03, 0x80, 0x00};
    flash_write(cmd_start, sizeof(cmd_start), true);
    delay(50);
    for (int i = 0; i < sizeof(loader_config); i += WEI_FALSH_PAGE_SIZE)
    {
        memcpy(cmd_flash + 4, loader_config + i, WEI_FALSH_PAGE_SIZE);
        flash_write(cmd_flash, sizeof(cmd_flash), true);
        delay(40);
    }
    if (sizeof(loader_config) % WEI_FALSH_PAGE_SIZE)
    {
        cmd_flash[2] = sizeof(loader_config) % WEI_FALSH_PAGE_SIZE;
        cmd_flash[3] = 0x00;
        memcpy(cmd_flash + 4,
               loader_config + sizeof(loader_config) -
                   (sizeof(loader_config) % 128),
               sizeof(loader_config) % 128);
        flash_write(cmd_flash, sizeof(loader_config) % WEI_FALSH_PAGE_SIZE + 4,
                    true);
        delay(40);
    }

    const uint8_t cmd_done[] = {0x51, 0x04, 0x00, 0x00};
    flash_write(cmd_done, sizeof(cmd_done), true);
    delay(1000);
} 

void WEI::reset()
{
    const uint8_t cmd_reset[] = {0x50, 0x06, 0x00, 0x00};
    flash_write(cmd_reset, sizeof(cmd_reset), true);
    delay(50);
}

void WEI::erase()
{
    enter_bootloader();
    load_config();
    reset();
}