#ifndef AutoCommandPedal_cpp
#define AutoCommandPedal_cpp

#include "../lib/AutoCommand.cpp"
#include <EEPROM.h>

#define MODE_CHANGE_DELAY 250
#define MODE_RESET_DELAY 2500
#define EEPROM_MODE_MEM_ADDR 0

class AutoCommandPedal : public AutoCommand
{
private:
  Button *m_button;
  ezOutput *m_led;
  ezOutput *m_output;

  bool m_isActiveMode = false;

  void hardUnlock()
  {
    setValetMode();
    setValetMode();
    tickMode();
  }

  void readMemory()
  {
    EEPROM.begin(0);
    m_isActiveMode = EEPROM.read(EEPROM_MODE_MEM_ADDR) == 20;
  }

  void resetMode()
  {
    m_isActiveMode = !m_isActiveMode;
    m_led->blink(200, 200, 0, 2);
    updateModeLed();
  }

  void setValetMode()
  {
    m_led->blink(200, 200, 0, 10);
    m_output->pulse(MODE_RESET_DELAY);
  }

  void tickMode()
  {
    m_output->pulse(MODE_CHANGE_DELAY);
  }

  void toggleMode()
  {
    m_isActiveMode = !m_isActiveMode;
    updateModeLed();
    m_output->blink(MODE_CHANGE_DELAY, MODE_CHANGE_DELAY, 0, 6);
  }

  void updateModeLed(unsigned long t_delay = 0)
  {
    if (m_isActiveMode)
    {
      m_led->high();
      EEPROM.write(EEPROM_MODE_MEM_ADDR, 20);
    }
    else
    {
      m_led->low();
      EEPROM.write(EEPROM_MODE_MEM_ADDR, 250);
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
    m_output->high();
    hardUnlock();
    readMemory();
    updateModeLed();
  }

  void loop()
  {
    m_button->loop();
    m_led->loop();
    m_output->loop();

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
};

#endif