#include "configuration.h"
#include "baudtimer.h"
#include <wirish.h>
#include <gpio.h>

#ifdef ___MAPLE
  #include <HardwareTimer.h>
  extern HardwareTimer timer4;
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
}



void baud_timer_restart() 
{
  #ifdef ___MAPLE
    timer4.setCount(0); 
    timer4.refresh();  
  #endif
}



unsigned int baud_time_get() 
{
  #ifdef ___MAPLE  
    timer4.pause();
    unsigned int tmp = timer4.getCount();
    timer4.setCount(0); 
    timer4.refresh();
    timer4.resume();
  #endif
  
  return tmp;
}
