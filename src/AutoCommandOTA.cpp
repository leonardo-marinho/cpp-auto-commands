#ifndef AutoCommandOTA_cpp
#define AutoCommandOTA_cpp

#include "../lib/AutoCommand.cpp"
#include "../lib/OTA.cpp"
#include <EEPROM.h>

#define MODE_CHANGE_DELAY 250
#define MODE_RESET_DELAY 2500
#define EEPROM_MODE_MEM_ADDR 0

class AutoCommandOTA : public AutoCommand
{
private:
  Button *m_button;
  ezOutput *m_led1;
  ezOutput *m_led2;
  OTA *m_ota;

  bool m_devMode = false;

  void updateLeds()
  {
    if (!m_devMode)
      return;

    if (m_ota->isConnected())
    {
      m_led1->blink(1000, 1000);
      m_led2->blink(500, 500);
    }
    else
    {
      m_led1->blink(250, 250);
      m_led2->blink(350, 350);
    }
  }

public:
  AutoCommandOTA(uint t_buttonPin, uint t_led1Pin, uint t_led2Pin, char *t_otaHostname, char *t_otaPassword, char *t_wifiSSID, char *t_wifiPassword)
  {
    m_button = new Button(t_buttonPin);
    m_led1 = new ezOutput(t_led1Pin);
    m_led2 = new ezOutput(t_led2Pin);
    m_ota = new OTA(t_otaHostname, t_otaPassword, t_wifiSSID, t_wifiPassword);
  }

  void setup()
  {
  }

  bool isActive()
  {
    return m_devMode && m_ota->isOTAReady();
  }

  void loop()
  {
    m_button->loop();
    m_led1->loop();
    m_led2->loop();

    updateLeds();

    if (m_devMode)
    {

      if (m_ota->isOTAReady())
      {
        m_ota->loop();
      }
      else if (!m_ota->isConnected())
      {
        m_ota->connect();
      }
      else if (!m_ota->isOTAReady())
      {
        m_ota->setupOTA();
      }
    }
    else
    {
      if (m_ota->isConnected())
      {
        m_ota->disconnect();
      }
    }

    if (m_button->isLongClicked())
    {
      m_devMode = !m_devMode;
    }
  }
};

#endif