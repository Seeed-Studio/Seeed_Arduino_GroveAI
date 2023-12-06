
#include "Seeed_Arduino_GroveAI_V2.h"

GroveAI_V2 ai;
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(1000);
  Serial.println("start");

  if (ai.begin()) {
    Serial.print("Version: ");
    Serial.println(ai.version());
    Serial.print("ID: 0x");
    Serial.println(ai.id(), HEX);
  }
  else {
    Serial.println("Algo begin failed");
  }
}

void loop()
{
  ai_result_t result;
  ai.invoke();
  if (ai.get_result(&result)) {
    print_result(result);
  }
}

void print_result(ai_result_t result)
{
  switch (result._type)
  {
  case RESULT_TYPE_BOX:
    Serial.println("RESULT_TYPE_BOX");
    for (uint8_t i = 0; i < result._num; i++) {
      Serial.print("x:");
      Serial.print(result._result[i]._box.x);
      Serial.print(" y:");
      Serial.print(result._result[i]._box.y);
      Serial.print(" w:");
      Serial.print(result._result[i]._box.w);
      Serial.print(" h:");
      Serial.print(result._result[i]._box.h);
      Serial.print(" score:");
      Serial.println(result._result[i]._box.score);
      Serial.print(" target:");
      Serial.println(result._result[i]._box.target);
    }
    break;
  default:
    break;
  }
}