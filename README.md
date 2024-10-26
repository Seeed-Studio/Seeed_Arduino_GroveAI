# Seeed_Arduino_GroveAI
English | [中文](./README_zh_CN.md)

## Overview
Seeed_Arduino_GroveAI is an Arduino library designed specifically for the Grove Vision AI V1, aimed at helping developers easily integrate and utilize the features of Grove Vision AI V1.

## Recommendation
We recommend using the more powerful [Grove Vision AI V2](https://www.seeedstudio.com/Grove-Vision-AI-Module-V2-p-5851.html), which offers better processing capabilities and additional features, making it suitable for more complex applications.

<div>
    <h3>Grove Vision AI V1</h3>
    <a href="https://www.seeedstudio.com/Grove-Vision-AI-Module-p-5457.html">Grove Vision AI V1</a>
    <img src="https://media-cdn.seeedstudio.com/media/catalog/product/cache/bb49d3ec4ee05b6f018e93f896b8a25d/1/1/114992866_front-05.jpg" alt="Grove Vision AI V1" style="max-width: 300px;">
</div>

<div>
    <h3>Grove Vision AI V2</h3>
    <a href="https://www.seeedstudio.com/Grove-Vision-AI-Module-V2-p-5851.html">Grove Vision AI V2</a>
    <img src="https://media-cdn.seeedstudio.com/media/catalog/product/cache/bb49d3ec4ee05b6f018e93f896b8a25d/4/-/4-101021112-grove-vision-ai-module-v2-45back.jpg" alt="Grove Vision AI V2" style="max-width: 300px;">
</div>

## Installation
1. Clone this repository into your Arduino libraries directory:
   ```bash
   git clone https://github.com/Seeed-Studio/Seeed_Arduino_GroveAI.git
   ```

2. In the Arduino IDE, select "File" -> "Examples" -> "Seeed_Arduino_GroveAI" to access example sketches.

## Usage Tutorial
Here are the basic steps to use Grove Vision AI V1:

```cpp
#include "Seeed_Arduino_GroveAI.h"
#include <Wire.h>

GroveAI ai(Wire);
uint8_t state = 0;

void setup() {
    Wire.begin();
    Serial.begin(115200);
    while (!Serial) {
        // Wait for Serial to be ready
    }

    Serial.println("Initializing...");
    if (ai.begin(ALGO_OBJECT_DETECTION, MODEL_EXT_INDEX_1)) { // Object detection with external model 1
        Serial.print("Version: 0x");
        Serial.println(ai.version(), HEX);
        Serial.print("ID: 0x");
        Serial.println(ai.id(), HEX);
        Serial.print("Algorithm: ");
        Serial.println(ai.algo());
        Serial.print("Model: ");
        Serial.println(ai.model());
        Serial.print("Confidence: ");
        Serial.println(ai.confidence());
        state = 1;
    } else {
        Serial.println("Algorithm initialization failed.");
    }
}

void loop() {
    if (state == 1) {
        uint32_t tick = millis();
        if (ai.invoke()) { // Start invocation
            while (ai.state() != CMD_STATE_IDLE) { // Wait for invocation to finish
                delay(20);
            }

            uint8_t len = ai.get_result_len(); // Get the number of detected objects
            if (len > 0) {
                Serial.print("Time consumed: ");
                Serial.println(millis() - tick);
                Serial.print("Number of detected objects: ");
                Serial.println(len);

                for (int i = 0; i < len; i++) {
                    object_detection_t data; // Prepare to receive data
                    ai.get_result(i, (uint8_t *)&data, sizeof(object_detection_t)); // Get result
                    printDetectionResult(data);
                }
            } else {
                Serial.println("No objects detected.");
            }
        } else {
            delay(1000);
            Serial.println("Invocation failed.");
        }
    } else {
        state = 0; // Reset state
    }
}

void printDetectionResult(const object_detection_t &data) {
    Serial.println("Result: Detected");
    Serial.print("Object: ");
    Serial.print(data.target);
    Serial.print("\tX: ");
    Serial.print(data.x);
    Serial.print("\tY: ");
    Serial.print(data.y);
    Serial.print("\tWidth: ");
    Serial.print(data.w);
    Serial.print("\tHeight: ");
    Serial.print(data.h);
    Serial.print("\tConfidence: ");
    Serial.println(data.confidence);
}
```

## FAQ
[FAQ](./FAQs.md)


## Contributing
Contributions are welcome! Please submit a pull request or raise issues on the Issues page.

## License
This project is licensed under the MIT License. For more details, please refer to the [LICENSE](LICENSE) file.


