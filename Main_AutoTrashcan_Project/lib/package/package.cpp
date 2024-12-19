#include <conf.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <camera_pins.h>
#include <esp_camera.h>
#include <esp_timer.h>
#include <stdarg.h>


using namespace websockets;

#ifdef DEVMODE
void checkFlashPSRAM(){
        if (psramFound()) {
        Serial.println("PSRAM is enabled and available!");
        Serial.print("PSRAM size is ");
        Serial.println(ESP.getPsramSize());
    } else {
        Serial.println("PSRAM is not available.");
    }

    Serial.print("The flash size is ");
    Serial.println(ESP.getFlashChipSize());
}
#endif

void onEventCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        #ifdef DEVMODE
        Serial.println("Connnection Opened");
        #endif
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        #ifdef DEVMODE
        Serial.println("Connnection Closed");
        #else
        ESP.restart();
        #endif
    } else if(event == WebsocketsEvent::GotPing) {
        #ifdef DEVMODE
        Serial.println("Got a Ping!");
        #endif
    } else if(event == WebsocketsEvent::GotPong) {
        #ifdef DEVMODE
        Serial.println("Got a Pong!");
        #endif
    }
}

camera_config_t configinit(){
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_VGA;
    config.pixel_format = PIXFORMAT_JPEG; // for streaming
    // config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    // config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    // for larger pre-allocated frame buffer.
    if(psramFound()){
        config.jpeg_quality = 10;
        config.fb_count = 2;
        config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
        // Limit the frame size when PSRAM is not available
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.fb_count = 1;
    }

    return config;
}

void playTone(uint8_t pin){
    tone(pin, 440, 150);
    vTaskDelay(pdMS_TO_TICKS(150));
    tone(pin, 523, 150);
    vTaskDelay(pdMS_TO_TICKS(150));
    tone(pin, 659, 150);
    vTaskDelay(pdMS_TO_TICKS(200));
    noTone(pin);
}

void playTone2(uint8_t pin) {
  tone(pin, 500, 200); // First tone, 500 Hz for 200 ms
  vTaskDelay(pdMS_TO_TICKS(250));                 // Pause between tones
  tone(pin, 400, 200); // Second tone, 400 Hz for 200 ms
  vTaskDelay(pdMS_TO_TICKS(250));                 // Pause between tones
  tone(pin, 300, 200); // Third tone, 300 Hz for 200 ms
  vTaskDelay(pdMS_TO_TICKS(250));
  noTone(pin);                 // Pause after the sequence
}

void delayMicrosecondsNonBlocking(uint32_t microseconds) {
    uint64_t start = esp_timer_get_time(); // Get the current time in µs
    while ((esp_timer_get_time() - start) < microseconds) {
        taskYIELD(); // Yield to allow other tasks to run
    }
}

#ifdef DEVMODE
void xprintln(const char* somestr) {
    Serial.println(somestr);
}

void xprintln(String somestr) {
    Serial.println(somestr);
}

void xprintln(IPAddress somestr) {
    Serial.println(somestr);
}

void xprintln(uint32_t somestr) {
    Serial.println(somestr);
}

void xprint(const char* somestr) {
    Serial.print(somestr);
}

void xprint(String somestr) {
    Serial.print(somestr);
}

void xprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    Serial.printf(format, args);
    va_end(args);
}

#else
void xprintln(const char* somestr) {}
void xprintln(String somestr) {}
void xprintln(IPAddress somestr) {}
void xprintln(uint32_t somestr) {}

void xprint(const char* somestr) {}
void xprint(String somestr) {}

void xprintf(const char* format, ...) {}
#endif
