/*
 Die RTC Uhr wird immer auf UTC gestellt
 Bei der Ausgabe der Zeit wird sie entsprechend den tzstr konvertiert.
 Dabei wird Zeitzohne und Sommerzeit berücksichtigt

 Die Zeit wird automatisch über das Compiledate gestellt.

 Sie kann aber auch über die serielle Schnittstelle (115200 Baud) verändert werden.
 Mit der Eingabe von ? wird eine Hilfe ausgegeben.

 Dazu muss in der Excel Datei
  #undef USE_RS232_OR_SPI_AS_INPUT
  #undef RECEIVE_LED_COLOR_PER_RS232
 eingetragen sein.
 Und die folgende Zeile deaktiviert werden:
   #define RTC_DEBUG

 Seit der Version 0.9.0 wird Zeit per WLAN gelesen. Dazu drückt man einfach den WPS Taster
 am Heimischen WLAN Router. Falls der Router keine WPS Taste hat kann die SSID und das Pwd
 auch über die serielle Schnittstelle (115200 Baud) eingegeben werden. Es werden bis
 zu 5 Netzwerke gespeichrt.
 Achtung: Dazu muss ein ESP Board Package >= 2.0.0 verwendet werden.

 Seit Version 1.0.0 gibt es ein Web Interface über das die Zeitzohne verstellt werden kann.
 Das wird evtl. nötig wenn die Sommerzeit irgend wann angeschaltet wird.

 Achtung:
 DS3232RTC_Include.h includiert die TimeLib. Diese unterstützt NICHT die Zeitzohne und die Sommerzeit!
 => Die Funktionen hour(), weekday(), ... beziehen sich auf UTC Zeit !
 Sie sollten nicht für die Ausgabe der Zeit bennutzt werden!
 Im Programm benötigt man die nur an ein paar stellen woe UTC verwendet wird.

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
 Die Temperatur wird normalerweise über eine DS18B20 eingelesen. Dieser ist am Display verbaut.
 Er sollte aber nicht im Gehäuse stecken sonst erwärmt er sich durch die LEDs.
 Alternativ kann die Bibliothek die interne temperatur der RTC ausgeben. DAzu muss USE_RTC_TEMP_SENS = 1
 definiert sein. Eigentlich ist diese Temperatur des Uhrenchips recht genau. Dummerweise
 erwärmt sich der Uhrenchip durch den benachbarten ESP32 im Gehäuse so sehr, dass die Temperatur zu hoch ist.


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
 => Die Klassen können eigentlich nicht untereinander kommunizieren. Als Abhilfe habe ich statische Variablen
    verwendet wo das nötig ist.


 Board Package >= 2.0.0 required
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Die Extention benötigt ein ESP32 Board Package ab der Version 2.0.0 weil
 - die Callback Funktion welche aufgerufen wird wenn Zeit Daten vorhanden sind nicht mit
   "sntp_set_time_sync_notification_cb" definiert werden kann.
   Ich habe zwar versucht ein Workaround zu basteln, aber das geht nicht. Zeit / Datum werden auf 1970 gestellt ;-(
 - Im WPS Modul fehlen:
   CONFIG_IDF_TARGET, ARDUINO_EVENT_WIFI_STA_GOT_IP, ARDUINO_EVENT_WPS_ER_SUCCESS
 - die perferences Funktion ".isKey" erst hier definiert ist. Es funktioniert auch ohne, aber denn
   werden unschöne Fehlermeldungen auf der Console ausgegeben wenn ein Eintrag nicht in den Preferences
   existiert. Im code wird mit "#ifdef ESP_ARDUINO_VERSION_MAJOR" geprüft ob die Version passt und der Code weggelassen
 => Am besten man compeliert das ganze mit PlatformIO

 Alternativ hann man in der Libraries Seite des Prog_Generators die Version "Requires Version" 1.0.4 in der
 ESP Zeile löschen die neueste Bibliothek installieren.


 Tests:            20.12.25:
 ~~~~~~
 - Ohne WLAN
   Zeit wird aus RTC gelesen
 - Ohne RTC Batterie
   Zeit stimmt, wenn die Uhrzeit per WLAN empfangen wird
 - Ohne RTC Modul
   Geht nicht. Zeigt immer compile Zeit. Das ist nicht schön, aber egal.


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
 18.12.25:  - Started the WPS functionality with the Linear Uhr
 20.12.25:  - Ver. 0.9.0
            - Incremented MAXDATES to 50 (Old 10)
            - Read time via WLAN
            - Connect automatical to WLAN if WPS button on the router is pressed
            - Two leds at the top flash for 5 seconds if WLAN if connected.  Also if WPS is connected
              If RTC_STATUS_LED0 is defined the LED0 is used as status instead
            - Serial command implemented. Perss ? to get a list
            - Wifi could be entered manually
            - New DallasTemperature library > 3.9.0 is supported
 05.01.26:  - Ver. 1.0.0
            - Corrected the Weekday. In the prior version the weekday was based on the UTC time
              => it was set at 1:00 in the winter/2:00 in the summer instead of 0:00 ;-(
            - Web Interface to change the time zone which is called with
                http://word_clock or
                http://word_clock_<Name> where <Name> could defined in the excel sheet with
                Example:
                  #define WPS_CLIENT_NAME "Wohnzimmer"
 06.01.26:  - Ver. 1.0.1
            - Clean up
 07.01.26:  - Added WPS Debug info


 ToDo:
 ~~~~~
 - Compile ohne WLAN damit die alte Funktionalität genutzt werden kann
 - WLAN abschalten zum Strom sparen? (100mA)
 - SSID und PWS per Web Interface eingeben. Das benötigt man nur falls man eine Uhr für jemanden
   vorbereiten will der keine WPS Taste am Heimischen Router hat.
 - Stellen per Web Interface => Braucht man nicht. Wenn er im WEB ist holt er sich die Zeit von dort
 - Helligkeit der StatusLED verändern? (in meinem Gehäuse mit Bewegungsmelder stöhrt sie etwas)
 - Die Erkennung einer leeren Batterie funktioniert noch nicht so richtig (BatEmpt)
 - Warum blinkt die StatusLED erst nach einer gewissen Zeit?
 - Ohne PIO testen => Geht nicht weil die MLL noch die Version 1.0.4 des ESP Board Packages verwendet
 - Doku
 - RTC_MOBA_TIME mit dem die MobaZeit angezeigt werden kann?



                  Temp. Sens
                  ,-------,
                  |DS18B20|
                  '-------'
      ESP32         | | |
   ,----------, GND-' | '-VCC                      ,----------------,
   |        D0| ------o-[4K7]-GND ,-----------,    |                |
   |       D27|-------------------| Heartbead |----|     WS2812     |
   |          |   ,-[33K]--VCC    '-----------'    | 256 LED Matrix |
   |       D35|---o-LDR----GND                     |                |
   |       D24|-----Tast3--GND                     '----------------'
   |       D25|-----Tast2--GND
   |       D12|-----Tast1--GND
   |          |                   ,----------------------,
   |          | VCC-o-[4K7]---,   |32K *              *  |
   |          |     '-[4K7]-, |   |SQW    DS3231        o|
   |   SDA D22|-------------o-|---|SCL                  o|
   |   SCL D21|---------------o---|SDA    ZS-0042       o|
   |       VCC|-------------------|VCC                  o|
   |       GND|-------------------|GND *                 |
   |        EN|---||--GND         '----------------------'
   '----------'   1uF


  EN-Pin 1uF to GND (Bootloader)
  Bei der ersten Lochstreifen Platine (Karl) ist der Temp. Sensor an D25

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

 Verschiedene Zeitfunktionen:
 gmtime() / localtime()
   gmtime()gibt die Zeit als koordinierte Weltzeit (UTC) zurück (basierend auf dem Nullmeridian),
   während localtime()` die Zeit in der lokalen Zeitzone des Systems zurückgibt, einschließlich
   Berücksichtigung von Sommerzeit, falls aktiviert. Beide Funktionen konvertieren eine absolute
   Zeit (Sekunden seit der Unix-Epoche) in ein aufgeschlüsseltes Zeitformat (Jahr, Monat, Tag,
   Stunde usw.), aber mit unterschiedlichem Zeitzonenbezug.

 setenv("TZ", ...) / configTzTime(..)
   Mit der Function setenv("TZ", ...) oder mit configTzTime() wird die Zeitzohne und die Sommerzeit
   definiert welche über localtime() ausgelesen wird.


*/

