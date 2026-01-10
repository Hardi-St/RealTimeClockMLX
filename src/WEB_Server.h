/*
 Web Inteface wurde mit ChatGPT erstellt



 ToDo:
 - Umstellen auf Englisch? War in der ersten Version "WEB_Server - Version mit Englisch_h"
 - Variablen anzeigen verbessern. Hier sollen auch die WLAN Daten anzezeigt werden.
   Wird nur dann benötigt wenn USE_TEST_BUTTONS aktiviert wird
*/

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <time.h>

// ======================================================
// KONFIGURATION
// ======================================================

#define DEF_TZSTR_DST   "CET-1CEST,M3.5.0/02:00:00,M10.5.0/03:00:00"
#define DEF_TZSTR_NODST "CET-1"

#define USE_TEST_BUTTONS 0

// ======================================================
// EXTERNE VARIABLEN / FUNKTIONEN
// ======================================================
extern float g_TempC;

void WritePreferencesTzStr(const char *Val);
String ReadPreferences_tzstr();
void WritePreferences(const char *VarName, const char *Val);
String ReadPreferences_ntp();

#ifdef LINEARUHR
  void SetBrightness(int _Brightness);
  byte ReadPreferences_Brightness();
  void SetCPBrightness(int _Brightness);
  byte ReadPreferences_CPBrightness();
#endif

void Clear_Var();

//----------------------
String List_Var_String()
//----------------------
{
  String s;
  s += "ntp: " + ReadPreferences_ntp() + "\n";
  s += "tzstr: " + ReadPreferences_tzstr() + "\n";
  return s;
}


// ======================================================
AsyncWebServer server(80);

// ======================================================
// DEUTSCHE WOCHENTAGE
// ======================================================
const char* wd_de[] = {
  "Sonntag","Montag","Dienstag","Mittwoch",
  "Donnerstag","Freitag","Samstag"
};


// ======================================================
// ZEIT
// ======================================================
String getTimeString()
{
  while (g_TimeLocked) ; // Wait
  return g_TimeString;
}

// ======================================================
// TEMPERATUR
// ======================================================
String getTempString()
{
  if (g_TempC < -100) return "---";
  return String(g_TempC, 1) + " &deg;C";
}

// ======================================================
// TZ LISTE
// ======================================================
struct TZEntry {
  const char* name;
  const char* tz;
};

TZEntry tzlist[] = {
  { "Deutschland", DEF_TZSTR_DST },
  { "Deutschland (Ohne Sommerzeit Umst.)",  DEF_TZSTR_NODST },
  { "England", "GMT0BST,M3.5.0/1,M10.5.0" },
  { "Frankreich", "CET-1 CEST,M3.5.0/02:00:00,M10.5.0/03:00:00" },  // Gleich wie Deutschland nur ein zusätzliches Leerzeichen
  { "Neuseeland", "NZST-12NZDT,M9.5.0,M4.1.0/3" }
};

String getTZName(const String& tz)
{
  for (auto &t : tzlist) {
    if (tz == t.tz) return String(t.name);
  }
  return "Benutzer definiert";
}

