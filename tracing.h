#ifndef __INCLUDE_TRACING_H_
#define __INCLUDE_TRACING_H_

//define IR Pin
#define analogPinIR0 A3
#define analogPinIR1 A4
#define analogPinIR2 A5
#define analogPinIR3 A6
#define analogPinIR4 A7

float IRstarts[5];

#define IRthreshold0 180
#define IRthreshold1 360
#define IRthreshold2 180
#define IRthreshold3 180
#define IRthreshold4 180

#define IRDigital0 (analogRead(analogPinIR0) >= IRthreshold0 ? HIGH : LOW)
#define IRDigital1 (analogRead(analogPinIR1) >= IRthreshold1 ? HIGH : LOW)
#define IRDigital2 (analogRead(analogPinIR2) >= IRthreshold2 ? HIGH : LOW)
#define IRDigital3 (analogRead(analogPinIR3) >= IRthreshold3 ? HIGH : LOW)
#define IRDigital4 (analogRead(analogPinIR4) >= IRthreshold4 ? HIGH : LOW)


// --- 循跡參數調整 ---
int Tp = 120;           // 基礎速度 (建議不要太快，比較好校正)
float Kp = 120.0;       // 校正強度 (若擺動太劇烈就調小)

// --- Linked List 結構 ---
struct turn {
  int way; // 0:直行, 1:左轉, 2:右轉, 3:迴轉
  turn *next;
};
turn *head = NULL;
turn *currentCmd = NULL;

void addTurn(int way) {
  turn *newNode = new turn{way, NULL};
  if (head == NULL) { head = newNode; currentCmd = head; }
  else {
    turn *temp = head;
    while (temp->next) temp = temp->next;
    temp->next = newNode;
  }
}

// --- 強力原地轉向函式 ---
void executeHardTurn(int way) {
  if (way == 0) { // 直行：直接衝過黑區
    motorWriting(Tp, Tp);
    delay(500); 
    return;
  }

  // 1. 執行轉向 (原地旋轉)
  if (way == 1) { // 左轉
    motorWriting(-160, 160); // 提高電壓確保轉得動
    delay(350);              // 轉向時間，需實測微調
  } 
  else if (way == 2) { // 右轉
    motorWriting(160, -160);
    delay(350); 
  } 
  else if (way == 3) { // 迴轉
    motorWriting(160, -160);
    delay(700); 
  }

  // 2. 轉完後尋找黑線，直到中央感測器碰到線才停止
  // 這樣可以修正「轉過頭」或「轉不夠」的問題
  while (analogRead(analogPinIR2) < IRthreshold2) {
    // 持續旋轉直到對準
  }
  motorWriting(0, 0); 
  delay(200); 
}

void tracingSetup() {
  //IR input
  pinMode(analogPinIR0,INPUT);
  pinMode(analogPinIR1,INPUT);
  pinMode(analogPinIR2,INPUT);
  pinMode(analogPinIR3,INPUT);
  pinMode(analogPinIR4,INPUT);

  delay (3000);
  IRstarts[0] = analogRead(analogPinIR0);
  IRstarts[1] = analogRead(analogPinIR1);
  IRstarts[2] = analogRead(analogPinIR2);
  IRstarts[3] = analogRead(analogPinIR3);
  IRstarts[4] = analogRead(analogPinIR4);
}

void tracingLoop() {
  // int IR0sensorValue = ;
  static float IR_errors[5]{0, 0, 0, 0, 0};
  IR_errors[0] = (double((analogRead(analogPinIR0)-IRstarts[0]))/IRstarts[0]) * IRDigital0;
  IR_errors[1] = (double((analogRead(analogPinIR1)-IRstarts[1]))/IRstarts[1]) * IRDigital1;
  IR_errors[2] = (double((analogRead(analogPinIR2)-IRstarts[2]))/IRstarts[2]) * IRDigital2;
  IR_errors[3] = (double((analogRead(analogPinIR3)-IRstarts[3]))/IRstarts[3]) * IRDigital3;
  IR_errors[4] = (double((analogRead(analogPinIR4)-IRstarts[4]))/IRstarts[4]) * IRDigital4;
  
  //馬達
  int d0 = IRDigital0;
  int d1 = IRDigital1;
  int d2 = IRDigital2;
  int d3 = IRDigital3;
  int d4 = IRDigital4;

  // A. 判定進入 Node 塊 (當中間三顆感測器都偵測到大面積黑色時)
  if (d1 && d2 && d3) {
    motorWriting(0, 0); // 立即停車
    Serial.println("NODE DETECTED! Waiting 10s...");
    delay(2000); // 暫時延遲 2 秒

    if (currentCmd != NULL) {
      executeHardTurn(currentCmd->way); // 執行 Linked List 指定的方向
      currentCmd = currentCmd->next;    // 指向下一筆路徑
    } else {
      Serial.println("No more commands. Stopping.");
      while(1); 
    }
  }

  // B. 正常循跡模式 (P 控制)
  else {
    // 計算誤差 error
    // 權重：左邊為負，右邊為正
    float error = (d0 * -2.0 + d1 * -1.0 + d3 * 1.0 + d4 * 2.0);
    
    // 如果全白 (d2 也是 0)，保持上次方向或稍微慢速前進
    if (!d0 && !d1 && !d2 && !d3 && !d4) {
       motorWriting(Tp - 20, Tp - 20); 
    } else {
       int correction = (int)(Kp * error);
       motorWriting(Tp + correction, Tp - correction);
    }
  }
}

#endif // __INCLUDE_TRACING_H_