#ifndef __RT_CLOCK_EXTENTION__
#define __RT_CLOCK_EXTENTION__

#if !defined(AVR) && !defined(ESP32)
  #error Platform is not supported
#endif

#define VER_STR  "Ver. 1.0.1"

// DHCP Name used for the web interface
#ifdef WPS_CLIENT_NAME // Add to the excel sheet:  #define WPS_CLIENT_NAME "Test"
  #define HOSTNAME  "word_clock_" WPS_CLIENT_NAME   // http://word_clock_Test
#else
  #define HOSTNAME  "word_clock"                    // http://word_clock
#endif


// Default values used if nothing is entered
#define DEF_NTP         "de.pool.ntp.org"
#define DEF_TZSTR       "CET-1CEST,M3.5.0/02:00:00,M10.5.0/03:00:00"


#ifndef USE_WPS                // USE_WPS could also be defined in Excel
  #define USE_WPS         1    // Use #define USE_WPS 0 to disable it
#endif

#define WORDCLOCK              // Used in Serial_Commands.h and WEB_Server.h

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

#if USE_WPS
  #define PREFERENCESNAME     "WordClock"   // Used also in Serial_Commands.h

  // Global Variables used also in Serial_Commands.h
  static bool     Time_Set_by_Wifi        = false;
  static uint32_t Start_Time_Set_by_Wifi  = 0;
  enum WLan_State_e
    {
    Connecting_Network, // Connecting Network           Gelb
    Press_WPS,          // WPS Taste an Router drücken  Blau
    Network_Connected,  // Network connected            Grün
    };
  static WLan_State_e WLan_State = Connecting_Network;
