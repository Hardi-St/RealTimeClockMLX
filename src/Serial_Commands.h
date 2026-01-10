/*
Modul to read in serial commands
*/


// Default values used if nothing is entered
#define DEF_NTP    "de.pool.ntp.org"
#define DEF_TZSTR  "CET-1CEST,M3.5.0/02:00:00,M10.5.0/03:00:00"


const char CmdHelp[] PROGMEM =
    "\n"
    "Serial commands:                                                         "  VER_STR "\n"
    "~~~~~~~~~~~~~~~~\n"
    "?                        Show this help                                              \n"
    "ssid  <Network Name>     Define your wlan network name                               \n"
    "pwd   <Password>         Define your wlan password                                   \n"
    "scan                     Scan available networks and prind ssid                      \n"
    "ntp   <time server>      Define the network time protocol server                     \n"
    "                         Default:                                                    \n"
    "                           " DEF_NTP                                                "\n"
    "tzstr <Time zone string> Define the time zone and the summer time                    \n"
    "                         Example (germany):                                          \n"
    "                           " DEF_TZSTR                                              "\n"
    "                         The TZ string contains the following infos:                 \n"
    "                         std offset[dst[offset][,start[/time],end[/time]]]           \n"
    "                         std:    CET   Central European Time                         \n"
    "                         offset: -1    Germany is one hour behind CET                \n"
    "                         dst:    CEST  Central European Summer Time                  \n"
    "                         start:  M3.5.0/02:00:00: change to summer time on           \n"
    "                                   M3 = 3 month (march)                              \n"
    "                                   5 = 5 week (last week of march)                   \n"
    "                                   0 = sunday 2h00                                   \n"
    "                         end:    M10.5.0/03:00:00: change to winter time on          \n"
    "                                   M10 = 10 month (october)                          \n"
    "                                   5 = 5 week (last week of october)                 \n"
    "                                   0 = sunday 3h00                                   \n"
    "                        The time zones could be found here:                          \n"
    "                        https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv\n"
    "list                    List the configuration variables                             \n"
    "clear                   Clear all variables                                          \n"
    "restart                 Save and restart                                             \n"
    "noprint                 Stop printing periodic messages like the actual time         \n"
 #ifdef LINEARUHR
    "time <h> [m] [s]        Set the time for tests (Not permanent)                       \n"
    "brightness  <Val>       Brightness of the LEDs between 1 and 255 (only WS2811 LEDs)  \n"
    "cpbright    <Val>       Brightness of the Charliplexing LEDs between 1 and 255       \n"
    "ct PosPin NegPin        Test one Chaliplexing LED (ct 0: Enable the clock again)     \n"
    "cp [wait]               Charliplexing Period test (Wait:[us], cp 200000: slow test)  \n"
 #endif
 #ifdef WORDCLOCK
    "time <h> [m] [s]        Set the time of the RTC if no WLAN is available              \n"
    "                        Attention: use the next function if the date has to be       \n"
    "                        corrected also.                                              \n"
    "datetime yy/m/d h:m:s   Set date and time if no WLAN is available.                   \n"
    "                        Example 'datetime 25/12/24 7:30:00'\n"
 #endif
    ;

#define MAX_MESSAGE_LENGTH  50

int Brightness;                           // Helligkeit 1..255
int CPBright;

//---------------
void RestartESP()
//---------------
{
  // Software reset via esp_restart
  Serial << F("Restarting...");
  delay(100);
  ESP.restart();
}


//---------------------------------------------------------
void WritePreferences(const char *VarName, const char *Val)
//---------------------------------------------------------
{
  Preferences LocPref;
  //Serial << "WritePreferences " << PREFERENCESNAME << ": " << VarName << "=" << Val << endl; // Debug
  LocPref.begin(PREFERENCESNAME);
  LocPref.putString(VarName, Val);
  LocPref.end();
}

//-----------------------------------------
void WritePreferencesTzStr(const char *Val)
//-----------------------------------------
{
  WritePreferences("tzstr", Val);
  Set_TimeZone_and_NTP_Server();
  DebugPrintTime = 0; // Immediately print the time
}


