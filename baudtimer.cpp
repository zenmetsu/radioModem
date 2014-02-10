#include "configuration.h"
#include "baudtimer.h"
#ifndef ___TEENSY
  #include <wirish.h>
  #include <gpio.h>
#endif

volatile bool LED_TG = false;

#ifdef ___OLED
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>

  extern Adafruit_SSD1306 display;
#endif

#ifdef ___MAPLE
  #include <HardwareTimer.h>
  extern HardwareTimer timer4;
#endif

#ifdef ___ARDUINO
  #include "Arduino.h"
  volatile boolean baudOverflow = 0;                  // overflow flag, needed because of Arduino's 
                                                      // limited timer multiplier selection :(
#endif

#ifdef ___TEENSY
  #include "Arduino.h"
  unsigned long countMicros = micros();
  #include <mk20dx128.h>
#endif

//Timer control routines
void baud_timer_init() 
{
  #ifdef ___MAPLE
    timer4.pause();                                    // Pause the timer while configuring it
    timer4.setMode(TIMER_CH1, TIMER_OUTPUT_COMPARE);   // Set up interrupt on channel 1
    timer4.setCount(0);                                // Reset count to zero
        
    timer4.setPrescaleFactor(72);                      // Timer counts at 72MHz/72 = 1MHz  1 count = 1uS
    timer4.setOverflow(0xFFFF);                        // reset occurs at 15.259Hz
    timer4.refresh();                                  // Refresh the timer's count, prescale, and overflow
    timer4.resume();                                   // Start the timer counting
  #endif

  #ifdef ___ARDUINO
    cli();                               // stop interrupts during configuration
    TCCR1A = 0;                          // Clear TCCR1A register
    TCCR1B = 0;                          // Clear TCCR1B register
    TCNT1  = 0;                          // Initialize counter value
    OCR1A  = 0xFFFF;                     // Set compare match register to maximum value
    TCCR1B |= (1 << WGM12);              // CTC mode
                                         // We want 1uS ticks, for 16MHz CPU, we use prescaler of 16
                                         // as 1MHz = 1uS period, but Arduino is lame and only has
                                         // 3 bit multiplier, we can have 8 (overflows too quickly)
                                         // or 64, which operates at 1/4 the desired resolution
    TCCR1B |= (1 << CS11);               // Configure for 8 prescaler
    TIMSK1 |= (1 << OCIE1A);             // enable compare interrupt
    sei();                               // re-enable interrupts 
  #endif    
  
  #ifdef ___TEENSY
    FTM0_MODE |= FTM_MODE_WPDIS;
    
    FTM0_CNT = 0;
    FTM0_CNTIN = 0;
    FTM0_SC |= FTM_SC_PS(7);
    FTM0_SC |= FTM_SC_CLKS(1);
    FTM0_MOD = 0xFFFF;
    FTM0_MODE |= FTM_MODE_FTMEN;
    /*
    PIT_LDVAL1 = 0x500000;
    PIT_TCTRL1 = TIE;
    PIT_TCTRL1 |= TEN;
    PIT_TFLG1 |= 1;
    */
  #endif  
}



void baud_timer_restart() 
{
  #ifdef ___MAPLE
    timer4.setCount(0); 
    timer4.refresh();  
  #endif
  
  #ifdef ___ARDUINO
    TCNT1 = 0;
    baudOverflow = false;   
  #endif   
  
  #ifdef ___TEENSY
    FTM0_CNT = 0x0;
  #endif     
}



unsigned int baud_time_get() 
{
  #ifdef ___MAPLE  
    timer4.pause();
    unsigned int tmp = timer4.getCount();  //get current timer count
    timer4.setCount(0);                    // reset timer count
    timer4.refresh();
    timer4.resume();
  #endif

  #ifdef ___ARDUINO
    TCCR1B &= 0xF8;                        // Clear CS10/CS11/CS12 bits, stopping the timer
    unsigned int tmp = int(TCNT1/2);       // get current timer count, dividing by two
                                           // because Arduino timer is counting at 2Mhz instead of 1Mhz
                                           // due to limited prescaler selection
    tmp += 0x8000 * baudOverflow;          // if timer overflowed, we will add 0xFFFF/2
    TCNT1 = 0;                             // reset timer count    
    baudOverflow = false;                  // reset baudOverflow after resetting timer    
    TCCR1B |= (1 << CS11);                 // Configure for 8 prescaler, restarting timer
  #endif

  #ifdef ___TEENSY
    unsigned int tmp = int(FTM0_CNT*2.8);
    FTM0_CNT = 0x0;
  #endif
  
  return tmp;
}

#ifdef ___ARDUINO
  ISR(TIMER1_COMPA_vect)
  {
    baudOverflow = true;                   // ISR called on overflow, set overflow flag
  }
#endif

#ifdef ___TEENSY
  void ftm0_isr(void)
  {
  }
#endif

