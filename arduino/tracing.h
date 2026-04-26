#ifndef __INCLUDE_TRACING_H_
#define __INCLUDE_TRACING_H_

#include "ble.h"

//define IR Pin
#define analogPinIR0 A3
#define analogPinIR1 A4
#define analogPinIR2 A5
#define analogPinIR3 A6
#define analogPinIR4 A7

float IRstarts[5];

#define IRthreshold0 110
#define IRthreshold1 110
#define IRthreshold2 90
#define IRthreshold3 110
#define IRthreshold4 110

#define IRDigital0 (analogRead(analogPinIR0) >= IRthreshold0 ? HIGH : LOW)
#define IRDigital1 (analogRead(analogPinIR1) >= IRthreshold1 ? HIGH : LOW)
#define IRDigital2 (analogRead(analogPinIR2) >= IRthreshold2 ? HIGH : LOW)
#define IRDigital3 (analogRead(analogPinIR3) >= IRthreshold3 ? HIGH : LOW)
#define IRDigital4 (analogRead(analogPinIR4) >= IRthreshold4 ? HIGH : LOW)



// --- 循跡參數調整 ---
const int Vhigh = 255;
const int Vlow = 200;
const int Tp = 240;           // 基礎速度
const float Kp = 25.4;        // 比例控制係數
const float Ki = 0.0004;      // 積分控制係數
const float Kd = 736.0;       // 微分控制係數
const float alpha = 0.98;     // 積分遺忘因子
const float beta = 0.4;       // 微分遺留因子

const int VRotateCorrection1 = 120;
const int VRotateCorrection2 = 55;


bool activated = false;
bool ended = false;
bool atNodeStraight = false;
bool readyForID = false;

void deactivate() {
  activated = false;
  ended = true;
  motorWriting(0, 0);
}

void checkActivated() {
  #ifndef __DISABLE_BT__
  hm10.clearInput();
  hm10.input_msg += 'k';
  hm10.input_msg += hm10.cmdSuffix;
  hm10.sendMsgUntilSuccess('a');
  // DEBUG_PRINTLN("Escaped");
  bool successed = false;
  {
    // DEBUG_PRINT("Response: ");
    // DEBUG_PRINTLN(hm10.response_msg);
    if(hm10.response_msg[0] == 'a') {
      DEBUG_PRINT("Activation!");
      delay(10);
      activated = true;
      atNodeStraight = true;
      successed = true;
      hm10.clearInput();
      hm10.sendSuccessMsg();
    }
    else {
      hm10.clearResponse();
    }
  }
  #else
  activated = true;
  atNodeStraight = true;
  #endif // __DISABLE_BT__
}


enum Turn {
  INVALID,
  FORWARD,
  LEFT,
  RIGHT,
  BACKWARD,
  END
};

struct Command {
  Turn turn;
};

Command queryTurn() {
  #ifndef __DISABLE_BT__
  DEBUG_PRINTLN("querying");
  String cmdStr;
  hm10.waitForResponse(200, 't');
  if(hm10.isResponseValid() && hm10.response_msg[0] == 't') {
    cmdStr = hm10.response_msg;
    hm10.clearResponse();
    hm10.clearInput();
    hm10.input_msg += 'q';
    hm10.input_msg += Communicator::cmdSuffix;
    hm10.sendMsg();
  }
  else {
    hm10.clearResponse();
    hm10.clearInput();
    // construct query
    hm10.input_msg += 'q';
    hm10.input_msg += Communicator::cmdSuffix;
    hm10.sendMsgUntilSuccess('t');
    cmdStr = hm10.response_msg;
  }

  // parse command
  Command result{INVALID};
  char c = cmdStr[1];
  switch(c) {
  case 'F':
    result.turn = FORWARD;
    break;
  case 'L':
    result.turn = LEFT;
    break;
  case 'R':
    result.turn = RIGHT;
    break;
  case 'B':
    result.turn = BACKWARD;
    break;
  case 'E':
    result.turn = END;
    break;
  default:
    result.turn = INVALID;
    break;
  }

  readyForID = (cmdStr[2] == '1');
  #else
  constexpr Command cmdList[] = {
    {LEFT},
    {BACKWARD},
    {FORWARD},
    {BACKWARD},
    {LEFT},
    {BACKWARD},
    {RIGHT},
    {BACKWARD},
    {FORWARD},
    {BACKWARD},
    {RIGHT},
    {BACKWARD}
  };
  constexpr int N = sizeof(cmdList) / sizeof(cmdList[0]);
  static int index = 0;
  Command result = cmdList[index];
  index = (index + 1) % N;
  #endif
  return result;
}

