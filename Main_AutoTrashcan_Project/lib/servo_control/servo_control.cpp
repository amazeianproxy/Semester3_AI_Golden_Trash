#include <Arduino.h>
#include <ESP32Servo.h>
#include <conf.h>
#include <esp_camera.h>
#include <package.h>

Servo myservo;

uint16_t calc_delay_from_angle_difference(uint16_t from, uint16_t to){
    if(from > to){
        xprintln("From-to:"+String((float(from-to)/360) * MSPERTURN));
        return uint16_t((float(from-to)/360) * MSPERTURN);
    } else {
        xprintln("To-from:"+String((float(to-from)/360) * MSPERTURN));
        return uint16_t((float(to-from)/360) * MSPERTURN);
    }
}

void setupServo() {
    ESP32PWM::allocateTimer(3);
    myservo.setPeriodHertz(50);
    
    delay(100);
    
    if (myservo.attached()) {
        myservo.detach();
        delay(200);
    }
    
    myservo.attach(pinServo, 1000, 2000);
}

void turnLeft(uint16_t delays) {
    myservo.writeMicroseconds(1300);
    vTaskDelay(pdMS_TO_TICKS(delays));
}

void turnRight(uint16_t delays) {
    myservo.writeMicroseconds(1700);
    vTaskDelay(pdMS_TO_TICKS(delays));
}

void neutral(uint16_t delays) {
    myservo.writeMicroseconds(1500);
    vTaskDelay(pdMS_TO_TICKS(delays));
}

// messages received from the websocket
void servoAssign(uint16_t* from, uint16_t to) {
    xprintln(String(*from) + String(to));
    if(to > *from && !(to > 360)){
        turnRight(calc_delay_from_angle_difference(*from, to));
        neutral(DELAYPHYSICAL);
        *from = to;
    } else if(to < *from && !(to < 0)){
        turnLeft(calc_delay_from_angle_difference(*from, to));
        neutral(DELAYPHYSICAL);
        *from = to;
    } else {
        neutral(0);
    }
    // xprintln("Finished");
}

