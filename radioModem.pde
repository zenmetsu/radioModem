#include "configuration.h"
#include "utility.h"
#include "fractionaltypes.h"
#include "baudtimer.h"
#include "frontend.h"
#include "arctanapprox.h"
#include "psk31.h"
#include "rtty.h"


#ifdef ___MAPLE
  HardwareTimer timer3(3);
  HardwareTimer timer4(4);
#endif

#ifdef ___TEENSY
  #include <Wire.h>
  #include <mk20dx128.h>
  IntervalTimer sampleTimer;
#endif

#ifdef ___ARDUINO
  #include <Wire.h>
#endif


#ifdef ___OLED

  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>

  Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
#endif 


#define DELAY_LENGTH 64

#ifdef ___MAPLE
  #define sbit(port, pin) port->regs->BSRR = BIT(pin)
  #define cbit(port, pin) port->regs->BRR = BIT(pin)
#endif

// Initialize demodulation components
void tdtl_init();

// Generic globals
volatile uint16_t process;
volatile F16 td, oldTd;
volatile boolean LED_TOG = false;


volatile unsigned short int sample_idx;
volatile F15 sample[DELAY_LENGTH];

F16 _D0, _D1, kftau;

unsigned int x = 0;
   
//Demodulation constants
F16	e = 0;//, ie = 0;
uint16_t doRTTY = 0, doPSK = 1;

void handler_adc();
void wtf();

void adc_init()
{
  #ifdef ___MAPLE  
    timer3.pause();
    timer3.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    timer3.setPeriod(int(1000000/SAMPLE_RATE)); // 122uS = 8192Hz
    timer3.setCompare1(1);  
    timer3.attachCompare1Interrupt(handler_adc);
  #endif
  
  #ifdef ___ARDUINO
    TCCR2A = 0;                                   // Clear TCCR2A register
    TCCR2B = 0;                                   // Clear TCCR2B register
    TCNT2  = 0;                                   // Initialize counter value
    OCR2A  = (16*10^6) / (SAMPLE_RATE * 8);       // Set compare match register
    TCCR2A |= (1 << WGM21);                       // Set CTC mode
    TCCR2B |= (1 << CS21);                        // Set CS21 for 8 prescaler
    TIMSK2 |= (1 << OCIE2A);                      // Enable Timer2 compare interrupt
  #endif
  
  #ifdef ___TEENSY
    
    sampleTimer.begin(handler_adc, int(1000000/SAMPLE_RATE));//int(1000000/SAMPLE_RATE));
  #endif
}  
   
void io_init()
{
  #ifdef ___MAPLE
    pinMode(ANALOG_IN, INPUT_ANALOG);
    pinMode(BOARD_LED_PIN,   OUTPUT);
  #endif 
  
  #ifdef ___ARDUINO
    #define BOARD_LED_PIN 13
    pinMode(BOARD_LED_PIN,   OUTPUT);
  #endif
  
  #ifdef ___TEENSY
    #define BOARD_LED_PIN 13
    pinMode(BOARD_LED_PIN, OUTPUT);
    pinMode(OLED_MOSI,  OUTPUT);
    pinMode(OLED_CLK,   OUTPUT);
    pinMode(OLED_DC,    OUTPUT);
    pinMode(OLED_RESET, OUTPUT);
    pinMode(OLED_CS,    OUTPUT);
    analogReadRes(12);

    SIM_SCGC6 |= SIM_SCGC6_PIT;           // CONFIGURE TIMERS
    SIM_SCGC6 |= SIM_SCGC6_FTM0;
    //PIT_MCR = 0x00;
    //NVIC_ENABLE_IRQ(IRQ_PIT_CH0);
  #endif    
  
  #ifdef ___OLED
    display.begin(SSD1306_SWITCHCAPVCC);
    display.setTextSize(1);
    display.clearDisplay();   // clears the screen and buffer    
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("radioModem");
    display.println(" by KF7IJB");
    display.display();
    delay(2000);
    display.clearDisplay();
    display.display();
  #endif    
}



