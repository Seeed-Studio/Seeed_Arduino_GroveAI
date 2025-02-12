#include "Seeed_Arduino_GroveAI.h"
#include <Wire.h>

GroveAI ai(Wire);
uint8_t state = 0;
void setup()
{
  Wire.begin();
  Serial.begin(115200);
  while (!Serial)
  {
    /* code */
  }

  Serial.println("begin");
  if (ai.begin(ALGO_OBJECT_DETECTION, MODEL_EXT_INDEX_1)) // Object detection and externel model 1
  {
    Serial.print("Version: 0x");
    Serial.println(ai.version(), HEX);
    Serial.print("ID: 0x");
    Serial.println(ai.id(), HEX);
    Serial.print("Algo: ");
    Serial.println(ai.algo());
    Serial.print("Model: ");
    Serial.println(ai.model());
    Serial.print("Confidence: ");
    Serial.println(ai.confidence());
    state = 1;
  }
  else
  {
    Serial.println("Algo begin failed.");
  }
}

void loop()
{
  if (state == 1)
  {
    uint32_t tick = millis();
    if (ai.invoke()) // begin invoke
    {
      while (1) // wait for invoking finished
      {
        CMD_STATE_T ret = ai.state();
        if (ret == CMD_STATE_IDLE)
        {
          break;
        }
        delay(20);
      }

      uint8_t len = ai.get_result_len(); // receive how many people detect
      if (len)
      {
        int time1 = millis() - tick;
        Serial.print("Time consuming: ");
        Serial.println(time1);
        Serial.print("Number of people: ");
        Serial.println(len);
        object_detection_t data; // get data

        for (int i = 0; i < len; i++)
        {
          ai.get_result(i, (uint8_t *)&data, sizeof(object_detection_t)); // get result
          Serial.println("result:detected");
          Serial.print("Detecting object: ");
          Serial.print(data.target);
          Serial.print("\tx:");
          Serial.print(data.x);
          Serial.print("\ty:");
          Serial.print(data.y);
          Serial.print("\tw:");
          Serial.print(data.w);
          Serial.print("\th:");
          Serial.print(data.h);
          Serial.print("\tconfidence:");
          Serial.println(data.confidence);
        }
      }
      else
      {
        Serial.println("No identification");
      }
    }
    else
    {
      delay(1000);
      Serial.println("Invoke Failed.");
    }
  }
  else
  {
    state = 0;
  }
}
