
#include "Settings.h"
#include "Domoticz.h"
#include <base64.hpp>


#include <Adafruit_Sensor.h>
#ifdef FEATURE_DHT
#include <DHT.h>
DHT dht(DHT_PIN, DHT_TYPE);
int IDX_DHT = -1;
#endif

#ifdef FEATURE_RELAY
#define FEATURE_SERVER
#endif

#include <ESP8266WiFi.h>

#ifdef FEATURE_SERVER
#include <ESP8266WebServer.h>
ESP8266WebServer server ( 80 );
#endif


unsigned long previousMillis;

#ifdef FEATURE_VOLTAGE
int IDX_VOLTAGE = -1;
ADC_MODE(ADC_VCC);
#endif

#ifdef FEATURE_RELAY
void relayFromServer() {
  int id = server.arg("id").toInt();
  int state = server.arg("state").toInt();

  Serial.printf("[Domoticz] From Server relay %d (pin %d) to %d...\n", id, relay_enabled[id], state);
  digitalWrite(relay_enabled[id], state == 0 ? LOW : HIGH);
  server.send(200, "text/plain", "{\"status\" : \"OK\"} ");
}
#endif

#ifdef FEATURE_SERVER
void statusPage(){
  String espName(ESP.getChipId());
  server.send(200, "text/html", "<html><head><title>"+espName+"</title></head><body><h1>"+espName+"</h1><pre>free memory : "+String(ESP.getFreeHeap())+"\nlast reset : "+ESP.getResetReason()+"\nmhz: "+ESP.getCpuFreqMHz()+"\nwatchdog:"+watchdog+"</pre></body></html>");
}
#endif

void WaitWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
  Serial.printf("[Wifi] OK\n");
}

String base64_encode(String s) {
  u8 in[256];
  u8 out[256];
  std::copy( s.begin(), s.end(), in );
  in[s.length()] = 0;
  int output_length = encode_base64(in, s.length(), out);
  out[output_length] = 0;
  String o;
  o.reserve(output_length);
  for (int i = 0; i < output_length; i ++)
    o += (char)out[i];
  return o;
}

Domoticz dom;

#ifdef FEATURE_MOTION
#include "Schedule.h"
int IDX_MOTION = -1;
void detectsMovement_AUX() {
  dom.sendDomoticz("/json.htm?type=command&param=switchlight&idx=" + String(IDX_MOTION) + "&switchcmd=On");
  Serial.printf("interruption done\n");
}
void ICACHE_RAM_ATTR detectsMovement() {
  Serial.printf("MOTION DETECTED ? %d\n", digitalRead(PIN_MOTION) == HIGH ? 1 : 0);
  schedule_function(detectsMovement_AUX);
}
#endif

#ifdef FEATURE_LIGHTSENSOR
  int IDX_LIGHTSENSOR = -1;
#endif

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.setDebugOutput(true);
  WiFi.begin(ssid, password);
  WaitWifi();
  dom.Setup(host, port);;

#ifdef FEATURE_LIGHTSENSOR
  pinMode( PIN_LIGHTSENSOR, INPUT );
  IDX_LIGHTSENSOR = dom.findIdxSensorOfHardware("Type", "Lux");
  if (IDX_LIGHTSENSOR==-1){
    IDX_LIGHTSENSOR = dom.idx_of_jsonvar(dom.sendDomoticz("/json.htm?type=createdevice&idx="+String(dom.IDX_HARDWARE)+"&sensorname=lux_"+String(dom.IDX_HARDWARE)+"&sensormappedtype=0xF601"));
  }
#endif

#ifdef FEATURE_SERVER
  server.on("/Switch", relayFromServer);
  server.on("/Status", statusPage);
  server.begin();
#endif

  Serial.printf("setup pins modes...\n");
#ifdef FEATURE_RELAY
  int count_relay = sizeof(relay_enabled) / sizeof(relay_enabled[0]);
  for (int i = 0; i < count_relay; i ++) {
    pinMode(relay_enabled[i], OUTPUT);
    digitalWrite(relay_enabled[i], LOW);
  }
#endif

#ifdef FEATURE_MOTION
  pinMode(PIN_MOTION, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_MOTION), detectsMovement, CHANGE);
#endif

  Serial.printf("setup domoticz...\n");

#ifdef FEATURE_DHT
  Serial.printf("DHT begin(%d, %d)\n", DHT_PIN, DHT_TYPE);
  dht.begin();
  IDX_DHT = dom.findIdxSensorOfHardware("Type", "Temp + Humidity");
  if (IDX_DHT == -1) {
    IDX_DHT = dom.createVirtualSensor("TempHum"+String(dom.IDX_HARDWARE), 82);
    Serial.printf("[Domoticz] final DHT IDX=%d...\n", IDX_DHT);
  }
