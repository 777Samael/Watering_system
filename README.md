ENG

# Watering system - Arduino

### Plant watering system based on Arduino Nano, real time clock (RTC DS3231) and 12V water pump.

Plant irrigation system.
I created this device to make it easier for me to water my garden on the balcony.
The main task of the device is watering plants according to the set schedule.
In addition, the device is equipped with LCD display, which displays the necessary information, 18650 batteries and a few other smaller elements.

The 18650 cells are charged using a photovoltaic panel. The housing has a standard 5.5 / 2.1mm connectors, so you can connect it to an ordinary power supply with a min. 12,6V.
In order to maintain the good condition of the cells, they are charged only when they reach the appropriate level of discharge.

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
- Cell charging status
- Date and time of the last manual watering
- Date and time of last watering from the schedule

List of parts:
- Arduino Nano (copy) with ATMEL ATMEGA328P-AU microcontroller and CH340G USB converter
- Shield I/O for Arduino NANO 3.0
- Real-time clock RTC DS3231
- 1602 HD44780 with IIC/I2C LCD serial interface adapter module
- 2x 5V relay module
- step down transformer
- 3x 18650 cell
- Battery management system BMS 3S
- 18650 4pcs holder
- Water pump 12V 4.2W (240 L/h described by seller) | Real perfomance (measured) 47 sec > 2L | 42.5 ml/s 153 L/h
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

I ordered most of the elements in China, other parts are from local sellers.

Voltage divider
The divider is an element made independently on the soldering prototype board with the use of resistors and screwed connectors.

#### The code for the voltage divider diagram at [Flastad.com](http://www.falstad.com/circuit/) is at the end of the readme.

-----------------------------------------------------------------------------------------------------------------------------

PL

# System nawadniania - Arduino

### System nawadniania roślin oparty o Arduino Nano, zegar czasu rzeczywistego (RTC DS3231) i pompę wody 12V.

System nawadniania roślin.
Jest to mój pierwszy "większy" projekt, dlatego wykonanie i kod są dość proste.
Stworzyłem to urządzenie żeby ułatwić sobie podlewanie mojego ogródka warzywnego na balkonie.
Głównym zadaniem urządzenia jest podlewanie roślin zgodnie z ustalonym harmonogramem.
Dodatkowo urządzenie wyposażone jest w wyświtlacz LCD, na którym wyświetlane są potrzebne informacje, akumulatory 18650 i kilka innych mnijeszych elementów.

Ogniwa 18650 ładowane są za pomocą panelu fotowoltaicznego. W obudowie zamontowałem standardowe złącze 5,5/2,1mm więc równie dobrze można podłączyć zwykły zasilacz dający min 12,6V.
Żeby utrzymać dobrą kondycję ogniw są one ładowane dopiero jak osiągną odpowiedni poziom rozładowania.

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
- Status ładowania ogniw
- Data i godzina ostatniego manualnego podlewania
- Data i godzina ostatniego podlewania z harmonogramu

Lista części:
- Arduino Nano (kopia) with ATMEL ATMEGA328P-AU microcontroller and CH340G USB converter
- Shield I/O for Arduino NANO 3.0
- Real-time clock RTC DS3231
- wyświtlacz LCD 1602 HD44780 with IIC/I2C serial interface adapter module
- 2x moduł przekaźnika 5V
- transformator step down
- 3x ogniwo 18650
- Battery management system BMS 3S
- koszyczek na 4 ogniwa 18650
- Pompka wody 12V 4.2W (240 L/h opisane przez sprzedawcę) | Rzeczywista wydajność 47 sek > 2L | 42,5 ml/s | 153 L/h
- samodzielnie zbudowany dzielnik napięcia
- obudowa Z15 (89 x 149 x 253) - Polystyrene Enclosure Box Z-15
- kolorowe diody LED 5mm (1x żółta, 3x czerwona, 1x niebieska, 1x zielona)
- wyłącznik pływakowy
- 2x przełącznik I/O
- rezystory (4x 1 kOhm, 1x 2kOhm)
- śruby M3 12mm i 16mm
- kable do gold pinów
- płyta mdf 4mm
- płyta akrylowa przeźroczysta

Większość elementów zamawiałem w chinach, pozostałe u lokalnych sprzedawców.

Dzielnik napięcia
Dzielnik jest elementem wykonanym samodzielnie na płytce prototypowej z użyciem rezystorów i złącz skręcanych. soldering prototype board

#### Na końcu readme znajduje się kod do schematu dzielnika napięcia do wykorzystania na stronie [Flastad.com](http://www.falstad.com/circuit/)

### Wskazówki

Wykonanie takiego układu nie jest bardzo skomplikowane dlatego poniżej znajdziecie opis rzeczy które mogą sprawić problemy i kilka rad dla początkujących.

Przede wszystkim każdy z elementów najlepiej jest podłączać i konfigurować oddzielnie.
Polecam zacząć od konfiguracji RTC i LCD. Dość popularnym modułem RTC jest model 1307, ale ze względu na podatność modułu na zmiany temperatury może on być problemowy kiedy potrzebujemy precyzji w odmierzaniu okresów podlewania. Z drugiej strony różnice na poziomie kilku minut miesięcznie nie powinny sprawić różnicy roślinom.

Ogniwa 18650 umieściłem w koszyczku, BMS jest przyklejony na taśmę dwustronną z tyłu, a w miejscu na czwarte ogniwo zmieściłem dzielnik napięcia.
Chciałem żeby przewody do odczytu napięcia na ogniwach były możliwe małe więc wykorzystałem żyły z przewodu ETH. W tym wypadku warto zwrócić uwagę żeby były to żyły plecione, ułatwia to późniejsze manipulowanie kablami.

Wszystkie elementy przymocowałem do płyty mdf przy pomocy śrub 3,5mm.

Obudowa jest dość duża jak na takie urządzenie, ale przez gold piny i dość sztywne przewody do gold pinów potrzebne było sporo miejsca.

------------------------------------------------
### Voltage divider code / Kod do dzielnika napięcia

[Flastad.com](http://www.falstad.com/circuit/)


|Code / Kod|
|---|
|$ 1 0.000005 4.43302224444953 58 5 43  |
|r 64 96 240 96 0 2000                  |
|r 240 96 240 176 0 1000                |
|v 64 240 64 160 0 0 40 4.2 0 0 0.5     |
|v 64 160 64 96 0 0 40 4.2 0 0 0.5      |
|v 64 304 64 240 0 0 40 4.2 0 0 0.5     |
|r 64 160 192 160 0 1000                |
|r 64 240 144 240 0 1000                |
|w 64 304 144 304 0                     |
|r 192 160 192 224 0 1000               |
|w 144 304 192 304 0                    |
|w 192 304 240 304 0                    |
|w 240 96 368 96 0                      |
|w 192 160 320 160 0                    |
|w 144 240 288 240 0                    |
|w 192 224 192 304 0                    |
|w 240 176 240 304 0                    |
|w 240 304 288 304 0                    |
|p 368 96 368 160 1 0                   |
|p 320 160 320 224 1 0                  |
|p 288 240 288 304 1 0                  |
|w 288 304 320 304 0                    |
|w 320 224 320 304 0                    |
|w 320 304 368 304 0                    |
|w 368 160 368 304 0                    |
|o 0 64 0 4099 20 0.00625 0 2 0 3       |
|38 0 0 1 101 Resistance                |
