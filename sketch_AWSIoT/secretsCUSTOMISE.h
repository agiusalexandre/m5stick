#include <pgmspace.h>

#define SECRET
#define THINGNAME "YOUR_THING_NAME"

const char WIFI_SSID[] = "YOUR_WIFI_SSID";
const char WIFI_PASSWORD[] = "YOUR_WIFI_PWD";
const char AWS_IOT_ENDPOINT[] = "YOUR_ENDPOINT";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
YOUR_CA
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
YOUR_CERT
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
YOUR_CERT_PR
)KEY";
