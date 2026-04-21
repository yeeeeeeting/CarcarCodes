#ifndef __INCLUDE_TRACING_H_
#define __INCLUDE_TRACING_H_

#include "ble.h"

//define IR Pin
#define analogPinIR0 A4
#define analogPinIR1 A3
#define analogPinIR2 A5
#define analogPinIR3 A6
#define analogPinIR4 A7

float IRstarts[5];

#define IRthreshold0 120
#define IRthreshold1 120
#define IRthreshold2 120
#define IRthreshold3 120
#define IRthreshold4 120

#define IRDigital0 (analogRead(analogPinIR0) >= IRthreshold0 ? HIGH : LOW)
#define IRDigital1 (analogRead(analogPinIR1) >= IRthreshold1 ? HIGH : LOW)
#define IRDigital2 (analogRead(analogPinIR2) >= IRthreshold2 ? HIGH : LOW)
#define IRDigital3 (analogRead(analogPinIR3) >= IRthreshold3 ? HIGH : LOW)
#define IRDigital4 (analogRead(analogPinIR4) >= IRthreshold4 ? HIGH : LOW)



// --- 循跡參數調整 ---
const int Vh = 250;
const int Vl = 170;
const int Tp = 200;           // 基礎速度 (建議不要太快，比較好校正)
const float Kp = 60.0;       // 校正強度 (若擺動太劇烈就調小)
const float Ki = 14.4;
const float Kd = 22.0;
const float alpha = 0.97;


bool activated = false;
bool readyForID = false;

void deactivate() {
  activated = false;
  motorWriting(0, 0);
}

void checkActivated() {
  if( !hm10.loadResponse() )
    return;
  if(hm10.response_msg[0] == 'a') {
    delay(10);
    activated = true;
    hm10.clearInput();
    hm10.sendSuccessMsg();
  }
  else {
    hm10.clearResponse();
  }
}


enum Turn {
  FORWARD,
  LEFT,
  RIGHT,
  BACKWARD
};

struct Command {
  bool valid;
  bool end;
  Turn turn;
};

Command queryTurn() {
  hm10.clearInput();
  // construct query
  hm10.input_msg += 'q';
  hm10.input_msg += Communicator::cmdSuffix;
  hm10.sendMsg();
  hm10.sendMsgUntilSuccess();

  // parse command
  Command result{true, false, FORWARD};
  char c = hm10.response_msg[0];
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
    result.end = true;
    break;
  default:
    result.valid = false;
    break;
  }

  readyForID = (hm10.response_msg[1] == '1');

  return result;
}

