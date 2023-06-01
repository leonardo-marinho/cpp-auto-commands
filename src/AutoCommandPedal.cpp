#ifndef AutoCommandPedal_cpp
#define AutoCommandPedal_cpp

#include "../lib/AutoCommand.cpp"
#include "../lib/PedalBLE.cpp"
#include <EEPROM.h>

#define MODE_CHANGE_DELAY 200
#define MODE_PARK_DELAY 2500
#define EEPROm_targetMode_MEM_ADDR 0
#define MAX_FEEDBACK_FAILS 150

#define NORMAL_MODE PedalBLEModesEnum::M_0
#define FAST_MODE PedalBLEModesEnum::M_100

class AutoCommandPedal : public AutoCommand
{
private:
  boolean m_seeking = false;
  PedalBLE *m_pedalBLE;
  Button *m_button;
  ezOutput *m_led;
  ezOutput *m_output;
  PedalBLEModesEnum m_targetMode = NORMAL_MODE;
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
    else if (m_targetMode == PARK)
    {
      m_led->blink(1000, 2000);
    }
    else if (m_targetMode == LOCK)
    {
      m_led->low();
    }
    else if (m_targetMode == FAST_MODE)
    {
      m_led->high();
    }
    else if (m_targetMode == NORMAL_MODE)
    {
      m_led->low();
    }
  }

  void tickParkMode()
  {
    m_output->blink(MODE_PARK_DELAY, MODE_PARK_DELAY, 0, 2);
  }

  void toggleMode()
  {
    m_targetMode = m_currMode == NORMAL_MODE ? FAST_MODE : NORMAL_MODE;
  }

  void tickMode()
  {
    m_output->blink(MODE_CHANGE_DELAY, MODE_CHANGE_DELAY, 0, 2);
  }

  void setMode(PedalBLEModesEnum t_mode)
  {
    m_targetMode = t_mode;
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
    Serial.println("[PEDAL] Mem Mode: " + String(m_memMode) + " Curr Mode: " + String(m_currMode) + " | Target Mode: " + String(m_targetMode) + " | Failed feedbacks: " + String(m_feedbackWaitFails));

    updateLeds();

    if (!m_pedalBLE->isConnected())
    {
      return;
    }

    if (m_feedbackWaitFails > MAX_FEEDBACK_FAILS)
    {
      m_output->high();
      return;
    }

    if (m_targetMode != m_currMode)
    {
      if (m_currMode != m_memMode && !m_feedbackWait)
      {
        if (m_targetMode == PARK || m_targetMode == LOCK)
        {
          tickParkMode();
        }
        else
        {
          tickMode();
        }
        m_memMode = m_currMode;
        m_feedbackWait = true;
        m_feedbackWaitFails = 0;
      }
      else if (m_currMode == m_memMode && m_feedbackWait)
      {
        m_feedbackWait = false;
      }
      else
      {
        m_feedbackWaitFails++;
      }

      return;
    }

    if (m_button->isClicked())
    {
      toggleMode();
    }
    else if (m_button->isLongClicked())
    {
      setMode(PARK);
    }
    else if (m_button->isMultiClicked())
    {
      setMode(LOCK);
    }
  }
};

#endif