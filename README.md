ENG

# Watering system - Arduino

Plant watering system using Arduino Nano, real time clock (RTC DS3231) and 12V water pump.

Plant irrigation system.
I created this device to make it easier for me to water my garden on the balcony.
The main task of the device is watering plants according to the set schedule.
In addition, the device is equipped with LCD display, which displays the necessary information, 18650 batteries and a few other smaller elements.

A description of the operation.
The code contains an array with watering schedule detailing each day of the week, the specific hour and length of watering. After verifying the correctness of reading the current date and time and the state of voltages on the cells, the water pump is started.

Checking 18650 cells consists of:
- voltage test on individual cells
- calculating the voltage difference between the individual cells
- calculation of the voltage sum on all cells

If the difference in voltage on the cells is greater than the one provided for in the code, the yellow LED lights up, the rest of the process goes unchanged.
If the sum of voltages on the cells is too small or too high, the corresponding red diode lights up and the watering from the schedule and manual watering are stopped.

If the watering button is pressed, the conditions for correcting the voltages are checked and the watering is started.

If you press the button responsible for displaying information on the LCD all data is displayed.
Informations displayed:
- Current date and time
- Voltages at individual cells
- Cell voltage errors
- Time reading error
- Cell charging status
- Date and time of the last manual watering
- Date and time of last watering from the schedule

List of worship:
- Arduino Nano (copy) with ATMEL ATMEGA328P-AU microcontroller and CH340G USB converter
- Shield I/O for Arduino NANO 3.0
- Real-time clock RTC DS3231
- 1602 HD44780 with IIC/I2C LCD serial interface adapter module
- 2x 5V relay
- step down transformer
- 3x 18650 cell
- Water pump 12V 4.2W 240 L/h | Perfomance 47 sec > 2L | 42.5 ml/s 153 L/h
- Battery management system BMS 3S
- self-built voltage divider
- Polystyrene Enclosure Box Z-15 (89 x 149 x 253)
- 5mm LEDs (1x yellow, 3x red, 1x blue, 1x green)
- float switch
- 2x I/O switch
- resistors (4x 1 kOhm, 1x 2kOhm)
- M3 screws 12mm and 16mm
- gold pins cables
- 4mm mdf board
- transparent acrylic glass

-----------------------------------------------------------------------------------------------------------------------------

PL

# Watering system - Arduino

System nawadniania roślin wykorzystujący Arduino Nano, zegar czasu rzeczywistego (RTC DS3231) i pompę wody 12V.

System nawadniania roślin.
Stworzyłem to urządzenie żeby ułatwić sobie podlewanie mojego ogródka warzywnego na balkonie.
Głównym zadaniem urządzenia jest podlewanie roślin zgodnie z ustalonym harmonogramem.
Dodatkowo urządzenie wyposażone jest w wyświtlacz LCD, na którym wyświetlane są potrzebne informacje, akumulatory 18650 i kilka innych mnijeszych elementów.

Opis działania.
Kod zawiera tablicę z harmonogramem podlewania z wyszczególnieniem każdego dnia tygodnia, konkretnej godziny i długości podlewania. Po sprawdzeniu poprawności odczytu aktualnej daty i godziny oraz stanu napięć na ogniwach uruchamiana jest pompka wody.

Sprawdzenie ogniw 18650 polega na:
- teście napięcia na poszczególnych ogniwach
- obliczeniu różnicy napięć między poszczególnymi ogniwami
- obliczeniu sumy napięcia na wszystkich ogniwach

Jeżeli różnica napięć na ogniwach jest większa niż przewidziana w kodzie to zostaje zapalona dioda żółta, pozostała część procesu przechodzi bez zmian.
Jeżeli suma napięć na ogniwach jest zbyt mała lub zbyt duża zapala się odpowiednia czerwona dioda, a podlewanie z harmonogramu i manualne zostaje wstrzymane.

Jeżeli zostanie naciśnięty przycisk podlewania zostają sprawdzone warunki poprawności napięć i zostaje uruchomione podlewanie.

Jeżeli zostanie naciśnięty przycisk odpowiadający za wyświetlanie informacji na LCD wszystkie dane zostają wyświetlone.
Wyświetlone informacje:
- Aktualna data i godzina
- Napięcia na poszczególnych ogniwach
- Błędy napięcia na ogniwach
- Błedy odczytu czasu ----------------------- !!!!!!!!
- Status ładowania ogniw
- Data i godzina ostatniego manualnego podlewania
- Data i godzina ostatniego podlewania z harmonogramu

Lista cześi:
- Arduino Nano (kopia) with ATMEL ATMEGA328P-AU microcontroller and CH340G USB converter
- Shield I/O for Arduino NANO 3.0
- Real-time clock RTC DS3231
- wyświtlacz LCD 1602 HD44780 with IIC/I2C serial interface adapter module
- 2x przekaźnik 5V
- transformator step down
- 3x ogniwo 18650
- Battery management system BMS 3S
- Pompka wody 12V 4.2W 240 L/h | Wydajność 47 sek > 2L | 42,5 ml/s | 153 L/h
- samodzielnie zbudowany dzielnik napięcia
- obudowa Z15 (89 x 149 x 253) - Polystyrene Enclosure Box Z-15
- kolorowe diody LED 5mm (1x żółta, 3x czerwona, 1x niebieska, 1x zielona)
- wyłącznik pływakowy - float switch
- przełącznik I/O
- rezystory (4x 1 kOhm, 1x 2kOhm)
- śruby M3 12mm i 16mm
- kable do gold pinów
- płyta mdf 4mm
- płyta akrylowa przeźroczysta