// ======================================================
// HTML
// ======================================================
// HTML codes see: https://www.designerinaction.de/tipps-tricks/web-development/html-umlaute-sonderzeichen/
String htmlPage()
{
  String savedTZ = ReadPreferences_tzstr();
  bool knownTZ = false;
  String page;
  page += "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page += "<title>Word Clock</title>";

  // CSS
  page += "<style>";
  page += "body{font-family:Arial;background:#f2f2f2;padding:10px;}";
  page += ".box{background:#fff;padding:15px;border-radius:8px;max-width:500px;}";
  page += "label{font-weight:bold;margin-top:10px;display:block;}";
  page += "input,select{width:100%;padding:6px;box-sizing:border-box;}";
  page += "button{padding:6px 12px;margin-top:6px;}";
  page += ".debug{margin-top:10px;}";
  page += "</style>";

  // JS
  page += "<script>";
  page += "setInterval(()=>{fetch('/time').then(r=>r.text()).then(t=>{document.getElementById('time').innerHTML=t;});},1000);";
  page += "function tzChanged(sel){document.getElementById('tzstr').value=sel.value;}";
  page += "</script>";
  page += "</head><body><div class='box'>";
#ifdef WPS_CLIENT_NAME
  page += "<h2>" WPS_MODEL_NAME " &ndash; " WPS_CLIENT_NAME "</h2>";
#else
  page += "<h2>" WPS_MODEL_NAME " </h2>";
#endif
  page += "<p><b>Version:</b> " VER_STR "</p>";

  page += "<p><b>Zeit:</b><br><span id='time'>" + getTimeString() + "</span></p>";
#ifdef WORDCLOCK
  page += "<p><b>Temperatur:</b> " + getTempString() + "</p>";
#endif
  page += "<p><b>Aktuelle Zeitzone:</b><br>";
  page += getTZName(savedTZ) + "<br><small>" + savedTZ + "</small></p>";
  page += "</p>";

  page += "<form action='/save'>";

  page += "<label>Zeitzone</label>";
  page += "<select name='tzsel' onchange='tzChanged(this)'>";

  for (auto &t : tzlist) {
    bool sel = (savedTZ == t.tz);
    if (sel) knownTZ = true;

    page += "<option value='" + String(t.tz) + "'";
    if (sel) page += " selected";
    page += ">" + String(t.name) + "</option>";
  }

  // Benutzerdefiniert
  page += "<option value='' ";
  if (!knownTZ) page += "selected";
  page += ">Benutzer definiert</option>";
  page += "</select>";

  page += "<label>TZ-String</label>";
  page += "<input id='tzstr' name='tzstr' value='" + ReadPreferences_tzstr() + "'>";

  page += "<label>NTP Server</label>";
  page += "<input name='ntp' value='" + ReadPreferences_ntp() + "'>";

#ifdef LINEARUHR
  page += "<label>Helligkeit WS2812 (1..255)</label>";
  page += "<input id='brightness' name='brightness' value='" + String(ReadPreferences_Brightness()) + "'>";

  page += "<label>Helligkeit Charliplexing (1..255)</label>";
  page += "<input id='CPBright'   name='CPBright'   value='" + String(ReadPreferences_CPBrightness()) + "'>";
#endif

  page += "<button>Speichern</button>";
  page += "</form>";

#if USE_TEST_BUTTONS
  page += "<hr>";
  page += "<div class='debug'><b>Nur f&uuml;r Tests:</b><br>";
  page += "<form action='/clear'><button>Daten l&ouml;schen</button></form>";
  page += "<form action='/list'><button>Variablen anzeigen</button></form>";
  page += "<form action='/restart'><button>ESP neu starten</button></form>";
  page += "</div>";
#endif
  page += "</div></body></html>";
  return page;
}

// ======================================================
// SETUP WEB
// ======================================================
void setupWeb()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/html; charset=utf-8", htmlPage());
  });

  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/plain; charset=utf-8", getTimeString());
  });

  server.on("/save", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (req->hasParam("tzstr"))      WritePreferencesTzStr(req->getParam("tzstr")->value().c_str());
    if (req->hasParam("ntp"))        WritePreferences("ntp", req->getParam("ntp")->value().c_str());

  #ifdef LINEARUHR
    if (req->hasParam("brightness")) SetBrightness  (atoi(req->getParam("brightness")->value().c_str()));
    if (req->hasParam("CPBright"))   SetCPBrightness(atoi(req->getParam("CPBright")->value().c_str()));
  #endif

    req->redirect("/");
  });

  server.on("/clear", HTTP_GET, [](AsyncWebServerRequest *req) {
    Clear_Var();
    req->redirect("/");
  });

  server.on("/list", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/plain; charset=utf-8", List_Var_String());
  });

  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/plain", "Restart...");
    delay(300);
    ESP.restart();
  });

  server.begin();
}


