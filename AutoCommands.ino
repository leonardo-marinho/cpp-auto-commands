#include "./config/Config.cpp"
#include "./lib/OTA.cpp"
#include "./src/AutoCommandPedal.cpp"
#include "./src/AutoCommandGate.cpp"

#define OTA_ENABLE true
#define LOOP_DELAY 50

OTA ota(config.otaConfig.otaHostname, config.otaConfig.otaPassword, config.otaConfig.wifiHostname, config.otaConfig.wifiPassword);
ezOutput commandBacklight(config.backlightPin);
AutoCommandPedal commandPedal(config.autoCommandPedal.buttonPin, config.autoCommandPedal.ledPin, config.autoCommandPedal.outputPin);
AutoCommandGate commandGate(config.autoCommandGate.buttonPin, config.autoCommandGate.ledPin, config.autoCommandGate.outputPin1, config.autoCommandGate.outputPin2);

void setup()
{
#ifdef OTA_ENABLE
  ota.setup();
#endif
  commandBacklight.high();
  commandPedal.setup();
  commandGate.setup();
}

void loop()
{
  ota.loop();
  commandPedal.loop();
  commandGate.loop();
  delay(LOOP_DELAY);
}