#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(2, 0, 0)
  #define USE_PREF_ISKEY
#endif

//--------------------------------------------------------------
String ReadPreferences(const char *VarName, const char *Default)
//--------------------------------------------------------------
{
  Preferences LocPref;
  LocPref.begin(PREFERENCESNAME);
  String Res;
  #ifdef USE_PREF_ISKEY // isKey() is not available in old ESP Library
    if (LocPref.isKey(VarName)) Res = LocPref.getString (VarName); // If the error "'class Preferences' has no member named 'isKey'"
                                                                   // is shown a new version of the ESP libraray has to be installed
  #else
    Res = LocPref.getString (VarName);
  #endif
  LocPref.end();
  //Serial << "ReadPreferences " << PREFERENCESNAME << ": " << VarName << "=" << (Res == "" ? Default : Res) << endl; // Debug
  if (Res == "")
       return Default;
  else return Res;
}

//--------------------------
String ReadPreferences_ntp()
//--------------------------
{
  return ReadPreferences("ntp", DEF_NTP);
}

//------------------------------------------------------
int ReadPreferencesInt(const char *VarName, int Default)
//------------------------------------------------------
{
  Preferences LocPref;
  LocPref.begin(PREFERENCESNAME);
  int Res;
  #ifdef USE_PREF_ISKEY // isKey() is not available in old ESP Library
    bool Exists = LocPref.isKey(VarName);
    if (Exists) Res = LocPref.getInt(VarName);
    LocPref.end();
    if (Exists)
         return Res;
    else return Default;
  #else
    // Attention: No Default value possible
    Res = LocPref.getInt(VarName);
    LocPref.end();
    return Res;
  #endif
}

//----------------------------------------------------
void WritePreferencesInt(const char *VarName, int val)
//----------------------------------------------------
{
  Preferences LocPref;
  LocPref.begin(PREFERENCESNAME);
  LocPref.putInt(VarName, val);
  LocPref.end();
}

#ifdef LINEARUHR
//--------------------------------
void SetBrightness(int _Brightness)
//--------------------------------
{
  if (_Brightness < 1)   _Brightness = 1;
  if (_Brightness > 255) _Brightness = 255;
  Brightness = _Brightness; // Set the global variable which is used in the loop()
  Preferences LocPref;
  LocPref.begin(PREFERENCESNAME);
  LocPref.putInt("brightness", Brightness);
  LocPref.end();
}


//-------------------------------
byte ReadPreferences_Brightness()
//-------------------------------
{
 int Bri = ReadPreferencesInt("brightness", 3);
 if (Bri <= 0) Bri = 3;
 return Bri;
}


//-----------------------------------
void SetCPBrightness(int _Brightness)
//-----------------------------------
{
  if (_Brightness < 1)   _Brightness = 1;
  if (_Brightness > 255) _Brightness = 255;
  CPBright = _Brightness; // Set the global variable which is used in the loop()
  Preferences LocPref;
  LocPref.begin(PREFERENCESNAME);
  LocPref.putInt("CPbright", CPBright);
  LocPref.end();
}

//---------------------------------
byte ReadPreferences_CPBrightness()
//---------------------------------
{
 int Bri = ReadPreferencesInt("CPbright", 255);
 if (Bri <= 0) Bri = 3;
 return Bri;
}


#endif


//----------------------------
String ReadPreferences_tzstr()
//----------------------------
{
  return ReadPreferences("tzstr", DEF_TZSTR);
}

