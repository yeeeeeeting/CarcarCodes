#include "ble.h"
#include "motor.h"
#include "tracing.h"
#include "ir_test.h"

//A:left B:right
// ---------------------------
//include RFID Lab
#include <SPI.h>
#include <MFRC522.h>

MFRC522 *mfrc522; 
//define RFID Pin
#define RST_PIN 3
#define SS_PIN  2

void setup() {
  bleSetup();
  //RFID input
  SPI.begin();
  mfrc522 = new MFRC522(SS_PIN, RST_PIN);
  mfrc522->PCD_Init();
  Serial.println(F("Read UID on a MIFARE PICC:"));

  tracingSetup();

  //馬達
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  //
  
  addTurn(1); addTurn(2); addTurn(0); addTurn(3);

  Serial.println("System Ready. Waiting 0.5s...");
  delay(500);
  //
  // Debug Monitor (USB)
  while (!Serial);
  Serial.println("Initializing HM-10...");
}

void loop() {
  //IR mode
  /*
  // int IR0sensorValue = ;
  Serial.print(IRDigital0);
  Serial.print(" ");
  // int IR1sensorValue = ;
  Serial.print(IRDigital1);
  Serial.print(" ");
  // int IR2sensorValue = ;
  Serial.print(IRDigital2);
  Serial.print(" ");
  // int IR3sensorValue = ;
  Serial.print(IRDigital3);
  Serial.print(" ");
  // int IR4sensorValue = ;
  Serial.print(IRDigital4);
  Serial.println("");
  */

  // tracingLoop();
  irTestLoop();

    //RFID mode
  if(!mfrc522->PICC_IsNewCardPresent()) {
    goto FuncEnd;
  }
  if(!mfrc522->PICC_ReadCardSerial()) {
    goto FuncEnd;
  } 
  Serial.println(F("*Card Detected:*"));
  mfrc522->PICC_DumpDetailsToSerial(&(mfrc522->uid));
  mfrc522->PICC_HaltA(); 
  mfrc522->PCD_StopCrypto1(); 
  FuncEnd:;
  delay(100);
}

