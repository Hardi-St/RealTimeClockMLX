/*
 Die RTC Uhr wird immer auf Winterzeit gestellt
 Bei der Ausgabe der Zeit wird im Sommer eine Stunde dazu gezählt

 Die Zeit wird automatisch über das Compiledate gestellt.

 Sie kann aber auch über die serielle Schnittstelle (115200 Baud) verändert werden.
 Syntax:   yy,mm,dd,hh,mm,ss
 Beispiel: 23,08,06,22,59,00

 Dazu muss in der Excel Datei
  #undef USE_RS232_OR_SPI_AS_INPUT
  #undef RECEIVE_LED_COLOR_PER_RS232
 eingetragen sein.
 Und die folgende Zeile deaktiviert werden:
   #define RTC_DEBUG


 Ausgabe der Zeit
 ----------------
 Es gibt verschieden Funktionen zur Ausgabe der Zeit:
  RTC_MINUTE
  RTC_HOUR11
  :
  RTC_HOUR_WC2
 Damit werden mehrere Variablen entsprechend der aktuellen Uhrzeit aktiviert
 Beispiel:
   EX.RT_Clock(#InCh, RTC_5MIN_OFFS , 0, MinOffs0, MinOffs4)

 Ausgabe der Temperatur
 ----------------------
 Die Bibliothek kann die interne temperatur der RTC ausgeben. EIgentlich ist diese recht genau. Dummerweise
 erwärmt sich der Uhrenchip dürch den benachbarten ESP32 im Gehäuse so sehr, dass die Temperatur zu hoch ist.
 Darum habe ich die Möglichkei eingebaut einen DALLAS DS18B20 Temperatur Sensor einzulesen.
 Wenn kein DS18B20 erkannt wird dann wird die Temperatur des RCT Chips verwendet.


 Geburtstage / Jahrestage
 ------------------------
 Mit der Funktion RTC_DAYOFYEAR können Events zu bestimmten Tagen ausgelöst werden
 Dazu können mehrere Daten angegeben werden.
 Beispiel:
   EX.RT_Clock(#InCh, RTC_DAYOFYEAR, "8.8. 9.8. 10.8.", Birthday1, Birthday3)
 Damit wird am 8.8. wird immer wieder zufällig die Variable Birthday1 gesetzt.
 Birthday2 und Birthday3 entsprechend am 9.8. und 10.8.

 Mit 0.0. deffiniert man ereignisse die jeden Tag auftreten. Das kann z.B. der Text "Moba LED LIB" sein.

 Zum Test können den Zeilen eine Taste zugewiesen werden.
 Mit jedem Tastendruck wird die nächste Variable aktiviert.


 Achtung:
 ~~~~~~~~
 Die MobaLedLib Extentions legen für jede Zeile in der Configuration eine eigene Instanz dieser Klasse an.
 => Es wird sehr viel Speicher belegt
 => Die Klassen können eigentlich nicht untereinander kommunizieren. Als Abhilfe habe ich Statische Variablen verwendet wo das nötig ist.



 Revision History:
 ~~~~~~~~~~~~~~~~~
 15.04.23:  - Started
 16.04.23:  - Setting the date/time based on the compile time
            - Automatic summer time switch
 05.08.23:  - Added DS18B20 one wire temperature sensor
 08.08.23:  - Avoid overflow with millis(): https://www.norwegiancreations.com/2018/10/arduino-tutorial-avoiding-the-overflow-issue-when-using-millis-and-micros/
            - Improved the RTC_DAYOFYEAR function
 09.08.23:  - Restarting the CPU every night at 3:00:00 to reset the millis() counter to prevent an overflow some where in the lib
 11.08.23:  - Cleared ExtActiv to prevent showing the birthdays at the wrong day if the buttons has been pressed before

 ToDo:
 ~~~~~
 - Doku
 - Die Arrays mit [MAXDATES] sollten dynamisch angelegt werden
   - Speicher sparen
   - Mehr als 10 mögliche Einträge
 - Uhrzeit über WLAN vom Internet laden
   Dazu wird eine Web Interfache zu Eingabe der SSID benötigt
   - Evtl. kann man auch die Temperatur aus dem Netz laden.
 - Stellen per Web Interface
 - Beim ersten Start nach dem complie wird die falsche Zeit mit dem Worten angezeigt.
   Die Zeit auf der seriellen Schnittstelle stimmt. Das könnte daran liegen, dass die Zeit
   nicht aus der RTC gelesen wird.
 - Stellen per Taster
   Die aktuelle Zeit & Datum sollte regelmäßig gespeichert werden sonst stimmt das Datum/Schaltjahr
   nicht mehr wenn man nur die Zeit verstellt.
   - Taster
     +/- 5 Minuten, Sekunde wird auf 0 gesetzt
     Lang = Rückwärts
 - RTC_MOBA_TIME




 RTC Connection to MobaLedLib
  | |       |  |
  | |    D12|  |----- Tast--|
  | | ESP32 |  |                   ,----------------------,
  | | exp.  |  | VCC-o-[4K7]---,   |32K *              *  |
  | | board |  |     '-[4K7]-, |   |SQW    DS3231        o|
  | |SDA D21| o|--\/---------o-|---|SCL                  o|
  | |SCL D22| o|--/\-----------o---|SDA    ZS-0042       o|
  | |       | o|-------------------|VCC                  o|
  | '-------' o|-------------------|GND *                 |
  '------------'                   '----------------------'

  D27    LED Bus
  EN-Pin 1uF to GND (Bootloader)
  D25    Optional One Wire Temp Sensor (DS18B20) (SW not implemented) Vorbereitet bei 2. Uhr
         4.7K gegen +3.3V

  Rundes DS3231 Modul:
  - Das Modul hat keine Pull Up. Diese können auf dem Modul (R1/R2) nachbestückt werden.
    Bei kurzen Leitungen geht es aber auch ohne.
  - Hier ist SCL/SDA passend zum ESP32 Adapter der MLL. Die Leitungen müssen nicht gekreutzt werden.
  - Als Batterie kann eine CR1220 eingesetzt werden wenn die beiden Kontaktfedern in der Bateriehalterung
    etwas nach unten gedrückt werden. Eine CR1225 passt vermutlich besser.
    Dummerweise ist die Batterie nicht im Lieferumfang enthalten.
    Evtl. könnte man auch einen Supercap anschließen.
    - Das große rechteckige DS3231 Modul nutzt eine CR2032
       CR1225  50mAh
       CR2032 320mAh
      Ein 0.22F Supercap geht nicht (Ist in 3 Stunden leer)


  Mainboard with Arduino Nano (Speicher reicht nicht für die ansteuerung aller LEDs!)
    MLL KEY_80
    11 = SDA
    12 = SCL
    13 = VCC
    14 = GND

*/

