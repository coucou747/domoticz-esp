/* FEATURES AVAILABLE :

FEATURE_DHT
used for temperature & humidity sensor like DHT11 or DHT22
define DHT_PIN (eg 0 for your esp01)
define DHT_TYPE (eg DHT22)

FEATURE_MOTION
used for presence sensor
define PIN_MOTION (eg D2)

FEATURE_VOLTAGE
used to show if the battery is low

FEATURE_RELAY
you have to set the array relay_enabled for the gpio used for this feature eg:
  const int relay_enabled[] = {2};

FEATURE_LIGHTSENSOR
efine PIN_LIGHTSENSOR (eg A0)
*/

/* config pour mes D1 litle

#define FEATURE_LIGHTSENSOR
#define PIN_LIGHTSENSOR A0

#define FEATURE_DHT
#define DHT_PIN D2
#define DHT_TYPE DHT22

#define FEATURE_MOTION
#define PIN_MOTION D6


*/


/* Config pour mon esp01
*/
#define FEATURE_RELAY
const int relay_enabled[] = {2};






#include "wificonfig.h"

static const int watchdog = 5 * 1000 * 60;
