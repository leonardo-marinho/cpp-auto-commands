#ifndef AutoCommandGate_cpp
#define AutoCommandGate_cpp

#include "../lib/AutoCommand.cpp"

#define MODE_CHANGE_DELAY 300

class AutoCommandGate : public AutoCommand
{
private:
  Button *m_button;
  ezOutput *m_led;
  ezOutput *m_output1;
  ezOutput *m_output2;

  void toggleGate1()
  {
    m_led->blink(0, 300, 0, 2);
    m_output1->pulse(MODE_CHANGE_DELAY);
  }

  void toggleGate2()
  {
    m_led->blink(100, 100, 0, 4);
    m_output2->pulse(MODE_CHANGE_DELAY);
  }

public:
  AutoCommandGate(uint t_buttonPin, uint t_ledPin, uint t_output1Pin, uint t_output2Pin)
  {
    m_button = new Button(t_buttonPin);
    m_led = new ezOutput(t_ledPin);
    m_output1 = new ezOutput(t_output1Pin);
    m_output2 = new ezOutput(t_output2Pin);
  }

  void setup()
  {
    m_led->low();
    m_output1->low();
    m_output2->low();
  }

  void loop()
  {
    m_button->loop();
    m_led->loop();
    m_output1->loop();
    m_output2->loop();

    if (m_button->isClicked())
    {
      toggleGate1();
    }
    else if (m_button->isDoubleClicked())
    {
      toggleGate2();
    }
  }
};

#endif