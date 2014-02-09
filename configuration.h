#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

// Uncomment ONE of the following
#define ___MAPLE
//#define ___TEENSY
//#define ___ARDUINO


// Uncomment ONE of the following for output method
#define ___OLED
//#define ___SERIAL


// PIN configurations

// Analog input
#define ANALOG_IN 3 

// Adafruit SSD1306 OLED
#define OLED_MOSI  27
#define OLED_CLK   28
#define OLED_DC    29
#define OLED_RESET 30   
#define OLED_CS    31   

#define TIE 0x2
#define TEN 0x1

#endif