#endif

static bool NoPrint = false;

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
static bool Disable_Outputs    = false;
static bool Bat_Empty_Detected = false;

// 06.01.26:  Braucht man nicht mehr
#define USE_RESTART_TIME 0
#if USE_RESTART_TIME
  static uint32_t Restart_Time   = 0;
#endif

// This variable must be the same in all instances
#define NO_ACTIVE_NR   0xFFFF
static uint16_t ActiveNr = NO_ACTIVE_NR;

// This variables must be the same in all instances
#define CONTR_VAR_CNT 4                // The controller variables are defined by the function RTC_CONTR_VAR in the constructor
static uint16_t Contr_Var;             // It generates four variables which are used to disable the normal
static bool     Contr_Var_Def = false; // time display when a special message (Birtday) is shown


static uint32_t DebugPrintTime = 7000; // Time to print the next time to the serial port

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

#if USE_WPS
  // Forward definitions
  void SetDateTime(const char *Arg);
  void Set_TimeZone_and_NTP_Server();
#endif

DS3232RTC myRTC;
static bool Initialized = false;

// *** One wire temperature sensor DS18B20 ***
#ifndef USE_RTC_TEMP_SENS      // By default the One Wire temperatur sensor is used because the internal sensor
  #define USE_RTC_TEMP_SENS 0  // in the RTC is heated up by the ESP32
#endif

// Global variables read in WEB_Server.h
float  g_TempC      = -99;
bool   g_TimeLocked = false;
String g_TimeString = "Nicht definiert";

#if !USE_RTC_TEMP_SENS
  #include <OneWire.h>
  #include <DallasTemperature.h>

  #ifndef ONE_WIRE_PIN
    #define ONE_WIRE_PIN   0   // Bei der 1. Lochstreifenplatine ist der TempSensor an D25 angeschlossen
  #endif
  OneWire OneWireBus(ONE_WIRE_PIN);
  DallasTemperature sensors(&OneWireBus);
#endif

#if USE_WPS                                                                                                   // 20.12.25:
   #define WPS_MANUFACTURER "MobaLedLib"
   #define WPS_MODEL_NAME   "Word Clock"

   #include "WPS.h"

   #include "esp_sntp.h"
   #include "Serial_Commands.h"

   #ifndef USE_WEB_SERVER
     #define USE_WEB_SERVER 1
   #endif
   #if USE_WEB_SERVER
     #include "WEB_Server.h"
   #endif
#else // USE_WPS
  #warning "WLAN / WPS funktions are not available because USE_WPS is set to 0."
  #ifndef ESP_ARDUINO_VERSION_MAJOR // dieses define gibt es erst ab ESP Board package 2.0.0
    #warning "WPS may be disabled because you are using an old ESP32 board package < 2.0.0"
  #endif
#endif // USE_WPS



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

