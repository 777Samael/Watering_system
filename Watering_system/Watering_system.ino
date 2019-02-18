//#include <Wire.h>
#include "DS3231.h"
#include <LiquidCrystal_I2C.h>

// Real Time Clock DS3231
RTClib RTC;

// LCD with I2C module
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Solar charging
int Charge_run = 0;
float Charge_limit_low  = 11.1;
float Charge_limit_high = 12.5;

// Variables for 18650 voltage calculations
float A0_input_volt = 0.0;
float A1_input_volt = 0.0;
float A2_input_volt = 0.0;
float V_total       = 0.0;
float V_total_min   = 10.95;

float A1_temp = 0.0;
float A2_temp = 0.0;

float A1_r1 = 1000.0;
float A1_r2 = 1000.0;

float A2_r1 = 2000.0;
float A2_r2 = 1000.0;

float A0_correction = 105.9;
float A1_correction = 105.8;
float A2_correction = 106.0;

int A0_value = 0;
int A1_value = 0;
int A2_value = 0;

int V_ok = 1;

void setup() {

  // Setting up output
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.backlight();
  //lcd.autoscroll();

  // LED pin for voltage differences alarm
  pinMode(2,OUTPUT);

  // LED pin for low voltage
  pinMode(3,OUTPUT);

  // LED pin for high voltage
  pinMode(4,OUTPUT);
  
  // Water pump relay pin
  pinMode(6,OUTPUT);
  digitalWrite(6, HIGH);

  // Solar charger relay pin
  pinMode(8,OUTPUT);
  digitalWrite(8, HIGH);

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
  A0_input_volt = (A0_value * 5.0) / 1024.0; //only for Vin 0V-5V
  A0_input_volt = (A0_input_volt / A0_correction) * 100;

  if (A0_input_volt < 0.1)
   {
     A0_input_volt = 0.0;
   }

  // A1 calculations
  A1_value = analogRead(A1);
  A1_input_volt = (A1_value * 5.0) / 1024.0; //only for Vin 0V-5V
  A1_input_volt = (A1_input_volt / A1_correction) * 100;

  if (A1_input_volt < 0.1)
   {
     A1_input_volt = 0.0;
   }

  // A2 calculations
  A2_value = analogRead(A2);
  A2_input_volt = (A2_value * 5.0) / 1024.0; //only for Vin 0V-5V
  A2_input_volt = (A2_input_volt / A2_correction) * 100;

  if (A2_input_volt < 0.1)
   {
     A2_input_volt  =0.0;
   }

  // Dokładne wartości napięcia na poszczególnych ogniwach
  A1_input_volt = (2 * A1_input_volt) - A0_input_volt;
  A2_input_volt = (3 * A2_input_volt) - (2 * A1_input_volt);

// Obsługa alarmu przy nieprawidłowych wartościach napięcia

  // jeżeli któreś z ogniw ma mniejsze lub większe napięcie o 0,1V niż pozostałe
  A0A1_dif = abs(A0_input_volt - A1_input_volt);
  A1A2_dif = abs(A1_input_volt - A2_input_volt);
  A2A0_dif = abs(A2_input_volt - A0_input_volt);

  // Całkowita wartość napięcia
  V_total = A0_input_volt + A1_input_volt + A2_input_volt

  if (A0A1_dif > 0.1 || A1A2_dif > 0.1 || A2A0_dif > 0.1) {
    
    digitalWrite(2, HIGH);
    V_ok = 0;
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Invalid V diff");
    
    lcd.setCursor(0,1);
    lcd.print("A0= ");
    lcd.print(A0_input_volt);
    delay(2000);

    lcd.setCursor(0,1);
    lcd.print("A1= ");
    lcd.print(A1_input_volt);
    delay(2000);

    lcd.setCursor(0,1);
    lcd.print("A2= ");
    lcd.print(A2_input_volt);
    delay(2000);
    
  } else {

    digitalWrite(2, LOW);
    V_ok = 0;
    lcd.clear();
    lcd.print("Voltage OK");
    
    lcd.setCursor(0,1);
    lcd.print("A0= ");
    lcd.print(A0_input_volt);
    delay(2000);

    lcd.setCursor(0,1);
    lcd.print("A1= ");
    lcd.print(A1_input_volt);
    delay(2000);

    lcd.setCursor(0,1);
    lcd.print("A2= ");
    lcd.print(A2_input_volt);
    delay(2000);
  }

  if (V_total < V_total_min) {

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Too low voltage");
    lcd.setCursor(0,1);
    lcd.print("V = ");
    lcd.print(V_total);
    
    digitalWrite(6, HIGH);
    delay(2000);
  }

// Decyzja o podłączeniu ogniw solarnych
// V min 10.95 (3.65/cell), V max 12.75 (4.25/cell)
// V low 11.10 (3.70/cell), V high 12.60 (4.20/cell)

  // Charging start condition
  if (V_total < Charge_limit_low && Charge_run = 0) {
    Charge_run = 1;
  }

  // Charging stop condition
  if (V_total > Charge_limit_high && Charge_run = 1) {
    Charge_run = 0;
  }

  // Start charging
  if (Charge_run = 1) {
    digitalWrite(8, LOW);
  }

 // Stop charging - discharging
  if (Charge_run = 1) {
    digitalWrite(8, HIGH);
  }

// Uruchomienie pompki wody

  lcd.clear();
  lcd.print("Leje wode");
  digitalWrite(6, LOW);
  delay(3000);
  lcd.clear();
  lcd.print("Nie leje wody");
  digitalWrite(6, HIGH);
  delay(3000);
  lcd.clear();
}
