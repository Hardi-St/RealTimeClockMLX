/*
 Tested with:
 - Fritz box 6660
 - Fritz WLAN-Repeater
 - Vodafone Rooter TG3442DE


 Erweiterte Version (Update):
 - Deutlicher WPS-Hinweis für den User
 - Einheitliche Funktionskopf-Kommentare mit Trennstrichen
 - API-Funktionen zum Hinzufügen/Setzen einer SSID
 - API-Funktion zur Anzeige aller gespeicherten Netzwerke
*/

#include "sdkconfig.h"
#if CONFIG_ESP_WIFI_REMOTE_ENABLED
  #error "WPS is only supported in SoCs with native Wi-Fi support"
#endif

#include <WiFi.h>
#include <esp_wps.h>
#include <Preferences.h>

#define ESP_WPS_MODE WPS_TYPE_PBC
#define MAX_NETWORKS 5

static char PREF_NS[32]    = "wifi";
static bool WPS_Connected_Event = false;
static bool WPS_Success_Event   = false;
static bool WPS_Press_WPS_Event = false;

// ============================================================
// WPS Funktionen
// ============================================================

#ifndef WPS_MANUFACTURER
  #define WPS_MANUFACTURER "MobaLedLib"
#endif

#ifndef WPS_MODEL_NAME
  #define WPS_MODEL_NAME   "ESP32"
#endif

#ifndef WPS_DEVICE_NAME
  #define WPS_DEVICE_NAME  "ESP DEVICE"
#endif

//------------------------------------------------------------------
void WPS_Start()
//------------------------------------------------------------------
{


  esp_wps_config_t config = WPS_CONFIG_INIT_DEFAULT(ESP_WPS_MODE);
  strcpy(config.factory_info.manufacturer, WPS_MANUFACTURER);  // "MobaLedLib"
  strcpy(config.factory_info.model_number, CONFIG_IDF_TARGET); // contains "esp32"

  #ifdef WPS_CLIENT_NAME                                       // "Word Clock-Wohnzimmer" / "Linear Uhr-Hardi"
    snprintf(config.factory_info.model_name, sizeof(config.factory_info.model_name), "%s-%s", WPS_MODEL_NAME, WPS_CLIENT_NAME);
  #else
    strcpy(config.factory_info.model_name,   WPS_MODEL_NAME);  // "Word Clock" / "Linear Uhr"
  #endif

  uint64_t mac;
  esp_read_mac((uint8_t*)&mac, ESP_MAC_WIFI_STA);

  char devName[33];
  snprintf(devName, sizeof(devName), WPS_DEVICE_NAME "-%04X", (uint16_t)(mac & 0xFFFF));
  printf("device_name:'%s'\n", devName); // debug
  strcpy(config.factory_info.device_name,  devName);           // "ESP DEVICE-" <MAC Adresse>

  if (esp_wifi_wps_enable(&config) != ESP_OK) return;
  esp_wifi_wps_start(0);
}

//------------------------------------------------------------------
void WPS_Stop()
//------------------------------------------------------------------
{
  esp_wifi_wps_disable();
}

// ============================================================
// Preferences – Multi-SSID Verwaltung
// ============================================================
//------------------------------------------------------------------
void LoadNetworks(String ssid[], String pwd[], int &count)
//------------------------------------------------------------------
{
  Preferences p;
  p.begin(PREF_NS, false);
  count = p.getUInt("count", 0);
  if (count > MAX_NETWORKS) count = MAX_NETWORKS;

  for (int i = 0; i < count; i++) {
    ssid[i] = p.getString(("ssid" + String(i)).c_str(), "");
    pwd[i]  = p.getString(("pwd"  + String(i)).c_str(), "");
  }
  p.end();
}

