#include "Domoticz.h"

Domoticz::Domoticz(String host_, int port_) :
  host(host_),
  port(port_)
{
  
    Serial.printf("[Domoticz  constructor %s, %d]\n", host.c_str(), port);
}

Domoticz::Domoticz()
{
    Serial.printf("[Domoticz  constructor NULL]\n");
}
void Domoticz::Setup(String host_, int port_){
  host =host_;
  port = port_;

  int hardwareID = ESP.getChipId();
  String hardwareName = "esp_" + String(hardwareID);
  IDX_HARDWARE = dom.findIdxHardware(hardwareName);
  if (IDX_HARDWARE == -1) {
    dom.sendDomoticz("/json.htm?type=command&param=addhardware&htype=15&name=" + hardwareName + "&enabled=true&datatimeout=0");
    IDX_HARDWARE = dom.findIdxHardware(hardwareName);
    Serial.printf("Hardware added to domoticz (ID %d)\n", IDX_HARDWARE);
  } 
}

JSONVar Domoticz::sendDomoticz(String url) {
  Serial.printf("[Send] %S\n", url.c_str());
  http.begin(host, port, url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    JSONVar result = JSON.parse(http.getString());
    http.end();
    return result;
  } else {
    Serial.printf("[Err status %d]\n", httpCode);
    http.end();
    JSONVar nulljson(false);
    return nulljson;
  }
  http.end();
  JSONVar nulljson(false);
  return nulljson;
}

int Domoticz::idx_of_jsonvar(JSONVar v){
  return v.hasOwnProperty("idx") ?String((const char*) v["idx"]).toInt() : -1;
}

int Domoticz::findIdxHardware(String name) {
  JSONVar result = sendDomoticz("/json.htm?type=hardware");
  if (result.hasOwnProperty("result")) {
    Serial.printf("[Domoticz findIdxHardware] Liste de hardwares non vide \n");
    JSONVar resultTab = result["result"];
    int l = resultTab.length();
    Serial.printf("[Domoticz findIdx] %d hardwares\n", l);
    for (int i = 0; i < l; i ++) {
      if (resultTab[i]["Name"] == name ) {
        return idx_of_jsonvar(resultTab[i]);
      }
    }
  }
  return -1;
}
int Domoticz::findIdx(int hardwareID) {
  JSONVar result = sendDomoticz("/json.htm?type=devices");
  if (result.hasOwnProperty("result")) {
    Serial.printf("[Domoticz findIdx] Liste de devices non vide \n");
    JSONVar resultTab = result["result"];
    int l = resultTab.length();
    Serial.printf("[Domoticz findIdx] %d devices\n", l);
    for (int i = 0; i < l; i ++) {
      if ((int)resultTab[i]["HardwareID"] == hardwareID ) {
        return idx_of_jsonvar(resultTab[i]);
      }
    }
  }
  return -1;
}
int Domoticz::findIdxSensorOfHardware(int idHardware, String property, int value, int skip) {
  JSONVar result = sendDomoticz("/json.htm?type=devices");
  if (result.hasOwnProperty("result")) {
    Serial.printf("[Domoticz findIdxSensorOfHardware] Liste de devices non vide\n");
    JSONVar resultTab = result["result"];
    int l = resultTab.length();
    Serial.printf("[Domoticz findIdxSensorOfHardware] %d devices\n", l);
    for (int i = 0; i < l; i ++) {
      if ((int)resultTab[i]["HardwareID"] == idHardware && resultTab[i].hasOwnProperty(property) && (int)resultTab[i][property] == value) {
        if (!skip) return idx_of_jsonvar(resultTab[i]);
        skip --;
      }
    }
  }
  return -1;
}
int Domoticz::findIdxSensorOfHardware(int idHardware, String property, int value) {
  return findIdxSensorOfHardware(idHardware, property, value);
}

int Domoticz::findIdxSensorOfHardware(int idHardware, String property, String value) {
  JSONVar result = sendDomoticz("/json.htm?type=devices");
  if (result.hasOwnProperty("result")) {
    Serial.printf("[Domoticz findIdxSensorOfHardware] Liste de devices non vide\n");
    JSONVar resultTab = result["result"];
    int l = resultTab.length();
    Serial.printf("[Domoticz findIdxSensorOfHardware] %d devices\n", l);
    for (int i = 0; i < l; i ++) {
      String propval = (const char*)resultTab[i][property];
      if ((int)resultTab[i]["HardwareID"] == idHardware && resultTab[i].hasOwnProperty(property) && propval == value) {
        return idx_of_jsonvar(resultTab[i]);
      }
    }
  }
  return -1;
}

int Domoticz::findIdxSensorOfHardware(String property, int value){
  return findIdxSensorOfHardware(IDX_HARDWARE, property, value);
}

int Domoticz::findIdxSensorOfHardware(String property, String value){
  return findIdxSensorOfHardware(IDX_HARDWARE, property, value);
}

int Domoticz::relayID(int nth){
  return findIdxSensorOfHardware(IDX_HARDWARE, "SwitchTypeVal", 0, nth);
}

JSONVar Domoticz::deviceStatus(int IDX){
  return sendDomoticz("/json.htm?type=devices&rid="+String(IDX));
}

bool Domoticz::isRelayOn(int IDX){
  if (IDX == -1){
    return false;
  }
  return String((const char*)deviceStatus(IDX)[ "result" ][0]["Status"]) == String("On");
}