#ifndef __RT_CLOCK_EXTENTION__
#define __RT_CLOCK_EXTENTION__
#if !defined(AVR) && !defined(ESP32)
  #error Platform is not supported
#endif

#define RTC_MINUTE      0      // Minutes (0..59)
#define RTC_HOUR11      1      // Hours   (0..11)
#define RTC_HOUR12      2      // Hours   (1..12)
#define RTC_HOUR24      3      // Hours   (0..23)
#define RTC_WDAY        4      // Weekday (1..7)  sunday is day 1
#define RTC_5MIN        5      // Used for the "Word Clock" which uses only 5 minute steps
#define RTC_5MIN_OFFS   6      // Used for the "Word Clock" minutes offset (0..4)
#define RTC_HOUR_WC1    7      // Hour is switched @ xx:20 => use "zwanzig nach xx"
#define RTC_HOUR_WC2    8      // Hour is switched @ xx:25 => use "zehn vor halb xx"

#define RTC_DAYOFYEAR   9      // True at a special day in the year (Birthday, ...)
#define RTC_CONTR_VAR   10     // Define controller variables which disable the normal display in case of a day of year event
#define RTC_OFF         11     // Disable all outputs (0 or Max+1)
#define RTC_TEMP_WC     12     // Temperatur for the Word Clock 1:18°C .. 12:29°C
//#define RTC_MOBA_TIME 13     // Show the Moba Time
//#define RTC_INC_5MIN  14     // Increment the time by 5 minutes and set the seconds to 0
//#define RTC_DEC_5MIN  15     // Decrement  "                         "

// Flags
#define _RTC_FIRST_FLAG 16
#define RTC_SINGLE    (_RTC_FIRST_FLAG<< 0 )    // Set single output variables instead of coding the variables binary

#define _RTC_MODE_MASK  (_RTC_FIRST_FLAG-1)


// The following defines could be changed in the excel table
#ifndef RTC_FIRST_DAYOFYEAR_DISP
#define RTC_FIRST_DAYOFYEAR_DISP    30 Sec  // If the module is turned on a special day (Birthday) the text is displayed after the given time
#endif

#ifndef RTC_DAYOFYEAR_PERIOD_MIN
#define RTC_DAYOFYEAR_PERIOD_MIN     7 Min  // Minimal period to show the day of year message
#endif

#ifndef RTC_DAYOFYEAR_PERIOD_MAX
#define RTC_DAYOFYEAR_PERIOD_MAX    15 Min  // Maximal period to show the day of year message
#endif

#ifndef RTC_DAYOFYEAR_IMP_DURATION
#define RTC_DAYOFYEAR_IMP_DURATION   2 Min  // Defines how long the variable is set to 1 in case of an event
#endif

// Daily events have different parameter
#ifndef RTC_RANDDAY_PERIOD_MIN
#define RTC_RANDDAY_PERIOD_MIN      20 Min  // Minimal period to show the daily event message
#endif

#ifndef RTC_RANDDAY_PERIOD_MAX
#define RTC_RANDDAY_PERIOD_MAX     240 Min  // Maximal period to show the daily event message
#endif

#ifndef RTC_RANDDAY_INP_DURATION
#define RTC_RANDDAY_INP_DURATION     2 Min  // Define how long the variable is set to 1 in case of an daily event
#endif

#ifndef RTC_ZEIT2_DELAY
#define RTC_ZEIT2_DELAY              2 Sec  // Delay to set the controller variable 2 which is used to disable the CopyLED() functions after the time LEDs have been dimmed down
#endif

#ifndef RTC_RESTART_HOUR                    // To reset the millis() counter and other possible problems the CPU is
#define RTC_RESTART_HOUR             4      // restartet everyday at this hour
#endif                                      // Set to 99 to disable the restart

#ifndef RTC_RESTART_MINUTE
#define RTC_RESTART_MINUTE           0      // Minute for the restart
#endif

#ifndef RTC_MIN_TEMP
#define RTC_MIN_TEMP                18      // Minimal temperatur for RTC_TEMP_WC
#endif

#ifndef RTC_MAX_TEMP
#define RTC_MAX_TEMP                29      // Maximal temperatur for RTC_TEMP_WC
#endif



/*
 Im debug Mode wird InCh zum togglen eines Zählers verwendet
 Normalerweise wird der entsprechende Wert aus der RTC gelesen
*/

#ifdef RTC_DEBUG // Could be set in the Excel table with "#define RTC_DEBUG"
  static uint16_t RTC_Minutes = 0;
  static uint8_t  RTC_SimWDay = 1;
  static uint8_t  RTC_SimTemp = RTC_MIN_TEMP;
#endif

// This variable must be the same in all instances
static bool Disable_Outputs = false;

// This variable must be the same in all instances
#define NO_ACTIVE_NR   0xFFFF
static uint16_t ActiveNr = NO_ACTIVE_NR;

// This variables must be the same in all instances
#define CONTR_VAR_CNT 4                // The controller variables are defined by the function RTC_CONTR_VAR
static uint16_t Contr_Var;             // It generates three variables which are used to disable the normal
static bool     Contr_Var_Def = false; // time display when a special message (Birtday) is shown

