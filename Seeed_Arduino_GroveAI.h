/***
 * Seeed_Arduino_GroveAI.h
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

#ifndef SEEED_ARDUINO_GROVEAI_H
#define SEEED_ARDUINO_GROVEAI_H

#include <Arduino.h>
#include <Wire.h>

#include <stdint.h>

/* Exported macros ---------------------------------------------------------*/
#define GROVE_AI_ADDRESS (0x62)
#define HIMAX_SYSTEM_ADDRESS (0x79)
#define HIMAX_FLASH_ADDRESS (0x62)

#define WEI_FALSH_PAGE_SIZE (128)

#define GROVE_AI_CAMERA_ID 0x0100

#define VISION_AI_CAMERA_ID 0x0200

#define FEATURE_SYSTEM 0x80
#define FEATURE_GPIO 0x90
#define FEATURE_IMU 0x93
#define FEATURE_ALGO 0xA0

#define FEATURE_INVAILD 0xFF
#define CMD_INVAILD 0xFF

#define CMD_SYS_READ_VERSION 0x01
#define CMD_SYS_VERSION_LENGTH 2
#define CMD_SYS_READ_ID 0x02
#define CMD_SYS_ID_LENGTH 2
#define CMD_SYS_READ_STATE 0x03
#define CMD_SYS_STATE_LENGTH 1
#define CMD_SYS_READ_ERROR_CODE 0x04
#define CMD_SYS_RESET 0x20

#define CMD_ALGO_READ_ALGO 0x00
#define CMD_ALGO_WRITE_ALGO 0x01
#define CMD_ALGO_ALGO_LENGTH 0x01
#define CMD_ALGO_READ_MODEL 0x10
#define CMD_ALGO_WRITE_MODEL 0x11
#define CMD_ALGO_MODEL_LENGTH 0x01
#define CMD_ALGO_READ_VALID_MODEL 0x12
#define CMD_ALGO_VALID_MODEL_LENGTH 0x04
#define CMD_ALGO_READ_PERIOD 0x20
#define CMD_ALGO_WRITE_PERIOD 0x21
#define CMD_ALGO_PERIOD_LENGTH 0x04
#define CMD_ALGO_READ_CONFIDENCE 0x40
#define CMD_ALGO_WRITE_CONFIDENCE 0x41
#define CMD_ALGO_CONFIDENCE_LENGTH 0x01
#define CMD_ALGO_READ_IOU 0x50
#define CMD_ALGO_WRITE_IOU 0x51
#define CMD_ALGO_IOU_LENGTH 0x01
#define CMD_ALGO_INVOKE 0xA0
#define CMD_ALGO_READ_RET_LEN 0xA1
#define CMD_ALGO_READ_RET_LEN_LENGTH 0x02
#define CMD_ALGO_READ_RET 0xA2
#define CMD_ALGO_CONFIG_SAVE 0xEE
#define CMD_ALGO_CONFIG_CLEAR 0xEF

#define CMD_GPIO_READ_STATE 0x00
#define CMD_GPIO_READ_STATE_LENGTH 0x01
#define CMD_GPIO_WRITE_STATE 0x01
#define CMD_GPIO_WRITE_STATE_LENGTH 0x02

#define CMD_IMU_READ_SAMPLE_STATE 0x00
#define CMD_IMU_READ_SAMPLE_LENGTH 0x01
#define CMD_IMU_WRITE_SAMPLE_STATE 0x02
#define CMD_IMU_WRITE_SAMPLE_LENGTH 0x01
#define CMD_IMU_READ_ACC_AVAIABLE 0xA0
#define CMD_IMU_READ_ACC_AVAIABLE_LENGTH 0x01
#define CMD_IMU_READ_ACC_X 0xA1
#define CMD_IMU_READ_ACC_X_LENGTH 0x04
#define CMD_IMU_READ_ACC_Y 0xA2
#define CMD_IMU_READ_ACC_Y_LENGTH 0x04
#define CMD_IMU_READ_ACC_Z 0xA3
#define CMD_IMU_READ_ACC_Z_LENGTH 0x04
#define CMD_IMU_READ_GYRO_AVAIABLE 0xB0
#define CMD_IMU_READ_GYRO_AVAIABLE_LENGTH 0x01
#define CMD_IMU_READ_GYRO_X 0xB1
#define CMD_IMU_READ_GYRO_X_LENGTH 0x04
#define CMD_IMU_READ_GYRO_Y 0xB2
#define CMD_IMU_READ_GYRO_Y_LENGTH 0x04
#define CMD_IMU_READ_GYRO_Z 0xB3
#define CMD_IMU_READ_GYRO_Z_LENGTH 0x04

#define CMD_IMU_SAMPLE_AVAILABLE 0x01
#define CMD_IMU_ACC_AVAILABLE 0x01
#define CMD_IMU_GYRO_AVAILABLE 0x01


