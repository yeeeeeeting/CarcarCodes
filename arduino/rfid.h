#ifndef __INCLUDE_RFID_H_
#define __INCLUDE_RFID_H_

// #define __RFID_SERIAL_DEBUG__

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
    Serial.print((int)rfid[i]);
  }
  Serial.println("");
}


void rfidSetup() {
  //RFID input
  SPI.begin();
  mfrc522 = new MFRC522(SS_PIN, RST_PIN);
  mfrc522->PCD_Init();
#ifdef __RFID_SERIAL_DEBUG__
  Serial.println(F("Read UID on a MIFARE PICC:"));
#endif // __RFID_SERIAL_DEBUG__
  lastRFID[0] = lastRFID[1] = lastRFID[2] = lastRFID[3] = 0;
}

byte* RFIDRead() {
  //RFID mode
  if(mfrc522->PICC_IsNewCardPresent() && mfrc522->PICC_ReadCardSerial()) {
#ifdef __RFID_SERIAL_DEBUG__
    Serial.println(F("*Card Detected:*"));
#endif // __RFID_SERIAL_DEBUG__

    byte* id = mfrc522->uid.uidByte;

#ifdef __RFID_SERIAL_DEBUG__
    int idSize = mfrc522->uid.size;
    for(int i = 0; i < idSize; ++i) {
      Serial.print("id[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.println(id[i], HEX);
    }
    Serial.println();
#endif // __RFID_SERIAL_DEBUG__

    mfrc522->PICC_HaltA(); 
    mfrc522->PCD_StopCrypto1();
    return id;
  }
  return nullptr;
}

void sendRFID(byte* id) {
  hm10.clearInput();
  hm10.input_msg += 'r';
  hm10.input_msg += static_cast<uint8_t>(id[0]);
  hm10.input_msg += static_cast<uint8_t>(id[1]);
  hm10.input_msg += static_cast<uint8_t>(id[2]);
  hm10.input_msg += static_cast<uint8_t>(id[3]);
  hm10.input_msg += Communicator::cmdSuffix;
  hm10.sendMsg();
}

void sendRFIDUntilSuccess(byte* id) {
  hm10.clearInput();
  hm10.input_msg += 'r';
  hm10.input_msg += static_cast<char>(id[0]);
  hm10.input_msg += static_cast<char>(id[1]);
  hm10.input_msg += static_cast<char>(id[2]);
  hm10.input_msg += static_cast<char>(id[3]);
  hm10.input_msg += Communicator::cmdSuffix;
  hm10.sendMsgUntilSuccess();
}

#endif // __INCLUDE_RFID_H_