//------------------------------------------------------------------
void SaveNetworks(String ssid[], String pwd[], int count)
//------------------------------------------------------------------
{
  Preferences p;
  p.begin(PREF_NS, false);
  p.putUInt("count", count);
  for (int i = 0; i < count; i++) {
    p.putString(("ssid" + String(i)).c_str(), ssid[i]);
    p.putString(("pwd"  + String(i)).c_str(), pwd[i]);
  }
  p.end();
}

//------------------------------------------------------------------
void PromoteNetwork(const String &ssid, const String &pwd)
//------------------------------------------------------------------
{
  String ssids[MAX_NETWORKS];
  String pwds[MAX_NETWORKS];
  int count;

  LoadNetworks(ssids, pwds, count);

  for (int i = 0; i < count; i++) {
    if (ssids[i] == ssid) {
      for (int j = i; j < count - 1; j++) {
        ssids[j] = ssids[j + 1];
        pwds[j]  = pwds[j + 1];
      }
      count--;
      break;
    }
  }

  if (count < MAX_NETWORKS) count++;

  for (int i = count - 1; i > 0; i--) {
    ssids[i] = ssids[i - 1];
    pwds[i]  = pwds[i - 1];
  }

  ssids[0] = ssid;
  pwds[0]  = pwd;

  SaveNetworks(ssids, pwds, count);
}

// ============================================================
// Öffentliche API-Funktionen
// ============================================================
//------------------------------------------------------------------
void WiFi_AddNetwork(const char *ssid, const char *pwd)
//------------------------------------------------------------------
{
  PromoteNetwork(String(ssid), String(pwd));
}

//------------------------------------------------------------------
void WiFi_ListNetworks()
//------------------------------------------------------------------
{
  String ssids[MAX_NETWORKS];
  String pwds[MAX_NETWORKS];
  int count;

  LoadNetworks(ssids, pwds, count);

  Serial.println(F(" Stored WiFi Networks:"));
  for (int i = 0; i < count; i++) {
    Serial.printf( "  %d: SSID='%s'  PWD='%s'\n", i, ssids[i].c_str(), pwds[i].c_str());
  }
}

//------------------------------------------------------------------
void WiFi_ClearNetworks()
//------------------------------------------------------------------
{
  Preferences p;
  p.begin(PREF_NS, false);
  p.clear();
  p.end();

  Serial.println(F("All stored WiFi networks deleted."));
}

// ============================================================
// WiFi Event
// ============================================================
//-------------------------------
void WiFiEvent(WiFiEvent_t event)
//-------------------------------
{
  if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
    Serial.println("Connected to " + WiFi.SSID());
    PromoteNetwork(WiFi.SSID(), WiFi.psk());
    WPS_Connected_Event = true;
  }

  if (event == ARDUINO_EVENT_WPS_ER_SUCCESS) {
    Serial.println("WPS success");
    WPS_Stop();
    delay(100);
    WiFi.begin();
    WPS_Success_Event = true;
  }

  // Debug
  Serial.printf("[WiFi] Event: ");
  switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_START:         Serial.println("STA_START");                                         break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:     Serial.print("CONNECTED to SSID: "); Serial.println(WiFi.SSID());    break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:        Serial.print("GOT_IP: ");            Serial.println(WiFi.localIP()); break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:  Serial.println("DISCONNECTED");                                      break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:         Serial.println("WPS SUCCESS");                                       break;
    case ARDUINO_EVENT_WPS_ER_FAILED:          Serial.println("WPS FAILED");                                        break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:         Serial.println("WPS TIMEOUT");                                       break;
    default:                                   Serial.print("OTHER:");              Serial.println(event);          break;
    }
}