//---------------------------
void SetTime(const char *Arg)
//---------------------------
{
  time_t t = now();
  struct tm Tm;
  Serial << "getenv(TZ)='" << getenv("TZ") << "'\n";
  localtime_r(&t, &Tm); // local offset
  int h = Tm.tm_hour;
  int m = Tm.tm_min;
  int s = Tm.tm_sec;
  printf("Old Time: %2i:%02i:%02i %2i.%02i.%02i\n", h, m, s, Tm.tm_mday, Tm.tm_mon+1, Tm.tm_year+1970); // Debug

  char tmp[20];
  strncpy(tmp, Arg, sizeof(tmp)-1);
  for (char *p = tmp; *p; p++)
    if (*p == ':') *p = ' ';
  int cnt = sscanf(tmp, "%i %i %i", &h, &m, &s);
  if (cnt >= 1) Tm.tm_hour = h;
  if (cnt >= 2) Tm.tm_min  = m;
  if (cnt >= 3) Tm.tm_sec  = s;
  t = mktime(&Tm);
  printf("Setting System Time: %s", asctime(&Tm));
  struct timeval newtv = { .tv_sec = t };

#ifdef LINEARUHR
  settimeofday(&newtv, NULL);
#endif

#ifdef WORDCLOCK
  myRTC.set(newtv.tv_sec);   // use the time_t value to ensure correct weekday is set
  setTime(newtv.tv_sec);
  t= now();
  localtime_r(&t, &Tm); // local offset
  printf("\nNeue Zeit: %2i:%02i:%02i %2i.%02i.%4i (Wird beim naechsten start ueberschrieben wenn WLAN vorhanden)\n",
         Tm.tm_hour, Tm.tm_min, Tm.tm_sec, Tm.tm_mday, Tm.tm_mon+1, Tm.tm_year+1900);
#endif
}


#ifdef LINEARUHR
//-----------------------------------
void CharlieplexTest(const char *Arg)
//-----------------------------------
// Input Only 34-39
{            //
 byte Pins[] = {12, 14, 27, 26, 25, 33, 32, 23, 22, 13, 21, 19, 18, 5, 17, 16, 4, 0 };
 int Hi, Lo;
 bool HiOk = false, LoOk = false;
 if (sscanf(Arg, "%i %i", &Hi, &Lo) == 2)
       {
       UpdateCP = false;
       Serial << F("CharlieplexTest ") << Hi << " " << Lo << "   {valid pins: ";
       for (int i = 0; i < sizeof(Pins); i++)
           {
           pinMode(Pins[i], INPUT);
           Serial << Pins[i] << " ";
           if (Pins[i] == Hi)  HiOk = true;
           if (Pins[i] == Lo)  LoOk = true;
           }
       Serial << "}\n";
       if (HiOk && LoOk)
            {
            digitalWrite(Hi, 1); pinMode(Hi, OUTPUT);
            digitalWrite(Lo, 0); pinMode(Lo, OUTPUT);
            }
       else Serial << "Invalid pins given\n";
       }
  else UpdateCP = true;
}
#endif

//----- --------
void List_Var()
//-------------
{
  Serial << F("\n");
  WiFi_ListNetworks();
  Serial << F(" ntp "        ) << ReadPreferences_ntp()                << endl;
  Serial << F(" tzstr "      ) << ReadPreferences_tzstr()              << endl;
#ifdef LINEARUHR
  Serial << F(" brightness " ) << ReadPreferences_Brightness()         << endl;
  Serial << F(" cpbright   " ) << ReadPreferences_CPBrightness()       << endl;
#endif
}

//--------------
void Clear_Var()
//--------------
{
  Preferences LocPref;
  LocPref.begin(PREFERENCESNAME);
  LocPref.clear();
  LocPref.end();
  WiFi_ClearNetworks();
  Serial << F("All variables cleared / set to default\n");
}

#define If_Msg_Start(Cmd) if (strncmp(message, Cmd, (CmdLen = strlen(Cmd))) == 0)
#define MsgPar (message+CmdLen)

char message[MAX_MESSAGE_LENGTH]; // Create a place to hold the incoming message
unsigned int message_pos = 0;


//-----------------
bool SerialCmdAct()
//-----------------
{
  return message_pos != 0;
}

String NewSSID;
String NewPwd;

//----------------------------------------------
void Add_SSIO_and_PWD_if_filled(bool Check_Both)
//----------------------------------------------
{
  if (NewSSID.length() > 0 && (Check_Both == false || NewPwd.length() > 0))
    {
    Serial << "Adding SSID & pwd\n";
    WiFi_AddNetwork(NewSSID.c_str(), NewPwd.c_str());
    NewSSID = "";
    NewPwd  = "";
    }
}

