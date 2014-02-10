#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

// Uncomment ONE of the following
//#define ___MAPLE
#define ___TEENSY
//#define ___ARDUINO


// Uncomment ONE of the following for output method
#define ___OLED
//#define ___SERIAL


// PIN configurations

// Analog input
#define ANALOG_IN 0 

// Adafruit SSD1306 OLED
#define OLED_MOSI  8
#define OLED_CLK   9
#define OLED_DC    10
#define OLED_RESET 11   
#define OLED_CS    12   

#define TIE 0x2
#define TEN 0x1

#endif
