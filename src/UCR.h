/*
  UCR ESP8266 lib
*/

#include <WiFiUdp.h>

#define BUTTON_COUNT 64
#define AXIS_COUNT 32
#define DELTA_COUNT 32
#define EVENT_COUNT 32

class UCR
{

public:
  UCR(const char *ssid, const char *password);
  UCR(const char *ssid, const char *password, unsigned int port);
  void begin();
  bool update();
  void resetValues();
  void name(const char *name);

  void addButton(const char *name, int index);
  void addAxis(const char *name, int index);
  void addDelta(const char *name, int index);
  void addEvent(const char *name, int index);

  bool readButton(int index);
  short readAxis(int index);
  short readDelta(int index);
  bool readEvent(int index);

  unsigned long lastUpdateMillis();

private:
  void setupWiFi();
  void setupMDNS();
  bool receiveUdp();

  WiFiUDP Udp;
  char _hostString[32] = {0};
  unsigned int _localPort = 8080;
  char _name[32] = {0};
  const char *_ssid;
  const char *_password;
  unsigned int _port;
  unsigned long _lastUpdateMillis;

  const char *_buttonList[BUTTON_COUNT] = {0};
  const char *_axisList[AXIS_COUNT] = {0};
  const char *_deltaList[DELTA_COUNT] = {0};
  const char *_eventList[EVENT_COUNT] = {0};

  bool buttonData[BUTTON_COUNT] = {0};
  short axisData[AXIS_COUNT] = {0};
  short deltaData[DELTA_COUNT] = {0};
  bool eventData[EVENT_COUNT] = {0};

  char incomingPacketBuffer[255];
};