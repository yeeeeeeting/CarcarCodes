#ifndef __INCLUDE_MOTOR_H_
#define __INCLUDE_MOTOR_H_

//馬達
#define PWMA 10
#define PWMB 11
#define AIN2 6
#define AIN1 7
#define BIN2 9
#define BIN1 8

#define LEFT_MOTOR_RATE 0.94
#define RIGHT_MOTOR_RATE 1.0

void motorSetup() {
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
}

/**
 * Helper to control motor
 */
void motorLeftWrite(int value) {
  const double rate = LEFT_MOTOR_RATE;
  if(value >= 0) {
    value = min(value, 255);
    value = value * rate;
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, value);
  }
  else {
    value = min(-value, 255);
    value = value * rate;
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, value);
  }
}

/**
 * Helper to control motor
 */
void motorRightWrite(int value) {
  const double rate = RIGHT_MOTOR_RATE;
  if(value >= 0) {
    value = min(value, 255);
    value = value * rate;
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, value);
  }
  else {
    value = min(-value, 255);
    value = value * rate;
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, value);
  }
}

inline int sign(int x) {
  return x < 0 ? -1 : (x > 0 ? 1 : 0);
}

void motorWriting(int vLeft, int vRight) {
  // static int lastRight = 0;
  motorRightWrite(vRight);
  // if(lastRight * vRight <= 0) {
  //   delay(80);
  // }
  motorLeftWrite(vLeft);
  // lastRight = sign(vRight);
}

#endif // __INCLUDE_MOTOR_H_