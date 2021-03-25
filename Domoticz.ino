#include "Domoticz.h"

Domoticz::Domoticz(String host_, int port_) :
  host(host_),
  port(port_),
  doc(1024*128)
{
    Serial.printf("[Domoticz  constructor %s, %d]\n", host.c_str(), port);
}

Domoticz::Domoticz() : doc(1024*32)
{
    Serial.printf("[Domoticz  constructor NULL]\n");
}
void Domoticz::Setup(String host_, int port_){
  host =host_;
  port = port_;

  int hardwareID = ESP.getChipId();
  String hardwareName = "esp_" + String(hardwareID);
  IDX_HARDWARE = findIdxHardware(hardwareName);
  if (IDX_HARDWARE == -1) {
    sendDomoticz("/json.htm?type=command&param=addhardware&htype=15&name=" + hardwareName + "&enabled=true&datatimeout=0");
    IDX_HARDWARE = findIdxHardware(hardwareName);
    Serial.printf("Hardware added to domoticz (ID %d)\n", IDX_HARDWARE);
  } 
}

DynamicJsonDocument* Domoticz::sendDomoticz(String url) {
  Serial.printf("[sendDomoticz] >> %S\n", url.c_str());
  http.useHTTP10(true);
  http.begin(host, port, url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    Serial.printf("[sendDomoticz] %d >>\n", http.getSize());
    
    deserializeJson(doc, http.getStream());
    http.end();
    //serializeJson(doc, Serial);
    //Serial.printf("\n");
    return &doc;
  } else {
    Serial.printf("[Err status %d]\n", httpCode);
    http.end();
    return NULL;
  }
}

int Domoticz::idx_of_jsonvar(JsonObject v){
  return v.containsKey("idx") ?String((const char*) v["idx"]).toInt() : -1;
}

int Domoticz::idx_of_jsonvar(DynamicJsonDocument *v){
  return v->containsKey("idx") ?String((const char*) (*v)["idx"]).toInt() : -1;
}

int Domoticz::findIdxHardware(String name) {
  DynamicJsonDocument *result = sendDomoticz("/json.htm?type=hardware");
  if (result != NULL && result->containsKey("result")) {
    JsonArray resultTab = (*result)["result"];
    Serial.printf("[Domoticz findIdxHardware] Liste de hardwares non vide %d\n", resultTab.size());
    for(JsonVariant v : resultTab) {
      const String vname = v["Name"];

    Serial.printf(" SEARCH %s GOT %s \n", vname.c_str(), name.c_str());
      
      if (vname == name ) {
        return idx_of_jsonvar(v.as<JsonObject>());
      }
    }
  }else{
    Serial.printf("BAD JSON\n");
  }
  return -1;
}
int Domoticz::findIdx(int hardwareID) {
  DynamicJsonDocument *result = sendDomoticz("/json.htm?type=devices&filter=all");
  if (result != NULL && result->containsKey("result")) {
    Serial.printf("[Domoticz findIdx] Liste de devices non vide \n");
    JsonArray resultTab = (*result)["result"];
    int l = resultTab.size();
    Serial.printf("[Domoticz findIdx] %d devices\n", l);
    for (int i = 0; i < l; i ++) {
      if ((int)resultTab[i]["HardwareID"] == hardwareID ) {
        return idx_of_jsonvar(resultTab[i].as<JsonObject>());
      }
    }
  }
  return -1;
}
int Domoticz::findIdxSensorOfHardware(int idHardware, String property, int value, int skip) {
  DynamicJsonDocument *result = sendDomoticz("/json.htm?type=devices&filter=all");
  
  Serial.printf("SEARCH hard :%d %s %d", idHardware, property.c_str(), value);
  if (result != NULL && result->containsKey("result")) {
    Serial.printf("[Domoticz findIdxSensorOfHardware] Liste de devices non vide\n");
    JsonArray resultTab = (*result)["result"];
    int l = resultTab.size();
    Serial.printf("[Domoticz findIdxSensorOfHardware] %d devices\n", l);
    for (int i = 0; i < l; i ++) {
      if ((int)resultTab[i]["HardwareID"] == idHardware && resultTab[i].containsKey(property) && (int)resultTab[i][property] == value) {
        if (!skip) return idx_of_jsonvar(resultTab[i].as<JsonObject>());
        skip --;
      }
    }
  }
  return -1;
}
int Domoticz::findIdxSensorOfHardware(int idHardware, String property, int value) {
  return findIdxSensorOfHardware(idHardware, property, value, 0);
}

int Domoticz::findIdxSensorOfHardware(int idHardware, String property, String value) {
  DynamicJsonDocument *result = sendDomoticz("/json.htm?type=devices&filter=all");
  if (result != NULL && result->containsKey("result")) {
    Serial.printf("[Domoticz findIdxSensorOfHardware] Liste de devices non vide\n");
    JsonArray resultTab = (*result)["result"];
    int l = resultTab.size();
    Serial.printf("[Domoticz findIdxSensorOfHardware] %d devices\n", l);
    for (int i = 0; i < l; i ++) {
      String propval = (const char*)resultTab[i][property];
      if ((int)resultTab[i]["HardwareID"] == idHardware && resultTab[i].containsKey(property) && propval == value) {
        return idx_of_jsonvar(resultTab[i].as<JsonObject>());
      }
    }
  }else{
    Serial.printf("NO RESULT search :%d %s %s", idHardware, property.c_str(), value.c_str());
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

DynamicJsonDocument* Domoticz::deviceStatus(int IDX){
  return sendDomoticz("/json.htm?type=devices&rid="+String(IDX));
}
bool Domoticz::isRelayOn(int IDX){
  if (IDX == -1){
    return false;
  }
  DynamicJsonDocument* d = deviceStatus(IDX);
  return d != NULL &&
  d->containsKey("result") && (*d)["result"][0]["Status"] == "On";
}

int Domoticz::createVirtualSensor(String name, int type){
  return idx_of_jsonvar(sendDomoticz("/json.htm?type=createvirtualsensor&idx=" + String(IDX_HARDWARE) + "&sensorname="+name+"&sensortype="+String(type)));
}


int Domoticz::createDevice(String name, int type, int subtype){
    return idx_of_jsonvar(sendDomoticz("/json.htm?type=createdevice&idx=" + String(IDX_HARDWARE) + "&sensorname="+name+"&devicetype="+String(type)+"&devicesubtype="+String(subtype)));
}

void Domoticz::sendValue(int IDX, String value){
    sendDomoticz("/json.htm?type=command&param=udevice&idx=" + String(IDX) + "&nvalue=0&svalue=" + value);
}
void Domoticz::sendSValue(int IDX, String value){
    sendDomoticz("/json.htm?type=command&param=udevice&idx=" + String(IDX) + "&svalue=" + value);
}
