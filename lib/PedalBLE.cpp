#ifndef PEDAL_BLE_CPP
#define PEDAL_BLE_CPP

#include "BLEDevice.h"

#define PEDAL_BLE_MAX_NORMAL_MODES 6

static uint8_t heartbeatData[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};
static size_t heartbeatDataSize = sizeof(heartbeatData) / sizeof(uint8_t);

static uint8_t nextModeData[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE};
static size_t nextModeDataSize = sizeof(nextModeData) / sizeof(uint8_t);

static uint8_t handshakeData[] = {0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF4};
static size_t handshakeDataSize = sizeof(handshakeData) / sizeof(uint8_t);

enum PedalBLEStatusEnum
{
  UNKNOWN,
  SETTING_UP,
  SCANNING,
  RETRING_SCAN,
  CONNECTING,
  RETRING_CONNECT,
  HANDSHAKE,
  HEARTBEATING
};

enum PedalBLEModesEnum
{
  LOCK = 7,
  PARK = 6,
  M_100 = 5,
  M_75 = 4,
  M_50 = 3,
  M_25 = 2,
  M_0 = 1,
  M_ECO = 0,
  M_NONE = -1,
};

static int __mode = PedalBLEModesEnum::M_NONE;

class PedalBLE
{
private:
  BLEAdvertisedDevice m_device;
  BLEClient *m_client;
  BLERemoteCharacteristic *m_characteristicWrite;
  BLERemoteCharacteristic *m_characteristicNotify;
  BLERemoteService *m_remoteService;
  BLEScan *m_scan;
  BLEUUID m_characteristicWriteUUID;
  BLEUUID m_characteristicNotifyUUID;
  BLEUUID m_serviceUUID;
  bool m_isConnected = false;
  bool m_isDeviceAvailable = false;
  bool m_hadHandShake = false;
  char *m_deviceName;
  PedalBLEStatusEnum m_state = PedalBLEStatusEnum::UNKNOWN;
  int m_connectRetries = 0;
  int m_heartbeatCount = 0;
  int m_scanRetries = 0;

  static void notifyCallback(
      BLERemoteCharacteristic *pBLERemoteCharacteristic,
      uint8_t *pData,
      size_t length,
      bool isNotify)
  {
    __mode = (int)pData[1];
  }

  void connectToServer()
  {
    m_client->connect(&(m_device));
    m_client->setMTU(247);

    m_remoteService = m_client->getService(m_serviceUUID);
    if (m_remoteService == nullptr)
    {
      m_client->disconnect();
      return;
    }

    m_characteristicWrite = m_remoteService->getCharacteristic(m_characteristicWriteUUID);
    if (m_characteristicWrite == nullptr)
    {
      m_client->disconnect();
      return;
    }
    if (m_characteristicWrite->canWrite())
      m_characteristicWrite->writeValue(handshakeData, handshakeDataSize, false);

    m_characteristicNotify = m_remoteService->getCharacteristic(m_characteristicNotifyUUID);
    if (m_characteristicNotify == nullptr)
    {
      m_client->disconnect();
      return;
    }
    if (m_characteristicNotify->canNotify())
      m_characteristicNotify->registerForNotify(PedalBLE::notifyCallback);

    m_isConnected = true;
    m_connectRetries++;
  }

  void scanDevices()
  {
    m_scan->start(5, false);
    onScanEnd(m_scan->getResults());
    m_scanRetries++;
  }

  void onScanEnd(BLEScanResults t_results)
  {
    const int resultsCount = t_results.getCount();
    for (int idx = 0; idx < resultsCount; idx++)
    {
      BLEAdvertisedDevice advertisedDevice = t_results.getDevice(idx);
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(m_serviceUUID) && advertisedDevice.getName() == m_deviceName)
      {
        m_device = advertisedDevice;
        m_isDeviceAvailable = true;
        m_scan->stop();
        break;
      }
    }
  }

  void handShake()
  {
    m_characteristicWrite->writeValue(handshakeData, handshakeDataSize, false);
  }

