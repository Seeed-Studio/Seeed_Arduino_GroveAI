# Protocol

This document is a protocol interaction document that describes how to provide instructions for I2C interaction with Grove AI and SenseCAP AI

## 1. Basic Information
**0x62** is the default address of Grove AI and SenseCAP AI. Checksums are optional depending on the firmware.

## 2. Command Format
- Read Command Format
    - Read Request Format

        |Feature|Command|[*Parameter*]|
        |:---:|:---:|:---:|
        |1 byte|1 byte|n byte|

    - Read Response Format
        |Feature|Command|Data|[*Checksum*]|
        |:---:|:---:|:---:|:---:|
        |1 byte|1 byte|n byte|1 byte|



- Write Command Format

    - Write Request Format

        |Feature|Command|Data|[*Checksum*]|
        |:---:|:---:|:---:|:---:|
        |1 byte|1 byte|n byte|1 byte|

- Checksum Rules

    - Checksum Calculation

        The checksum is calculated by the following CRC8 polynomial:

        ```
        CRC8: x^8+x^5+x^4+x^0
        Initial value: 0x00
        ```

## 3. Command List

- System: 0x80

    |Feature|Command|Description|Read/Write|Length|
    |---|---|---|---|---|
    |0x80|0x01|Read Firmware Version|Read|2|
    |0x80|0x02|Read Hardware ID|Read|2|
    |0x80|0x03|Read Status|Read|1|
    |0x80|0x04|Read Error Code|Read|1|
    |0x80|0x20|Reboot|Write|0|

    - Read Firmware Version

        Read firmware version, return 2 bytes, the first byte is major version, the second byte is minor version.

        example:
        ```
        request: 0x80 0x01
        response: 0x01 0x30 [checksum]
        ```
    
    - Read Hardware ID

        Read hardware id, return 2 bytes, the first byte is major id, the second byte is minor id.

        example:
        ```
        request: 0x80 0x02
        response: 0x01 0x00 [checksum]
        ```
    
    - Read Status

        Read status, return 1 byte, the value is 0x00 when the device is ready, the value is 0x01 when the device is busy.

        |Status|Description|
        |:---:|---|
        |0x00|Ready|
        |0x01|Busy|
        |0x02|Error|

        example:
        ```
        request: 0x80 0x03
        response: 0x00 [checksum]
        ```

    - Read Error Code

        Read error code, return 1 byte, the value is 0x00 when the device is ready, the value is 0x01 when the device is busy.

        |Error Code|Description|
        |:---:|---|
        |0x00|No Error|
        |0x01|Model Invalid Or Not Existent|
        |0x02|Model Parsing Failure|
        |0x03|Memory Allocation Failure|
        |0x04|Pre-processed Data Failure|
        |0x05|Post-processed Data Failure|
        |0x06|Algorithm Initialization Failure|
        |0x07|Algorithm Mismatch With the Model|
        |0x08|Algorithm Parameter Invalid|
        |0x09|Algorithm Invoke Failure|
        |0x0A|Algorithm Get Results Failure|
        |0x0B|Sensor Not Supported Yet|
        |0x0C|Sensor Parameter Invalid|
        |0x0D|Camera Initialization Failure|
        |0x0E|Camera De-Initialization Failure|
        |0x0F|Camera Parameter Invalid|

        example:
        ```
        request: 0x80 0x04
        response: 0x00 [checksum]
        ```

    - Reboot

        reboot the device, no response.

        example:
        ```
        request: 0x80 0x20
        ```