/*
    Input Event                     ,-----------------------,
                                ----'                       '----

    Contr_Var+0 = Zeit_Aus          ,-----------------------,      Contr_Var+0 = Disable the time display imidiately
                                ----'                       '----

    Contr_Var+1 = Zeit_An       ----,                       ,----  Contr_Var+1 = Enable the time display imidiately
                                    '-----------------------'
                                     2 sec
    Contr_Var+2 = Zeit_An_Verz  ----------,                 ,----  Contr_Var+2 = Delayed enable to dim down the LEDs of the time display
                                          '-----------------'

    Contr_Var+3 = Flash                   ,-----------------,      Contr_Var+3 = Enabled if it's a birthday text which is shown on a special day
    If it's as special day      ----------'.................'----                Not enabled if it's a random message like "Stummi Forum"
    Random messages have no flash

    RTC_DAYOFYEAR variable                ,-----------------,
                                ----------'                 '----

*/

//----------------- RTC -----------------------

// Arduino DS3232RTC Library
// https://github.com/JChristensen/DS3232RTC
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Example sketch to display the date and time from a DS3231
// or DS3232 RTC every second. Display the temperature once per
// minute. (The DS3231 does a temperature conversion once every
// 64 seconds. This is also the default for the DS3232.)
//
// Set the date and time by entering the following on the Arduino
// serial monitor:
//  year,month,day,hour,minute,second,
//
// Where
//  year can be two or four digits,
//  month is 1-12,
//  day is 1-31,
//  hour is 0-23, and
//  minute and second are 0-59.
//
// Entering the final comma delimiter (after "second") will avoid a
// one-second timeout and will allow the RTC to be set more accurately.
//
// No validity checking is done, invalid values or incomplete syntax
// in the input will result in an incorrect RTC setting.
//
// Jack Christensen 08Aug2013

#undef sec // Problem with the MLL define

#include <DS3232RTC_Include.h>   // https://github.com/JChristensen/DS3232RTC
#include <Streaming.h>           // https://github.com/janelia-arduino/Streaming
#include <EEPROM.h>

DS3232RTC myRTC;
static bool Initialized = false;

// *** One wire temperature sensor DS18B20 ***

#ifndef USE_RTC_TEMP_SENS      // By default the One Wire temperatur sensor is used because the internal sensor
  #define USE_RTC_TEMP_SENS 0  // in the RTC is heated up by the ESP32
#endif

#if !USE_RTC_TEMP_SENS
  #include <OneWire.h>
  #include <DallasTemperature.h>

  #ifndef ONE_WIRE_PIN
    #define ONE_WIRE_PIN   0   // Bei der 2. Lochstreifenplatine ist der TempSensor an D25 (S3)
  #endif
  OneWire OneWireBus(ONE_WIRE_PIN);
  DallasTemperature sensors(&OneWireBus);
#endif

