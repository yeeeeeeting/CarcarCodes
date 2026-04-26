#ifndef __INCLUDE_BLE_H_
#define __INCLUDE_BLE_H_

#define CUSTOM_NAME "HM10_TM8" // Max length is 12 characters [1]

// #define __BLE_DEBUG_SERIAL__

#ifdef __BLE_DEBUG_SERIAL__
  #define BLE_DEBUG_PRINT(x) DEBUG_PRINT(x)
  #define BLE_DEBUG_PRINTLN(x) DEBUG_PRINTLN(x)
#else
  #define BLE_DEBUG_PRINT(x)
  #define BLE_DEBUG_PRINTLN(x)
#endif

class Communicator {
private:
  static constexpr long baudRates[9] = {9600, 19200, 38400, 57600, 115200, 4800, 2400, 1200, 230400};
  static bool moduleReady;

public:
  static constexpr char cmdSuffix[] = "ED";
  static constexpr char cmdFailPrefix = 'x';
  static constexpr char cmdSuccessPrefix = 'v';
  static String input_msg;
  static String response_msg;

private:
  void sendATCommand(const char* command);
  bool waitForResponse(const char* expected, unsigned long timeout);

  Communicator();

public:
  static Communicator& getObj();
  void bleSetup();
  void sendMsg();
  bool loadResponse();
  void flushResponse();
  bool isResponseValid();
  void sendMsgUntilSuccess(char);
  void clearInput();
  void clearResponse();
  void sendSuccessMsg();
  void waitForResponse(int, char);
};

bool Communicator::moduleReady = false;
String Communicator::input_msg;
String Communicator::response_msg;
constexpr long Communicator::baudRates[9];
constexpr char Communicator::cmdSuffix[];
constexpr char Communicator::cmdFailPrefix;
constexpr char Communicator::cmdSuccessPrefix;

Communicator::Communicator() {
  input_msg.reserve(20);
  response_msg.reserve(20);
}

Communicator& Communicator::getObj() {
  static Communicator obj;
  return obj;
}


/**
  * Helper to send AT commands (Uppercase, no \r or \n) [6]
  */
void Communicator::sendATCommand(const char* command) {
    Serial3.print(command);
    waitForResponse("", 1000); 
}

/**
* Helper to check response for specific substrings
*/
bool Communicator::waitForResponse(const char* expected, unsigned long timeout) {
  unsigned long start = millis();
  Serial3.setTimeout(timeout);
  String response = Serial3.readString();

#ifdef __BLE_DEBUG_SERIAL__
  if (response.length() > 0) {
    BLE_DEBUG_PRINT("HM10 Response: ");
    BLE_DEBUG_PRINTLN(response);
  }
#endif // __BLE_DEBUG_SERIAL__

  return (response.indexOf(expected) != -1);
}

