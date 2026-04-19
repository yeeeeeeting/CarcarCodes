#ifndef __INCLUDE_BLE_H_
#define __INCLUDE_BLE_H_

#define CUSTOM_NAME "HM10_8team" // Max length is 12 characters [1]

#define __BLE_SERIAL_DEBUG__

class Communicator {
private:
  static constexpr long baudRates[9] = {9600, 19200, 38400, 57600, 115200, 4800, 2400, 1200, 230400};
  static bool moduleReady;

public:
  static constexpr char cmdEnd[] = "ED";
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
  void clearInput();
  void clearResponse();
};

bool Communicator::moduleReady = false;
String Communicator::input_msg;
String Communicator::response_msg;
constexpr long Communicator::baudRates[9];
constexpr char Communicator::cmdEnd[];

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

#ifdef __BLE_SERIAL_DEBUG__
  if (response.length() > 0) {
    Serial.print("HM10 Response: ");
    Serial.println(response);
  }
#endif // __BLE_SERIAL_DEBUG__

  return (response.indexOf(expected) != -1);
}

void Communicator::bleSetup() {
#ifdef __BLE_SERIAL_DEBUG__
  Serial.begin(115200); // Debug Monitor (USB)
  while (!Serial);
  Serial.println("Initializing HM-10...");
#endif // __BLE_SERIAL_DEBUG__

  // 1. Automatic Baud Rate Detection
  for (int i = 0; i < 9; i++) {

#ifdef __BLE_SERIAL_DEBUG__
    Serial.print("Testing baud rate: ");
    Serial.println(Communicator::baudRates[i]);
#endif // __BLE_SERIAL_DEBUG__
    
    Serial3.begin(Communicator::baudRates[i]);
    Serial3.setTimeout(100);
    delay(100);

    // 2. Force Disconnection
    // Sending "AT" while connected forces the module to disconnect [2].
    Serial3.print("AT"); 
    
    if (waitForResponse("OK", 800)) {

#ifdef __BLE_SERIAL_DEBUG__
      Serial.println("HM-10 detected and ready.");
#endif // __BLE_SERIAL_DEBUG__

      moduleReady = true;
      break; 
    } else {
      Serial3.end();
      delay(100);
    }
  }

  if (!moduleReady) {
#ifdef __BLE_SERIAL_DEBUG__
    Serial.println("Failed to detect HM-10. Check 3.3V VCC and wiring.");
#endif // __BLE_SERIAL_DEBUG__
    return;
  }

  // 3. Restore Factory Defaults
#ifdef __BLE_SERIAL_DEBUG__
  Serial.println("Restoring factory defaults...");
#endif // __BLE_SERIAL_DEBUG__
  sendATCommand("AT+RENEW"); // Restores all setup values
  delay(500);

  // 4. Set Custom Name via Macro
#ifdef __BLE_SERIAL_DEBUG__
  Serial.print("Setting name to: ");
  Serial.println(CUSTOM_NAME);
#endif // __BLE_SERIAL_DEBUG__
  String nameCmd = "AT+NAME" + String(CUSTOM_NAME);
  sendATCommand(nameCmd.c_str()); // Max length is 12
  
  // 5. Enable Connection Notifications
#ifdef __BLE_SERIAL_DEBUG__
  Serial.println("Enabling notifications...");
#endif // __BLE_SERIAL_DEBUG__
  sendATCommand("AT+NOTI1"); // Notify when link is established/lost

  // 6. Get the Bluetooth MAC Address
#ifdef __BLE_SERIAL_DEBUG__
  Serial.println("Querying Bluetooth Address");
#endif // __BLE_SERIAL_DEBUG__
  sendATCommand("AT+ADDR?");

  // 7. Restart the module to apply changes
#ifdef __BLE_SERIAL_DEBUG__
  Serial.println("Restarting module...");
#endif // __BLE_SERIAL_DEBUG__
  sendATCommand("AT+RESET"); // Restart the module
  delay(1000);
  Serial3.begin(9600); // Now the module would use baudrate 9600

#ifdef __BLE_SERIAL_DEBUG__
  Serial.println("Initialization Complete.");
#endif // __BLE_SERIAL_DEBUG__
}

void Communicator::clearInput() {
  input_msg = "";
}

void Communicator::clearResponse() {
  response_msg = "";
}

void Communicator::sendMsg() {
  Serial3.print(input_msg);
}

bool Communicator::loadResponse() {
  if(Serial3.available()) {
    while(Serial3.available()) {
      char c = Serial3.read();
      response_msg += c;
      if(response_msg.endsWith(cmdEnd)) {
        return true;
      }
    }
  }
  else {
    return false;
  }
}

Communicator& hm10 = Communicator::getObj();

#endif // __INCLUDE_BLE_H_
