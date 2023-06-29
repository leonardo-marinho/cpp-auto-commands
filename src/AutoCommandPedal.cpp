#ifndef AutoCommandPedal_cpp
#define AutoCommandPedal_cpp

#include "../lib/AutoCommand.cpp"
#include <EEPROM.h>

#define MODE_CHANGE_DELAY 250
#define MODE_RESET_DELAY 2500
#define EEPROM_MODE_MEM_SIZE 12
#define EEPROM_MODE_MEM_ADDR 0

class AutoCommandPedal : public AutoCommand
{
private:
  Button *m_button;
  ezOutput *m_led;
  ezOutput *m_output;

  int m_lastModeBeforeSpecial = 2;

  bool m_isChanging = false;
  bool m_isChangingBlinked = false;
  int m_isChangingCounter = 0;

  bool isActiveMode()
  {
    return EEPROM.read(EEPROM_MODE_MEM_ADDR) == 1;
  }

  bool isParkMode()
  {
    return EEPROM.read(EEPROM_MODE_MEM_ADDR) == 3;
  }

  void resetMode()
  {
    m_isChanging = true;
    m_led->blink(200, 200, 0, 2);
    updateModeLed();
  }

  void toggleMode()
  {
    if (isParkMode())
    {
      tickMode();
      EEPROM.put(EEPROM_MODE_MEM_ADDR, m_lastModeBeforeSpecial);
    }
    else if (isActiveMode())
    {
      m_output->blink(MODE_CHANGE_DELAY, MODE_CHANGE_DELAY, 0, 4);
      EEPROM.put(EEPROM_MODE_MEM_ADDR, 2);
    }
    else
    {
      m_output->blink(MODE_CHANGE_DELAY, MODE_CHANGE_DELAY, 0, 8);
      EEPROM.put(EEPROM_MODE_MEM_ADDR, 1);
    }

    EEPROM.commit();

    m_isChanging = true;
    updateModeLed();
  }

  void setValetMode()
  {
    m_output->pulse(MODE_RESET_DELAY);
    EEPROM.put(EEPROM_MODE_MEM_ADDR, 3);

    EEPROM.commit();

    m_isChanging = true;
    updateModeLed();
  }

  void updateModeLed(unsigned long t_delay = 0)
  {

    if (m_isChanging && !m_isChangingBlinked)
    {
      m_led->blink(250, 250);
      m_isChangingBlinked = true;
    }
    else if (isActiveMode())
    {
      m_led->high();
    }
    else if (isParkMode())
    {
      m_led->blink(1000, 2000);
    }
    else
    {
      m_led->low();
    }
  }

public:
  AutoCommandPedal(uint t_buttonPin, uint t_ledPin, uint t_outputPin)
  {
    m_button = new Button(t_buttonPin);
    m_led = new ezOutput(t_ledPin);
    m_output = new ezOutput(t_outputPin);
  }

  void setup()
  {
    EEPROM.begin(EEPROM_MODE_MEM_SIZE);
    m_output->high();
    updateModeLed();
  }

  void loop()
  {
    m_button->loop();
    m_led->loop();
    m_output->loop();

    if (m_isChanging)
    {
      int counterMax = 20;
      if (isActiveMode() || isParkMode())
        counterMax = 40;

      if (m_isChangingCounter++ > counterMax)
      {
        m_isChanging = false;
        m_isChangingCounter = 0;
        m_isChangingBlinked = false;
      }
      else
      {
        return;
      }
    }

    updateModeLed();

    if (m_button->isClicked())
    {
      toggleMode();
    }
    else if (m_button->isDoubleClicked())
    {
      setValetMode();
    }
    else if (m_button->isLongClicked())
    {
      resetMode();
    }
    else if (m_button->isMultiClicked())
    {
      tickMode();
    }
  }

  void tickMode()
  {
    m_led->blink(200, 200, 0, 2);
    m_output->pulse(MODE_CHANGE_DELAY);
    m_lastModeBeforeSpecial = isActiveMode() ? 1 : 2;
  }
};

#endif