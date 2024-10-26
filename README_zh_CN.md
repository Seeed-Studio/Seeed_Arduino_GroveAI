# Grove Vision AI

[English](./README.md) | 中文

# Seeed_Arduino_GroveAI

## 推荐
我们推荐使用更强性能的 [Grove Vision AI V2](https://www.seeedstudio.com/Grove-Vision-AI-Module-V2-p-5851.html)，它提供更好的处理能力和更多的功能，适合更复杂的应用场景。

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

## 概述
Seeed_Arduino_GroveAI 是一个专为 Grove Vision AI V1 设计的 Arduino 库，旨在帮助开发者轻松集成和使用 Grove Vision AI V1 的功能。

## 安装
1. 将此库克隆到您的 Arduino 库目录中：
   ```bash
   git clone https://github.com/Seeed-Studio/Seeed_Arduino_GroveAI.git
   ```

2. 在 Arduino IDE 中，选择“文件” -> “示例” -> “Seeed_Arduino_GroveAI” 来获取库的使用示例。

## 使用教程
以下是使用 Grove Vision AI V1 的基本步骤：

```cpp
#include "Seeed_Arduino_GroveAI.h"
#include <Wire.h>

GroveAI ai(Wire);
uint8_t state = 0;

void setup() {
    Wire.begin();
    Serial.begin(115200);
    while (!Serial) {
        // 等待串口准备好
    }

    Serial.println("Initializing...");
    if (ai.begin(ALGO_OBJECT_DETECTION, MODEL_EXT_INDEX_1)) { // 使用外部模型 1 进行目标检测
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
        if (ai.invoke()) { // 开始调用
            while (ai.state() != CMD_STATE_IDLE) { // 等待调用完成
                delay(20);
            }

            uint8_t len = ai.get_result_len(); // 获取检测到的对象数量
            if (len > 0) {
                Serial.print("Time consumed: ");
                Serial.println(millis() - tick);
                Serial.print("Number of detected objects: ");
                Serial.println(len);

                for (int i = 0; i < len; i++) {
                    object_detection_t data; // 准备接收数据
                    ai.get_result(i, (uint8_t *)&data, sizeof(object_detection_t)); // 获取结果
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
        state = 0; // 重置状态
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
[FAQ](./FAQs_zh_CN.md)

## 贡献
欢迎贡献代码和建议！请提交 Pull Request 或在 Issues 页面中提出问题。

## 许可证
该项目遵循 MIT 许可证，具体信息请参阅 [LICENSE](LICENSE) 文件。




