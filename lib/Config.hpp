#ifndef Config_hpp
#define Config_hpp

struct OTAConfig
{
  char *otaHostname;
  char *otaPassword;
  char *wifiHostname;
  char *wifiPassword;
};

struct AutoCommandPedalConfig
{
  int buttonPin;
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
  OTAConfig otaConfig;
  AutoCommandPedalConfig autoCommandPedal;
  AutoCommandGateConfig autoCommandGate;
  int backlightPin;
  int backlightGnd;
};

#endif