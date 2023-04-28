#ifndef AutoCommandPedal_cpp
#define AutoCommandPedal_cpp

#include "../lib/AutoCommand.cpp"
#include "../lib/PedalBLE.cpp"
#include <EEPROM.h>

#define MODE_CHANGE_DELAY 200
#define MODE_PARK_DELAY 2500
#define EEPROm_targetMode_MEM_ADDR 0

class AutoCommandPedal : public AutoCommand
{
private:
  boolean m_seeking = false;
  PedalBLE *m_pedalBLE;
  Button *m_button;
  ezOutput *m_led;
  ezOutput *m_output;
  PedalBLEModesEnum m_targetMode = PedalBLEModesEnum::M_0;
  PedalBLEModesEnum m_currMode = PedalBLEModesEnum::M_NONE;
  PedalBLEModesEnum m_memMode = PedalBLEModesEnum::M_NONE;

public:
  AutoCommandPedal(uint t_buttonPin, uint t_ledPin, uint t_outputPin, char *t_deviceName, BLEUUID t_serviceUUID, BLEUUID t_charUUID, BLEUUID t_char2UUID)
  {
    m_button = new Button(t_buttonPin);
    m_led = new ezOutput(t_ledPin);
    m_output = new ezOutput(t_outputPin);
    m_pedalBLE = new PedalBLE(t_deviceName, t_serviceUUID, t_charUUID, t_char2UUID);
  }

  void setup()
  {
    m_led->high();
    m_output->high();
    m_pedalBLE->setup();
  }

  void loop()
  {
    m_button->loop();
    m_led->loop();
    m_output->loop();
    m_pedalBLE->loop();

    m_currMode = m_pedalBLE->getMode();
    Serial.println("[PEDAL] Conn: " + String(m_pedalBLE->getStateName(m_pedalBLE->getState())));
    Serial.println("[PEDAL] Curr Mode: " + String(m_pedalBLE->getModeName(m_currMode)) + " Target Mode: " + String(m_pedalBLE->getModeName(m_targetMode)));

    // Handle connection
    if (!m_pedalBLE->isConnected())
    {
      Serial.println("[PEDAL] Connecting to BLE");
      return;
    }

    // Waiting for feedback
    if (m_memMode != PedalBLEModesEnum::M_NONE)
    {
      if (m_currMode != m_memMode)
      {
        Serial.println("[PEDAL] Received feedback");
        m_memMode = PedalBLEModesEnum::M_NONE;
      }
      else
      {
        Serial.println("[PEDAL] Waiting for feedback");
        return;
      }
    }

    // Handle mode change
    if (m_currMode != m_targetMode)
    {
      if (m_targetMode != PedalBLEModesEnum::M_NONE)
      {
        m_memMode = m_currMode;
        if ((m_currMode == PedalBLEModesEnum::PARK && m_targetMode != PedalBLEModesEnum::M_0) || m_targetMode == PedalBLEModesEnum::PARK || m_targetMode == PedalBLEModesEnum::LOCK)
        {
          Serial.println("[PEDAL] Sending long pulse");
          m_output->pulse(MODE_PARK_DELAY);
        }
        else
        {
          Serial.println("[PEDAL] Sending pulse");
          m_output->pulse(MODE_CHANGE_DELAY);
        }
      }
    }

    // Led state
    if (m_currMode == m_targetMode)
    {
      if (m_currMode == PedalBLEModesEnum::M_0)
      {
        Serial.println("[PEDAL] Setting led to M_0 mode");
        m_led->low();
      }
      else if (m_currMode == PedalBLEModesEnum::M_100)
      {
        Serial.println("[PEDAL] Setting led to M_100 mode");
        m_led->high();
      }
      else if (m_currMode == PedalBLEModesEnum::PARK)
      {
        Serial.println("[PEDAL] Setting led to PARK mode");
        m_led->blink(500, 2000);
      }
      else if (m_currMode == PedalBLEModesEnum::LOCK)
      {
        Serial.println("[PEDAL] Setting led to LOCK mode");
        m_led->low();
      }
    }

    // Handle click
    if (m_button->isClicked())
    {
      Serial.println("[PEDAL] Received single click");
      if (m_targetMode == PedalBLEModesEnum::M_0)
      {
        m_led->high();
        m_targetMode = PedalBLEModesEnum::M_100;
      }
      else
      {
        m_led->low();
        m_targetMode = PedalBLEModesEnum::M_0;
      }
    }
    else if (m_button->isDoubleClicked())
    {
      Serial.println("[PEDAL] Received double click");
      m_led->blink(100, 500, 0, 2);
      m_targetMode = PedalBLEModesEnum::PARK;
    }
    else if (m_button->isLongClicked())
    {
      Serial.println("[PEDAL] Received long click");
      m_led->blink(100, 250, 0, 4);
      m_targetMode = PedalBLEModesEnum::LOCK;
    }
  }
};

#endif