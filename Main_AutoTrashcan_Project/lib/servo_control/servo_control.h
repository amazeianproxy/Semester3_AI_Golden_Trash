#include <Arduino.h>
void servoAssign(uint16_t* from, uint16_t to);
void turnLeft(uint32_t delays);
void turnRight(uint32_t delays);
void neutral(uint32_t delays = 0);
uint16_t calc_delay_from_angle_difference(uint16_t from, uint16_t to);
void setupServo();