
//#define FEATURE_DISPLAY

#define FEATURE_DHT
#define DHT_PIN 0
#define DHT_TYPE DHT22


//#define FEATURE_MOTION
//#define PIN_MOTION D2

//#define FEATURE_VOLTAGE

#define FEATURE_RELAY
const int relay_enabled[] = {2};

#include "wificonfig.h"

static const int watchdog = 5 * 1000 * 60;
