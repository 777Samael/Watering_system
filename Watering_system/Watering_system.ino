/*
*  TO DO NOW
*   dodać Serial.print do testów
*   dodać lcd.print do testów / na stałe
*  
*  TO DO LATER
*   water level indicator / water pump power cutoff -> low water level signal !!!
*   save logs to SD card
*   connect via WiFi
*   V_limits_ok == false && buttonFlag && waterNow -> use buzzer
*   other uses of buzzer
*   
*   DONE
*   sprawdzić wydajność pompki, czy na pewno 45.5 ml / sek
*   Wydajność 47 sek > 2L - 42,5 ml sek
*/

//#include <Wire.h>
#include "DS3231.h"
#include <LiquidCrystal_I2C.h>
#include <TimerOne.h>

// Real Time Clock DS3231
//RTClib RTC;
DS3231 RTC;
bool Century=false;
bool h12;
bool PM;

//tmElements_t tm;

// LCD with I2C module
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

class plannedEvent{
  public:
  plannedEvent(const char* value);
  int Hour, Min, Sec;
  int WeekDay;
  int Dlugosc;
};

// Watering schedule
plannedEvent::plannedEvent(const char* value)
{
  sscanf(value, "%d %d %d %d %d", &WeekDay, &Hour, &Min, &Sec, &Dlugosc);
}
//Dlugosc  ---- 10 to sekunda !!!
// 1 to niedziela
plannedEvent schedule[]={  // term->schedule
  plannedEvent("1 08 00 00 32"),
  plannedEvent("1 20 00 00 32"),
  plannedEvent("2 08 00 00 32"),
  plannedEvent("2 20 00 00 32"),
  plannedEvent("3 08 00 00 32"),
  plannedEvent("3 20 00 00 32"),
  plannedEvent("4 08 00 00 32"),
  plannedEvent("4 20 00 00 32"),
  plannedEvent("5 08 00 00 32"),
  plannedEvent("5 20 00 00 32"),
  plannedEvent("6 08 00 00 32"),
  plannedEvent("6 20 00 00 32"),
  plannedEvent("7 08 00 00 32"),
  plannedEvent("7 20 00 00 32"),
  };

int eventCount = 14;

// I/O pins
int buttonPin       = 2;  // On/Off pin for custom watering
int voltageDiffPin  = 3;  // LED pin for voltage differences alarm     voltageDiffPin -> voltageDiffLED ??? itd
int LowVoltagePin   = 4;  // LED pin for low / high voltage alarm
int HighVoltagePin  = 5;  // LED pin for low / high voltage alarm
int chargingPin     = 6;  // LED pin for charging indicator
int wateringLED     = 7;  // watering is ON, read time error (pinLED)

int waterPumpPin  = 8;  // Water pump relay pin (pinDigit)
int solarPanelPin = 9;  // Solar charger relay pin

// Variables for custom watering using button
volatile int buttonFlag     = 0;
volatile bool waterNow      = false;
volatile int checkTimeFlag  = 0;

// Solar charging
bool Charge_run = false;
float Charge_limit_low  = 10.0;
float Charge_limit_high = 12.5;

// Variables for 18650 voltage calculations
float A0_input_volt = 0.0;
float A1_input_volt = 0.0;
float A2_input_volt = 0.0;
float V_total       = 0.0;
float V_total_min   = 9.0;
float V_total_max   = 12.8;

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

bool V_diff_ok    = true;
bool V_limits_ok  = true;

void setup() {

// Setting up output
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.backlight();
  //lcd.autoscroll();

// Setting up pins

  // Read watering button
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin),buttonClicked, CHANGE);
  pinMode(wateringLED,OUTPUT);

  // LEDs
  pinMode(voltageDiffPin,OUTPUT); // LED pin for voltage differences alarm
  pinMode(LowVoltagePin,OUTPUT);  // LED pin for low voltage
  pinMode(HighVoltagePin,OUTPUT); // LED pin for high voltage
  pinMode(chargingPin,OUTPUT);    // LED pin for charging indicator
  pinMode(wateringLED,OUTPUT);    // watering is ON, read time error (pinLED)

  // Water pump relay pin
  pinMode(waterPumpPin,OUTPUT);
  digitalWrite(waterPumpPin, HIGH);

  // Solar charger relay pin
  pinMode(solarPanelPin,OUTPUT);
  digitalWrite(solarPanelPin, HIGH);
  
  // Initialize interruptions?
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(ReadTimeNow);

}