//---------------------------------------------------------------------------------
boolean summertime_RAMsave(int year, byte month, byte day, byte hour, byte tzHours)
//---------------------------------------------------------------------------------
// European Daylight Savings Time calculation by "jurs" for German Arduino Forum
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
// return value: returns true during Daylight Saving Time, false otherwise
// https://forum.arduino.cc/t/sommerzeitumstellung/151105/13
{
  if (month<3 || month>10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
  if (month>3 && month<10) return true;  // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
  if ((month==3  && (hour + 24 * day)>=(1 + tzHours + 24*(31 - (5 * year /4 + 4) % 7))) ||
      (month==10 && (hour + 24 * day)< (1 + tzHours + 24*(31 - (5 * year /4 + 1) % 7))))
    return true;
  else
    return false;
}

//--------------------------------
void printI00(int val, char delim)
//--------------------------------
// Print an integer in "00" format (with leading zero),
// followed by a delimiter character to Serial.
// Input value assumed to be between 0 and 99.
{
    if (val < 10) Serial << '0';
    Serial << _DEC(val);
    if (delim > 0) Serial << delim;
    return;
}

//----------------------
void printTime(time_t t)
//----------------------
// print time to Serial
{
    printI00(hour(t), ':');
    printI00(minute(t), ':');
    printI00(second(t), ' ');
}

//----------------------
void printDate(time_t t)
//----------------------
// print date to Serial
{
    printI00(day(t), 0);
    Serial << monthShortStr(month(t)) << _DEC(year(t));
}

//--------------------------
void printDateTime(time_t t)
//--------------------------
// print date and time to Serial
{
    boolean SummerTime = summertime_RAMsave(year(t), month(t), day(t), hour(t), 1);                           // 16.04.23:
    if (SummerTime) t += 3600;
    Serial.print(SummerTime?"SZ ":"WZ ");

    printDate(t);
    Serial << ' ';
    printTime(t);
}

/*
 Save of compile date/time to the Alarm time:
  Alarm1:       setAlarm(alarmType, uint8_t seconds, uint8_t minutes, uint8_t hours, day[1..31]);
  Alarm2:       setAlarm(alarmType, Month (minutes), YearHigh (hours), YearLow (daydate));

*/

//----------------------------------
int atoi_strtok_with_check(int &Err)
//----------------------------------
{
  char *p = strtok(NULL, ",");
  if (p && *p)
       return atoi(p);
  else {
       Err++;
       return 99;
       }
}


//---------------------------------------------------------
void Parse_DateTime_Str(char *Txt, int16_t SecOffset = -99)                                                   // 16.04.23:
//---------------------------------------------------------
// SecOffset is used to add the compile and upload time
// to the given string.
// To update the date/time only once from compile date/time
// SecOffset must be > -99. In this case the date/time is compared to the previosly
// stored value in the EEPROM and only used if the date/time is different.
{
  // note that the tmElements_t Year member is an offset from 1970,
  // but the RTC wants the last two digits of the calendar year.
  // use the convenience macros from the Time Library to do the conversions.
  time_t t;
  tmElements_t tm;
  int y = atoi(strtok(Txt, ","));
  if (y >= 100 && y < 1000)
       Serial << F("Error: Year must be two digits or four digits!") << endl;
  else {
       if (y >= 1000)
           tm.Year = CalendarYrToTm(y);
       else    // (y < 100)
           tm.Year = y2kYearToTm(y);

       int Err = 0;
       if (!Err) tm.Month  = atoi_strtok_with_check(Err);
       if (!Err) tm.Day    = atoi_strtok_with_check(Err);
       if (!Err) tm.Hour   = atoi_strtok_with_check(Err);
       if (!Err) tm.Minute = atoi_strtok_with_check(Err);
       if (!Err) tm.Second = atoi_strtok_with_check(Err);
       if (Err) {
                Serial << F("Wrong date/time entered '") << Txt << F("'\n") <<
                          F("Syntax:  yy,mm,dd,hh,mm,ss\n"
                            "Example: 23,08,06,22,59,00");
                return ;
                }
       t = makeTime(tm);

//       myRTC.setAlarm(DS3232RTC::ALM1_MATCH_DATE, tm.Second, tm.Minute, tm.Hour, tm.Day);
//       DS3232RTC::ALARM_TYPES_t alarmType = DS3232RTC::ALM1_MATCH_DATE;
//       uint8_t seconds, minutes, hours, day;
//       myRTC.getAlarm(alarmType, seconds, minutes, hours, day);
//       Serial << F("getAlarm:") << alarmType << ' ' << hours << ':' << minutes << ':' << seconds << " Day:" << day << endl;
/*
14:46:59.202 -> getAlarm:0 14:33:2 Day:10
14:46:59.202 -> RTC set to: SZ 10May2023 14:33:37
                getAlarm:0 14:33:2 Day:10

*/


       if (SecOffset > -99)
            {
            t += SecOffset;
            #if 1
              // Store/read the compile time to the Alarm registers in the RTC
              DS3232RTC::ALARM_TYPES_t alarmType = DS3232RTC::ALM1_MATCH_DATE;
              uint8_t Seconds, Minutes, Hours, Day, Month, YearHi, YearLo, Year100;
              myRTC.getAlarm(alarmType, Seconds, Minutes, Hours, Day);
              alarmType = DS3232RTC::ALM2_MATCH_DATE;
              myRTC.getAlarm(alarmType, Month, YearHi, YearLo);
              Year100 = YearHi * 16 + YearLo;
              Serial << F("getAlarm:") << Hours << ':' << Minutes << ':' << Seconds << F(" Day:") << Day << '.' << Month << '.' << Year100 << endl;

              if (Hours == hour(t) && Minutes == minute(t) && Seconds == second(t) &&
                  Day == day(t) && Month == month(t) &&  Year100 == year(t)%100) return ; // time and date was set before

              myRTC.setAlarm(DS3232RTC::ALM1_MATCH_DATE, second(t), minute(t), hour(t), day(t));
              Year100 = year(t)%100;
              myRTC.setAlarm(DS3232RTC::ALM2_MATCH_DATE, month(t),  Year100/16, Year100 % 16);
              Serial << F("Compile time written to the alarm registers\n");

              //22:44:14.569 -> SZ 10May2023 22:43:46

            #else
              /*
              // EEPROM (Not working at the momend with the ESP32 for some reasons, Workung with the ATMega328)
              uint16_t Addr = EEPROM.length() - sizeof(time_t);  // 4 byte
              time_t Old;
              EEPROM.get(Addr, Old);
              if (Old == t) return ;
              EEPROM.put(Addr, t);
              EEPROM.get(Addr, Old); // Wozu das?
              */
            #endif
            }
       if (summertime_RAMsave(y, tm.Month, tm.Day, tm.Hour, 1))
          t -= 3600;
       myRTC.set(t);   // use the time_t value to ensure correct weekday is set
       setTime(t);
       Serial << F("RTC set to compile time: ");
       printDateTime(t);
       Serial << endl;
       }
}

//----------------------------
void CompileTime_to_DateTime()                                                                                // 16.04.23:
//----------------------------
{
  char DateTimeStr[] =
    {
    //                              01234567890
    // Example of __DATE__ string: "Jul 27 2012"
    // Example of __TIME__ string: "21:06:19"
    //
    // Creates string:
    // "2021,07,27,21,06,19"

    // YY year
    __DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10], ',',

    // First month letter, Oct Nov Dec = '1' otherwise '0'
    (__DATE__[0] == 'O' || __DATE__[0] == 'N' || __DATE__[0] == 'D') ? '1' : '0',

    // Second month letter
    (__DATE__[0] == 'J') ? ( (__DATE__[1] == 'a') ? '1' :       // Jan, Jun or Jul
                             ((__DATE__[2] == 'n') ? '6' : '7') ) :
    (__DATE__[0] == 'F') ? '2' :                                // Feb
    (__DATE__[0] == 'M') ? (__DATE__[2] == 'r') ? '3' : '5' :   // Mar or May
    (__DATE__[0] == 'A') ? (__DATE__[1] == 'p') ? '4' : '8' :   // Apr or Aug
    (__DATE__[0] == 'S') ? '9' :                                // Sep
    (__DATE__[0] == 'O') ? '0' :                                // Oct
    (__DATE__[0] == 'N') ? '1' :                                // Nov
    (__DATE__[0] == 'D') ? '2' :                                // Dec
    0, ',',

    __DATE__[4]==' ' ? '0' : __DATE__[4],  // First day letter, replace space with digit
    __DATE__[5], ',',                      // Second day letter

    __TIME__[0], __TIME__[1], ',', __TIME__[3], __TIME__[4], ',', __TIME__[6], __TIME__[8],  ',', // Time
    '\0'
    };

  Parse_DateTime_Str(DateTimeStr, 43);  // Number = compile and upload time [s]
                                        // Unfortunately this time is not constant
                                        // => The time will not be set exactly ;-(
                                        //    But it's O.K. for the Word Clock
}