// ============================================================
// Setup-Funktion
// ============================================================
//------------------------------------------------------------------
void WPS_Setup(const char *prefName = "wifi", uint16_t timeout = 30)
//------------------------------------------------------------------
{
  WPS_Connected_Event = false;
  WPS_Success_Event   = false;
  WPS_Press_WPS_Event = false;
  strncpy(PREF_NS, prefName, sizeof(PREF_NS));

  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiEvent);

  String ssids[MAX_NETWORKS];
  String pwds[MAX_NETWORKS];
  int count = 0;

  LoadNetworks(ssids, pwds, count);

  bool connected = false;

  for (int i = 0; i < count && !connected; i++) {
    Serial.println("Try WiFi: " + ssids[i]);
    if (pwds[i].length() == 0)
         WiFi.begin(ssids[i].c_str());
    else WiFi.begin(ssids[i].c_str(), pwds[i].c_str());

    int t = 0;
    while (WiFi.status() != WL_CONNECTED && t++ < timeout * 2)
        {
        delay(500);
        #ifdef WAIT_PROC
           WAIT_PROC;
        #else
           Serial.print(".");
        #endif
        }


    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      PromoteNetwork(ssids[i], pwds[i]);
      break;
    }
    WiFi.disconnect(true);
  }

  if (!connected) {
    WPS_Press_WPS_Event = true;
    Serial.println(F("Failed to connect\n"
                     "Try to connect via WPS\n\n"
                     "#######################################\n"
                     "## Please press WPS-button on router ##\n"
                     "#######################################\n"));

  // *** WICHTIG: WiFi komplett resetten ***
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_MODE_NULL);
  delay(200);
  WiFi.mode(WIFI_MODE_STA);

  WPS_Start();
  }
}

//--------------
void Wifi_Scan()
//--------------
{
  Serial.println("Scanning WiFi...");
  WiFi.disconnect(false);
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.printf("Found %d networks\n", n);

  for (int i = 0; i < n; i++)
    {
    Serial.printf("%d: %s (%d dBm) %s\n",
                i,
                WiFi.SSID(i).c_str(),
                WiFi.RSSI(i),
                (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "OPEN" : "SECURED");
    }
}

#include <esp_wifi.h>

//------------------------------------------------------------------
void WiFi_LongScan_Debug()
//------------------------------------------------------------------
// Das Funktioniert nicht ;-(
// Ist aber auch egal
{
  WiFi.disconnect(false);
  delay(100);

  wifi_scan_config_t scanConf;
  memset(&scanConf, 0, sizeof(scanConf));

  scanConf.ssid = NULL;
  scanConf.bssid = NULL;
  scanConf.channel = 0;          // alle Kanäle
  scanConf.show_hidden = true;
  scanConf.scan_type = WIFI_SCAN_TYPE_ACTIVE;

  scanConf.scan_time.active.min = 400; // ms pro Kanal
  scanConf.scan_time.active.max = 800;

  Serial.println(F("\nStarting long WiFi scan..."));
  esp_wifi_scan_start(&scanConf, true);

  uint16_t n = 0;
  esp_wifi_scan_get_ap_num(&n);

  Serial.printf("Scan finished, found %d networks\n", n);

  wifi_ap_record_t *recs = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * n);
  if (!recs)
     {
     Serial.printf("Error allocating %i bytes for wifi_ap_record\n", sizeof(wifi_ap_record_t) * n);
     return;
     }
  Serial.printf("Allocated %i bytes for wifi_ap_record\n", sizeof(wifi_ap_record_t) * n);

  uint16_t n2 = n;
  esp_wifi_scan_get_ap_records(&n2, recs);

  Serial.printf("Going to print %i lines (n2=%i)\n", n, n2);

  for (int i = 0; i < n; i++) {
    Serial.printf(
      "%2d: %-32s | RSSI %4d | CH %2d | %s | %02X:%02X:%02X:%02X:%02X:%02X\n",
      i,
      (char *)recs[i].ssid,
      recs[i].rssi,
      recs[i].primary,
      (recs[i].authmode == WIFI_AUTH_OPEN) ? "OPEN" : "SECURED",
      recs[i].bssid[0], recs[i].bssid[1], recs[i].bssid[2],
      recs[i].bssid[3], recs[i].bssid[4], recs[i].bssid[5]
    );
  }

  free(recs);
  Serial.println("End");
}

