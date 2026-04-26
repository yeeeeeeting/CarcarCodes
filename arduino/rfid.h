#ifndef __INCLUDE_RFID_H_
#define __INCLUDE_RFID_H_

// #define __RFID_DEBUG_SERIAL__

#ifdef __RFID_DEBUG_SERIAL__
  #define RFID_DEBUG_PRINT(x) DEBUG_PRINT(x)
  #define RFID_DEBUG_PRINTLN(x) DEBUG_PRINTLN(x)
#else
  #define RFID_DEBUG_PRINT(x)
  #define RFID_DEBUG_PRINTLN(x)
#endif


#include "ble.h"

// include RFID Lab
#include <SPI.h>
#include <MFRC522.h>

MFRC522 *mfrc522; 
//define RFID Pin
#define RST_PIN 3
#define SS_PIN  2

byte lastRFID[4]{};

bool RFIDEqual(byte* lhs, byte* rhs) {
  for(int i = 0; i < 4; ++i) {
    if(lhs[i] != rhs[i])
      return false;
  }
  return true;
}

void cpyRFID(byte* mem, byte* target) {
  for(int i = 0; i < 4; ++i) {
    mem[i] = target[i];
  }
}

void printRFID(const byte* rfid) {
  for(int i = 0; i < 4; ++i) {
    Serial.print((int)rfid[i], HEX);
  }
  Serial.println("");
}


void rfidSetup() {
  //RFID input
  SPI.begin();
  mfrc522 = new MFRC522(SS_PIN, RST_PIN);
  mfrc522->PCD_Init();
  RFID_DEBUG_PRINTLN(F("Read UID on a MIFARE PICC:"));
  lastRFID[0] = lastRFID[1] = lastRFID[2] = lastRFID[3] = 0;
}

byte* RFIDRead() {
  //RFID mode
  if(mfrc522->PICC_IsNewCardPresent() && mfrc522->PICC_ReadCardSerial()) {
    RFID_DEBUG_PRINTLN(F("*Card Detected:*"));

    byte* id = mfrc522->uid.uidByte;

#ifdef __RFID_DEBUG_SERIAL__
    int idSize = mfrc522->uid.size;
    for(int i = 0; i < idSize; ++i) {
      RFID_DEBUG_PRINT("id[");
      RFID_DEBUG_PRINT(i);
      RFID_DEBUG_PRINT("]: ");
      RFID_DEBUG_PRINTLN(id[i], HEX);
    }
    RFID_DEBUG_PRINTLN();
#endif // __RFID_DEBUG_SERIAL__

    mfrc522->PICC_HaltA(); 
    mfrc522->PCD_StopCrypto1();
    return id;
  }
  return nullptr;
}


uint8_t hexCode1(byte b) {
  return (b >> 4) & 0x0f;
}
uint8_t hexCode2(byte b) {
  return (b) & 0x0f;
}
char hexChar(uint8_t code) {
  if(code < 10) {
    return '0' + code;
  }
  else {
    return 'A' + (code-10);
  }
}

void sendRFID(byte* id) {
  #ifndef __DISABLE_BT__
  hm10.clearInput();
  hm10.input_msg += 'r';
  hm10.input_msg += hexChar(hexCode1(id[0]));
  hm10.input_msg += hexChar(hexCode2(id[0]));
  hm10.input_msg += hexChar(hexCode1(id[1]));
  hm10.input_msg += hexChar(hexCode2(id[1]));
  hm10.input_msg += hexChar(hexCode1(id[2]));
  hm10.input_msg += hexChar(hexCode2(id[2]));
  hm10.input_msg += hexChar(hexCode1(id[3]));
  hm10.input_msg += hexChar(hexCode2(id[3]));
  hm10.input_msg += Communicator::cmdSuffix;
  RFID_DEBUG_PRINT("RFID sending: ");
  RFID_DEBUG_PRINTLN(hm10.input_msg);
  hm10.sendMsg();
  #endif
}
/*
void sendRFIDUntilSuccess(byte* id) {
  #ifndef __DISABLE_BT__
  hm10.clearInput();
  hm10.input_msg += 'r';
  hm10.input_msg += hexChar(hexCode1(id[0]));
  hm10.input_msg += hexChar(hexCode2(id[0]));
  hm10.input_msg += hexChar(hexCode1(id[1]));
  hm10.input_msg += hexChar(hexCode2(id[1]));
  hm10.input_msg += hexChar(hexCode1(id[2]));
  hm10.input_msg += hexChar(hexCode2(id[2]));
  hm10.input_msg += hexChar(hexCode1(id[3]));
  hm10.input_msg += hexChar(hexCode2(id[3]));
  hm10.input_msg += Communicator::cmdSuffix;
  RFID_DEBUG_PRINT("RFID sending: ");
  RFID_DEBUG_PRINTLN(hm10.input_msg);
  hm10.sendMsgUntilSuccess();
  #endif
}
*/
#endif // __INCLUDE_RFID_H_