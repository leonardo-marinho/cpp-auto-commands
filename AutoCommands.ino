#include "./config/Config.cpp"
#include "./src/AutoCommandPedal.cpp"
#include "./src/AutoCommandGate.cpp"

#define OTA_ENABLE true
#define LOOP_DELAY 50

#define DEVICE_NAME "PdBooster"
static BLEUUID serviceUUID("0000E0FF-3C17-D293-8E48-14FE2E4DA212");
static BLEUUID characteristicWriteUUID("FFE1");
static BLEUUID characteristicNotifyUUID("FFE2");

ezOutput commandBacklight(config.backlightPin);
ezOutput commandBacklightGnd(config.backlightGnd);
AutoCommandPedal commandPedal(config.autoCommandPedal.buttonPin, config.autoCommandPedal.ledPin, config.autoCommandPedal.outputPin, DEVICE_NAME, serviceUUID, characteristicWriteUUID, characteristicNotifyUUID);
AutoCommandGate commandGate(config.autoCommandGate.buttonPin, config.autoCommandGate.ledPin, config.autoCommandGate.outputPin1, config.autoCommandGate.outputPin2);

void setup()
{
  Serial.begin(115200);
  commandBacklight.high();
  commandBacklightGnd.low();
  commandPedal.setup();
  commandGate.setup();
}

void loop()
{
  commandPedal.loop();
  commandGate.loop();
  delay(LOOP_DELAY);
}