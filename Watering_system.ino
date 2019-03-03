/* TO DO NOW
*   
*  TO DO LATER
*   water level indicator / water pump power cutoff -> low water level signal !!!
*   save logs to SD card
*   connect via WiFi
*   V_limits_ok == false && waterButtonFlag && waterNow -> use buzzer
*   other uses of buzzer
*   
*  DONE
*   sprawdzić wydajność pompki, czy na pewno 45.5 ml / sek
*   Wydajność 47 sek > 2L - 42,5 ml sek
*   dodać Serial.print do testów
*   read time error on different pin?
*   add variable for last watering datetime
*   turn on LCD and display all data when button is clicked
*   dodać lcd.print do testów / na stałe - turn off lcd on start adn turn on when button clicked
*   describe all variables
*/

#include <Wire.h>
#include "DS3231.h"
#include <LiquidCrystal_I2C.h>
#include <TimerOne.h>

// Real Time Clock DS3231
DS3231 RTC;
bool Century = false;
bool h12 = false;
bool PM = false;

// LCD with I2C module
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
//string lastWatering;

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
int voltageDiffLED  = 4;  // LED pin for voltage differences alarm - YELLOW
int lowVoltageLED   = 5;  // LED pin for low voltage alarm - RED
int highVoltageLED  = 6;  // LED pin for high voltage alarm - RED
int timeErrorLED    = 7;  // LED pin for read time error - RED
int chargingLED     = 8;  // LED pin for charging indicator - BLUE
int wateringLED     = 9;  // watering is ON, - GREEN
int waterPumpPin    = 10;  // Water pump relay pin
int solarPanelPin   = 11;  // Solar charger relay pin

// Variables for custom watering using the buttons
volatile int waterButtonFlag  = 0;      // watering button clicked indicator
volatile bool waterNow        = false;  // water pump activation indicator
volatile int checkTimeFlag    = 0;      // flag for time interval interruptions

// Variables for displaying data using the button
volatile int dispButtonFlag   = 0;      // display button clicked indicator
volatile bool displayrNow     = false;  // display activation indicator
String dateWaterScheduleLCD;
String timeWaterScheduleLCD;
String dateWaterCustomLCD;
String timeWaterCustomLCD;

// Solar charging
bool Charge_run = false;          // solar charging indicator
float Charge_limit_low  = 10.0;   // voltage value to start charging
float Charge_limit_high = 12.5;   // voltage value to stop charging

// Variables for 18650 voltage calculations
float A0_input_volt = 0.0;  // calculated input voltage on cell 1
float A1_input_volt = 0.0;  // calculated input voltage on cell 2
float A2_input_volt = 0.0;  // calculated input voltage on cell 3
float V_total       = 0.0;  // sum of valtages on all cells
float V_total_min   = 9.0;  // minimal safe voltage value
float V_total_max   = 12.8; // maximal safe coltage value

float A0_correction = 105.9;  // voltage read correction on cell 1
float A1_correction = 105.8;  // voltage read correction on cell 2
float A2_correction = 106.7;  // voltage read correction on cell 3

int A0_value = 0; // raw analog input value from cell 1
int A1_value = 0; // raw analog input value from cell 2
int A2_value = 0; // raw analog input value from cell 3

bool V_diff_ok    = true; // voltage difference between cells
bool V_limits_ok  = true; // voltage limits for whole battery pack

void setup() {

// Setting up output
  Wire.begin();       // Start the I2C interface
  Serial.begin(9600); // Start the serial interface
  lcd.begin(16,2);    // Init the LCD 2x16
  //lcd.backlight();    // turn of backlight
  //lcd.autoscroll(); // turn on auto scroll

// Setting up pins

  // Read watering button
  pinMode(waterButtonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(waterButtonPin),buttonClicked, CHANGE);
  pinMode(wateringLED,OUTPUT);
  
  // Read display on/off button
  pinMode(dispButtonPin, INPUT_PULLUP);

  // LEDs
  pinMode(voltageDiffLED,OUTPUT); // LED pin for voltage differences alarm
  pinMode(lowVoltageLED,OUTPUT);  // LED pin for low voltage
  pinMode(highVoltageLED,OUTPUT); // LED pin for high voltage
  pinMode(timeErrorLED,OUTPUT); // LED pin for read time error
  pinMode(chargingLED,OUTPUT);    // LED pin for charging indicator
  pinMode(wateringLED,OUTPUT);    // watering is ON, read time error (pinLED)

  // Water pump relay pin
  pinMode(waterPumpPin,OUTPUT);
  digitalWrite(waterPumpPin, HIGH);

  // Solar charger relay pin
  pinMode(solarPanelPin,OUTPUT);
  digitalWrite(solarPanelPin, HIGH);
  
  // Initialize interruptions
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(ReadTimeNow);
}

