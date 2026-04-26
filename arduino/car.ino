#define __RFID_SERIAL_DEBUG__

// comment this macro for maze running; uncomment this for local test
// #define __DISABLE_BT__

// #define __DEBUG_SERIAL__

#ifdef __DEBUG_SERIAL__
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#include "motor.h"

#ifndef __DISABLE_BT__
#include "ble.h"
#endif

#include "rfid.h"
#include "tracing.h"
#include "ir_test.h"

//A:left B:right
// ---------------------------

void setup() {
  #ifdef __DEBUG_SERIAL__
  Serial.begin(115200);
  #endif

  #ifndef __DISABLE_BT__
  hm10.bleSetup();
  #endif

  rfidSetup();
  motorSetup();
  tracingSetup();

  Serial.println("System Ready. Waiting 0.5s...");
  delay(500);
  //
}

void loop() {

  // motorWriting(200, 200);
  // return;

  if(ended) {
    return;
  }

  if(!activated) {
    checkActivated();
    return;
  }

  // irTestLoop();
  // delay(300);
  // return;
  tracingLoop();

  delay(10);
}