typedef enum
{
    CMD_GPIO_MODE_INPUT = 0x00,
    CMD_GPIO_MODE_OUTPUT = 0x01,
    CMD_GPIO_MODE_NONE = 0x02
} CMD_GPIO_MODE_T;

typedef enum
{
    CMD_GPIO_STATE_LOW = 0x00,
    CMD_GPIO_STATE_HIGH = 0x01,
} CMD_GPIO_STATE_T;

typedef enum
{
    ALGO_OBJECT_DETECTION = 0,
    ALGO_OBJECT_COUNTING = 1,
    ALGO_IMAGE_CLASSIFICATION = 2,
    ALGO_METER_READING = 3,
    ALGO_MAX,
} ALGO_INDEX_T;

typedef enum
{
    MODEL_PRE_INDEX_0 = 0x00,
    MODEL_EXT_INDEX_1 = 0x01,
    MODEL_EXT_INDEX_2 = 0x02,
    MODEL_EXT_INDEX_3 = 0x03,
    MODEL_EXT_INDEX_4 = 0x04,
    MODEL_PRE_INDEX_1 = 0x11,
    MODEL_MAX = 0x20,
} MODEL_INDEX_T;

/**
 * @brief cmd state enum
 *
 * This enumeration type is used to record CMD interaction status
 *
 */
typedef enum cmd_state
{
    CMD_STATE_IDLE = 0x0,
    CMD_STATE_RUNNING = 0x01,
    CMD_STATE_ERROR = 0x02
} CMD_STATE_T;

/**
 * @brief sys event
 *
 *
 *
 */
typedef struct
{
    uint16_t version;
    uint16_t id;
    CMD_STATE_T state;
} cmd_sys_event_t;

/**
 * @brief
 *
 *
 *
 */
typedef struct
{
    uint8_t algo;
    uint8_t model;
    uint8_t state;
    uint8_t iou;
    uint8_t confidence;
} cmd_algo_event_t;

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    uint8_t confidence;
    uint8_t target;
} object_detection_t;

typedef struct
{
    uint8_t target;
    uint8_t confidence;
} image_classification_t;

typedef struct
{
    uint8_t target;
    uint8_t count;
} object_counting_t;

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint32_t value;
} meter_reading_t;

class WEI
{
private:
    TwoWire *_wire_com;
    uint32_t _pinSDA;
    uint32_t _pinSCL;

public:
    WEI(TwoWire &wire_com = Wire, uint32_t pinSDA = PIN_WIRE_SDA, uint32_t pinSCL = PIN_WIRE_SCL): _wire_com(&wire_com), _pinSDA(pinSDA), _pinSCL(pinSCL) {}
    ~WEI() {};
    uint32_t ID();
    void enter_bootloader();
    void load_config();
    void erase();
    void reset();

private:
    void start_repeat();
    void read_reg(uint8_t addr, uint8_t reg, uint8_t *data);
    void write_reg(uint8_t addr, uint8_t reg, uint8_t data);
    void sys_read(uint32_t reg, uint32_t *data);
    void sys_write(uint32_t reg, uint32_t data);
    void flash_read(uint8_t *data, uint32_t len);
    void flash_write(const uint8_t *data, uint32_t len, bool crc = true);
    uint16_t flash_crc16(const uint8_t *data, uint32_t len);
};
class GroveAI
{
private:
    cmd_sys_event_t _system;
    cmd_algo_event_t _algo;
    TwoWire *_wire_com;
    uint8_t _slave_addr;
    int32_t _signal_pin;
    bool _crc_enable;

public:
    GroveAI(TwoWire &wire, uint8_t address = GROVE_AI_ADDRESS, int32_t signal_pin = -1, bool _crc_enable = false);
    ~GroveAI();
    bool begin(ALGO_INDEX_T algo, MODEL_INDEX_T model, uint8_t confidence = 50);
    uint16_t version();
    uint16_t id();
    CMD_STATE_T state();
    ALGO_INDEX_T algo();
    MODEL_INDEX_T model();
    uint8_t confidence();
    bool invoke();
    void reset();
    bool config_save();
    bool config_clear();
    uint16_t get_result_len();
    uint32_t get_vaild_model();
    bool get_result(uint16_t index, uint8_t *buff, uint8_t len);

protected:
    bool read(uint8_t feature, uint8_t cmd, uint8_t *param, uint8_t param_len, uint8_t *buf, uint16_t len);
    void write(uint8_t feature, uint8_t cmd, uint8_t *buf, uint16_t len);
    ALGO_INDEX_T set_algo(ALGO_INDEX_T algo);
    MODEL_INDEX_T set_model(MODEL_INDEX_T model);
    uint8_t set_confidence(uint8_t confidence);
    uint8_t set_iou(uint8_t iou);
    ALGO_INDEX_T get_algo();
    MODEL_INDEX_T get_model();
    uint8_t get_confidence();
    uint8_t get_iou();
    bool check_busy();
};

#endif
