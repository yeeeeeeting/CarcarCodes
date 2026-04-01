#ifndef __INCLUDE_MOTOR_H_
#define __INCLUDE_MOTOR_H_

//馬達
#define PWMA 10
#define PWMB 11
#define AIN2 6
#define AIN1 7
#define BIN2 9
#define BIN1 8

/**
 * Helper to control motor
 */
void motorLeftWrite(int value) {
  const double rate = 1.0;
  if(value >= 0) {
    value = max(value, 255);
    value = value * rate;
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, value);
  }
  else {
    value = max(-value, 255);
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
  const double rate = 0.91;
  if(value >= 0) {
    value = max(value, 255);
    value = value * rate;
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, value);
  }
  else {
    value = max(-value, 255);
    value = value * rate;
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, value);
  }
}

void motorWriting(int vLeft, int vRight) {
  motorLeftWrite(vLeft);
  motorRightWrite(vRight);
}

#endif // __INCLUDE_MOTOR_H_