void Communicator::bleSetup() {
#ifdef __BLE_DEBUG_SERIAL__
  while (!Serial);
  BLE_DEBUG_PRINTLN("Initializing HM-10...");
#endif // __BLE_DEBUG_SERIAL__

  // 1. Automatic Baud Rate Detection
  for (int i = 0; i < 9; i++) {

    BLE_DEBUG_PRINT("Testing baud rate: ");
    BLE_DEBUG_PRINTLN(Communicator::baudRates[i]);
    
    Serial3.begin(Communicator::baudRates[i]);
    Serial3.setTimeout(100);
    delay(100);

    // 2. Force Disconnection
    // Sending "AT" while connected forces the module to disconnect [2].
    Serial3.print("AT"); 
    
    if (waitForResponse("OK", 800)) {

      BLE_DEBUG_PRINTLN("HM-10 detected and ready.");

      moduleReady = true;
      break; 
    } else {
      Serial3.end();
      delay(100);
    }
  }

  if (!moduleReady) {
    BLE_DEBUG_PRINTLN("Failed to detect HM-10. Check 3.3V VCC and wiring.");
    return;
  }

  // 3. Restore Factory Defaults
  BLE_DEBUG_PRINTLN("Restoring factory defaults...");
  sendATCommand("AT+RENEW"); // Restores all setup values
  delay(1000);

  // 4. Set Custom Name via Macro
  BLE_DEBUG_PRINT("Setting name to: ");
  BLE_DEBUG_PRINTLN(CUSTOM_NAME);
  String nameCmd = "AT+NAME" + String(CUSTOM_NAME);
  sendATCommand(nameCmd.c_str()); // Max length is 12
  delay(500);
  sendATCommand("AT+RESET"); // Restart the module
  
  // 5. Enable Connection Notifications
  BLE_DEBUG_PRINTLN("Enabling notifications...");
  sendATCommand("AT+NOTI1"); // Notify when link is established/lost

  // 6. Get the Bluetooth MAC Address
  BLE_DEBUG_PRINTLN("Querying Bluetooth Address");
  sendATCommand("AT+ADDR?");

  // 7. Restart the module to apply changes
  BLE_DEBUG_PRINTLN("Restarting module...");
  sendATCommand("AT+RESET"); // Restart the module
  BLE_DEBUG_PRINTLN("AT RESET");
  delay(2000);
  Serial3.begin(9600); // Now the module would use baudrate 9600

  flushResponse();

  BLE_DEBUG_PRINTLN("Initialization Complete.");
}

void Communicator::clearInput() {
  input_msg = "";
}

void Communicator::clearResponse() {
  response_msg = "";
}

void Communicator::sendMsg() {
  BLE_DEBUG_PRINT("Send: ");
  BLE_DEBUG_PRINTLN(input_msg);
  Serial3.write((const uint8_t*)(input_msg.c_str()), input_msg.length());
}

bool Communicator::loadResponse() {
  if(Serial3.available()) {
    while(Serial3.available()) {
      char c = Serial3.read();
      response_msg += c;
      if(isResponseValid()) {
        BLE_DEBUG_PRINT("recieved: ");
        BLE_DEBUG_PRINTLN(response_msg);
        return true;
      }
    }
  }
  return false;
}

void Communicator::flushResponse() {
  while(Serial3.available()) {
    char c = Serial3.read();
  }
  BLE_DEBUG_PRINTLN("Flush Responses");
}

bool Communicator::isResponseValid() {
  return response_msg.length() > 0 && response_msg.endsWith(cmdSuffix);
}

void Communicator::sendMsgUntilSuccess(char expect = '\0') {
  if(input_msg.length() <= 0)
    return;
  clearResponse();
  // test if response is ok
  while(true) {
    sendMsg();
    delay(100);
    // loading
    bool success = false;
    while(true) {
      if(!loadResponse()) {
        delay(100);
        break;
      }
      // Serial.println("loaded");
      if(!isResponseValid()) {
        delay(10);
        continue;
      }
      // Serial.println("Valid");
      if(expect == '\0' || response_msg[0] == expect) {
        BLE_DEBUG_PRINTLN("Expected");
        success = true;
        break;
      }
      else {
        BLE_DEBUG_PRINTLN("Msg but unexpected");
      }
    }
    
    if(success)
      break;
    clearResponse();
    // abnormal response -> re-send msg
  }
}

void Communicator::sendSuccessMsg() {
  clearInput();
  input_msg += cmdSuccessPrefix;
  input_msg += cmdSuffix;
  sendMsg();
}

void Communicator::waitForResponse(int timeout, char expect = '\0') {
  int start = millis();
  while(millis() <= start + timeout) {
    if(!loadResponse())
      continue;
    if(expect == '\0' || response_msg[0] == expect) {
      break;
    }
    else {
      clearResponse();
    }
  }
}

Communicator& hm10 = Communicator::getObj();

#endif // __INCLUDE_BLE_H_