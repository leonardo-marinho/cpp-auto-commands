#include "./config/Config.cpp"
#include "./src/AutoCommandPedal.cpp"
#include "./src/AutoCommandGate.cpp"
#ifdef OTA_ENABLE
#include "./src/AutoCommandOTA.cpp"
#endif

#define LOOP_DELAY 50

ezOutput commandBacklight(config.backlightPin);
ezOutput commandBacklightGnd(config.backlightGnd);
AutoCommandPedal commandPedal(config.autoCommandPedal.buttonPin, config.autoCommandPedal.ledPin, config.autoCommandPedal.outputPin);
AutoCommandGate commandGate(config.autoCommandGate.buttonPin, config.autoCommandGate.ledPin, config.autoCommandGate.outputPin1, config.autoCommandGate.outputPin2);
#ifdef OTA_ENABLE
AutoCommandOTA commandDev(config.autoCommandOTA.buttonPin, config.autoCommandOTA.ledPin1, config.autoCommandOTA.ledPin2, config.autoCommandOTA.otaHostname, config.autoCommandOTA.otaPassword, config.autoCommandOTA.wifiHostname, config.autoCommandOTA.wifiPassword);
#endif

void setup()
{
  commandBacklight.high();
  commandBacklightGnd.low();
  commandPedal.setup();
  commandGate.setup();
  #ifdef OTA_ENABLE
  commandDev.setup();
  #endif
}

void loop()
{
  #ifdef OTA_ENABLE
  commandDev.loop();
  #endif
  commandPedal.loop();
  commandGate.loop();
  delay(LOOP_DELAY);
}