//----------------------------
void Add_SSID(const char *Par)
//----------------------------
{
  NewSSID = Par;
  Add_SSIO_and_PWD_if_filled(true);
}

//---------------------------
void Add_PWD(const char *Par)
//---------------------------
{
  NewPwd = Par;
  Add_SSIO_and_PWD_if_filled(false);
}

//-----------------------------
void ProcSerialCmd(char inByte)
//-----------------------------
{
  static bool Activ = false;
  if (Activ) return ; // Prevent recursiv calls in case the calling proc also waits for serial characters using ProcButtonsAndSerialKeys()
  Activ = true;
  if ( inByte != '\n' && inByte != '\r' && (message_pos < MAX_MESSAGE_LENGTH - 1) ) // Message coming in (check not terminating character) and guard for over message size
       {
       // Add the incoming byte to our message
       if (inByte == 8) // Del
            {
            if (message_pos > 0) message_pos--;
            }
       else {
            message[message_pos] = inByte;
            message_pos++;
            }
       }
  else if (message_pos)
       { // Full message received...
       bool CP_LEDs[39];
       for (int i = 0; i < 39; i++) CP_LEDs[i] = true;
       message[message_pos] = '\0'; // Add null character to string

       delay(20); // To receive the '\n'
       if (Serial.available() && Serial.peek() == '\n') Serial.read(); // Remove '\n' because this will be interpreted as abort when
                                                                       // reading the loco data in Check_Abort_Button_or_SerialChar()
       message_pos = 0;
       //Dprintf("'%s'\n", message); // Debug
       uint16_t CmdLen = 0;
       If_Msg_Start("?")                Serial.println(CmdHelp);
       else If_Msg_Start("ssid ")       Add_SSID(MsgPar);
       else If_Msg_Start("pwd ")        Add_PWD(MsgPar);
       else If_Msg_Start("pwd")         Add_PWD(""); // empty password
       else If_Msg_Start("ntp ")        WritePreferences("ntp", MsgPar);
       else If_Msg_Start("tzstr ")      WritePreferencesTzStr(MsgPar);
       else If_Msg_Start("lscan")       WiFi_LongScan_Debug();
       else If_Msg_Start("scan")        Wifi_Scan();
       else If_Msg_Start("list")        List_Var();
       else If_Msg_Start("clear")       Clear_Var();
       else If_Msg_Start("restart")     RestartESP();
       else If_Msg_Start("R")           RestartESP();
       else If_Msg_Start("time ")      {SetTime(MsgPar);DebugPrintTime = 0;}
       else If_Msg_Start("time")        DebugPrintTime = 0;
       else If_Msg_Start("noprint")     NoPrint = !NoPrint;
     #ifdef WORDCLOCK
       else If_Msg_Start("datetime ")  {SetDateTime(MsgPar);DebugPrintTime = 0;}
     #endif
     #ifdef LINEARUHR
       else If_Msg_Start("brightness ") SetBrightness(atoi(MsgPar));
       else If_Msg_Start("cpbright " )  SetCPBrightness(atoi(MsgPar));
       else If_Msg_Start("ct ")         CharlieplexTest(MsgPar);
       else If_Msg_Start("cp ")         { UpdateCP = false; ShowCharlieplexingLEDs(CP_LEDs, true, atoi(MsgPar)); UpdateCP = true; }
       else If_Msg_Start("cp")          { UpdateCP = false; ShowCharlieplexingLEDs(CP_LEDs, true);               UpdateCP = true; }
     #endif // LINEARUHR
       else {
            Serial << F("Unknown serial command '") << message << F("'. Enter ? for help.\n");
            }
       }
    Activ = false;
}

//-------------------
bool ProcSerialKeys()
//-------------------
{
  if (Serial.available()) // Test with the PC Cursor keys
       {
       char c = Serial.read();
       if (c != 255) ProcSerialCmd(c);  // For some reasons some times 255 is read
       }
  return SerialCmdAct();
}
