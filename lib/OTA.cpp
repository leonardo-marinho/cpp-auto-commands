#ifndef OTA_cpp
#define OTA_cpp

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

class OTA
{
private:
  char *m_wifiSSID;
  char *m_wifiPassword;

  void setupOTA()
  {
    ArduinoOTA.begin();
  }

  void setupWiFi()
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(m_wifiSSID, m_wifiPassword);
  }

public:
  OTA(char *t_otaHostname, char *t_otaPassword, char *t_wifiSSID, char *t_wifiPassword)
      : m_wifiSSID(t_wifiSSID),
        m_wifiPassword(t_wifiPassword)
  {
    ArduinoOTA.setHostname(t_otaHostname);
    ArduinoOTA.setPassword(t_otaPassword);
  }

  void setup()
  {
    setupWiFi();
    setupOTA();
  }

  void loop()
  {
    ArduinoOTA.handle();
  }
};

#endif