// --- 強力原地轉向函式 ---
void executeHardTurn(Turn turn) {
  if (turn == FORWARD) { // 直行：直接衝過黑區
    motorWriting(Tp, Tp);
    delay(200);
    atNodeStraight = true;
    return;
  }

  // 1. 執行轉向 (原地旋轉)
  if (turn == LEFT) { // 左轉

    motorWriting(100, 255);
    delay(560);

    motorWriting(-VRotateCorrection1, VRotateCorrection1); 
    while(!(IRDigital0 || IRDigital1));

    motorWriting(-VRotateCorrection2, VRotateCorrection2);
    while(!(IRDigital2 || IRDigital3));
    // delay(20);
  } 
  else if (turn == RIGHT) { // 右轉
    motorWriting(255, 100);
    delay(560);
  
    motorWriting(VRotateCorrection1, -VRotateCorrection1); 
    while(!(IRDigital4 || IRDigital3));

    motorWriting(VRotateCorrection2, -VRotateCorrection2);
    while(!(IRDigital2 || IRDigital1));
  } 
  else if (turn == BACKWARD) { // 迴轉
  
    motorWriting(230, -255);
    delay(420);

    motorWriting(170, -240);
    delay(60);
    
    motorWriting(VRotateCorrection1, -VRotateCorrection1); 
    while(!(IRDigital4 || IRDigital3));
  
    motorWriting(VRotateCorrection2, -VRotateCorrection2); 
    while(!(IRDigital2 || IRDigital1));
    // delay(20);
  }

  motorWriting(Tp, Vhigh);
  delay(50);
  // motorWriting(0, 0); 
  // delay(200); 
}

void tracingSetup() {
  //IR input
  pinMode(analogPinIR0,INPUT);
  pinMode(analogPinIR1,INPUT);
  pinMode(analogPinIR2,INPUT);
  pinMode(analogPinIR3,INPUT);
  pinMode(analogPinIR4,INPUT);

  activated = false;
  ended = false;
  readyForID = false;
}

float centerOfMass(const int *weight, const int *pos, int len) {
  int num = 0, den = 0;
  for(int i = 0; i < len; ++i) {
    den += weight[i];
    num += weight[i] * pos[i];
  }
  return den == 0 ? 0.0 : 1.0 * num / den;
}

