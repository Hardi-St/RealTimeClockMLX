# Real time clock MobaLedLib Extension

The library adds a real time clock to then MobaLedLib.

More information see https://wiki.mobaledlib.de.

Questions / suggestions / praise / ...
  MobaLedLib@gmx.de

**Revision History:**

**Ver.: 0.0.1** 29.09.23:  initial release
**Ver.: 0.1.0** 29.12.25:  Added WLAN with WPS support
                           - Incremented MAXDATES to 50 (Old 10)
                           - Read time via WLAN
                           - Connect automatically to WLAN if WPS button on the router is pressed
                           - Two leds at the top flash for 5 seconds if WLAN if connected.  Also if WPS is connected
                             If RTC_STATUS_LED0 is defined the LED0 is used as status instead
                           - Serial command implemented. Enter ? to get a list of commands
                           - Wifi could be entered manually
                           - New DallasTemperature library > 3.9.0 is supported
**Ver.: 1.0.1** 06.01.26:  - Finished WLAN / MPS support
                           - Web interface to change the time zone: http://word_clock.local
