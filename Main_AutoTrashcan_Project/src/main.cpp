#include <conf.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoWebsockets.h>
#include <esp_camera.h>
#include <ArduinoJson.h>
#include <servo_control.h>
#include <package.h>
#include <Base64.h>  // Include a Base64 library (some cores include this by default)

 
using namespace websockets;

WebsocketsClient client;

bool withinSensor;
camera_fb_t *sharedFB = NULL;
String jsonStr;
String encodedImage;
float objectDistance = 0;
uint16_t servo_angle = 0;
uint16_t servo_curr_angle = 0;
uint16_t angle = 0;
bool connected = false;
QueueHandle_t servoTaskQ;
bool finished_confirmation;


void onMessageCallback(WebsocketsMessage msg);
void networkingcore(void *parameter);
void taskingcore(void *parameter);
void servoServer(void *parameter);
void ultsonDetection(void *parameter);

void setup() {
  #ifdef DEVMODE
  Serial.begin(115200);
  checkFlashPSRAM();
  #endif
  
  xprintln("\n\nTask 1: Initializing camera");
  camera_config_t config = configinit();
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    xprintf("Camera init failed with error 0x%x\n", err);
    return;
  }
  sensor_t *s = esp_camera_sensor_get();
  #ifdef INVERTCAM
  s->set_vflip(s, 1); // flip it back
  #endif
  s->set_brightness(s, 3); // up the brightness just a bit
  s->set_saturation(s, 0); // lower the saturation
  s->set_framesize(s, FRAMESIZE_VGA);


  // pins and semaphores initialization
  xprintln("\n\nTask 2: Pins, semaphores, and variables initialization");
  pinMode(pinBuzz, OUTPUT); // LED pin using transistor BJT 2N2222
  setupServo();
  servoTaskQ = xQueueCreate(1, sizeof(int));


  // Baru WiFi dan yang lainnya
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  xprint("\n\nTask 3 : Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    xprint(".");
  }
  xprintln("\nConnected to WiFi");

  xprintln("\n\nTask 4: mDNS intializing");
  IPAddress serverIP;
  // Initialize mDNS
  if (!MDNS.begin("goldentrash1")) {
    xprintln("Error starting mDNS");
    MDNS.end();
  } else {
    xprintln("mDNS started");
    xprintln("\n\nTask 5: Resolving mDNS IP");
    // Resolve mDNS hostname to IP address
    xprintln("Resolving mDNS hostname");
    serverIP = MDNS.queryHost(WEBSOCKET_HOST_NAME);
    if (serverIP == INADDR_NONE) {
      xprintln("Error resolving mDNS hostname, using default static IP.");
      serverIP.fromString(SERVER_STATIC_IP);  // Fallback to static IP
    } else {
      xprintln("mDNS hostname resolved to IP address");
    }
  }

  // Debug: print resolved IP address
  xprint("Resolved server IP: ");
  xprintln(serverIP);

  xprintln("\n\nTask 6: Initializing Websocket");
  // Connect to WebSocket server
  client.onMessage(onMessageCallback);
  client.onEvent(onEventCallback);

  xprint("Trying to connect to the websocket (" + serverIP.toString() + ")");
  int reseted = 0;
  while(!connected){
    if(reseted > 5){
      ESP.restart();
    }

    reseted++;
    xprintln("\nTry");
    client.end(); // Close any existing connection
    connected = client.connect(serverIP.toString(),std::stoi(WEBSOCKET_PORT), WEBSOCKET_ROUTE);
    delay(500);
  }


  xprintln("\n\nTask 7: Tasking delegation");

  // dynamically delegate the tasks, using the load balancer from FreeRTOS
  xTaskCreate(networkingcore, "Websocket tasks", 20000, NULL, 1, NULL);
  xTaskCreate(servoServer, "Servo task", 5000, NULL, 1, NULL);
  xprintln("\n\nInitialization and system all set for use!!");
  playTone2(pinBuzz);
}

// Free-RTOS task assignment functions
void networkingcore(void *parameter){
  vTaskDelay(pdMS_TO_TICKS(200));
  xprintln("Networking task started");

  for(;;){
    
    if (!client.available()) {
      // Reconnect jika koneksi terputus
      xprintln("Connection lost, trying to reconnect...");
      client.end();
      vTaskDelay(pdMS_TO_TICKS(1000));
      
      int retries = 0;
      while (!client.connect(WiFi.localIP().toString(), std::stoi(WEBSOCKET_PORT), WEBSOCKET_ROUTE) && retries < 3) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        retries++;
      }
      
      if (retries >= 3) {
        ESP.restart();
      }
    }

    client.poll();


    if(sharedFB) {
      esp_camera_fb_return(sharedFB);
      sharedFB = NULL;
    }

    sharedFB = esp_camera_fb_get();

    if (!sharedFB) {
      xprintln("Camera capture failed");
      continue;
    }
    // Upload frame to server


    // Encode binary data to Base64 store it to a string
    encodedImage = base64::encode(sharedFB->buf, sharedFB->len);
    
    // json formating
    // Create a JSON object
    JsonDocument jsonDoc;
    jsonDoc["type"] = "image";
    jsonDoc["format"] = "jpeg->base64";  // You can specify format here  (doesn't really matter)
    jsonDoc["frame"] = encodedImage;
    serializeJson(jsonDoc, jsonStr);
    // websocket uploading
    client.send(jsonStr);
    vTaskDelay(pdMS_TO_TICKS(DELAYCAPTURE));
  }
}

void servoServer(void *parameter){
  vTaskDelay(pdMS_TO_TICKS(300));
  xprintln("Servo server task started");
  for(;;){

    uint16_t angle;
    if(xQueueReceive(servoTaskQ, &angle, portMAX_DELAY)){
      if(servo_curr_angle-angle != 0){
        // change the angle of the servo which need we delay the servo until it is on the intended angle, which introduces the delay
        servoAssign(&servo_curr_angle, angle); 
      }
    }
  }
}

// websocket function callbacks
void onMessageCallback(WebsocketsMessage msg){
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc,msg.data());
  const char* theType = doc["type"];
  
  if(strcmp(theType, "servo") == 0){
    servo_angle = doc["angle"].as<uint16_t>();
    xprintln("Servo angle set to: " + String(servo_angle));
    if(servo_angle != servo_curr_angle){
      xQueueSend(servoTaskQ, &servo_angle, 0);
      playTone(pinBuzz);
    }
  }

  if(strcmp(theType, "connection") == 0){
    client.ping();

    const char* message = doc["message"];
    xprintln(message);

    xQueueSend(servoTaskQ, &servo_angle, 0);
    playTone(pinBuzz);

    xprintln("Building payback's JSON");
    JsonDocument jsonDoc;
    jsonDoc["type"] = "payback";
    jsonDoc["message"] = "Hi server I am a client!";
    serializeJson(jsonDoc, jsonStr);
    // websocket uploading
    xprintln("trying to send payback");
    xprintln(jsonStr);
    client.send(jsonStr);

  }
  
}

void loop(){
  // do nothing
}