void loop() {

// Time reading from RTC
  //DateTime tNow = RTC.now();
  int yearNow       = RTC.getYear();          // current year from real time clock
  int monthNow      = RTC.getMonth(Century);  // current month from real time clock
  int dayNow        = RTC.getDate();          // current day from real time clock
  int wDayNow       = RTC.getDoW();           // current weekday from real time clock (1 = SUNDAY)
  int hourNow       = RTC.getHour(h12, PM);   // current hour from real time clock
  int minuteNow     = RTC.getMinute();        // current minute from real time clock
  int secondNow     = RTC.getSecond();        // current second from real time clock

  // Current date and time to display on lcd
  String dateNowLCD = "Date: 20" + String(yearNow) + "/" + get2digits(monthNow) + "/" + get2digits(dayNow);
  String timeNowLCD = "Time: " + get2digits(hourNow) + ":" + get2digits(minuteNow) + ":" + get2digits(secondNow);
  
  float A0A1_dif  = 0.0; // voltage difference between cell 1 and 2
  float A1A2_dif  = 0.0; // voltage difference between cell 2 and 3
  float A2A0_dif  = 0.0; // voltage difference between cell 3 and 1

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
    /*Serial.println("Invalid V diff");
    Serial.print("A0= ");
    Serial.println(A0_input_volt);
    Serial.print("A1= ");
    Serial.println(A1_input_volt);
    Serial.print("A2= ");
    Serial.println(A2_input_volt);
    delay(2000);*/

  } else {

    // Turn off alarm LED and change the value of the voltage check variable
    V_diff_ok = true;
    digitalWrite(voltageDiffLED, LOW);
    
    // Display info message
    /*Serial.println("Voltage OK");
    Serial.print("A0= ");
    Serial.println(A0_input_volt);
    Serial.print("A1= ");
    Serial.println(A1_input_volt);
    Serial.print("A2= ");
    Serial.println(A2_input_volt);
    delay(2000);*/
  }

  // Checking if the voltage value on the 18650 packet is too low or too high
    
  // Turn on alarm LED and change the value of the voltage check variable
  if (V_total < V_total_min) {

    V_limits_ok = false;
    digitalWrite(lowVoltageLED, HIGH);
    
    // Display error message
    /*Serial.println("Too low voltage");
    Serial.print("V_min = ");
    Serial.println(V_total_min);
    Serial.print("V_total = ");
    Serial.println(V_total);
    delay(2000);*/
  }

  if (V_total > V_total_max) {

    V_limits_ok = false;
    digitalWrite(highVoltageLED, HIGH);
    
    // Display error message
    /*Serial.println("Too high voltage");
    Serial.print("V_max = ");
    Serial.println(V_total_max);
    Serial.print("V_total = ");
    Serial.println(V_total);
    delay(2000);*/
  }

  if (V_total > V_total_min && V_total < V_total_max) {
    
    V_limits_ok = true;
    digitalWrite(lowVoltageLED, LOW);
    digitalWrite(highVoltageLED, LOW);
    
    // Display info message
    /*Serial.println("Voltage in range.");
    Serial.print("V_total = ");
    Serial.println(V_total);
    delay(2000);*/
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
    dateWaterCustomLCD = "Date: 20" + String(yearNow) + "/" + get2digits(monthNow) + "/" + get2digits(dayNow);
    timeWaterCustomLCD = "Time: " + get2digits(hourNow) + ":" + get2digits(minuteNow) + ":" + get2digits(secondNow);
    // Serial.println("The button is pressed, the water pump is working.");
  }

  if (waterButtonFlag == 0 && waterNow){
    
    digitalWrite(waterPumpPin,HIGH);
    digitalWrite(wateringLED,LOW);
    // Serial.println("The button has been released, the water pump stopped working.");
  }

  if (waterButtonFlag == 0 && checkTimeFlag){
    
    if (yearNow < 50) {   // Check if read datetime is not 1/1/1960

      digitalWrite(timeErrorLED,LOW);
      //Serial.println("Read from RTC is OK");

      for (int i = 0; i < eventCount; i++) {

        plannedEvent event = schedule[i];
        // Serial.println("Looping through event schedule.");

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

            dateWaterScheduleLCD = "Date: 20" + String(yearNow) + "/" + get2digits(monthNow) + "/" + get2digits(dayNow);
            timeWaterScheduleLCD = "Time: " + get2digits(hourNow) + ":" + get2digits(minuteNow) + ":" + get2digits(secondNow);
  
            // Serial.println("Watering finished.");

            delay(60000);   // Delay not to fall into the same loop for the second time.
          }
        }
      }

      checkTimeFlag=0;
    } else {

      if (yearNow > 50) {  // In case of disconnection of RTC or RTC has a malfunction

        digitalWrite(timeErrorLED,HIGH);
        // Serial.println("The DS3231 is stopped.  Please run the SetTime or check the circuitry.");
      }

      delay(9000);
    }
  }