//--------------
void RTC_setup()
//--------------
// Is called only once
{
//  Serial.begin(115200);
  Serial << F("Compile date: " __DATE__ " " __TIME__ "\n" ); // Don't delete this. It's importand to check the programm version

  myRTC.begin();

  // setSyncProvider() causes the Time library to synchronize with the
  // external RTC by calling RTC.get() every five minutes by default.
  setSyncProvider(myRTC.get);
  Serial << F("RTC Sync");
  if (timeStatus() != timeSet) Serial << F(" FAIL!");
  Serial << endl;
  CompileTime_to_DateTime();                                                                                  // 16.04.23:

  #if 0 // The #defines could be changed in the excel table. Therefore they could be printed here
    Serial << F("#defines:\n");
    Serial << F("RTC_FIRST_DAYOFYEAR_DISP:  ") << RTC_FIRST_DAYOFYEAR_DISP   << endl;
    Serial << F("RTC_DAYOFYEAR_PERIOD_MIN:  ") << RTC_DAYOFYEAR_PERIOD_MIN   << endl;
    Serial << F("RTC_DAYOFYEAR_PERIOD_MAX:  ") << RTC_DAYOFYEAR_PERIOD_MAX   << endl;
    Serial << F("RTC_DAYOFYEAR_IMP_DURATION:") << RTC_DAYOFYEAR_IMP_DURATION << endl;
    Serial << F("RTC_RANDDAY_PERIOD_MIN:    ") << RTC_RANDDAY_PERIOD_MIN     << endl;
    Serial << F("RTC_RANDDAY_PERIOD_MAX:    ") << RTC_RANDDAY_PERIOD_MAX     << endl;
    Serial << F("RTC_RANDDAY_INP_DURATION:  ") << RTC_RANDDAY_INP_DURATION   << endl;
  #endif

  #if !USE_RTC_TEMP_SENS
    //sensors.begin();                   // Crash if no sensor connected (IntegerDivideByZero)
    sensors.setWaitForConversion(false);  // makes it async otherwise the reading of the temperatur takes 500ms
    // DeviceAddress deviceAddress;
    // if (sensors.getAddress(deviceAddress, 0)) sensors.setResolution(deviceAddress, 12);
    // sensors.setResolution(12);         // By default te resolution is 9bit = 0.5 deg C. But the resolution is still 0.5 deg C if it's set to 12 bit. Mem Usage 510 byte ;-(
    //                                    // For some reasons the temperatur is shown with 0.1 deg C for some sensors, but not for all
    //                                    // even if the type is exact the same DALLAS 18B20 2034C4 +817AB
    //                                    // According to the data sheet the resolution could be convigured. But how?
  #endif
}


//--------------
void RTC_loop()
//--------------
{
  static time_t tLast;
  #if !defined USE_RS232_OR_SPI_AS_INPUT &&  !defined RECEIVE_LED_COLOR_PER_RS232 // Problem with DCC
    // check for input to set the RTC, minimum length is 12, i.e. yy,m,d,h,m,s
    if (Serial.available() >= 12 || (Serial.available() > 1 && Serial.peek() == '?'))
        {
        char Buf[31], *p, *e, c;
        p = Buf;
        e = p + sizeof(Buf) - 1;
        while (Serial.available() > 0)
            {
            c = Serial.read();
            if (p < e)  *(p++) = c;
            delay(1); // To be able to receive futher characters
            }
        *p = '\0';
        Serial << "Buf:'" << Buf << "'\n";
        Parse_DateTime_Str(Buf);                                                                                // 16.04.23:
        }
  #endif

  #ifdef DEBUG_REALTIMECLOCK_TIME // Debug: Print the time to the serial output
    if (Disable_Outputs == false)
       {
       time_t t;
       t = now();
       if (t != tLast)
          {
          tLast = t;
          printDateTime(t);
          if (second(t) == 0) // Internal temperatur (Not accurate because of ESP32 heat up)
              {
              float c = myRTC.temperature() / 4.;
              Serial << F("  ") << c << F(" C  ")
              //Serial << c * 9. / 5. + 32. << F(" F");
              }
          Serial << endl;
          }
       }
  #endif
}


//----------------- End RTC -----------------------






#include <MLLExtension.h>
//#include <string>
//using namespace std;

//----------------------------------
int ReadNr(const char* &p, char Del)
//----------------------------------
// Read a number from p until the delimmiter "Del" is found.
// p is moved to the position after the delimmiter
{
  char Buff[20];
  char *Pos = Buff;
  char *End = Pos + sizeof(Buff) -1;
  while (*p && *p != Del && Pos < End)
    {
    *Pos = *p;
    Pos++;
    p++;
    }
  *Pos = '\0';
  if (*p == Del) p++;
  //Serial << "Buff:'" << Buff << "' \n";
  return atoi(Buff);
}

#define MAXDATES 10

