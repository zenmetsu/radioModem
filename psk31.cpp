#include "configuration.h"
#include "psk31.h"
#include "baudtimer.h"

#ifdef ___SERIAL
  #include <WProgram.h>
  #include <HardwareSerial.h>
#endif

#ifdef ___OLED
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>

  extern Adafruit_SSD1306 display;
#endif




#define sbit(port, pin) port->regs->BSRR = BIT(pin)
#define cbit(port, pin) port->regs->BRR = BIT(pin)




//PSK31 global communicator variables
#define psk0 0
#define psk1 1
#define pskAny 2
#define pskNone 3

//Integral for PSK
#define IE 4
F16 e_d[IE];
uint16_t e_d_idx;

F16 _ie;

//Useful PSK Constants
int16_t pskSymbolTime;
int16_t pskSwitchTime;
	
F16 pi, piErr;

//PSK Decode state variables
int32_t pskDam;
uint16_t pskWatch, pskSpace, pskCharacter, pskSymbolCount;


uint16_t PSK_TO_PSK_IDX[128] = {
0b1,
0b11,
0b101,
0b111,
0b1011,
0b1101,
0b1111,
0b10101,
0b10111,
0b11011,
0b11101,
0b11111,
0b101011,
0b101101,
0b101111,
0b110101,
0b110111,
0b111011,
0b111101,
0b111111,
0b1010101,
0b1010111,
0b1011011,
0b1011101,
0b1011111,
0b1101011,
0b1101101,
0b1101111,
0b1110101,
0b1110111,
0b1111011,
0b1111101,
0b1111111,
0b10101011,
0b10101101,
0b10101111,
0b10110101,
0b10110111,
0b10111011,
0b10111101,
0b10111111,
0b11010101,
0b11010111,
0b11011011,
0b11011101,
0b11011111,
0b11101011,
0b11101101,
0b11101111,
0b11110101,
0b11110111,
0b11111011,
0b11111101,
0b11111111,
0b101010101,
0b101010111,
0b101011011,
0b101011101,
0b101011111,
0b101101011,
0b101101101,
0b101101111,
0b101110101,
0b101110111,
0b101111011,
0b101111101,
0b101111111,
0b110101011,
0b110101101,
0b110101111,
0b110110101,
0b110110111,
0b110111011,
0b110111101,
0b110111111,
0b111010101,
0b111010111,
0b111011011,
0b111011101,
0b111011111,
0b111101011,
0b111101101,
0b111101111,
0b111110101,
0b111110111,
0b111111011,
0b111111101,
0b111111111,
0b1010101011,
0b1010101101,
0b1010101111,
0b1010110101,
0b1010110111,
0b1010111011,
0b1010111101,
0b1010111111,
0b1011010101,
0b1011010111,
0b1011011011,
0b1011011101,
0b1011011111, 
0b1011101011,
0b1011101101,
0b1011101111,
0b1011110101,
0b1011110111, 
0b1011111011,
0b1011111101,
0b1011111111, 
0b1101010101,
0b1101010111,
0b1101011011,
0b1101011101,
0b1101011111,
0b1101101011, 
0b1101101101,
0b1101101111,
0b1101110101,
0b1101110111, 
0b1101111011,
0b1101111101,
0b1101111111,
0b1110101011,
0b1110101101,
0b1110101111,
0b1110110101,
0b1110110111,
0b1110111011
};

