#include "UCR.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#define BUTTONS "Buttons"
#define AXES "Axes"
#define DELTAS "Deltas"
#define EVENTS "Events"

#define MSG_HEARTBEAT 0
#define MSG_DESCRIPTOR 1
#define MSG_DATA 2

UCR::UCR(const char *ssid, const char *password)
{
    UCR(ssid, password, 8080);
}

UCR::UCR(const char *ssid, const char *password, unsigned int port)
{
    _ssid = ssid;
    _password = password;
    _port = port;
}

unsigned long UCR::lastUpdateMillis()
{
    return _lastUpdateMillis;
}

void UCR::begin()
{
    Serial.println("UCR begin");

    setupWiFi();
    setupMDNS();
    Udp.begin(_port);
}

void UCR::setupWiFi() 
{
    Serial.println("Setup WiFi");

    if (strlen(_name) == 0) 
    {
        sprintf(_hostString, "UCR_%06X", ESP.getChipId());
    }
    else
    {
        strncpy(_hostString, _name, (sizeof(_hostString) - 1));
    }

    Serial.print("Hostname: ");
    Serial.println(_hostString);

    WiFi.hostname(_hostString);
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("Connected to ");
    Serial.println(_ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void UCR::setupMDNS()
{
    Serial.println("Setup MDNS");
    if (!MDNS.begin(_hostString))
    {
        Serial.println("Error setting up MDNS responder!");
    }
    Serial.println("mDNS responder started");
    MDNS.addService("ucr", "udp", _localPort);
}

void UCR::name(const char *name)
{
    strncpy(_name, name, (sizeof(_name) - 1));
}

void UCR::addButton(const char *name, int index)
{
    if (index >= BUTTON_COUNT) return;
    _buttonList[index] = name;
}

void UCR::addAxis(const char *name, int index)
{
    if (index >= AXIS_COUNT) return;
    _axisList[index] = name;
}

void UCR::addDelta(const char *name, int index)
{
    if (index >= DELTA_COUNT) return;
    _deltaList[index] = name;
}

void UCR::addEvent(const char *name, int index)
{
    if (index >= EVENT_COUNT) return;
    _eventList[index] = name;
}

bool UCR::readButton(int index){
    if (index >= BUTTON_COUNT) return 0;
    return buttonData[index];
}

short UCR::readAxis(int index){
    if (index >= AXIS_COUNT) return 0;
    return axisData[index];
}

short UCR::readDelta(int index){
    if (index >= DELTA_COUNT) return 0;
    return deltaData[index];
}

bool UCR::readEvent(int index){
    if (index >= EVENT_COUNT) return 0;
    return eventData[index];
}

bool UCR::update()
{
    bool updated = false;
    memset(eventData, 0, sizeof(eventData));
    memset(deltaData, 0, sizeof(deltaData));

    MDNS.update();
    if (!receiveUdp())
    {
        return false;
    }

    _lastUpdateMillis = millis();
    return true;
}

void UCR::resetValues(){
    memset(buttonData, 0, sizeof(buttonData));
    memset(axisData, 0, sizeof(axisData));
    memset(deltaData, 0, sizeof(deltaData));
    memset(eventData, 0, sizeof(eventData));
}

void addDescriptorList(JsonDocument *doc, const char *name, const char **list, int size)
{
    JsonArray buttons = doc->createNestedArray(name);
    for (int i = 0; i < size; i++)
    {
        if (list[i] != nullptr)
        {
            JsonObject button = buttons.createNestedObject();
            button["Name"] = list[i];
            button["Value"] = i;
        }
    }
}

bool UCR::receiveUdp()
{
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {

        int len = Udp.read(incomingPacketBuffer, sizeof(incomingPacketBuffer));
        if (len > 0)
        {
            incomingPacketBuffer[len] = 0;
        }
        Serial.printf("UDP packet contents: %s\n", incomingPacketBuffer);

        // Parse packet
        StaticJsonDocument<500> request;
        DeserializationError error = deserializeMsgPack(request, incomingPacketBuffer);

        if (error)
        {
            Serial.print("deserializeMsgPack() failed: ");
            Serial.println(error.c_str());
            return false;
        }

        int msgType = request["MsgType"];

        Serial.println("Got good packet!");
        Serial.print("MsgType: ");
        Serial.println(msgType);

        // Respond
        StaticJsonDocument<500> response;
        response["MsgType"] = msgType;

        if (msgType == 1)
        {
            addDescriptorList(&response, BUTTONS, _buttonList, BUTTON_COUNT);
            addDescriptorList(&response, AXES, _axisList, AXIS_COUNT);
            addDescriptorList(&response, DELTAS, _deltaList, DELTA_COUNT);
            addDescriptorList(&response, EVENTS, _eventList, EVENT_COUNT);

            serializeJsonPretty(response, Serial);
            Serial.println();
        }

        if (msgType == 2)
        {
            for(JsonObject o : request[BUTTONS].as<JsonArray>()){
                buttonData[o["Index"].as<int>()] = o["Value"].as<bool>();
            }
            for(JsonObject o : request[AXES].as<JsonArray>()){
                axisData[o["Index"].as<int>()] = o["Value"].as<short>();
            }
            for(JsonObject o : request[DELTAS].as<JsonArray>()){
                deltaData[o["Index"].as<int>()] = o["Value"].as<short>();
            }
            for(JsonObject o : request[EVENTS].as<JsonArray>()){
                eventData[o["Index"].as<int>()] = o["Value"].as<bool>();
            }
        }

        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        serializeMsgPack(response, Udp);
        Udp.endPacket();

        return true;
    }

    return false;
}