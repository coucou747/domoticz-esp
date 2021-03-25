
#ifndef DOMOTICZ_H
#define DOMOTICZ_H
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

class Domoticz {
  private:
    HTTPClient http;
    String host;
    int port;
    
    DynamicJsonDocument doc;
    
  public:
    int IDX_HARDWARE;
    Domoticz(String host, int port);
    Domoticz();
    void Setup(String host, int port);
    DynamicJsonDocument* sendDomoticz(String url) ;
    int idx_of_jsonvar(JsonObject v);
    int idx_of_jsonvar(DynamicJsonDocument* v);
    int findIdxHardware(String name);
    int findIdx(int hardwareID);
    int findIdxSensorOfHardware(int idHardware, String property, int value, int skip);
    int findIdxSensorOfHardware(int idHardware, String property, int value);
    int findIdxSensorOfHardware(int idHardware, String property, String value);
    int findIdxSensorOfHardware(String property, int value);
    int findIdxSensorOfHardware(String property, String value);
    int relayID(int nth);
    int createVirtualSensor(String name, int type);
    int createDevice(String name, int type, int subtype);
    void sendValue(int IDX, String value);
    void sendSValue(int IDX, String value);
    DynamicJsonDocument* deviceStatus(int IDX) ;
    bool isRelayOn(int IDX) ;
};

#endif