public:
  PedalBLE(char *t_deviceName, BLEUUID t_serviceUUID, BLEUUID t_charUUID, BLEUUID t_char2UUID)
      : m_characteristicWriteUUID(t_charUUID),
        m_characteristicNotifyUUID(t_char2UUID),
        m_client(BLEDevice::createClient()),
        m_deviceName(t_deviceName),
        m_scan(BLEDevice::getScan()),
        m_serviceUUID(t_serviceUUID) {}

  void setup()
  {
    BLEDevice::init("");
    BLEDevice::setMTU(247);
    m_scan->setInterval(1349);
    m_scan->setWindow(449);
    m_scan->setActiveScan(true);
  }

  void loop()
  {
    bool isDeviceAvailable = m_isDeviceAvailable;
    bool isConnected = m_isConnected;
    if (!isDeviceAvailable && !isConnected)
    {
      scanDevices();

      int retries = m_scanRetries;
      if (retries == 1)
      {
        m_state = PedalBLEStatusEnum::SCANNING;
        return;
      }
      m_state = PedalBLEStatusEnum::RETRING_SCAN;
      return;
    }

    if (isDeviceAvailable && !isConnected)
    {
      connectToServer();

      int retries = m_connectRetries;
      if (retries == 1)
      {
        m_state = PedalBLEStatusEnum::CONNECTING;
        return;
      }

      m_state = PedalBLEStatusEnum::RETRING_CONNECT;
      return;
    }

    if (isConnected)
    {
      bool hadHandShake = m_hadHandShake;
      int heartbeatCount = m_heartbeatCount++;

      if (!hadHandShake && heartbeatCount == 3)
      {
        handShake();
        m_state = PedalBLEStatusEnum::HANDSHAKE;
        return;
      }

      m_characteristicWrite->writeValue(heartbeatData, heartbeatDataSize, false);
      m_state = PedalBLEStatusEnum::HEARTBEATING;
      return;
    }

    m_state = PedalBLEStatusEnum::UNKNOWN;
  }

  PedalBLEStatusEnum getState()
  {
    return m_state;
  }

  char *getStateName(PedalBLEStatusEnum t_state)
  {
    if (t_state == PedalBLEStatusEnum::UNKNOWN)
      return "UNKNOWN";
    else if (t_state == PedalBLEStatusEnum::SCANNING)
      return "SCANNING";
    else if (t_state == PedalBLEStatusEnum::RETRING_SCAN)
      return "RETRING_SCAN";
    else if (t_state == PedalBLEStatusEnum::CONNECTING)
      return "CONNECTING";
    else if (t_state == PedalBLEStatusEnum::RETRING_CONNECT)
      return "RETRING_CONNECT";
    else if (t_state == PedalBLEStatusEnum::HANDSHAKE)
      return "HANDSHAKE";
    else if (t_state == PedalBLEStatusEnum::HEARTBEATING)
      return "HEARTBEATING";
    else
      return "UNKNOWN";
  }

  void nextMode()
  {
    m_characteristicWrite->writeValue(nextModeData, nextModeDataSize, false);
  }

  bool isConnected()
  {
    return getState() == PedalBLEStatusEnum::HEARTBEATING;
  }

  PedalBLEModesEnum getMode()
  {
    return static_cast<PedalBLEModesEnum>(__mode);
  }

  char *getModeName(PedalBLEModesEnum t_mode)
  {
    if (t_mode == PedalBLEModesEnum::M_100)
      return "100%";
    else if (t_mode == PedalBLEModesEnum::M_75)
      return "75%";
    else if (t_mode == PedalBLEModesEnum::M_50)
      return "50%";
    else if (t_mode == PedalBLEModesEnum::M_25)
      return "25%";
    else if (t_mode == PedalBLEModesEnum::M_0)
      return "0%";
    else if (t_mode == PedalBLEModesEnum::M_ECO)
      return "ECO";
    else if (t_mode == PedalBLEModesEnum::PARK)
      return "PARK";
    else if (t_mode == PedalBLEModesEnum::LOCK)
      return "LOCK";
    else
      return "UNKNOWN";
  }
};

#endif