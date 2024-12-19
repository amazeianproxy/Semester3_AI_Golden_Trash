#include "ArduinoWebsockets.h"

using namespace websockets;

void onEventCallback(WebsocketsEvent event, String data);
// void onMessageCallback(WebsocketsMessage message);
camera_config_t configinit();
void checkFlashPSRAM();
void playTone(uint8_t);
void playTone2(uint8_t pin);
void delayMicrosecondsNonBlocking(uint32_t microseconds);

void xprintln(const char* somestr);
void xprintln(String somestr);
void xprintln(IPAddress somestr);
void xprintln(uint32_t somestr);
void xprint(const char* somestr);
void xprint(String somestr);
void xprintf(const char* format, ...);