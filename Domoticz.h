
#ifndef DOMOTICZ_H
#define DOMOTICZ_H
#include <Arduino_JSON.h>
#include <ESP8266HTTPClient.h>

class Domoticz {
  private:
    HTTPClient http;
    String host;
    int port;
    
  public:
    int IDX_HARDWARE;
    Domoticz(String host, int port);
    Domoticz();
    void Setup(String host, int port);
    JSONVar sendDomoticz(String url) ;
    int idx_of_jsonvar(JSONVar v);
    int findIdxHardware(String name);
    int findIdx(int hardwareID);
    int findIdxSensorOfHardware(int idHardware, String property, int value, int skip);
    int findIdxSensorOfHardware(int idHardware, String property, int value);
    int findIdxSensorOfHardware(int idHardware, String property, String value);
    int findIdxSensorOfHardware(String property, int value);
    int findIdxSensorOfHardware(String property, String value);
    int relayID(int nth);

    JSONVar deviceStatus(int IDX) ;
    bool isRelayOn(int IDX) ;
};

#endif
