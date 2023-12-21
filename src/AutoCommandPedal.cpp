#ifndef AutoCommandPedal_cpp
#define AutoCommandPedal_cpp

#include "../lib/AutoCommand.cpp"
#include "../lib/PedalBLE.cpp"
#include <EEPROM.h>

#define MODE_CHANGE_DELAY 200
#define MODE_PARK_DELAY 2500
#define MAX_FEEDBACK_FAILS 80

#define NORMAL_MODE PedalBLEModesEnum::M_0
#define PARK_MODE PedalBLEModesEnum::PARK
#define FAST_MODE PedalBLEModesEnum::M_100
#define LOCK_MODE PedalBLEModesEnum::LOCK

class AutoCommandPedal : public AutoCommand
{
private:
  boolean m_firstRun = true;
  boolean m_seeking = false;
  bool m_isSportMode = false;
  PedalBLE *m_pedalBLE;
  Button *m_button;
  Button *m_buttonEmergency;
  ezOutput *m_led;
  ezOutput *m_output;
  PedalBLEModesEnum m_targetMode = NORMAL_MODE;
  PedalBLEModesEnum m_currMode = PedalBLEModesEnum::M_NONE;
  PedalBLEModesEnum m_memMode = PedalBLEModesEnum::M_NONE;
  PedalBLEModesEnum m_clutchReturnTo = PedalBLEModesEnum::M_NONE;
  int m_feedbackWait = 0;
  int m_feedbackWaitFails = 0;

  void acknowledgeError()
  {
    m_feedbackWaitFails = 0;
    m_feedbackWait = false;
    m_memMode = PedalBLEModesEnum::M_NONE;
  }

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
  AutoCommandPedal(uint t_buttonPin, uint t_buttonEmergencyPin, uint t_ledPin, uint t_outputPin, char *t_deviceName, BLEUUID t_serviceUUID, BLEUUID t_charUUID, BLEUUID t_char2UUID)
  {
    m_button = new Button(t_buttonPin);
    m_buttonEmergency = new Button(t_buttonEmergencyPin);
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
    m_buttonEmergency->loop();
    m_led->loop();
    m_output->loop();
    m_pedalBLE->loop();

    m_currMode = PedalBLE::getMode();
    Serial.println("[PEDAL] Mem Mode: " + String(m_memMode) + " Curr Mode: " + String(m_currMode) + " | Target Mode: " + String(m_targetMode) + " | Sport mode: " + String(m_isSportMode) + " | Failed feedbacks: " + String(m_feedbackWaitFails) + " | Waiting for feedback: " + String(m_feedbackWait));

    updateLeds();

    if (!m_pedalBLE->isConnected() || m_currMode == PedalBLEModesEnum::M_NONE)
    {
      return;
    }

    if (m_firstRun)
    {
      if (m_currMode == PARK_MODE || m_currMode == LOCK_MODE || m_currMode == NORMAL_MODE)
      {
        m_targetMode = m_currMode;
      }
      else
      {
        m_targetMode = NORMAL_MODE;
      }
      m_firstRun = false;
    }

    if (m_feedbackWaitFails >= MAX_FEEDBACK_FAILS)
    {
      if (m_button->isClicked())
      {
        acknowledgeError();
        setMode(NORMAL_MODE);
      }
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

    if (!m_isSportMode)
    {
      if (m_clutchReturnTo != PedalBLEModesEnum::M_NONE)
      {
        setMode(m_clutchReturnTo);
        m_clutchReturnTo = PedalBLEModesEnum::M_NONE;
      }
    }
    else if (m_isSportMode)
    {
      if (m_button->isMultiClicked() || m_buttonEmergency->isClicked())
      {
        setMode(LOCK);
      }
      if (m_clutchReturnTo != PedalBLEModesEnum::M_NONE)
        return;
    }

    if (m_currMode == LOCK)
    {
      if (m_button->isMultiClicked() || m_buttonEmergency->isClicked())
        setMode(NORMAL_MODE);
    }
    else if (m_currMode == PARK_MODE)
    {
      if (m_isSportMode)
      {
        setMode(NORMAL_MODE);
        m_clutchReturnTo = PARK_MODE;
        return;
      }

      if (m_button->isDoubleClicked())
        toggleMode();

      if (m_buttonEmergency->isClicked())
        setMode(LOCK_MODE);
    }
    else
    {
      if (m_isSportMode && m_currMode == NORMAL_MODE)
      {
        setMode(FAST_MODE);
        m_clutchReturnTo = NORMAL_MODE;
        return;
      }
      else if (m_button->isClicked())
      {
        toggleMode();
      }
      else if (m_button->isDoubleClicked())
      {
        setMode(PARK);
      }
      else if (m_button->isMultiClicked() || m_buttonEmergency->isClicked())
      {
        setMode(LOCK);
      }
    }
  }

  void
  setClutchSportMode(bool t_isSportMode)
  {
    m_isSportMode = t_isSportMode;
  }
};

#endif