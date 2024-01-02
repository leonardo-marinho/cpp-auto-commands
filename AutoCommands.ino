#include "./config/Config.cpp"
#include "./src/AutoCommandPedal.cpp"
#include "./src/AutoCommandGate.cpp"
#include "./lib/Mcp.cpp"

#define OTA_ENABLE false
#define LOOP_DELAY 50

#define SS_PIN 5

#define DEVICE_NAME "PdBooster"
static BLEUUID serviceUUID("0000E0FF-3C17-D293-8E48-14FE2E4DA212");
static BLEUUID characteristicWriteUUID("FFE1");
static BLEUUID characteristicNotifyUUID("FFE2");


bool clutchSportMode = false;

MCP mcp(SS_PIN);
AutoCommandPedal commandPedal(config.autoCommandPedal.buttonPin, config.autoCommandPedal.buttonEmergencyPin, config.autoCommandPedal.ledPin, config.autoCommandPedal.outputPin, DEVICE_NAME, serviceUUID, characteristicWriteUUID, characteristicNotifyUUID);
AutoCommandGate commandGate(config.autoCommandGate.buttonPin, config.autoCommandGate.ledPin, config.autoCommandGate.outputPin1, config.autoCommandGate.outputPin2);

int cycle = 0;
int clutchCount = 0;

void onClutchMsgReceive(MCP &mcp, can_frame frame)
{
  mcp.debugCanMsg(frame);

  if (frame.data[5] == 0x24)
  clutchSportMode = true;
  else if(frame.data[5] == 0x20)
  clutchSportMode = false;
}

void setup()
{
  Serial.begin(115200);
  mcp
      .begin(CAN_500KBPS, MCP_8MHZ)
      .watch(0x0ad).onReceiveCallback(onClutchMsgReceive)
      .watchAll();

  commandPedal.setup();
  commandGate.setup();
}

void loop()
{
  if (cycle == 100) {
    commandPedal.setClutchSportMode(clutchSportMode);
  commandPedal.loop();
  commandGate.loop();
  cycle = 0;
  delay(LOOP_DELAY);
  } else {
  mcp.loop();
  }

  cycle++;
}