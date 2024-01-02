#ifndef PEDAL_BLE_CPP
#define PEDAL_BLE_CPP

#include "BLEDevice.h"

#define PEDAL_BLE_MAX_NORMAL_MODES 6

enum PedalBLEStatusEnum
{
  UNKNOWN,
  SETTING_UP,
  SCANNING,
  RETRYING_SCAN,
  CONNECTING,
  RETRYING_CONNECT,
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
  BLERemoteCharacteristic *m_characteristicNotify;
  BLERemoteService *m_remoteService;
  BLEScan *m_scan;
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
    bool connected = m_client->connect(&(m_device));

    if (!connected)
    {
      m_client->disconnect();
      return;
    }

    m_remoteService = m_client->getService(m_serviceUUID);
    if (m_remoteService == nullptr)
    {
      m_client->disconnect();
      return;
    }

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

public:
  PedalBLE(char *t_deviceName, BLEUUID t_serviceUUID, BLEUUID t_charUUID, BLEUUID t_char2UUID)
      : m_characteristicNotifyUUID(t_char2UUID),
        m_client(BLEDevice::createClient()),
        m_deviceName(t_deviceName),
        m_scan(BLEDevice::getScan()),
        m_serviceUUID(t_serviceUUID) {}

  void setup()
  {
    BLEDevice::init("");
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
      m_state = PedalBLEStatusEnum::RETRYING_SCAN;
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

      m_state = PedalBLEStatusEnum::RETRYING_CONNECT;
      return;
    }

    if (isConnected)
    {
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
    else if (t_state == PedalBLEStatusEnum::RETRYING_SCAN)
      return "RETRYING_SCAN";
    else if (t_state == PedalBLEStatusEnum::CONNECTING)
      return "CONNECTING";
    else if (t_state == PedalBLEStatusEnum::RETRYING_CONNECT)
      return "RETRYING_CONNECT";
    else if (t_state == PedalBLEStatusEnum::HANDSHAKE)
      return "HANDSHAKE";
    else if (t_state == PedalBLEStatusEnum::HEARTBEATING)
      return "HEARTBEATING";
    else
      return "UNKNOWN";
  }

  bool isConnected()
  {
    return getState() == PedalBLEStatusEnum::HEARTBEATING;
  }

  static PedalBLEModesEnum getMode()
  {
    return static_cast<PedalBLEModesEnum>(__mode);
  }

  static char *getModeName(PedalBLEModesEnum t_mode)
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