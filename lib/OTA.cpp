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

  bool m_otaReady = false;

public:
  OTA(char *t_otaHostname, char *t_otaPassword, char *t_wifiSSID, char *t_wifiPassword)
      : m_wifiSSID(t_wifiSSID),
        m_wifiPassword(t_wifiPassword)
  {
    ArduinoOTA.setHostname(t_otaHostname);
    ArduinoOTA.setPassword(t_otaPassword);
  }

  void connect()
  {
    if (isConnected())
      return;
    WiFi.mode(WIFI_STA);
    WiFi.begin(m_wifiSSID, m_wifiPassword);
    delay(2000);
  }

  bool isConnected()
  {
    return WiFi.status() == WL_CONNECTED;
  }

  void setupOTA()
  {
    if (isConnected() && !isOTAReady())
    {
      ArduinoOTA.begin();
      m_otaReady = true;
    }
  }

  bool isOTAReady()
  {
    return m_otaReady;
  }

  void disconnect()
  {
    if (isConnected())
    {
      ArduinoOTA.end();
      WiFi.disconnect();
      m_otaReady = false;
    }
  }

  void loop()
  {
    ArduinoOTA.handle();
  }
};

#endif