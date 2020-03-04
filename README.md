# UCR ESP8266 Library for Arduino

This Arduino library allows [UCR](https://github.com/Snoothy/UCR) to use an ESP8266 as an output device. The ESP8266 is automatically discovered on the network by UCR once the ESP8266 has connected.

This library aims to make remote control, from any supported input device in UCR, simple and straight forward by removing the complexity of networking to the ESP8266.



The following output types are supported by the library:

- **Button**: Released (`0`) or pressed (`1`)
- **Axis**: `-32768` to `32767`
- **Delta**: Delta change for this given packet
- **Event**: Set to `1` for a single `update()`



## Installation

The library requires the installation of Arduino core for ESP8266. The installation guide can be found here: https://github.com/esp8266/Arduino

This library can then be installed from the Arduino library manager by going to `Tools > Manage libraries...` and searching for `UCR ESP8266` and clicking install. Make sure to install the dependency for `ArduinoJson` when prompted.



## Example usage

```c
const char *ssid = "Network SSID";
const char *password = "Network password";
unsigned int localPort = 8080;

// Instantiate UCR library
UCR ucr(ssid, password, localPort);

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  // Set device name
  ucr.name("UCR-Demo");
    
  // Register supported IO
  ucr.addButton("LED", 0);
  ucr.addAxis("Accelerator", 0);
  ucr.addAxis("Steering", 1);
  ucr.addEvent("Horn", 0);

  // 
  ucr.begin();
}

void loop()
{
  ucr.update();
  digitalWrite(LED_BUILTIN, ucr.readButton(0));
}
```

