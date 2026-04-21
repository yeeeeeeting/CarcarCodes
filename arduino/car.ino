#define __RFID_SERIAL_DEBUG__

#include "motor.h"
#include "ble.h"
#include "rfid.h"
#include "tracing.h"
#include "ir_test.h"

//A:left B:right
// ---------------------------

void setup() {
  // Serial.begin(115200);
  hm10.bleSetup();
  rfidSetup();
  motorSetup();
  tracingSetup();

  Serial.println("System Ready. Waiting 0.5s...");
  delay(500);
  //
}

void loop() {

  if(!activated) {
    checkActivated();
    return;
  }

  // irTestLoop();
  tracingLoop();

  delay(50);
}

