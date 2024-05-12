---
name: Bug report
about: Create a report to help us improve
title: ''PIN_WIRE_SDA' was not declared in this scope'
labels: ''
assignees: ''

---

**Describe the bug**
In file included from C:\Users\6766s\OneDrive\Documents\Arduino\object_detection\object_detection.ino:3:
c:\Users\6766s\OneDrive\Documents\Arduino\libraries\Seeed_Arduino_GroveAI/Seeed_Arduino_GroveAI.h:230:53: error: 'PIN_WIRE_SDA' was not declared in this scope
     WEI(TwoWire &wire_com = Wire, uint32_t pinSDA = PIN_WIRE_SDA, uint32_t pinSCL = PIN_WIRE_SCL): _wire_com(&wire_com), _pinSDA(pinSDA), _pinSCL(pinSCL) {}
                                                     ^~~~~~~~~~~~
c:\Users\6766s\OneDrive\Documents\Arduino\libraries\Seeed_Arduino_GroveAI/Seeed_Arduino_GroveAI.h:230:53: note: suggested alternative: 'FUN_IE_S'
     WEI(TwoWire &wire_com = Wire, uint32_t pinSDA = PIN_WIRE_SDA, uint32_t pinSCL = PIN_WIRE_SCL): _wire_com(&wire_com), _pinSDA(pinSDA), _pinSCL(pinSCL) {}
                                                     ^~~~~~~~~~~~
                                                     FUN_IE_S
c:\Users\6766s\OneDrive\Documents\Arduino\libraries\Seeed_Arduino_GroveAI/Seeed_Arduino_GroveAI.h:230:85: error: 'PIN_WIRE_SCL' was not declared in this scope
     WEI(TwoWire &wire_com = Wire, uint32_t pinSDA = PIN_WIRE_SDA, uint32_t pinSCL = PIN_WIRE_SCL): _wire_com(&wire_com), _pinSDA(pinSDA), _pinSCL(pinSCL) {}
                                                                                     ^~~~~~~~~~~~
c:\Users\6766s\OneDrive\Documents\Arduino\libraries\Seeed_Arduino_GroveAI/Seeed_Arduino_GroveAI.h:230:85: note: suggested alternative: 'FUN_IE_S'
     WEI(TwoWire &wire_com = Wire, uint32_t pinSDA = PIN_WIRE_SDA, uint32_t pinSCL = PIN_WIRE_SCL): _wire_com(&wire_com), _pinSDA(pinSDA), _pinSCL(pinSCL) {}
                                                                                     ^~~~~~~~~~~~
                                                                                     FUN_IE_S

exit status 1

Compilation error: exit status 1

**To Reproduce**
While compiling object detection example for SEEED XIAO_ESP32C3

**Expected behavior**
Successful compilation

**Screenshots**
If applicable, add screenshots to help explain your problem.

**Desktop (please complete the following information):**
 - OS: windows 11 Pro 23H2
 

**Additional context**
Add any other context about the problem here.