uint16_t PSK_IDX_TO_ASCII[128] = {
0x20, // SP
0x65, // 'e'
0x74, // 't'
0x6f, // 'o'
0x61, // 'a'
0x69, // 'i'
0x6e, // 'n'
0x72, // 'r'
0x73, // 's'
0x6c, // 'l'
0x0a, // LF
0x0d, // CR
0x68, // 'h'
0x64, // 'd'
0x63, // 'c'
0x2d, // '-'
0x75, //HD_low_u,
0x6d, //HD_low_m,
0x66, //HD_low_f,
0x70, //HD_low_p,
0x3d, //HD_equals,
0x2e, //HD_period,
0x67, //HD_low_g,
0x79, //HD_low_y,
0x62, //HD_low_b,
0x77, //HD_low_w,
0x54, //HD_cap_T,
0x53, //HD_cap_S,
0x2c, //HD_comma,
0x45, //HD_cap_E,
0x76, //HD_low_v,
0x41, //HD_cap_A,
0x49, //HD_cap_I,
0x4f, //HD_cap_O,
0x43, //HD_cap_C,
0x52, //HD_cap_R,
0x44, //HD_cap_D,
0x30, //HD_zero,
0x4d, //HD_cap_M,
0x31, //HD_one,
0x6b, //HD_low_k,
0x50, //HD_cap_P,
0x4c, //HD_cap_L,
0x46, //HD_cap_F,
0x4e, //HD_cap_N,
0x78, //HD_low_x,
0x42, //HD_cap_B,
0x32, //HD_two,
0x09, // HT Tab
0x3a, //HD_colon,
0x29, //HD_rightparenthesis,
0x28, //HD_leftparenthesis,
0x47, //HD_cap_G,
0x33, //HD_three,
0x48, //HD_cap_H,
0x55, //HD_cap_U,
0x35, //HD_five,
0x57, //HD_cap_W,
0x22, //HD_doublequote,
0x36, //HD_six,
0x5f, //HD_underscore,
0x2a, //HD_multiply,
0x58, //HD_cap_X,
0x34, //HD_four,
0x59, //HD_cap_Y,
0x4b, //HD_cap_K,
0x27, //HD_apostrophe,
0x38, //HD_eight,
0x37, //HD_seven,
0x2f, //HD_divide, ///
0x56, //HD_cap_V,
0x39, //HD_nine,
0x7c, // '|'
0x3b, //HD_semicolon,
0x71, //HD_low_q,
0x7a, //HD_low_z,
0x3e, //HD_greaterthan,
0x24, //HD_dollar,
0x51, //HD_cap_Q,
0x2b, //HD_add,
0x6a, //HD_low_j,
0x3c, //HD_lessthan,
0x5c, // '\'
0x23, //HD_pound,
0x5b, //HD_leftbracket,
0x5d, //HD_rightbracket,
0x4A, //HD_cap_J,
0x21, //HD_exclamation,
0x00, // NUL
0x5A, //HD_cap_Z, 1010101101
0x3f, //HD_question,
0x7d, //HD_rightcurlybracket,
0x7b, //HD_leftcurlybracket,
0x26, //HD_and,
0x40, //HD_at,
0x5e, //HD_power,
0x25, //HD_percent,
0x7e, // '~'
0x01, // SOH
0x0c, // FF
0x60, // '`'
0x04, // EOT
0x02, // STX
0x06, // ACK
0x11, // DC1
0x10, // DLE
0x1e, // RS
0x07, // BEL
0x08, // BS
0x1b, // ESC
0x17, // ETB
0x14, // DC4
0x1c, // FS
0x05, // ENQ
0x15, // NAK
0x16, // SYN
0x0b, // VT
0x0e, // SO
0x03, // ETX
0x18, // CAN
0x19, // EM
0x1f, // US
0x0f, // SI
0x12, // DC2
0x13, // DC3
0x7f, // DEL
0x1a, // SUB
0x1d, // GS
};

void psk31_print(uint16_t character);

void psk31_init() {
	//pskSymbolTime = (uint16)((((float)Fosc / 2.0) / 8.0) / 31.25); // The timer is driven by Fosc / 2 through a 256 prescaler and we are looking for 31.25 baud symbols                           
        pskSymbolTime = (uint16_t)((float)(72000000/(72*31.25)));             // 32000 ticks = 32mS                                            
	pskSwitchTime = (uint16_t)(pskSymbolTime / 4);                                  

	pi = floatToF16(3.14159265f);
	piErr = pi - floatToF16(3.14159265f / 4.0f);
	
	pskDam = 0;
	pskWatch = psk1;
	pskSpace = 0;
	pskCharacter = 0;
	pskSymbolCount = 0;
	
	_ie = 0;
	e_d_idx = 0;
	
	for(int i = 0; i < IE; i++)
		e_d[i] = 0;
    #ifdef ___SERIAL
      Serial.begin(9600);
    #endif
}