//--------------------------
void printDateTime(time_t t)
//--------------------------
// print date and time to Serial
{
  tm tm;
  localtime_r(&t, &tm);
  printf("%s %2i.%02i.%i% 2i:%02i:%02i \n", tm.tm_isdst?"SZ":"WZ", tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

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
  time_t new_local_t;
  char OrgTxt[30];
  strncpy(OrgTxt, Txt, sizeof(OrgTxt)-1);
  int y = atoi(strtok(Txt, ","));
  if (y >= 100 && y < 1000)
       Serial << F("Error: Year must be two digits or four digits!") << endl;
  else {
       int Err = 0;
       struct tm tm;
       // Der Wert von tm_isdst ist entscheidend:
       //    1: Sommerzeit (Daylight Saving Time) aktiv.
       //    0: Keine Sommerzeit.
       //   -1: Die Funktion versucht, ob Sommerzeit gilt (basierend auf Umgebungsvariablen/OS).
       tm.tm_isdst = -1; // Sommerzeit automatisch bestimmen
       if (y >= 1000)
            tm.tm_year = y - 1900;
       else tm.tm_year = y + 100;
       if (!Err) tm.tm_mon  = atoi_strtok_with_check(Err)-1;
       if (!Err) tm.tm_mday = atoi_strtok_with_check(Err);
       if (!Err) tm.tm_hour = atoi_strtok_with_check(Err);
       if (!Err) tm.tm_min  = atoi_strtok_with_check(Err);
       if (!Err) tm.tm_sec  = atoi_strtok_with_check(Err);
       if (Err) {
                Serial << F("Wrong date/time entered '") << OrgTxt << F("'\n") <<
                          F("Syntax:  yy,mm,dd,hh,mm,ss\n"
                            "Example: 23,08,06,22,59,00");
                return ;
                }
       new_local_t = mktime(&tm); // Converts Local time to UTC

       if (SecOffset > -99) // Called from CompileTime_to_DateTime()
            {
            new_local_t += SecOffset;
            // Store/read the compile time to the Alarm registers in the RTC
            DS3232RTC::ALARM_TYPES_t alarmType = DS3232RTC::ALM1_MATCH_DATE;
            uint8_t Seconds, Minutes=0, Hours, Day, Month, YearHi, YearLo, Year100;
            myRTC.getAlarm(alarmType, Seconds, Minutes, Hours, Day);
            alarmType = DS3232RTC::ALM2_MATCH_DATE;
            myRTC.getAlarm(alarmType, Month, YearHi, YearLo);
            Year100 = YearHi * 16 + YearLo;

            struct tm Tmp_tm;
            localtime_r(&new_local_t, &Tmp_tm);      // Convert the time to Tmp_tm
            time_t new_utc_t   = mktime(&Tmp_tm); // Converts Local time to UTC
            // DS3232RTC_Include.h includiert die TimeLib. Diese unterstützt NICHT die Zeitzohne und die Sommerzeit!
            // => Die Funktionen hour(), weekday(), ... beziehen sich auf UTC Zeit !
            // Sie sollten nicht für die Ausgabe der Zeit bennutzt werden!
            // In den folgenden Zeilen können die Funktionen benutzt werden da sie sich auf UTC beziehen.

            //printf("getAlarm:    %2i:%02i:%02i %2i.%02i.%02i\n", Hours, Minutes, Seconds, Day, Month, Year100);
            //printf("new_utc_t:   %2i:%02i:%02i %2i.%02i.%02i\n", hour(new_utc_t), minute(new_utc_t), second(new_utc_t), day(new_utc_t), month(new_utc_t), year(new_utc_t)%100);
            if (/*Hours == hour(new_local_t) && */ Minutes == minute(new_local_t) && Seconds == second(new_local_t) // Check only minutes and seconds to
                /*&& Day == day(new_local_t) && Month == month(new_local_t) &&  Year100 == year(new_local_t)%100*/) // be able to change the tzstr
                {
                Serial << "Compile time matching with alarm time\n";
                return ; // time and date was set before
                }
            Bat_Empty_Detected = true;
            myRTC.set(new_local_t);   // Must be written BEFORE the alarm registers because
                                      // the processor is resetted a second time after uploading the program
                                      // by the serial monitor
                                      // To prevent that this happens before between writing the alarm time
                                      // the RTC time must be written before.
            myRTC.setAlarm(DS3232RTC::ALM1_MATCH_DATE, second(new_utc_t), minute(new_utc_t), hour(new_utc_t), day(new_utc_t));
            Year100 = year(new_utc_t)%100;
            myRTC.setAlarm(DS3232RTC::ALM2_MATCH_DATE, month(new_utc_t),  Year100/16, Year100 % 16);
            printf("Compile time written to the alarm registers: %2i.%02i.%02i %2i:%02i:%02\n",
                    hour(new_utc_t), minute(new_utc_t), second(new_utc_t),
                    day(new_utc_t), month(new_utc_t), Year100);
            }
       time_t t;
       //printf("Converted time: %2i:%02i:%02i %2i.%02i.%i\n", tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900); // Debug ausgabe
       t = mktime(&tm); // Converts Local time to UTC
       myRTC.set(t);
       setTime(t);
       //Serial << F("RTC time set to: "); printDateTime(t); Serial << endl;
       //Serial << "Check Time: "; printDateTime(now());

       #if USE_RESTART_TIME // 06.01.26:  Braucht man nicht mehr
         Restart_Time = millis() + 3000;
       #endif
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
  // strcpy(DateTimeStr, "2021,07,27,21,06,19"); // Debug
  Parse_DateTime_Str(DateTimeStr, 43);  // Number = compile and upload time [s]
                                        // Unfortunately this time is not constant
                                        // => The time will not be set exactly ;-(
                                        //    But it's O.K. for the Word Clock
}

//-------------------------------
void SetDateTime(const char *Arg)
//-------------------------------
{
  char tmp[30];
  strncpy(tmp, Arg, sizeof(tmp)-1);
  for (char *p = tmp; *p; p++)
      if (*p == ' ' || *p == '/' || *p == ':') *p = ',';
  Parse_DateTime_Str(tmp);
}

#if __has_include(<core_version.h>)
  #include <core_version.h> // Enthält ARDUINO_ESP32_RELEASE
#endif

//--------------
void RTC_setup()
//--------------
// Is called only once
{
  Serial << F("RealTimeClockMLX\n");
  Serial << F("Compile date: " __DATE__ " " __TIME__ "\n" ); // Don't delete this. It's importand to check the programm version

  #ifdef ESP_ARDUINO_VERSION_MAJOR // dieses define gibt es erst ab ESP Board package 2.0.0
     Serial << F("ESP32 Arduino core version:") << ESP_ARDUINO_VERSION_MAJOR << endl;
  #else
     Serial << F("ESP32 Arduino core version aelter als 2.0.0\n");
  #endif
  #ifdef ARDUINO_ESP32_RELEASE
     Serial << F("ESP32 Arduino release:") << ARDUINO_ESP32_RELEASE << endl;
  #endif
  /*
   Versionsüberprüfung:
   - core_version.h enthält bei allen bei mir installierten Versionen:
      #define ARDUINO_ESP32_RELEASE "1_0_4"    /   #define ARDUINO_ESP32_GIT_DESC 1.0.4
      #define ARDUINO_ESP32_RELEASE "1_0_6"    /   #define ARDUINO_ESP32_GIT_DESC 1.0.6
      #define ARDUINO_ESP32_RELEASE "2_0_11"   /   #define ARDUINO_ESP32_GIT_DESC 2.0.11
      #define ARDUINO_ESP32_RELEASE "2_0_17"   /   #define ARDUINO_ESP32_GIT_DESC 2.0.17
     dazu muss die datei aber eingelesen werden:
      #include <core_version.h>

   - ESP_ARDUINO_VERSION_MAJOR / .._MINOR / .._PATCH findet man in esp_arduino_version.h.
     Allerdings erst ab Version 2. Mann muss also erst mit
       #ifdef ESP_ARDUINO_VERSION_MAJOR
     prüfen ob die Version existiert
  */


  myRTC.begin();
  setSyncProvider(myRTC.get);
  Serial << F("RTC Sync ");
  if (timeStatus() != timeSet)
        Serial << F("FAIL!");
  else  Serial << F("OK");
  Serial << endl;

  CompileTime_to_DateTime();                                                                                  // 16.04.23:

  setSyncProvider(myRTC.get); // Sync again

  #ifdef RTC_PRINT_DEFINES // The #defines could be changed in the excel table. Therefore they could be printed here
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
    // Bei der Version 3.9.0 und vorangegangenen stürzt der ESP ab wenn kein Sensor angeschlossen ist         // 21.12.25:
    // und sensors.begin() aufgerufen wird. Die Temperatur wird hier auch ohne das begin() eingelesen.
    // Bei neueren Versionen der Bibliothek muss sensors.begin() aufgerufen werden sonst funktioniert
    // es nicht.
    int major, minor, patch;
    sscanf(DALLASTEMPLIBVERSION, "%d.%d.%d", &major, &minor, &patch);
    if (major > 3 || (major == 3 && minor >= 9 && patch > 0))
      sensors.begin();                   // Crash if no sensor connected (IntegerDivideByZero) in versions < 3.9.0

    sensors.setWaitForConversion(false);  // makes it async otherwise the reading of the temperatur takes 500ms
    // DeviceAddress deviceAddress;
    // if (sensors.getAddress(deviceAddress, 0)) sensors.setResolution(deviceAddress, 12);
    // sensors.setResolution(12);         // By default te resolution is 9bit = 0.5 deg C. But the resolution is still 0.5 deg C if it's set to 12 bit. Mem Usage 510 byte ;-(
    //                                    // For some reasons the temperatur is shown with 0.1 deg C for some sensors, but not for all
    //                                    // even if the type is exact the same DALLAS 18B20 2034C4 +817AB
    //                                    // According to the data sheet the resolution could be convigured. But how?
  #endif
}

#if USE_WPS
  static String tzstr;  // Variables must be defined static to be available always
  static String ntp;

  //--------------------------------
  void Set_TimeZone_and_NTP_Server()
  //--------------------------------
  {
    // NTP Zeit abfragen
    tzstr = ReadPreferences_tzstr();
    ntp   = ReadPreferences_ntp();
    //Serial << F("configTzTime('") << tzstr << F("', '") << ntp << F("')\n"); // Debug
    configTzTime(tzstr.c_str(), ntp.c_str());
    tzset(); // Weis nicht ob das schon in configTzTime enthalten ist
  }

  //-------------------------------------------
  void timeavailable(struct timeval *unused_tv)
  //-------------------------------------------
  // Callback function (gets called when time adjusts via NTP)
  {
    Serial.println("Got time adjustment from NTP!\n");
    // Die empfangene struct timeval *tv ist die UTC-Zeit.
    // Sie wird hier nicht benutzt da die Uhr bereits von der TimeLib gestellt wurde.
    Time_Set_by_Wifi = true;
    Start_Time_Set_by_Wifi = millis();

    time_t now;
    tm tm;
    time(&now); // Aktuelle Uhrzeit auslesen
    localtime_r(&now, &tm);
    printf("Actual time: %2i:%02i:%02i %2i.%02i.%i\n", tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900); // Debug ausgabe

    Serial << "Set DS3232RTC...\n";
    myRTC.set(now); // Setzt die RTC Hardware Uhrzeit auf die NTP-Zeit (UTC)

    setSyncProvider(myRTC.get); // Set the ESP time
  }
#endif // USE_WPS

//--------------
void RTC_loop()
//--------------
{
  #if USE_WPS
    if (WPS_Connected_Event)
       {
       WPS_Connected_Event= false;
       WLan_State = Network_Connected;
       }
    if (WPS_Press_WPS_Event)
       {
       WPS_Press_WPS_Event = false;
       WLan_State = Press_WPS;
       }

     #if USE_RESTART_TIME
       if (Restart_Time > 0 && millis() >= Restart_Time) // Ist aktiv wenn die Batterie leer ist
          {
          Serial << F("Restart_Time: ") << Restart_Time << endl;  // Debug
          if (Bat_Empty_Detected) WritePreferencesInt("BatEmpt", 1);
          Restart_Time = 0;
          delay(3000);
          RestartESP(); // For some reasons the date is wrong if the tzstr was changed
                        // from CET-1CEST,M3.5.0/02:00:00,M10.5.0/03:00:00 to GMT-3
                        // The time is correct
         }
    #endif // USE_RESTART_TIME

    if (WPS_Success_Event)
       {
       WPS_Success_Event = false;
       Serial << F("WPS_Success_Event\n");  // Debug
       if (Bat_Empty_Detected) WritePreferencesInt("BatEmpt", 2); // Wozu braucht man das ?
                                                                  // Vielleicht sollte es nur dann gesetzt werden wenn
                                                                  // die Batterie leer war und ich die Info über den neustart
                                                                  // retten wollte. Aber ich habe wahrscheinlich die if(BatEmpt)
                                                                  // Abfrage vergessen ;-(
                                                                  //
       // delay(3000);                                                                                        // 30.12.25:  Disabled
       // RestartESP();                                                                                       //    "
       }
    #if !defined USE_RS232_OR_SPI_AS_INPUT &&  !defined RECEIVE_LED_COLOR_PER_RS232 // Problem with DCC
      if (Serial.available() > 0) DebugPrintTime = millis() + 60000; // Pause time print for 60 seconds
      ProcSerialKeys();
    #endif
    #endif // USE_WPS


  #ifdef DEBUG_REALTIMECLOCK_TIME // Debug: Print the time to the serial output
    static time_t tLast;
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

#if USE_WEB_SERVER
  //--------------------
  void updateTimeCache()
  //--------------------
  {
    static uint32_t LastCall = 0;
    if (millis() - LastCall > 1000)
        {
        LastCall = millis();

        time_t  time = now();
        tm t;
        localtime_r(&time, &t);

        char buf[40];
        snprintf(buf, sizeof(buf),
          "%02d.%02d.%04d %02d:%02d:%02d",
          t.tm_mday, t.tm_mon+1, t.tm_year+1900,
          t.tm_hour, t.tm_min, t.tm_sec);
        g_TimeLocked = true;
        g_TimeString = String(wd_de[t.tm_wday]) + ", " + buf;
        g_TimeLocked = false;
        }
  }
#endif


#define MAXDATES 50                                                                                           // 20.12.25:  Old 10

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
    uint32_t    Next_WPS_Status_Update = 0;

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

       #if USE_WPS
         Set_TimeZone_and_NTP_Server();
       #endif
       RTC_setup();

       #if USE_WPS
         int BatEmpt = ReadPreferencesInt("BatEmpt", 0);
         if (BatEmpt > 0)
            {
            printf("Battery empty detected: %i\n", BatEmpt);
            Bat_Empty_Detected = true;
            WritePreferencesInt("BatEmpt", 0);
            }

         // set notification call-back function
         sntp_set_time_sync_notification_cb(timeavailable);
         WPS_Setup(PREFERENCESNAME);                                                                          // 20.12.25:

         #if USE_WEB_SERVER
           MDNS.begin(HOSTNAME);
           setupWeb();
           Serial.println(F("Web Interface bereit:"));
           Serial.println("http://" + String(HOSTNAME));
         #else
           Serial << F("WEB server is disabled per #define USE_WEB_SERVER 0\n");
         #endif
       #endif // USE_WPS
       } // if(!Initialized)

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
      if (Clock_Type & RTC_SINGLE || BarMode) // RTC_SINGLE used for Weekday, temperature, ... BarMode is used for the 4 dots
           {
           uint8_t  Cnt = 0;
           for (uint8_t VarNr = DstVar1; VarNr <= DstVarN; VarNr++, Cnt++)
               if (BarMode)
                    mobaLedLib.Set_Input(VarNr,Val >= Cnt );
               else mobaLedLib.Set_Input(VarNr,Val == Cnt );
           }
      else { // Normal mode used for the clock words
           uint16_t Mask = 1;
           for (uint8_t VarNr = DstVar1; VarNr <= DstVarN; VarNr++, Mask <<= 1)
               {
               #if 0 // Debug
                 if ((Clock_Type & _RTC_MODE_MASK) == RTC_5MIN)
                    {
                    bool Old = mobaLedLib.Get_Input(VarNr);
                    bool New = Val & Mask;
                    if (Old != New) printf("Set_Input(%i, %i) (old: %i)\n", VarNr, (Val & Mask)>0, mobaLedLib.Get_Input(VarNr)>0);    // Debug
                    }
               #endif
               mobaLedLib.Set_Input(VarNr, Val & Mask);
               }
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
    #endif // RTC_DEBUG

    #if USE_WPS
      //---------------------------------------------------------
      private: void Display_WLAN_Status(MobaLedLib_C& mobaLedLib)
      //---------------------------------------------------------
      {
       if (millis() - Next_WPS_Status_Update >= 20)
          {
          Next_WPS_Status_Update = millis();

          #ifdef RTC_STATUS_LED0
            uint8_t HSV_Hue;
            switch (WLan_State)
              { // HSV colors see: https://github.com/FastLED/FastLED/wiki/FastLED-HSV-Colors
              case Connecting_Network: HSV_Hue = 40;  break;  // Orange/Gelb
              case Press_WPS:          HSV_Hue = 160; break;  // Blau
              case Network_Connected:  HSV_Hue = 96;  break;  // Grün
              }
            static uint32_t Upd_Time = 0;
            static uint8_t  HSV_Val  = 127;
            static int8_t   Dir      = +1;
            static uint8_t  Step     = 10;  // 10 = Langsam, 50 = sehr schnell (Fehler), 30 = Schnell
            #ifdef RTC_SHOW_BAT_EMPTY // Normalerweise deaktiviert weil es noch nicht richtig Funktioniert nicht
              if (Bat_Empty_Detected)  Step = 30;
            #endif
            if (millis() - Upd_Time > 20)
              {
              Upd_Time = millis();
              if ((int)HSV_Val+Dir >= 255) Dir = -Step;
              if ((int)HSV_Val+Dir <= 30)  Dir = +Step;
              HSV_Val += Dir;
              mobaLedLib.leds[0] = CHSV(HSV_Hue, 255, HSV_Val);
              }
          #else // RTC_STATUS_LED0 not defined
            // Wenn Wifi verbunden ist, dann Blaulicht anzeigen
            if (Time_Set_by_Wifi)
               {
               if (Contr_Var_Def) // Enabled if function RTC_CONTR_VAR is used
                  {
                  bool Act = ((millis() - Start_Time_Set_by_Wifi) < 3000);
                  mobaLedLib.Set_Input(Contr_Var+0,  Act); // Zeit0 = Zeit Anzeige Aus (Minuten Punkte und Zeit abschalten)
                  mobaLedLib.Set_Input(Contr_Var+1, !Act); // Zeit1 = Zeit Anzeige An  (Text "Es ist Uhr" abschalten)
                  mobaLedLib.Set_Input(Contr_Var+3,  Act); // Flashing blue lights
                  if (Act == false) Time_Set_by_Wifi = false;
                  }
               }
          #endif // RTC_STATUS_LED0
          }
      }
    #endif // USE_WPS

    //-----------------------------------------
    public:void loop2(MobaLedLib_C& mobaLedLib) // loop2 runs at the main core
    //-----------------------------------------
    {
      uint8_t CType = Clock_Type & _RTC_MODE_MASK;

      #ifdef RTC_DEBUG
         uint8_t DebugPrint = Debug_Set_RTC_Minutes(mobaLedLib, CType);  // Debug
      #else
         RTC_loop();
         time_t  t = now();
         tm tm;
         localtime_r(&t, &tm);
         //printf("%s %2i.%02i.%i% 2i:%02i:%02i \n", tm.tm_isdst?"SZ":"WZ", tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
         uint16_t RTC_Minutes = tm.tm_hour * 60 + tm.tm_min;

         if (FirstInstanze)
            {
            #if USE_WPS
              Display_WLAN_Status(mobaLedLib);

              #if USE_WEB_SERVER
                updateTimeCache();
              #endif
            #endif // USE_WPS

            if (NoPrint == false && millis() > DebugPrintTime)
               {
               DebugPrintTime = millis() + 1000;
               printf("\r\t\tZeit: %2i:%02i:%02i %2i.%02i.%4i %s\t", // Achtung: Die Ausgabe wird nicht sofort an der seriellen Konsole
                        tm.tm_hour, tm.tm_min, tm.tm_sec,               // angezeigt weil kein '\n' am Ende kommt. Dadurch sieht es so aus als
                        tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900,       // würde das Programm hängen. Das liegt aber am PC
                        tm.tm_isdst?"SZ":"WZ");
               }
            #if USE_WPS && USE_RESTART_TIME
            if (tm.tm_hour == RTC_RESTART_HOUR && tm.tm_min == RTC_RESTART_MINUTE && tm.tm_sec == 0)
                 RestartESP();
            #endif
            }
      #endif // RTC_DEBUG

      #if !USE_RTC_TEMP_SENS // Don't use the RTC internal temperatur sensor
        { // USE the DS18B20 temperatur sensor
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
        case RTC_WDAY:      Val =   tm.tm_wday+1;                                             break;  // 1..7, 1 = sunday         05.01.26:  Old: weekday(t)
        case RTC_TEMP_WC:
             #if USE_RTC_TEMP_SENS
                            TempC = (myRTC.temperature()+2) / 4.0;                            break;  // +2 for correct rounding
             #else
                            TempC = round((g_TempC = sensors.getTempCByIndex(0)));
                            /* Macht keinen Sinn da die Temperatur durch die Eigenerwärmung im Gehäuse ganz falsch ist    05.01.26:
                               Außerdem kann es zu Abstürzen durch doppeltem Zugrif auf die RTC führen
                            if (TempC == -127) // In case no DS18B20 is connected we use the internal sensor in the RTC
                                TempC = ((g_TempC = myRTC.temperature()+2) / 4.0);
                            */
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
        case RTC_DAYOFYEAR: { // Birthday or other event
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
                            } // End Birthday
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


      #ifdef RTC_DEBUG
         if (DebugPrint) Debug_Print(RTC_Minutes, CType, Val, TempC);
      #endif

      //pinMode(14, OUTPUT); digitalWrite(14,Disable_Outputs); // Debug
      if (Disable_Outputs) Val = DisabVal;
      Set_Variables(mobaLedLib, Val, CType == RTC_5MIN_OFFS);
    }
}; // end class RT_Clock


#endif // __RT_CLOCK_EXTENTION__
