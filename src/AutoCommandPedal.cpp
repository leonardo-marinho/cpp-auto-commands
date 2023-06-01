#ifndef AutoCommandPedal_cpp
#define AutoCommandPedal_cpp

#include "../lib/AutoCommand.cpp"
#include "../lib/PedalBLE.cpp"
#include <EEPROM.h>

#define MODE_CHANGE_DELAY 300
#define MODE_PARK_DELAY 2500
#define EEPROm_targetMode_MEM_ADDR 0
#define MAX_FEEDBACK_FAILS 10

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
  int m_feedbackWait = 0;
  int m_feedbackWaitFails = 0;

  void updateLeds()
  {
    if (m_feedbackWaitFails >= MAX_FEEDBACK_FAILS)
    {
      m_led->blink(100, 100);
    }
    else if (m_currMode != m_targetMode)
    {
      m_led->blink(250, 250);
    }
    else if (m_targetMode == M_100)
    {
      m_led->high();
    }
    else if (m_targetMode == M_0)
    {
      m_led->low();
    }
  }

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

    m_currMode = PedalBLE::getMode();
    Serial.println("[PEDAL] Mem Mode: " + String(m_memMode) + " Curr Mode: " + String(m_currMode) + " Target Mode: " + String(m_targetMode));

    updateLeds();

    if (!m_pedalBLE->isConnected())
    {
      return;
    }

    if (m_feedbackWaitFails > MAX_FEEDBACK_FAILS)
    {
      return;
    }

    if (m_targetMode != m_currMode)
    {
      if (m_currMode != m_memMode && !m_feedbackWait)
      {
        tickMode();
        m_memMode = m_currMode;
        m_feedbackWait = true;
        m_feedbackWaitFails = 0;
      }
      else if (m_currMode == m_memMode && m_feedbackWait)
      {
        m_feedbackWait = false;
      }
      else if (m_currMode != m_memMode && m_feedbackWait)
      {
        m_feedbackWaitFails++;
      }

      return;
    }

    if (m_button->isClicked())
    {
      toggleMode();
    }
  }

  void toggleMode()
  {
    m_targetMode = m_currMode == M_0 ? M_100 : M_0;
  }

  void tickMode()
  {
    m_output->blink(MODE_CHANGE_DELAY, MODE_CHANGE_DELAY, 0, 2);
  }

  void setMode(int t_mode)
  {
    Serial.println("Setting mode" + String(t_mode));
    m_targetMode = (PedalBLEModesEnum)t_mode;
  }
};

#endif