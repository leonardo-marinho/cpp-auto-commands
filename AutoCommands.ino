#define OTA_ENABLE false

#include "./config/Config.cpp"
#ifdef OTA_ENABLE
#include "./lib/OTA.cpp"
#endif
#include "./src/AutoCommandPedal.cpp"
#include "./src/AutoCommandGate.cpp"

#define LOOP_DELAY 50

#ifdef OTA_ENABLE
OTA ota(config.otaConfig.otaHostname, config.otaConfig.otaPassword, config.otaConfig.wifiHostname, config.otaConfig.wifiPassword);
#endif
ezOutput commandBacklight(config.backlightPin);
ezOutput commandBacklightGnd(config.backlightGnd);
AutoCommandPedal commandPedal(config.autoCommandPedal.buttonPin, config.autoCommandPedal.ledPin, config.autoCommandPedal.outputPin);
AutoCommandGate commandGate(config.autoCommandGate.buttonPin, config.autoCommandGate.ledPin, config.autoCommandGate.outputPin1, config.autoCommandGate.outputPin2);

void setup()
{
#ifdef OTA_ENABLE
  ota.setup();
#endif
  commandBacklight.high();
  commandBacklightGnd.low();
  commandPedal.setup();
  commandGate.setup();
}

void loop()
{
#ifdef OTA_ENABLE
  ota.loop();
#endif
  commandPedal.loop();
  commandGate.loop();
  delay(LOOP_DELAY);
}