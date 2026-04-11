#include "ble.h"
#include "motor.h"
#include "tracing.h"
#include "rfid.h"
// #include "ir_test.h"

//A:left B:right
// ---------------------------

Communicator hm10;

void setup() {
  hm10.bleSetup();
  rfidSetup();
  motorSetup();
  tracingSetup();

  addTurn(1); addTurn(2); addTurn(0); addTurn(3);

  Serial.println("System Ready. Waiting 0.5s...");
  delay(500);
  //
}

void loop() {
  // tracingLoop();
  // irTestLoop();

  byte* id = rfidRead();

  delay(100);
}

