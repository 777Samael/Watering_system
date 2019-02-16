//#include <Wire.h>
#include "DS3231.h"
#include <LiquidCrystal_I2C.h>

// Real Time Clock DS3231
RTClib RTC;

// LCD with I2C module
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Variables for 18650 voltage calculations
float A0_input_volt = 0.0;
float A1_input_volt = 0.0;
float A2_input_volt = 0.0;

float A1_temp=0.0;
float A2_temp=0.0;

float A1_r1=1000.0;
float A1_r2=1000.0;

float A2_r1=2000.0;
float A2_r2=1000.0;

float A0_correction=106.0;
float A1_correction=106.0;
float A2_correction=105.5;

int A0_value=0;
int A1_value=0;
int A2_value=0;

void setup() {

  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.backlight();
  //lcd.autoscroll();

  // LED pin for voltage differences alarm
  pinMode(4,OUTPUT);

}

void loop() {

// Odczyt czasu z RTC
  DateTime teraz = RTC.now();
  //int rok       = teraz.year();
  //int miesiac   = teraz.month();
  //int dzien     = teraz.day();
  int godzina     = teraz.hour();
  int minuta      = teraz.minute();
  int sekunda     = teraz.second();
  float A0A1_dif  = 0.0;
  float A1A2_dif  = 0.0;
  float A2A0_dif  = 0.0;

// Sprawdzanie napięcia na ogniwach

// A0 calculations
  A0_value = analogRead(A0);
  A0_input_volt = (A0_value * 5.0)/1024.0; //only for Vin 0V-5V
  A0_input_volt = (A0_input_volt/A0_correction)*100;

  if (A0_input_volt < 0.1)
   {
     A0_input_volt=0.0;
   }

// A1 calculations
  A1_value = analogRead(A1);
  A1_input_volt = (A1_value * 5.0)/1024.0; //only for Vin 0V-5V
  A1_input_volt = (A1_input_volt/A1_correction)*100;

  if (A1_input_volt < 0.1)
   {
     A1_input_volt=0.0;
   }

// A2 calculations
  A2_value = analogRead(A2);
  A2_input_volt = (A2_value * 5.0)/1024.0; //only for Vin 0V-5V
  A2_input_volt = (A2_input_volt/A2_correction)*100;

  if (A2_input_volt < 0.1)
   {
     A2_input_volt=0.0;
   }

  // Dokładne wartości napięcia na poszczególnych ogniwach
  A1_input_volt = (2 * A1_input_volt) - A0_input_volt;
  A2_input_volt = (3 * A2_input_volt) - (2 * A1_input_volt);

// Obsługa alarmu przy nieprawidłowych wartościach napięcia

  // jeżeli któreś z ogniw ma mniejsze lub większe napięcie o 0,1V niż pozostałe
  A0A1_dif = abs(A0_input_volt - A1_input_volt);
  A1A2_dif = abs(A1_input_volt - A2_input_volt);
  A2A0_dif = abs(A2_input_volt - A0_input_volt);

  if (A0A1_dif > 0.1 || A1A2_dif > 0.1 || A2A0_dif > 0.1) {
    
    digitalWrite(4, HIGH);
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Invalid voltage");
    
    lcd.setCursor(0,1);
    lcd.print(" A0= ");
    lcd.print(A0_input_volt);

    lcd.setCursor(0,1);
    lcd.print(" A1= ");
    lcd.print(A1_input_volt);

    lcd.setCursor(0,1);
    lcd.print(" A2= ");
    lcd.print(A2_input_volt);
  }

// Decyzja o podłączeniu ogniw solarnych

// Uruchomienie pompki wody

}
