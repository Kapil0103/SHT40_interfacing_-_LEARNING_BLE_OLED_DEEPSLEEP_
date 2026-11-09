#include "variant.h"
#include "wiring_analog.h"
#include "wiring_constants.h"
#include "wiring_digital.h"
  #include "motor.h"

  #define ENA D3
  #define IN1 D8
  #define IN2 D9

  bool motorSetup(){
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    digitalWrite(IN1, HIGH);
    digitalWrite(IN1, LOW);

    analogWrite(ENA, 0);

    return true;
  }

  void motorSetSpeed(int speed){
    speed = constrain(speed, 0, 100);

    int pwmValue = map(speed, 0, 100, 0, 255);

    analogWrite(ENA, pwmValue);
  }

  void motorStop(){
    analogWrite(ENA, 0);
  }