void loop() {

// Odczyt czasu z RTC
  //DateTime tNow = RTC.now();
  int yearNow     = RTC.getYear();
  //int monthNow    = RTC.getMonth(Century);
  //int dayNow      = RTC.getDate();
  int wDayNow     = RTC.getDoW();
  int hourNow     = RTC.getHour(h12, PM);
  int minuteNow   = RTC.getMinute();
  int secondNow   = RTC.getSecond();
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
  V_total = A0_input_volt + A1_input_volt + A2_input_volt;

  if (A0A1_dif > 0.1 || A1A2_dif > 0.1 || A2A0_dif > 0.1) {

    // Turn on alarm LED and change the value of voltage check variable
    V_diff_ok = false;
    digitalWrite(2, HIGH);

    // Display error message
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

    // Turn off alarm LED and change the value of voltage check variable
    V_diff_ok = true;
    digitalWrite(2, LOW);
    
    // Display info message
    lcd.clear();
    lcd.setCursor(0,0);
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

  // Jeżeli watość napięcia na pakiecie 18650 jest zbyt niska lub zbyt wysoka
    
  // Turn on alarm LED and change the value of voltage check variable
  if (V_total < V_total_min) {

    V_limits_ok = false;
    digitalWrite(3, HIGH);
    
    // Display error message
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Too low voltage");

  lcd.setCursor(0,1);
    lcd.print("V_min = ");
    lcd.print(V_total_min);
    delay(2000);

    lcd.setCursor(0,1);
    lcd.print("V = ");
    lcd.print(V_total);
    delay(2000);
  }

  if (V_total > V_total_max) {

    V_limits_ok = false;
    digitalWrite(4, HIGH);
    
    // Display error message
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Too high voltage");
    
    lcd.setCursor(0,1);
    lcd.print("V_max = ");
    lcd.print(V_total_max);
    delay(2000);

    lcd.setCursor(0,1);
    lcd.print("V = ");
    lcd.print(V_total);
    delay(2000);
  }

  if (V_total > V_total_min && V_total < V_total_max) {
    
    V_limits_ok = true;
    digitalWrite(4, LOW);
    
    // Display info message
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Voltage in range");
    
    lcd.setCursor(0,1);
    lcd.print("V = ");
    lcd.print(V_total);
    delay(2000);
  }

// Decyzja o podłączeniu ogniw solarnych
// V min 10.95 (3.65/cell), V max 12.75 (4.25/cell)
// V low 11.10 (3.70/cell), V high 12.60 (4.20/cell)

  // Charging start condition
  if (V_total < Charge_limit_low && Charge_run == false) {
    Charge_run = true;
  }

  // Charging stop condition
  if (V_total > Charge_limit_high && Charge_run == true) {
    Charge_run = false;
  }

  // Start charging
  if (Charge_run == true) {
    digitalWrite(solarPanelPin, LOW);
  }

 // Stop charging - discharging
  if (Charge_run == false) {
    digitalWrite(solarPanelPin, HIGH);
  }

// Uruchomienie pompki wody

  if(buttonFlag && waterNow && V_limits_ok){
    delay(250);
    digitalWrite(waterPumpPin,LOW);
    digitalWrite(wateringLED,HIGH);
    
  }

  if(buttonFlag == 0 && waterNow){
    digitalWrite(waterPumpPin,HIGH);
    digitalWrite(wateringLED,LOW);
    
  }

  if(buttonFlag == 0 && checkTimeFlag){
    if (yearNow < 50) {
      //Serial.print("Ok, Time = ");
      
      //print2digits(tm.Hour);
      //Serial.write(':');
      //print2digits(tm.Minute);
      //Serial.write(':');
      //print2digits(tm.Second);
      
      //Serial.print(", Date (D/M/Y) = ");
      //Serial.print(tm.Day);
      //Serial.write('/');
      //Serial.print(tm.Month);
      //Serial.write('/');
      //Serial.print(tmYearToCalendar(tm.Year));
      //Serial.println();

      for (int i = 0; i < eventCount; i++) {

        plannedEvent event = schedule[i]; // term->event; term->schedule
        //Serial.println(weekday(makeTime(tm)));
        //Serial.print(i + 1);
        //.println();
        if (wDayNow == event.WeekDay){ // weekdaycalc-> wDayNow 
          
          //Serial.println("Week Day OK");
          //Serial.print(event.Hour);
          //Serial.write(':');
          //Serial.print(event.Min);
          //Serial.write(':');
          //Serial.print(event.Sec);
          //Serial.println("");
          
      // Watering
          if(hourNow == event.Hour && minuteNow == event.Min && secondNow == event.Sec){ // tm.Hour -> hourNow; tm.Minute -> minuteNow; tm.Second -> secondNow
            //Serial.println("Time OK. Podlewamy!");
            //Serial.println(event.Dlugosc);
  
            digitalWrite(waterPumpPin,LOW);
            digitalWrite(wateringLED,HIGH);
            delay(event.Dlugosc * 100);
  
            digitalWrite(waterPumpPin,HIGH);
            digitalWrite(wateringLED,LOW);
  
            //Serial.println("Podlane");

            delay(1000);
          }
        }
      }

      if (secondNow == 30){
      ledBlink(wateringLED, 5, 100);

      }
      checkTimeFlag=0;

    } else {
    // In case of disconnection of RTC or RTC has a malfunction
      if (yearNow < 50) {
        //Serial.println("The DS3231 is stopped.  Please run the SetTime");
        //Serial.println("example to initialize the time and begin running.");
        //Serial.println();
      ledBlink(wateringLED, 2, 250);

      } else {
        //Serial.println("DS3231 read error!  Please check the circuitry.");
        //Serial.println();
      ledBlink(wateringLED, 2, 250);
      }

      delay(9000);
    }
  }

    /*
    lcd.clear();
    lcd.print("Leje wode");
    digitalWrite(waterPumpPin, LOW);
    delay(3000);
    lcd.clear();
    lcd.print("Nie leje wody");
    digitalWrite(waterPumpPin, HIGH);
    delay(3000);
    lcd.clear();
    */
  }

void ledBlink(int pinLED, int blinkCount, int intervalTime) {

  for (int i; i < blinkCount; i++) {
    digitalWrite(pinLED, HIGH);
    delay(intervalTime);
    digitalWrite(pinLED, LOW);
    delay(intervalTime);
  }
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void buttonClicked(){
  if(digitalRead(buttonPin)== LOW){
    buttonFlag = 1;
  }
  else{
    buttonFlag = 0;
  }
  waterNow= true;
}

void ReadTimeNow(){
  checkTimeFlag=1;
}