//***********************************
class RT_Clock : public MLLExtension
//***********************************
{
  private:
    uint8_t     InCh;
    uint8_t     Clock_Type;
    uint8_t     DstVar1;      // ToDo: Check if more than 256 variables are possible at the ESP32 => Nein, zur Zeit nicht ;-(
    uint8_t     DstVarN;
    const char *ParTxt;
    uint32_t    DayOfYearDisplayTime[MAXDATES];
    uint8_t     Next_ExtStartNr;
    bool        Par_Read;
    uint8_t     DatesCnt;
    uint8_t     Day[MAXDATES];
    uint8_t     Month[MAXDATES];
    bool        OldActiv[MAXDATES];
    bool        Old_ExtInp;
    bool        ExtActiv;
    uint32_t    Set_Zeit2_Time;
    bool        FirstInstanze;

  #ifdef RTC_DEBUG
    uint16_t Ctr;
    uint8_t  Old_Inp;
  #endif


    //-----------------------------------------------------------------------------------------------------
    public:RT_Clock(uint8_t InCh, uint8_t Clock_Type, const char *ParTxt, uint8_t DstVar1, uint8_t DstVarN) // Constructor
    //-----------------------------------------------------------------------------------------------------
    // Attention: No serial output possible in the Constructor !!
    {
      this->InCh           = InCh;
      this->Clock_Type     = Clock_Type;
      this->ParTxt         = ParTxt;
      this->DstVar1        = DstVar1;
      this->DstVarN        = DstVarN;
      for (uint8_t i = 0; i < MAXDATES; i++)
          DayOfYearDisplayTime[i] = RTC_FIRST_DAYOFYEAR_DISP + random(0,50);
      //ExtInpStartTime      = 0;
      Next_ExtStartNr      = 0;
      DatesCnt             = 0;
      Old_ExtInp           = false;
      ExtActiv             = false;
      Set_Zeit2_Time       = 0;
      FirstInstanze        = false;
      switch (Clock_Type & _RTC_MODE_MASK)
        {
        case RTC_CONTR_VAR:     // Controller variables
                                if (DstVarN - DstVar1 + 1 == CONTR_VAR_CNT)
                                   {
                                   Contr_Var_Def = true;
                                   Contr_Var = DstVar1;
                                   }
                                break;
        case RTC_DAYOFYEAR:     Read_Par();
                                for (uint8_t i = 0; i < DatesCnt; i++)
                                    {
                                    // Don't show the standard events (like "Stummi Forum") at the start of the program
                                    if (Day[i] == 0) DayOfYearDisplayTime[i] = random(RTC_RANDDAY_PERIOD_MIN, RTC_RANDDAY_PERIOD_MAX);
                                    }
                                break;
        }
	}

    //-----------------------------------------
    public:void setup(MobaLedLib_C& mobaLedLib)
    //-----------------------------------------
    {
    #ifdef RTC_DEBUG
      Ctr  = 0;
      Old_Inp = 0;
    #endif
    Par_Read = 0;
    if (!Initialized) // Initialize the RTC only once
       {
       Initialized = true;
       FirstInstanze = true;
       RTC_setup();
       }
     if (Contr_Var_Def)
        {
        mobaLedLib.Set_Input(Contr_Var+1, 1); // Zeit1 = Zeit_An
        mobaLedLib.Set_Input(Contr_Var+2, 1); // Zeit2 = Zeit_Verz
        }
	}

    //---------------------
    private:void Read_Par()
    //---------------------
    {
      Par_Read = true;
      //Serial << F("ParTxt:") << ParTxt << endl;
      const char *p = ParTxt;
      while (DatesCnt < MAXDATES)
        {
        Day[DatesCnt] = ReadNr(p, '.');
        if (p && *p) Month[DatesCnt] = ReadNr(p, '.');
        else         return;
        DatesCnt++;
        }
    }

    //--------------------------------------------------------------------------------
    private:void Set_Variables(MobaLedLib_C& mobaLedLib, uint8_t Val, uint8_t BarMode)
    //--------------------------------------------------------------------------------
    {
      if (Clock_Type & RTC_SINGLE || BarMode)
           {
           uint8_t  Cnt = 0;
           for (uint8_t VarNr = DstVar1; VarNr <= DstVarN; VarNr++, Cnt++)
               if (BarMode)
                    mobaLedLib.Set_Input(VarNr,Val >= Cnt );
               else mobaLedLib.Set_Input(VarNr,Val == Cnt );
           }
      else {
           uint16_t Mask = 1;
           for (uint8_t VarNr = DstVar1; VarNr <= DstVarN; VarNr++, Mask <<= 1)
               mobaLedLib.Set_Input(VarNr, Val & Mask);
           }
    }

    #ifdef RTC_DEBUG
      //----------------------------------------------------------------------------
      private:uint8_t Debug_Set_RTC_Minutes(MobaLedLib_C& mobaLedLib, uint8_t CType)
      //----------------------------------------------------------------------------
      {
         uint8_t Act_Inp = mobaLedLib.Get_Input(InCh);
         uint8_t Inc, DebugPrint = 0;
         if (Act_Inp == INP_TURNED_ON && Old_Inp == INP_OFF) // INP_TURNED_ON kommt aus irgend einem Grund mehrfach ?!?
            {
            if (CType == RTC_WDAY)
                 {
                 RTC_SimWDay++;
                 if (RTC_SimWDay > 7) RTC_SimWDay = 1;
                 }
            else if (CType == RTC_TEMP_WC)
                 {
                 RTC_SimTemp++;
                 if (RTC_SimTemp > RTC_MAX_TEMP + 1) RTC_SimTemp = RTC_MIN_TEMP-1;
                 }
            else {
                 switch (CType)
                   {
                   case RTC_MINUTE:    Inc = 1;  break;
                   case RTC_HOUR11:    Inc = 60; break;
                   case RTC_HOUR12:    Inc = 60; break;
                   case RTC_HOUR24:    Inc = 60; break;
                   case RTC_5MIN:      Inc = 5;  break;
                   case RTC_5MIN_OFFS: Inc = 1;  break;
                   case RTC_HOUR_WC1:  Inc = 60; break;
                   case RTC_HOUR_WC2:  Inc = 60; break;
                   }
                 RTC_Minutes += Inc;
                 }
            DebugPrint = 1;
            }
         Old_Inp = Act_Inp;
         return DebugPrint;
      }

      //--------------------------------------------------------------------------------------
      private:void Debug_Print(uint16_t RTC_Minutes, uint8_t CType, uint8_t Val, int8_t TempC)
      //--------------------------------------------------------------------------------------
      {
        if (RTC_Minutes > 24 * 60) RTC_Minutes = 0;
        uint8_t Min40 = ((RTC_Minutes+40)/60)%12; if(Min40==0) Min40=12;
        uint8_t Min35 = ((RTC_Minutes+35)/60)%12; if(Min35==0) Min35=12;
        Serial << F("CType:")    << CType;
        Serial << F(" Time:")    << RTC_Minutes/60 << ':';  if ((RTC_Minutes%60)<10) Serial << '0'; Serial << RTC_Minutes%60;
        Serial << F(" Minutes:") << RTC_Minutes;
        Serial << F(" Val:")     << Val;
        Serial << F(" WC1:")     << Min40;
        Serial << F(" WC2:")     << Min35;
        Serial << F(" WDay:")    << (RTC_Minutes%7)+1;
        Serial << F(" TempC:")   << TempC;
        Serial << F(" SimTemp:") << RTC_SimTemp;
        Serial << endl;
      }
    #endif

    //----------------------------------------
    public:void loop2(MobaLedLib_C& mobaLedLib) // loop2 runs at the main core
    //----------------------------------------
    {
      uint8_t CType = Clock_Type & _RTC_MODE_MASK;

      #ifdef RTC_DEBUG
         uint8_t DebugPrint = Debug_Set_RTC_Minutes(mobaLedLib, CType);  // Debug
      #else
         RTC_loop();
         time_t  t = now();
         boolean SummerTime = summertime_RAMsave(year(t), month(t), day(t), hour(t), 1);
         if (SummerTime) t += 3600;
         uint16_t RTC_Minutes = hour(t) * 60 + minute(t);

         if (FirstInstanze && hour(t) == RTC_RESTART_HOUR && minute(t) == RTC_RESTART_MINUTE && second(t) == 0)
            {
            Serial << F("Restarting...\n");
            while (1)
                {
                // Wait for watchdog restart
                }
            }
      #endif // RTC_DEBUG

      #if !USE_RTC_TEMP_SENS
        {
        static uint32_t Last_TempRead = 0;
        uint32_t t = millis();
        if (t - Last_TempRead > 1000) // Overflow save calculation
           {
           Last_TempRead = t;
           sensors.requestTemperatures(); // The temperatur is read in async mode (setWaitForConversion(false)) because it takes about 500ms to read the temperatur
           #ifdef DEBUG_TEMP_SENSOR
             Serial << F("Temperatur: ") << sensors.getTempCByIndex(0) << endl; // Debug
           #endif
           }
        }
      #endif

      uint8_t Val = 0, DisabVal = 0;
      int8_t  TempC;
      switch (CType)
        {
        case RTC_MINUTE:    Val =   RTC_Minutes % 60;                          DisabVal = 60; break;  // 0..59
        case RTC_HOUR11:    Val =  (RTC_Minutes / 60) % 12;                    DisabVal = 12; break;  // 0..11
        case RTC_HOUR12:    Val = ((RTC_Minutes / 60) % 12) + 1;                              break;  // 1..12
        case RTC_HOUR24:    Val =   RTC_Minutes / 60;                          DisabVal = 24; break;  // 0..23
      #ifdef RTC_DEBUG
        case RTC_WDAY:      Val =   RTC_SimWDay;                                              break;  // 1..7, 1 = sunday, 2=Mo, 3=Di, 4=Mi, 5=Do, 6=Fr, 7=Sa
        case RTC_TEMP_WC:   TempC = RTC_SimTemp;                                              break;  // 1..12 (18°C - 29°C) 0 = Aus
      #else
        case RTC_WDAY:      Val =   weekday(t);                                               break;  // 1..7, 1 = sunday
        case RTC_TEMP_WC:
             #if USE_RTC_TEMP_SENS
                            TempC = (myRTC.temperature()+2) / 4.0;                            break;  // +2 for correct rounding
             #else
                            TempC = round(sensors.getTempCByIndex(0));
                            if (TempC == -127) // In case no DS18B20 is connected we use the internal sensor in the RTC
                                TempC = (myRTC.temperature()+2) / 4.0;
                            break;
             #endif
      #endif
        case RTC_5MIN:      Val =  (RTC_Minutes % 60) / 5;                                    break;  // 0..11
        case RTC_5MIN_OFFS: Val =   RTC_Minutes % 5;                                          break;  // 0..4
        case RTC_HOUR_WC1:  Val = ((RTC_Minutes+40)/60)%12; if(Val==0) Val=12;                break;  // 1..12 Hour is switched @ xx:20 => use "zwanzig nach xx"
        case RTC_HOUR_WC2:  Val = ((RTC_Minutes+35)/60)%12; if(Val==0) Val=12;                break;  // 1..12 Hour is switched @ xx:25 => use "zehn vor halb xx"
        case RTC_OFF:       {
                            uint8_t Inp = mobaLedLib.Get_Input(InCh);
                            Disable_Outputs = Inp_Is_On(Inp);                  // Disable all
                            }
                            return ;
        case RTC_DAYOFYEAR: {
                            #ifdef RTC_DEBUG
                              if (InCh != SI_1) Val = Inp_Is_On(mobaLedLib.Get_Input(InCh));
                              mobaLedLib.Set_Input(DstVar1, Val);
                            #else
                              bool Inp = (InCh != SI_1 && Inp_Is_On(mobaLedLib.Get_Input(InCh)));
                              bool ExtInp = (Inp && Old_ExtInp == false);
                              //if (ExtInp) Serial << "ExtInp!!\n";
                              Old_ExtInp = Inp;

                              uint32_t mil = millis();
                              uint8_t Nr = 0;
                              for (Nr = 0; Nr < DatesCnt; Nr++)
                                 {
                                 Val = 0;
                                 if (Day[Nr] == 0 || (Day[Nr] == day(t) && Month[Nr] == month(t)) || (ExtInp && Nr == Next_ExtStartNr) || ExtActiv)
                                    {
                                    if (ExtInp && Nr == Next_ExtStartNr) // The Output could be enabled/disabled with an external trigger (Button)
                                       {
                                       if (ActiveNr == NO_ACTIVE_NR)
                                            { ExtActiv = true;  DayOfYearDisplayTime[Nr] = mil; }// Enable
                                       else { ExtActiv = false;                                  // Disable and set the next start time
                                              // Wird aufgerufen wenn bereits ein anderer Event läuft
                                              // Damit wird dafür gesorgt, dass der Event später nochmal kommt
                                              if (Day[Nr] > 0)
                                                   DayOfYearDisplayTime[Nr] = mil + random(RTC_DAYOFYEAR_PERIOD_MIN, RTC_DAYOFYEAR_PERIOD_MAX);
                                              else DayOfYearDisplayTime[Nr] = mil + random(RTC_RANDDAY_PERIOD_MIN,   RTC_RANDDAY_PERIOD_MAX);
                                              //Serial << "Day:" << Day[Nr] << " Nr:" << Nr << " DayOfYearDisplayTime=" << DayOfYearDisplayTime[Nr] - mil << endl;
                                            }
                                       }

                                    if (mil >= DayOfYearDisplayTime[Nr]) // ToDo: Avoid overflow
                                       {
                                       if (mil >= DayOfYearDisplayTime[Nr] + (Day[Nr]>0?RTC_DAYOFYEAR_IMP_DURATION:RTC_RANDDAY_INP_DURATION) || (ActiveNr != NO_ACTIVE_NR && ActiveNr != DstVar1+Nr))
                                            { // Set next time to display the text
                                            if (Day[Nr] > 0)
                                                 DayOfYearDisplayTime[Nr] = mil + random(RTC_DAYOFYEAR_PERIOD_MIN, RTC_DAYOFYEAR_PERIOD_MAX);
                                            else DayOfYearDisplayTime[Nr] = mil + random(RTC_RANDDAY_PERIOD_MIN,   RTC_RANDDAY_PERIOD_MAX);
                                            //Serial << "Set next time for Nr:" << Nr << endl;
                                            if (ActiveNr == DstVar1+Nr) ExtActiv = false;                     // 11.08.23:
                                            }
                                       else {
                                            Val = 1;
                                            //Serial << "SET Val = 1 for Nr:" << Nr << endl;
                                            }
                                       }
                                    }
                                 if (OldActiv[Nr] != Val)
                                    {
                                    //Serial << "OldActiv[" << Nr << "] dif " <<  OldActiv[Nr] << " Val:"  << Val << endl;
                                    OldActiv[Nr] = Val;
                                    if (Contr_Var_Def) // Enabled if function RTC_CONTR_VAR is used
                                         {
                                         mobaLedLib.Set_Input(Contr_Var+0,  Val);     // Zeit0 = Zeit Anzeige Aus
                                         mobaLedLib.Set_Input(Contr_Var+1, !Val);     // Zeit1 = Zeit Anzeige An
                                         if (Val == 1)
                                              {
                                              Set_Zeit2_Time = mil + RTC_ZEIT2_DELAY;
                                              //Serial << "Start Next_ExtStartNr:" << Next_ExtStartNr << " Nr:" << Nr << endl; // Debug
                                              }
                                         else {
                                              mobaLedLib.Set_Input(Contr_Var+2, 1);   // Zeit2 = Zeit_An_Verz  Sofort anschalten damit die Zeit LEDs aufgeblendet werden
                                              mobaLedLib.Set_Input(DstVar1+Nr,  0);   // Steuervariable für Day of Year Anzeige ausschalten
                                              mobaLedLib.Set_Input(Contr_Var+3, 0);   // Flashing blue lights
                                              if (ActiveNr != NO_ACTIVE_NR)
                                                 {
                                                 Next_ExtStartNr++;
                                                 if (Next_ExtStartNr >= DatesCnt) Next_ExtStartNr = 0;
                                                 //Serial << "End Next_ExtStartNr:" << Next_ExtStartNr << endl; // Debug
                                                 }
                                              }
                                         }
                                    else mobaLedLib.Set_Input(DstVar1+Nr, Val);
                                    if (Val) ActiveNr = DstVar1+Nr;
                                    else     ActiveNr = NO_ACTIVE_NR;
                                    //Serial << "ActiveNr:" << ActiveNr << endl; // Debug
                                    }

                                 if (Set_Zeit2_Time != 0 && mil >= Set_Zeit2_Time && ActiveNr != NO_ACTIVE_NR) // The special display (Birthday) is enabled delayed to be able to dim down the normal time display
                                    {
                                    Set_Zeit2_Time = 0;
                                    if (Contr_Var_Def)
                                       {
                                       mobaLedLib.Set_Input(Contr_Var+2,  0); // Zeit2 = Zeit_An_Verz  Verzögert abschalten damit die LEDs abgeblendet werden können
                                       mobaLedLib.Set_Input(ActiveNr,   1);   // Steuervariable für Day of Year Anzeige anschalten
                                       //Serial << "Day[" << ActiveNr-DstVar1 << "]=" << Day[ActiveNr-DstVar1] << "  ActiveNr:" << ActiveNr << "  DstVar1:" << DstVar1 << endl;
                                       if (Day[ActiveNr-DstVar1] != 0) // Enable only if Birthday
                                          mobaLedLib.Set_Input(Contr_Var+3,  1); // Flashing blue lights
                                       }
                                    }
                                 }
                            #endif // RTC_DEBUG
                            }
                            return;
        case RTC_CONTR_VAR: return;
        default: Serial << F("Unhandeled CType in switch:") << CType << endl;
                 return;
        }

      if (CType == RTC_TEMP_WC) // Attention: The temperatur is only read once per minute => Don't expect fast changes
         {
         // +/-1 used for flashing if out of range
         if (TempC < RTC_MIN_TEMP-1) TempC = RTC_MIN_TEMP-1;
         if (TempC > RTC_MAX_TEMP+1) TempC = RTC_MAX_TEMP+1;
         //Serial << F("TempC:") << TempC << endl;
         if (TempC < RTC_MIN_TEMP && (millis() % 2000) > 1000)  TempC = RTC_MIN_TEMP;
         if (TempC > RTC_MAX_TEMP)
            {
            if ((millis() % 2000) > 1000)  TempC = RTC_MAX_TEMP;
            else                           TempC = RTC_MIN_TEMP-1; // => Val = 0 => Disable the LED for 500 ms
            }
         Val = (TempC - RTC_MIN_TEMP+1);
         }

/*
08:33:45.472 -> TempC:21
08:33:45.472 -> CType: 11 Time: 0:00 RTC_Minutes: 0 Val: 247 WC1: 12 WC2: 12 WDay: 1
08:33:45.472 -> TempC:21


 Probleme:
 - Warum flackert die erste LED?
   - das macht sie nur wenn noch andere LEDs an sind
 - Der Text "Uhr" geht beim widereinschalten kurz an.
   Es sieht so aus als würden die LEDs schlagartig an gehen und dann ausgeblendet werden
    Logik: Uhr_Txt = Zeit_An AND VolleStd
 - Das Disable_Outputs geht nur wenn in der Funktion ein delay(1)
   oder eine serielle Ausgabe eingabaut ist.
   Dabei spielt die Position keine Rolle ?!?
   => Gelöst: Jetzt wird loop2 verwendet. Damit funktioniert es

*/
      #ifdef RTC_DEBUG
         if (DebugPrint) Debug_Print(RTC_Minutes, CType, Val, TempC);
      #endif

      //pinMode(14, OUTPUT); digitalWrite(14,Disable_Outputs); // Debug
      if (Disable_Outputs) Val = DisabVal;
      Set_Variables(mobaLedLib, Val, CType == RTC_5MIN_OFFS);
    }
};


#endif // __RT_CLOCK_EXTENTION__
