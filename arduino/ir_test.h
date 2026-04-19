
/*
//define IR Pin
#define analogPinIR0 A3
#define analogPinIR1 A4
#define analogPinIR2 A5
#define analogPinIR3 A6
#define analogPinIR4 A7
*/

void irTestLoop() {
  static float IR_errors[5]{0, 0, 0, 0, 0};
  IR_errors[0] = (double((analogRead(analogPinIR0)-IRstarts[0]))/IRstarts[0]) * IRDigital0;
  IR_errors[1] = (double((analogRead(analogPinIR1)-IRstarts[1]))/IRstarts[1]) * IRDigital1;
  IR_errors[2] = (double((analogRead(analogPinIR2)-IRstarts[2]))/IRstarts[2]) * IRDigital2;
  IR_errors[3] = (double((analogRead(analogPinIR3)-IRstarts[3]))/IRstarts[3]) * IRDigital3;
  IR_errors[4] = (double((analogRead(analogPinIR4)-IRstarts[4]))/IRstarts[4]) * IRDigital4;


  Serial.println("IR analog:");
  Serial.print( analogRead(analogPinIR0) );
  Serial.print(" ");
  Serial.print( analogRead(analogPinIR1) );
  Serial.print(" ");
  Serial.print( analogRead(analogPinIR2) );
  Serial.print(" ");
  Serial.print( analogRead(analogPinIR3) );
  Serial.print(" ");
  Serial.print( analogRead(analogPinIR4) );
  Serial.println("\nIR errors:");
  Serial.print(IR_errors[0]);
  Serial.print(" ");
  Serial.print(IR_errors[1]);
  Serial.print(" ");
  Serial.print(IR_errors[2]);
  Serial.print(" ");
  Serial.print(IR_errors[3]);
  Serial.print(" ");
  Serial.print(IR_errors[4]);
  Serial.println("\nIR digital:");
  Serial.print(IRDigital0);
  Serial.print(" ");
  Serial.print(IRDigital1);
  Serial.print(" ");
  Serial.print(IRDigital2);
  Serial.print(" ");
  Serial.print(IRDigital3);
  Serial.print(" ");
  Serial.print(IRDigital4);
  Serial.print("\n\n");
}