void tracingLoop() {
  // DEBUG_PRINT("at: ");
  // DEBUG_PRINTLN(atNodeStraight);
  static int dgt[5]{0, 0, 0, 0, 0};
  static const int IR_poses[5]{-2, -1, 0, 1, 2};
  static double prev_error = 0;
  static double diff_error = 0;
  static double cumulative_error = 0;
  static unsigned long long prev_time = millis();

  byte* rfid = RFIDRead();
/*
  if(rfid != nullptr) {
    printRFID(rfid);
    printRFID(lastRFID);
    DEBUG_PRINTLN(RFIDEqual(rfid, lastRFID));
  }
*/
  if(rfid != nullptr && !RFIDEqual(rfid, lastRFID)) {
    DEBUG_PRINTLN("DETECTED and back");
    readyForID = false;
    cpyRFID(lastRFID, rfid);
    motorWriting(0, 0);
    sendRFID(rfid); // sendRFIDUntilSuccess(rfid);
    //motorWriting(-Vhigh, -Vhigh);
    //delay(50);
    motorWriting(0, 0);
    delay(10);
    executeHardTurn(BACKWARD);
    prev_error = 0;
    diff_error = 0;
    cumulative_error = 0;
    prev_time = millis();
    return;
  }

  dgt[0] = IRDigital0;
  dgt[1] = IRDigital1;
  dgt[2] = IRDigital2;
  dgt[3] = IRDigital3;
  dgt[4] = IRDigital4;

  // A. 判定進入 Node 塊 (當中間三顆感測器都偵測到大面積黑色時)
  if (dgt[1] && dgt[2] && dgt[3] && !atNodeStraight) {
    motorWriting(0, 0); // 立即停車
    DEBUG_PRINTLN("NODE DETECTED! Waiting 0.02s...");
    delay(20); // 暫時延遲 0.02 秒

    Command currentCmd{INVALID};
    while(currentCmd.turn == INVALID)
      currentCmd = queryTurn();

    if (currentCmd.turn != END) {
      executeHardTurn(currentCmd.turn);
    } else {
      DEBUG_PRINTLN("No more commands. Stopping.");
      deactivate();
      while(1); 
    }
    atNodeStraight = true;

    if(currentCmd.turn != FORWARD) {
      prev_error = 0;
      diff_error = 0;
      cumulative_error = 0;
    }
    prev_time = millis();
    return;
  }

  int highCnt = dgt[0] + dgt[1] + dgt[2] + dgt[3] + dgt[4];
  bool isAllZero = (highCnt == 0);
  double error = ((isAllZero || atNodeStraight || highCnt >= 3 || (!dgt[1] && dgt[2] && !dgt[3])) ? (prev_error * alpha) : centerOfMass(dgt, IR_poses, 5));

  if(atNodeStraight && (highCnt <= 2)) {
    atNodeStraight = false;
    return;
  }


  // B. 正常循跡模式 (P 控制)
  else {
    // 計算誤差 error
    // 權重：左邊為負，右邊為正
    // DEBUG_PRINT(dgt[0]), DEBUG_PRINT(' '), DEBUG_PRINT(dgt[1]), DEBUG_PRINT(' '), DEBUG_PRINT(dgt[2]), DEBUG_PRINT(' '), DEBUG_PRINT(dgt[3]), DEBUG_PRINT(' '), DEBUG_PRINTLN(dgt[4]);
    
    unsigned long long current_time = millis();
    long long delta_time = current_time - prev_time;
    cumulative_error = cumulative_error * alpha + error * delta_time;
    cumulative_error = constrain(cumulative_error, -2000.0, 2000.0);
    double delta_error = error - prev_error;
    diff_error  = diff_error * beta + delta_error / delta_time;
    // DEBUG_PRINT(error), DEBUG_PRINT(' '), DEBUG_PRINT(diff_error), DEBUG_PRINT(' '), DEBUG_PRINTLN(cumulative_error);

    if(atNodeStraight) {
      cumulative_error *= 0.3;
      diff_error *= -0.8;
    }

    // If straight
    if(abs(error) < 1 && abs(diff_error) < 0.05 && abs(cumulative_error) < 20.0) {
      if(!readyForID) {
        motorWriting(Vhigh, Vhigh);
      }
      else {
        motorWriting(Vlow, Vlow);
      }
    }
    
    // 如果全白 (d2 也是 0)，倒退
    else if (isAllZero) {
      motorWriting(Vlow, Vlow); 
    }
    else {
      const int V0 = (readyForID ? Vlow : Tp);
      int correction = (int)(Kp * error + Ki * cumulative_error + Kd * diff_error);
      motorWriting(V0 + correction, V0 - correction);
    }

    prev_error = error;
    prev_time = current_time;
  }
}

#endif // __INCLUDE_TRACING_H_