#endif



#ifdef FEATURE_VOLTAGE
    IDX_VOLTAGE = dom.findIdxSensorOfHardware("SubType", "Voltage");
    if (IDX_VOLTAGE == -1) IDX_VOLTAGE = dom.createDevice("Voltage"+String(dom.IDX_HARDWARE), 243, 8);
    Serial.printf("Voltage sensor IDX = %d\n", IDX_VOLTAGE);
#endif

#ifdef FEATURE_RELAY
    Serial.printf("there is %d relays\n", count_relay);
    if (dom.relayID(0) == -1){
      String WifiString(WiFi.localIP().toString());
      Serial.printf("this wifi %s\n", WifiString.c_str());
      for (int i = 0; i < count_relay; i ++) {
        String name("Switch_" + String(i));
        int IDX_RELAY = dom.createVirtualSensor(name, 6);
        Serial.printf("Switch (%s) added to domoticz (ID %d)\n", name.c_str(), IDX_RELAY);
        String url_On = base64_encode("http://" + WifiString + "/Switch?id=" + i + "_" + "&state=1");
        String url_Off = base64_encode("http://" + WifiString + "/Switch?id=" + i + "_" + "&state=0");
        dom.sendDomoticz("/json.htm?addjvalue=0&addjvalue2=0&customimage=0&description=&idx=" + String(IDX_RELAY) + "&name=" + name + "&options=&protected=false&strparam1=" + url_On + "&strparam2=" + url_Off + "&switchtype=0&type=setused&used=true");
      }
    }
    for (int i = 0; i < count_relay; i ++) {
      int idx = dom.relayID(i);
      Serial.printf("init idx=%d relay(%d)...\n", idx, i);
      bool relay_on = dom.isRelayOn( idx );
      Serial.printf("value %d\n", relay_on ? 1 : 0);
      digitalWrite(relay_enabled[i], relay_on ? HIGH : LOW); // TODO demander au serveur ?
    }
#endif

#ifdef FEATURE_MOTION
    Serial.printf("try to define Motion sensor IDX\n");
    IDX_MOTION = dom.findIdxSensorOfHardware("SwitchTypeVal", 8);
    if (IDX_MOTION == -1){
      String motion_name = "Motion_Sensor";
      IDX_MOTION = dom.createVirtualSensor(motion_name, 6);
      dom.sendDomoticz("/json.htm?addjvalue=60&addjvalue2=0&customimage=0&description=&idx=" + String(IDX_MOTION) + "&name=" + motion_name + "&options=&protected=false&strparam1=&strparam2=&switchtype=8&type=setused&used=true");
      Serial.printf("Motion sensor added to domoticz (ID %d)\n", IDX_MOTION);
    }
    Serial.printf("Motion sensor IDX = %d\n", IDX_MOTION);
#endif

  previousMillis = millis() - watchdog;
}

int hum2humsat(float h) {
  if ( h > 70 ) return 3;
  if ( h > 45) return 1;;
  if ( h > 30) return 0;
  return 2;
}

int vol2lum(float Vout){
  float RLDR = (10000 * (3.3 - Vout))/Vout;
  int phys=500/(RLDR/1000);
  return phys;
}

void loop() {
#if defined(FEATURE_SERVER)
  server.handleClient();
#endif

  unsigned long currentMillis = millis();
  if ( currentMillis - previousMillis > watchdog ) {
    previousMillis = currentMillis;
#ifdef FEATURE_MOTION
    Serial.printf("move ? %d\n", digitalRead(PIN_MOTION) == HIGH ? 1 : 0);
#endif
    if (WiFi.status() != WL_CONNECTED) {
      WaitWifi();
    }
#ifdef FEATURE_VOLTAGE
    Serial.printf("voltage : %s\n", String(ESP.getVcc() / 1024.0f, 3).c_str());
    dom.sendValue(IDX_VOLTAGE,  String(ESP.getVcc() / 1024.0f, 3));
#endif


#ifdef FEATURE_DHT
    float temperature = dht.readTemperature();
    float hum = dht.readHumidity();
    int hum_sat = hum2humsat(hum);
    dom.sendValue(IDX_DHT,  String(temperature) + ";" + hum + ";" + hum_sat);
#endif

#ifdef FEATURE_LIGHTSENSOR
  dom.sendSValue(IDX_LIGHTSENSOR,  String(vol2lum(analogRead(PIN_LIGHTSENSOR) / 1024.0f)));
#endif

#if defined(FEATURE_SERVER) || defined(FEATURE_MOTION)
  delay(500);
#else
  delay(5000);
#endif
  }
}
