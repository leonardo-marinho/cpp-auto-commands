#ifndef Button_cpp
#define Button_cpp

#include <Arduino.h>

#define DEFAULT_PULL_TYPE LOW
#define DEFAULT_DEBOUNCE_TIME 500
#define DEFAULT_LONG_CLICK_TIME 1000

enum ButtonState
{
  NONE,
  CLICKED,
  LONG_CLICKED,
  DOUBLE_CLICKED,
  MULTI_CLICKED
};

class Button
{
private:
  uint8_t m_pin;
  bool m_prevState = !DEFAULT_PULL_TYPE;
  bool m_pullType = DEFAULT_PULL_TYPE;
  unsigned long m_lastTime = 0;
  unsigned long m_debounceTime = DEFAULT_DEBOUNCE_TIME;
  unsigned long m_longClickTime = DEFAULT_LONG_CLICK_TIME;
  unsigned int m_clickCount = 0;
  ButtonState m_state = ButtonState::NONE;

  void begin()
  {
    pinMode(m_pin, INPUT_PULLUP);
  }

  unsigned long getElapsedTime(unsigned long t_currTime)
  {
    return t_currTime - m_lastTime;
  }

  void handleClick(unsigned long t_elapsedTime)
  {
    if (t_elapsedTime < m_debounceTime)
    {
      m_clickCount++;
    }

    if (m_clickCount == 2)
    {
      m_state = ButtonState::DOUBLE_CLICKED;
    }
  }

  bool haveClicks()
  {
    return m_clickCount > 0;
  }

  bool haveState()
  {
    return m_state != ButtonState::NONE;
  }

  void incrementClickCount()
  {
    m_clickCount++;
  }

  bool isDebounceCooldown(unsigned long t_elapsedTime)
  {
    return t_elapsedTime < m_debounceTime;
  }

  void resetClickCount()
  {
    m_clickCount = 0;
  }

  void resetState()
  {
    m_state = ButtonState::NONE;
  }

  void resolveEvent(unsigned long t_elapsedTime)
  {
    if (m_clickCount == 1)
    {
      if (t_elapsedTime > m_longClickTime)
      {
        m_state = ButtonState::LONG_CLICKED;
      }
      else
      {
        m_state = ButtonState::CLICKED;
      }
    }
    else if (m_clickCount == 2)
    {
      m_state = ButtonState::DOUBLE_CLICKED;
    }
    else if (m_clickCount > 2)
    {
      m_state = ButtonState::MULTI_CLICKED;
    }
  }

public:
  Button(uint8_t t_pin)
      : m_pin(t_pin)
  {
    this->begin();
  }

  void loop()
  {
    uint8_t currState = digitalRead(m_pin);
    unsigned long currTime = millis();
    unsigned long elapsedTime = getElapsedTime(currTime);

    if (haveState())
    {
      resetState();
      resetClickCount();
    }

    if (isTriggered())
    {
      if (!haveClicks() || (haveClicks() && isDebounceCooldown(elapsedTime)))
      {
        incrementClickCount();
      }
    }
    else if (!isPressed() && !isTriggered() && haveClicks() && !isDebounceCooldown(elapsedTime))
    {
      resolveEvent(elapsedTime);
    }

    if (isTriggered())
    {
      m_lastTime = currTime;
    }

    m_prevState = currState;
  }

  int getClickCount()
  {
    return m_clickCount;
  }

  bool getStateRaw()
  {
    return digitalRead(m_pin);
  }

  bool isClicked()
  {
    return m_state == ButtonState::CLICKED;
  }

  bool isDoubleClicked()
  {
    return m_state == ButtonState::DOUBLE_CLICKED;
  }

  bool isLongClicked()
  {
    return m_state == ButtonState::LONG_CLICKED;
  }

  bool isMultiClicked()
  {
    return m_state == ButtonState::MULTI_CLICKED;
  }

  bool isPressed()
  {
    return getStateRaw() == m_pullType;
  }

  bool isReleased()
  {
    return getStateRaw() != m_pullType && m_prevState == m_pullType;
  }

  bool isTriggered()
  {
    return getStateRaw() == m_pullType && m_prevState != m_pullType;
  }
};

#endif