// Turn on lcd and display all data

  if (dispButtonPin == LOW) {

  lcd.backlight();
  lcd.display();
  
  // Display basic data
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Watering system");
  lcd.setCursor(0,1);
  lcd.print("Hello :)");
  delay(3000);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Current DateTime");
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(dateNowLCD);
  lcd.setCursor(0,1);
  lcd.print(timeNowLCD);
  delay(3000);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Watering system");
  lcd.setCursor(0,1);
  lcd.print("Hello :)");
  delay(3000);

  // Voltage values
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Voltage values");

  lcd.setCursor(0,1);
  lcd.print("A0 = ");
  lcd.println(A0_input_volt);
  delay(2000);

  lcd.setCursor(0,1);
  lcd.print("A1 = ");
  lcd.println(A1_input_volt);
  delay(2000);

  lcd.setCursor(0,1);
  lcd.print("A2 = ");
  lcd.println(A2_input_volt);
  delay(3000);
  
  // Voltage errors
  if (V_diff_ok && V_limits_ok) {
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("No voltage errs");
    delay(2000);
  } else {
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Voltage errors");
    delay(2000);

    if (V_diff_ok == false) {

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Invalid V diff");
      delay(2000);
  
      lcd.setCursor(0,1);
      lcd.print("A0A1_dif = ");
      lcd.println(A0A1_dif);
      delay(2000);
    
      lcd.setCursor(0,1);
      lcd.print("A1A2_dif = ");
      lcd.println(A1A2_dif);
      delay(2000);
    
      lcd.setCursor(0,1);
      lcd.print("A2A0_dif = ");
      lcd.println(A2A0_dif);
      delay(3000);
    }

    if (V_limits_ok == false) {
      
      if (V_total < V_total_min) {

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Too low voltage");
    
        lcd.setCursor(0,1);
        lcd.print("V_min = ");
        lcd.print(V_total_min);
        delay(2000);
    
        lcd.setCursor(0,1);
        lcd.print("V_total = ");
        lcd.print(V_total);
        delay(3000);
      }

      if (V_total > V_total_max) {

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Too high voltage");
        
        lcd.setCursor(0,1);
        lcd.print("V_max = ");
        lcd.print(V_total_max);
        delay(2000);
    
        lcd.setCursor(0,1);
        lcd.print("V_total = ");
        lcd.print(V_total);
        delay(3000);
      }
    }
  }

  // Charging status
  if (Charge_run) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Battery charging");
    lcd.setCursor(0,1);
    lcd.print("In progress");
    delay(3000);
  } else {
    lcd.setCursor(0,0);
    lcd.print("Battery charging");
    lcd.setCursor(0,1);
    lcd.print("Stopped");
    delay(3000);
  }
  
  // Last watering datetime
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Last watering");
  delay(2000);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Custom");
  lcd.setCursor(0,1);
  lcd.print("Date and Time");
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(dateWaterCustomLCD);
  lcd.setCursor(0,1);
  lcd.print(timeWaterCustomLCD);
  delay(3000);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Scheduled");
  lcd.setCursor(0,1);
  lcd.print("Date and Time");
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(dateWaterScheduleLCD);
  lcd.setCursor(0,1);
  lcd.print(timeWaterScheduleLCD);
  delay(3000);

  lcd.clear();
  lcd.noBacklight();
  lcd.noDisplay();
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

String get2digits(int number) { // return number lower that 10 as string with 0 as prefix
  String str;
	if (number >= 0 && number < 10) {
		str = "0" + String(number);
	} else {
    str = String(number);
	}
	return str;
}

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
