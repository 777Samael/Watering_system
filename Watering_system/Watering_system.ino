#include <Wire.h>
#include "DS3231.h"

RTClib RTC;

void setup() {
  Wire.begin();
  pinMode(1, OUTPUT);
  pinMode(4,OUTPUT);
  digitalWrite(1,LOW);

}

void loop() {

  delay(3000);
  digitalWrite(1,HIGH);
  delay(5000);
  digitalWrite(1,LOW);

}
