//dev mode (enable reset functionality for button )
#define DEVMODE
#define INVERTCAM

// environment vairables
#define WEBSOCKET_HOST_NAME "goldentrash.local"
#define WEBSOCKET_PORT "8756"
#define WEBSOCKET_ROUTE "/ws"
#define WIFI_SSID "wifi_ssid"
#define WIFI_PASSWORD "wifi_password"
#define SERVER_STATIC_IP "your_server_local_ip"

// proces's constants
#define MSPERTURN 1900
#define DELAYPHYSICAL 5000 // delay to let the user have time to put in the trash
#define DELAYCAPTURE 300 // delay capture every half a second

// pins
#define pinServo 33
#define pinBuzz 32


// constant for LEDC
#define LEDC_TIMER_BIT 13    // 13 bit precision
#define LEDC_BASE_FREQ 50    // 50 Hz PWM for standard servos
#define LEDC_CHANNEL_SERVO 3     // Using one of the 