- ALGO: 0xA0

    |Feature|Command|Description|Read/Write|Length|
    |---|---|---|---|---|
    |0xA0|0x00|Read Algorithm Configuration|Read|1|
    |0xA0|0x01|Write Algorithm Configuration|Write|1|
    |0xA0|0x10|Read Selected Model|Read|1|
    |0xA0|0x11|Write Selected Model|Write|1|
    |0xA0|0x12|Read Valid Model|Read|4|
    |0xA0|0x40|Read Confidence|Read|1|
    |0xA0|0x41|Write Confidence|Write|1|
    |0xA0|0xA0|Start Inference|Write|0|
    |0xA0|0xA1|Read Inference Result Set Length|Read|2|
    |0xA0|0xA2|Read Inference Result|Read|Variable|
    |0xA0|0xEE|Save Configuration|Write|0|
    |0xA0|0xEF|Erase Configuration|Write|0|

    - Read Algorithm Configuration

        Read algorithm configuration, return 1 byte, and the value is the algorithm configuration.

        |Algorithm Configuration|Description|
        |:---:|---|
        |0x00|Object Detection|
        |0x01|Object Counting|
        |0x02|Image Classification|
        |0x03|Meter Reading|

        example:
        ```
        request: 0xA0 0x00 
        response: 0x00 [checksum]
        ```

    - Write Algorithm Configuration

        write algorithm configuration, need a 1 byte parameter, and the value is the algorithm configuration.

        |Algorithm Configuration|Description|
        |:---:|---|
        |0x00|Object Detection|
        |0x01|Object Counting|
        |0x02|Image Classification|
        |0x03|Meter Reading|

        example:
        ```
        request: 0xA0 0x01 0x00 [checksum]
        ```
    
    - Read Selected Model

        Read selected model, return 1 byte, and the value is the selected model.

        |Selected Model|Description|
        |:---:|---|
        |0x01|External Model 0|
        |0x02|External Model 1|
        |0x03|External Model 2|
        |0x04|External Model 3|
        |0x11|Person Detction|
        |0x12|Person & Panda Classify|
        |0x13|Meter Reading|

        example:
        ```
        request: 0xA0 0x10
        response: 0x01 [checksum]
        ```
    
    - Write Selected Model

        Write selected model, need a 1 byte parameter, and the value is the selected model.

        |Selected Model|Description|
        |:---:|---|
        |0x01|External Model 0|
        |0x02|External Model 1|
        |0x03|External Model 2|
        |0x04|External Model 3|
        |0x11|Person Detction|
        |0x12|Person & Panda Classify|
        |0x13|Meter Reading|

        example:
        ```
        request: 0xA0 0x11 0x01 [checksum]
        ```

    - Read Valid Model

        Read valid model, return 4 bytes, and the value is the valid model. The bit is 1 when the model is valid, and the bit is 0 when the model is invalid.

        |0|1|2|3|...|17|18|19|31|
        |:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
        |External Model 0|External Model 1|External Model 2|External Model 3|...|Person Detction|Person & Panda Classify|Meter Reading|Unkown|

        example:
        ```
        request: 0xA0 0x12
        response: 0x00 0x00 0x00 0x01 [checksum]
        ```
    
    - Read Confidence

        Read confidence, return 1 byte, and the value is the confidence. The confidence is the threshold of the inference result, and the value is 0~100.

        example:
        ```
        request: 0xA0 0x40
        response: 0x00 [checksum]
        ```

    - Write Confidence

        Write confidence, need a 1 byte parameter, and the value is the confidence. The confidence is the threshold of the inference result, and the value is 0~100.

        example:
        ```
        request: 0xA0 0x41 0x00 [checksum]
        ```
    
    - Start Inference
    
        Start inference, no response.
    
        example:
        ```
        request: 0xA0 0xA0
        ```

    - Read Inference Result Set Length

        Read inference result set length, return 2 bytes, and the value is the inference result set length.

        example:
        ```
        request: 0xA0 0xA1
        response: 0x00 0x01 [checksum]
        ```

    - Read Inference Result

        Read inference result, need a 2 byte parameter, and the value is the inference result index. Return the inference result, and the length is variable, see the [Algorithm Result Format](#4-algorithm-result-format).

        example:
        ```
        request: 0xA0 0xA2 0x00 0x00
        response: 0x00 0x05 0x00 0x05 0x00 0x20 0x00 0x20 0x00 0x38 [checksum]
        ```
    
    - Save Configuration

        Save configuration, no response. The configuration will be saved to flash, and the configuration will be loaded when the device is powered on.

        example:
        ```
        request: 0xA0 0xEE
        ```
    
    - Erase Configuration

        Erase configuration, no response.

        example:
        ```
        request: 0xA0 0xEF
        ```


## 4. Algorithm Result Format

- Object Detection

    object detection result format:

    |Byte|Name|Description|
    |:---:|---|---|
    |0-1|X|X coordinate of the object|
    |2-3|Y|Y coordinate of the object|
    |4-5|Width|Width of the object|
    |6-7|Height|Height of the object|
    |8|Confidence|Confidence of the object|
    |9|Target|Target of the object|


- Object Counting

    object counting result format:

    |Byte|Name|Description|
    |:---:|---|---|
    |0|Target|Target of the object|
    |1|Count|Count of the object|

- Image Classification

    image classification result format:

    |Byte|Name|Description|
    |:---:|---|---|
    |0|Target|Target of the image|
    |1|Confidence|Confidence of the image|


- Meter Reading

    meter reading result format:

    |Byte|Name|Description|
    |:---:|---|---|
    |0-1|X|X coordinate of the meter|
    |2-3|Y|Y coordinate of the meter|
    |4-7|Value|Value of the meter, int32_t|

    **The unit is 0.0001. For example, the value is 1234567, and the meter reading is 123.4567.**

   
    