// --- 強力原地轉向函式 ---
void executeHardTurn(Turn turn) {
  if (turn == FORWARD) { // 直行：直接衝過黑區
    motorWriting(Tp, Tp);
    delay(500); 
    return;
  }

  // 1. 執行轉向 (原地旋轉)
  if (turn == LEFT) { // 左轉
    motorWriting(Vh, Vh);
    delay(280);

    motorWriting(Vl, Vh);
    delay(110);

    motorWriting(0, 0);
    delay(6);

    motorWriting(-250, 250); // 提高電壓確保轉得動
    delay(150);              // 轉向時間，需實測微調

    motorWriting(-145, 210);
    delay(100);
  
    motorWriting(0, 0);
    delay(6);
  
    motorWriting(-12, 12);
  } 
  else if (turn == RIGHT) { // 右轉
    motorWriting(Vh, Vh);
    delay(280);

    motorWriting(Vh, Vl);
    delay(110);

    motorWriting(0, 0);
    delay(6);

    motorWriting(250, -250); // 提高電壓確保轉得動
    delay(150);              // 轉向時間，需實測微調

    motorWriting(210, -145);
    delay(100);
  
    motorWriting(0, 0);
    delay(6);
  
    motorWriting(12, -12);
  } 
  else if (turn == BACKWARD) { // 迴轉
    motorWriting(250, -250);
    delay(270);
    // motorWriting(0, 0);
    // delay(1000);
  
    motorWriting(220, -10);
    delay(130);
    
    // motorWriting(0, 0);
    // delay(1000);
    motorWriting(12, -9); 
  }

  // 2. 轉完後尋找黑線，直到中央感測器碰到線才停止
  // 這樣可以修正「轉過頭」或「轉不夠」的問題
  while (analogRead(analogPinIR2) < IRthreshold2) {
    // 持續旋轉直到對準
  }
  motorWriting(Vh, Vh);
  delay(120);
  motorWriting(0, 0); 
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
  static int dgt[5]{0, 0, 0, 0, 0};
  static const int IR_poses[5]{-2, -1, 0, 1, 2};
  static double prev_error = 0;
  static double cumulative_error = 0;

  byte* rfid = RFIDRead();
  /*
  if(rfid != nullptr) {
    printRFID(rfid);
    printRFID(lastRFID);
    Serial.println(RFIDEqual(rfid, lastRFID));
  }
  */
  if(rfid != nullptr && !RFIDEqual(rfid, lastRFID)) {
    Serial.println("DETECTED and back");
    readyForID = false;
    cpyRFID(lastRFID, rfid);
    motorWriting(0, 0);
    sendRFIDUntilSuccess(rfid);
    //motorWriting(-Vh, -Vh);
    //delay(50);
    motorWriting(0, 0);
    delay(10);
    executeHardTurn(BACKWARD);
  }

  dgt[0] = IRDigital0;
  dgt[1] = IRDigital1;
  dgt[2] = IRDigital2;
  dgt[3] = IRDigital3;
  dgt[4] = IRDigital4;

  // A. 判定進入 Node 塊 (當中間三顆感測器都偵測到大面積黑色時)
  if (dgt[1] && dgt[2] && dgt[3]) {
    motorWriting(0, 0); // 立即停車
    Serial.println("NODE DETECTED! Waiting 0.02s...");
    delay(20); // 暫時延遲 0.02 秒

    Command currentCmd{false};
    while(!currentCmd.valid)
      currentCmd = queryTurn();

    if (!currentCmd.end) {
      executeHardTurn(currentCmd.turn);
    } else {
      Serial.println("No more commands. Stopping.");
      deactivate();
      while(1); 
    }

    prev_error = 0;
    cumulative_error = 0;
  }

  // B. 正常循跡模式 (P 控制)
  else {
    bool isAllZero = !dgt[0] && !dgt[1] && !dgt[2] && !dgt[3] && !dgt[4];
    // 計算誤差 error
    // 權重：左邊為負，右邊為正
    // Serial.print(dgt[0]), Serial.print(' '), Serial.print(dgt[1]), Serial.print(' '), Serial.print(dgt[2]), Serial.print(' '), Serial.print(dgt[3]), Serial.print(' '), Serial.println(dgt[4]);
    double error = isAllZero ? prev_error : centerOfMass(dgt, IR_poses, 5);
    cumulative_error = cumulative_error * alpha + error;
    cumulative_error = constrain(cumulative_error, -12.0, 12.0);
    double diff_error = error - prev_error;
    // Serial.print(error), Serial.print(' '), Serial.print(diff_error), Serial.print(' '), Serial.println(cumulative_error);

    // If straight
    if(abs(error) < 0.5 && abs(diff_error) < 0.5) {
      if(!readyForID)
        motorWriting(Vh, Vh);
      else
        motorWriting(Vl, Vl);
    }
    
    // 如果全白 (d2 也是 0)，保持上次方向或稍微慢速前進
    else if (isAllZero) {
      motorWriting(Tp - 20, Tp - 20); 
    }
    else {
      int correction = (int)(Kp * error + Ki * cumulative_error + Kd * diff_error);
      motorWriting(Tp + correction, Tp - correction);
    }

    prev_error = error;
  }
}

#endif // __INCLUDE_TRACING_H_