void psk31_process(F16 e) 
{      
  uint16_t Telaps = baud_time_get();                               // elapsed time

  _ie += e - e_d[e_d_idx];                                       // ie will be the delta from this reading and the prior one
  e_d[e_d_idx] = e;                                              // e_d is delay buffer, store e as e_d[0]... see next statement regarding index
  e_d_idx = (e_d_idx + 1) & (IE - 1);                            // increment e_d_idx and reset on overflow

  F16 tie = (_ie < 0) ? (_ie ^ 0xFFFFFFFF) + 1 : _ie;            // tie = abs(_ie);
		
  pskDam += Telaps;                                              // pskDam (psk Symbol Time) incremented by elapsed time

  if(pskDam >= pskSwitchTime && tie >= piErr)                    // if pskDam >= switchTime and change exceeds piErr (phase switch happened)
  {
    pskDam = 0;
    if(pskWatch == pskAny | pskWatch == psk0)                    // if expecting any or zero
    {
      pskCharacter = (pskCharacter << 1) & 0xFFFE;               // pskCharacter shifted left and zero appended to end
      pskWatch = psk1;                                           // expect a 1... cannot have two zeros back-to-back since it ends character
      pskSymbolCount++;                                          // increment pskSymbolCount
    }  
    else                                                         // else phase changed while expecting 1, so character is ended
    {
      pskCharacter >>= 1;                                        // right shift once to trim trailing zero
      if(pskSymbolCount > 0)                                     // if symbol count is non-zero
      {
        psk31_print(pskCharacter);                               // display character
        if(pskCharacter != 0b1) pskSpace = 0;
      } 
      else if(!pskSpace) 
      {
        psk31_print(0b1);
        pskSpace = 1;
      }		
      pskCharacter = 0;
      pskWatch = psk1;
      pskSymbolCount = 0;
    }	
  }
	
  if(pskDam >= (pskSymbolTime + (pskSwitchTime/4)))              // no phase switch... thus symbol is a 1  NOTE: I had to divide the switch time
  {                                                              // by 4 in order to avoid dropping a '1' symbol when more than 4 in a row were received.
                                                                 // however if this is done globally, like when receiving a '0' symbol, then the code will
                                                                 // generate a space after each character. Probably a flaw in my PSK logic.  :(
                                                                 
    pskCharacter = (pskCharacter << 1) | 0x1;                    // shift values left and append a 1
    pskDam = 0;                                                  // reset symbol time
    pskWatch = pskAny;                                           // expect anything next symbol
    pskSymbolCount++;                                            // increase symbol count
  }
}



void psk31_print(uint16_t character) 
{ 
  int16_t idxOffset = 128 / 2;
  uint16_t idx = idxOffset;
	
  while(idxOffset > 0) 
  {		
    if(idx > 128) return;
			
    uint16_t val = PSK_TO_PSK_IDX[idx];		
    idxOffset >>= 1;
		
    if(val > character) idx -= idxOffset;
    else if(val < character) idx += idxOffset;
    else break;
			
    if(idxOffset == 0) 
    {
      val = PSK_TO_PSK_IDX[idx];
      if(val > character && idx == 1) 
      {
	idx = 0;
	break;
      }
      return;
    }	
  }
  #ifdef ___OLED
    display.print((char)PSK_IDX_TO_ASCII[idx]);
    display.display();
  #endif   
  
  #ifdef ___SERIAL
    Serial.print((char)PSK_IDX_TO_ASCII[idx]);
  #endif
}