void stop_adc()
{
  #ifdef ___MAPLE
    timer3.pause();
  #endif  
  
  #ifdef ___ARDUINO
     TCCR2B &= ~(1 << WGM21);                        // Clears WGM21 bit, stopping the timer
  #endif
  
  #ifdef ___TEENSY
    sampleTimer.end();
  #endif
  
}  



void start_adc()
{
  #ifdef ___MAPLE
    timer3.resume();
  #endif  
  
  #ifdef ___ARDUINO
     TCCR2B |= (1 << WGM21);                         // Sets WGM21 bit, starting the timer
  #endif  
  
  #ifdef ___TEENSY
    sampleTimer.begin(handler_adc, int(1000000/SAMPLE_RATE));
  #endif  
}



void setup()
{
  //Global intialization
  td = oldTd = 0;
  process = 0;
  sample_idx = 0; 

  io_init();  
  adc_init();
  baud_timer_init();
  atan_init();
  psk31_init();
  rtty_init();
  frontend_init();
  tdtl_init();
  
  start_adc();
  baud_timer_restart();
}



void loop()
{
  while(1) {  
  if(process == 0)
    continue;
    
    F16 kfx = F16sub(oldTd, kftau);
    F16 kfy = oldTd;
  
    uint16_t kx_idx = sample_idx + F16Toint(kfx);
    F15 kx_alpha = F16ToF15(kfx);
    
    uint16_t ky_idx = sample_idx;
    F15 ky_alpha = F16ToF15(kfy);
  
    
    F15 xt = F15add(
                    F15mul(F15neg(kx_alpha), sample[(kx_idx - 1) & (DELAY_LENGTH - 1)]),
                    F15mul(F15inc(kx_alpha), sample[kx_idx & (DELAY_LENGTH -1)])
                   );
                   
    F15 y  = F15add(
                    F15mul(F15neg(ky_alpha), sample[(ky_idx - 1) & (DELAY_LENGTH - 1)]),
                    F15mul(F15inc(ky_alpha), sample[ky_idx & (DELAY_LENGTH - 1)])
                   );

    e = atan_lookup(y, xt);
 
    F16 cd = F16unsafeMul(4, _D1, e);
 
    
    stop_adc();                    //Disable interrupts on this update
    td = F16add(td, F16neg(cd));
    start_adc();                   //resume interrupts after the update
    
    if(doRTTY)
    {
      rtty_process(e);
      process = 0;
    } 
    else if(doPSK)
    {
      process = 1;
      psk31_process(e);
      process = 0;
    }    
  }          
}


void tdtl_init() {
	const float pi = 3.14159265f;
	float Fs, Ts, psi, tau, Fmark, Fspace, F0, W0, T0, K1, G1;
	psi = pi / 3.0f;

	Fs = (float)SAMPLE_RATE;       // Sampling frequency
	Ts = (1.0f / Fs);              // Sampling period = 125µS if 8000Hz
	Fmark = 1275.0f;
	Fspace = 1445.0f;
	F0 = (Fmark + Fspace) / 2.0f;  // F0 = 1360Hz
	W0 = 2.0f * pi * F0;           // radian frequency of F0
	T0 = 1.0f / F0;

	tau = psi / W0;

	K1 = 1.0f;
	G1 = K1 / W0;

	_D0 = floatToF16(Fs * T0);
	_D1 = floatToF15(Fs * G1);
	kftau = floatToF16(Fs * tau);
	td = kftau;
}

void wtf()
{
  digitalWrite(BOARD_LED_PIN, HIGH);
}

void handler_adc()
{
  td = F16dec(td);                                  
  
  F15 input = (int)(analogRead(ANALOG_IN));
  input = (input - 0x800) << 4;
  //input = (input - 0x200) << 6;
  
  input = frontend_filter(input);
  
  sample_idx = (sample_idx + 1) & (DELAY_LENGTH - 1);
  sample[sample_idx] = input;

  if(td < 0) 
  {
    oldTd = td;
    td = F16add(td, _D0);
    process = 1;
  }	   
}

#ifdef ___ARDUINO
  ISR(TIMER2_COMPA_vect)
  {
    handler_adc();
  }
#endif
