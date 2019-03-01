/*
*  TO DO NOW
*   dodać lcd.print do testów / na stałe
*   read time error on different pin?
*   add variable for last watering datetime
*  
*  TO DO LATER
*   water level indicator / water pump power cutoff -> low water level signal !!!
*   turn on LCD and display all data when button is clicked
*   save logs to SD card
*   connect via WiFi
*   V_limits_ok == false && waterButtonFlag && waterNow -> use buzzer
*   other uses of buzzer
*   describe all variables
*   
*   DONE
*   sprawdzić wydajność pompki, czy na pewno 45.5 ml / sek
*   Wydajność 47 sek > 2L - 42,5 ml sek
*   dodać Serial.print do testów
*/

#include <Wire.h>
#include "DS3231.h"
#include <LiquidCrystal_I2C.h>
#include <TimerOne.h>

// Real Time Clock DS3231
//RTClib RTC;
DS3231 RTC;
bool Century = false;
bool h12 = false;
bool PM = false;

// LCD with I2C module
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
string lastWatering;

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
//Length -> 10 is the seconds
// 1 is Sunday
plannedEvent schedule[]={
  plannedEvent("1 08 00 00 32"),
  plannedEvent("1 23 36 00 32"),
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
int waterButtonPin  = 2;  // On/Off pin for custom watering
int dispButtonPin   = 3;  // Turn on LCD and display all the necessary data
int voltageDiffLED  = 4;  // LED pin for voltage differences alarm - ORANGE / RED
int lowVoltageLED   = 5;  // LED pin for low voltage alarm - RED
int highVoltageLED  = 6;  // LED pin for high voltage alarm - RED
int timeErrorLED    = 7;  // LED pin for read time error - RED
int chargingLED     = 8;  // LED pin for charging indicator - BLUE
int wateringLED     = 9;  // watering is ON, - GREEN
int waterPumpPin    = 10;  // Water pump relay pin (pinDigit)
int solarPanelPin   = 11;  // Solar charger relay pin

// Variables for custom watering using the buttons
volatile int waterButtonFlag  = 0;      // watering button clicked indicator
volatile bool waterNow        = false;  // water pump activation indicator
volatile int checkTimeFlag    = 0;      // flag for time interval interruptions

// Variables for displaying data using the button
volotile int dispButtonFlag   = 0;      // display button clicked indicator
volatile bool displayrNow     = false;  // display activation indicator

// Solar charging
bool Charge_run = false;          // solar charging indicator
float Charge_limit_low  = 10.0;   // voltage value to start charging
float Charge_limit_high = 12.5;   // voltage value to stop charging

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
float A2_correction = 106.7;

int A0_value = 0;
int A1_value = 0;
int A2_value = 0;

bool V_diff_ok    = true;
bool V_limits_ok  = true;

void setup() {

// Setting up output
  Wire.begin();       // Start the I2C interface
  Serial.begin(9600); // Start the serial interface
  lcd.begin(16,2);    // Init the LCD 2x16
  lcd.backlight();    // turn of backlight
  //lcd.autoscroll(); // turn on auto scroll

// Setting up pins

  // Read watering button
  pinMode(waterButtonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(waterButtonPin),buttonClicked, CHANGE);
  pinMode(wateringLED,OUTPUT);
  
  // Read display on/off button
  pinMode(dispButtonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(dispButtonPin),buttonClicked, CHANGE);

  // LEDs
  pinMode(voltageDiffLED,OUTPUT); // LED pin for voltage differences alarm
  pinMode(lowVoltageLED,OUTPUT);  // LED pin for low voltage
  pinMode(highVoltageLED,OUTPUT); // LED pin for high voltage
  pinMode(chargingLED,OUTPUT);    // LED pin for charging indicator
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

// Time reading from RTC
  //DateTime tNow = RTC.now();
  int yearNow     = RTC.getYear();
  int monthNow    = RTC.getMonth(Century);
  int dayNow      = RTC.getDate();
  int wDayNow     = RTC.getDoW();
  int hourNow     = RTC.getHour(h12, PM);
  int minuteNow   = RTC.getMinute();
  int secondNow   = RTC.getSecond();
  
  // add variable for last watering datetime
  float A0A1_dif  = 0.0;
  float A1A2_dif  = 0.0;
  float A2A0_dif  = 0.0;

  /*Serial.print("yearNow = ");
  Serial.println(yearNow);
  Serial.print("monthNow = ");
  Serial.println(monthNow);
  Serial.print("dayNow = ");
  Serial.println(dayNow);
  Serial.print("wDayNow = ");
  Serial.println(wDayNow);
  Serial.print("hourNow = ");
  Serial.println(hourNow);
  Serial.print("minuteNow = ");
  Serial.println(minuteNow);
  Serial.print("secondNow = ");
  Serial.println(secondNow);
  Serial.print("A0A1_dif = ");
  Serial.println(A0A1_dif);
  Serial.print("A1A2_dif = ");
  Serial.println(A1A2_dif);
  Serial.print("A2A0_dif = ");
  Serial.println(A2A0_dif);*/
  //delay(5000);

// Checking the voltage on the cells

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

  /*Serial.print("A0_input_volt = ");
  Serial.println(A0_input_volt);
  Serial.print("A1_input_volt = ");
  Serial.println(A1_input_volt);
  Serial.print("A2_input_volt = ");
  Serial.println(A2_input_volt);*/
  //delay(5000);

// Alarm handling at incorrect voltage values

  // Checking if any of the cells have a 0.1 volt lower or greater voltage than the other ones
  A0A1_dif = abs(A0_input_volt - A1_input_volt);
  A1A2_dif = abs(A1_input_volt - A2_input_volt);
  A2A0_dif = abs(A2_input_volt - A0_input_volt);

  /*Serial.print("A0A1_dif = ");
  Serial.println(A0A1_dif);
  Serial.print("A1A2_dif = ");
  Serial.println(A1A2_dif);
  Serial.print("A2A0_dif = ");
  Serial.println(A2A0_dif);*/
  // delay(5000);

  // Całkowita wartość napięcia
  V_total = A0_input_volt + A1_input_volt + A2_input_volt;

  // Serial.print("V_total = ");
  // Serial.println(V_total);
  // delay(5000);

  if (A0A1_dif > 0.1 || A1A2_dif > 0.1 || A2A0_dif > 0.1) {

    // Turn on alarm LED and change the value of the voltage check variable
    V_diff_ok = false;
    digitalWrite(voltageDiffLED, HIGH);

    // Display error message
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Invalid V diff");
    // Serial.println("Invalid V diff");

    lcd.setCursor(0,1);
    lcd.print("A0= ");
    lcd.print(A0_input_volt);
    // Serial.print("A0= ");
    // Serial.println(A0_input_volt);
    // delay(2000);

    lcd.setCursor(0,1);
    lcd.print("A1= ");
    lcd.print(A1_input_volt);
    // Serial.print("A1= ");
    // Serial.println(A1_input_volt);
    // delay(2000);

    lcd.setCursor(0,1);
    lcd.print("A2= ");
    lcd.print(A2_input_volt);
    // Serial.print("A2= ");
    // Serial.println(A2_input_volt);
    // delay(2000);

  } else {

    // Turn off alarm LED and change the value of the voltage check variable
    V_diff_ok = true;
    digitalWrite(voltageDiffLED, LOW);
    
    // Display info message
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Voltage OK");
     Serial.println("Voltage OK");
    
    lcd.setCursor(0,1);
    lcd.print("A0= ");
    lcd.print(A0_input_volt);
     Serial.print("A0= ");
     Serial.println(A0_input_volt);
     delay(2000);

    lcd.setCursor(0,1);
    lcd.print("A1= ");
    lcd.print(A1_input_volt);
     Serial.print("A1= ");
     Serial.println(A1_input_volt);
     delay(2000);

    lcd.setCursor(0,1);
    lcd.print("A2= ");
    lcd.print(A2_input_volt);
     Serial.print("A2= ");
     Serial.println(A2_input_volt);
     delay(2000);
  }

  // Checking if the voltage value on the 18650 packet is too low or too high
    
  // Turn on alarm LED and change the value of the voltage check variable
  if (V_total < V_total_min) {

    V_limits_ok = false;
    digitalWrite(lowVoltageLED, HIGH);
    
    // Display error message
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Too low voltage");
    // Serial.println("Too low voltage");

    lcd.setCursor(0,1);
    lcd.print("V_min = ");
    lcd.print(V_total_min);
    // Serial.print("V_min = ");
    // Serial.println(V_total_min);
    // delay(2000);

    lcd.setCursor(0,1);
    lcd.print("V_total = ");
    lcd.print(V_total);
    // Serial.print("V_total = ");
    // Serial.println(V_total);
    // delay(2000);
  }

  if (V_total > V_total_max) {

    V_limits_ok = false;
    digitalWrite(highVoltageLED, HIGH);
    
    // Display error message
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Too high voltage");
    // Serial.println("Too high voltage");
    
    lcd.setCursor(0,1);
    lcd.print("V_max = ");
    lcd.print(V_total_max);
    // Serial.print("V_max = ");
    // Serial.println(V_total_max);
    // delay(2000);

    lcd.setCursor(0,1);
    lcd.print("V_total = ");
    lcd.print(V_total);
    // Serial.print("V_total = ");
    // Serial.println(V_total);
    // delay(2000);
  }

  if (V_total > V_total_min && V_total < V_total_max) {
    
    V_limits_ok = true;
    digitalWrite(lowVoltageLED, LOW);
    digitalWrite(highVoltageLED, LOW);
    
    // Display info message
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Voltage in range");
    // Serial.println("Voltage in range.");
    
    lcd.setCursor(0,1);
    lcd.print("V_total = ");
    lcd.print(V_total);
    // Serial.print("V_total = ");
    // Serial.println(V_total);
    // delay(2000);
  }

// Decision about connecting solar cells
// V min 09.00 (3.00/cell), V max 12.80 (4.27/cell)
// V low 11.10 (3.70/cell), V high 12.60 (4.20/cell)

  // Charging start condition
  if (V_total < Charge_limit_low && V_total > V_total_min && Charge_run == false) {
    
    Charge_run = true;
    // Serial.println("Solar charing is needed.");
  }

  // Charging stop condition
  if (V_total > Charge_limit_high && Charge_run == true) {
    
    Charge_run = false;
    // Serial.println("Solar charing is not needed or stopped.");
  }

  // Start charging
  if (Charge_run == true) {
    
    digitalWrite(chargingLED, HIGH);
    digitalWrite(solarPanelPin, LOW);
    // Serial.println("Solar charging started.");
  }

  // Stop charging - discharging
  if (Charge_run == false) {
    
    digitalWrite(chargingLED, LOW);
    digitalWrite(solarPanelPin, HIGH);
    // Serial.println("Solar charging stoped or is not needed.");
  }

  // Starting the water pump
  if (waterButtonFlag && waterNow && V_limits_ok){
    
    delay(250);
    digitalWrite(waterPumpPin,LOW);
    digitalWrite(wateringLED,HIGH);
    // Serial.println("The button is pressed, the water pump is working.");
  }

  if (waterButtonFlag == 0 && waterNow){
    
    digitalWrite(waterPumpPin,HIGH);
    digitalWrite(wateringLED,LOW);
    // Serial.println("The button has been released, the water pump stopped working.");
  }

  if (waterButtonFlag == 0 && checkTimeFlag){
    
    // Serial.println("Inside: if(waterButtonFlag == 0 && checkTimeFlag)");
    
    if (yearNow < 50) {   // Check if read datetime is not 1/1/1960
      
      //Serial.println("Read from RTC is OK");

      for (int i = 0; i < eventCount; i++) {

        // Serial.println("Looping through event schedule.");
        plannedEvent event = schedule[i];

        if (wDayNow == event.WeekDay){

          // Serial.println("Weekday matches the schedule element.");
          
      // Watering
          if (hourNow == event.Hour && minuteNow == event.Min /*&& secondNow == event.Sec*/){

            // Serial.println("Hour and minute matches the schedule element. Watering...");
  
            digitalWrite(waterPumpPin,LOW);
            digitalWrite(wateringLED,HIGH);
            delay(event.Dlugosc * 100);
  
            digitalWrite(waterPumpPin,HIGH);
            digitalWrite(wateringLED,LOW);
  
            // Serial.println("Watering finished.");

            delay(60000);   // Delay not to fall into the same loop for the second time.
          }
        }
      }

      if (secondNow == 30){ // Just checking if system is working.

        ledBlink(wateringLED, 5, 250);
      }

      checkTimeFlag=0;
    } else {

      if (!yearNow > 50) {  // In case of disconnection of RTC or RTC has a malfunction
        
        // Serial.println("The DS3231 is stopped.  Please run the SetTime or check the circuitry.");
        ledBlink(wateringLED, 3, 250);
      }

      delay(9000);
    }
  }
  
  if (dispButtonPin == LOW) {
	  
	// Display all data
	lcd.clear();
    lcd.setCursor(0,0);
	lcd.print("Hello :)");
	lcd.setCursor(0,1);
	lcd.print("Watering system");
	
	// Date and time
	lcd.clear();
    lcd.setCursor(0,0);
	lcd.print("Date and time");
	lcd.print("yearNow = ");
    lcd.println(yearNow);
    lcd.print("monthNow = ");
    lcd.println(monthNow);
    lcd.print("dayNow = ");
    lcd.println(dayNow);
    lcd.print("wDayNow = ");
    lcd.println(wDayNow);
    lcd.print("hourNow = ");
    lcd.println(hourNow);
    lcd.print("minuteNow = ");
    lcd.println(minuteNow);
    lcd.print("secondNow = ");
    lcd.println(secondNow);
    lcd.print("A0A1_dif = ");
    lcd.println(A0A1_dif);
    lcd.print("A1A2_dif = ");
    lcd.println(A1A2_dif);
    lcd.print("A2A0_dif = ");
    lcd.println(A2A0_dif);
	// Voltage values
	
	// Voltage errors
	
	// Charging status
	
	// Last watering datetime
  }
}

void ledBlink(int pinLED, int blinkCount, int intervalTime) {

  for (int i; i < blinkCount; i++) {
    digitalWrite(pinLED, HIGH);
    delay(intervalTime);
    digitalWrite(pinLED, LOW);
    delay(intervalTime);
  }
}

/*void print2digits(int number) {   // Nice to have
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}*/

void buttonClicked(){
  if(digitalRead(waterButtonPin)== LOW){
    waterButtonFlag = 1;
  }
  else{
    waterButtonFlag = 0;
  }
  waterNow= true;
}

void ReadTimeNow(){
  checkTimeFlag=1;
}
