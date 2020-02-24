#include <Arduino.h>
#include "UCR.h"
#include <Servo.h>

#ifndef STASSID
#define STASSID "TODO"
#define STAPSK "TODO"
#endif

#define LED_BUILTIN 2
#define AXIS_MAX 32767
#define AXIS_MIN -32768

const char *ssid = STASSID;
const char *password = STAPSK;
unsigned int localPort = 8080;

// Other vars
#define DEADMANE_TIMEOUT 500
#define ACCELERATOR_PIN 15
#define STEERING_PIN 5
#define ACCELERATOR 0
#define STEERING 1
UCR ucr(ssid, password, localPort);
Servo steering;
Servo accelerator;

void setup()
{
  Serial.println("\r\nsetup()");
  Serial.begin(115200);
  delay(100);

  pinMode(LED_BUILTIN, OUTPUT);

  ucr.name("UCR-Traxxas");
  ucr.addButton("LED", 0);
  ucr.addAxis("Accelerator", ACCELERATOR);
  ucr.addAxis("Steering", STEERING);
  ucr.addEvent("Horn", 0);

  ucr.begin();

  // Arm Traxxas ESC
  accelerator.attach(ACCELERATOR_PIN);
  accelerator.write(91);
  delay(1000);

  steering.attach(STEERING_PIN);
  steering.write(90);
}

void loop()
{
  ucr.update();

  analogWrite(LED_BUILTIN, map(abs((long)ucr.readAxis(0)), 0,AXIS_MAX, 1024, 0));

  if (millis() - ucr.lastUpdateMillis() <= DEADMANE_TIMEOUT)
  {
    accelerator.write(map(ucr.readAxis(ACCELERATOR),AXIS_MIN,AXIS_MAX,0,180));
    steering.write(map(ucr.readAxis(STEERING),AXIS_MIN,AXIS_MAX,35,145));

    if (ucr.readEvent(0)) {
      // TODO
    }
  }
  else
  {
    Serial.println("Connection lost");
    steering.write(90);
    accelerator.write(91);
  }
}
