#ifndef Config_hpp
#define Config_hpp

struct AutoCommandOTAConfig
{
  char *otaHostname;
  char *otaPassword;
  char *wifiHostname;
  char *wifiPassword;
  int buttonPin;
  int ledPin1;
  int ledPin2;
};

struct AutoCommandPedalConfig
{
  int buttonPin;
  int buttonEmergencyPin;
  int ledPin;
  int outputPin;
};

struct AutoCommandGateConfig
{
  int buttonPin;
  int ledPin;
  int outputPin1;
  int outputPin2;
};

struct Config
{
  AutoCommandOTAConfig autoCommandOTA;
  AutoCommandPedalConfig autoCommandPedal;
  AutoCommandGateConfig autoCommandGate;
  int backlightPin;
  int backlightGnd;
};

#endif