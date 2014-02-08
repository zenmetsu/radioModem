#include "configuration.h"
#include <stdint.h>

#ifdef ___MAPLE
  #include <libmaple_types.h>
#endif

#include "rtty.h"
#include "baudtimer.h"
#include "math.h"

#ifndef atanf
  #define expf exp
#endif

//RTTY notes:
//RTTYrite has a " in stead of a +, we use that.
//Also, it sends BEL in place of '.

const unsigned int RTTY_LETTERS_TO_ASCII[32] = {
0x80, //RTTY_NULL,//0x00
0x45, //HD_cap_E,//0x01
0x81, //RTTY_LF,//0x02
0x41, //HD_cap_A,//0x03
0x82, //RTTY_SP,//0x04
0x53, //HD_cap_S,//0x05
0x49, //HD_cap_I,//0x06
0x55, //HD_cap_U,//0x07
0x83, //RTTY_CR,//0x08
0x44, //HD_cap_D,//0x09
0x52, //HD_cap_R,//0x0A
0x4a, //HD_cap_J,//0x0B
0x4e, //HD_cap_N,//0x0C
0x46, //HD_cap_F,//0x0D
0x43, //HD_cap_C,//0x0E
0x4b, //HD_cap_K,//0x0F
0x54, //HD_cap_T,//0x10
0x5a, //HD_cap_Z,//0x11
0x4c, //HD_cap_L,//0x12
0x57, //HD_cap_W,//0x13
0x48, //HD_cap_H,//0x14
0x59, //HD_cap_Y,//0x15
0x50, //HD_cap_P,//0x16
0x51, //HD_cap_Q,//0x17
0x4f, //HD_cap_O,//0x18
0x42, //HD_cap_B,//0x19
0x47, //HD_cap_G,//0x1A
0x84, //RTTY_FIGS,//0x1B
0x4d, //HD_cap_M,//0x1C
0x58, //HD_cap_X,//0x1D
0x59, //HD_cap_V,//0x1E
0x85, //RTTY_LTRS//0x1F
};

const unsigned int RTTY_FIGURES_TO_ASCII[32] = {
0x80, //RTTY_NULL,//0x00
0x33, //HD_three,//0x01
0x81, //RTTY_LF,//0x02
0x2d, //HD_subtract,//0x03
0x82, //RTTY_SP,//0x04
0x27, //HD_apostrophe,//0x05
0x38, //HD_eight,//0x06
0x37, //HD_seven,//0x07
0x83, //RTTY_CR,//0x08
0x86, //RTTY_ENQ,//0x09
0x34, //HD_four,//0x0A
0x87, //RTTY_BEL,//0x0B
0x2c, //HD_comma,//0x0C
0x21, //HD_exclamation,//0x0D
0x3a, //HD_colon,//0x0E
0x28, //HD_leftparenthesis,//0x0F
0x35, //HD_five,//0x10
0x22, //HD_doublequote,//0x11 //Should be HD_add, but RTTYrite is non-standard
0x29, //HD_rightparenthesis,//0x12
0x32, //HD_two,//0x13
0x23, //HD_pound,//0x14
0x36, //HD_six,//0x15
0x30, //HD_zero,//0x16
0x31, //HD_one,//0x17
0x39, //HD_nine,//0x18
0x3f, //HD_question,//0x19
0x26, //HD_and,//0x1A
0x84, //RTTY_FIGS,//0x1B
0x2e, //HD_period,//0x1C
0x2f, //HD_divide,//0x1D
0x3b, //HD_semicolon,//0x1E
0x85, //RTTY_LTRS//0x1F
};

uint16_t validateRTTY(uint16_t character);
void printRTTY(uint16_t character);

//RTTY global communicator variables
#define rttyNone 2
#define rttyMark 1
#define rttySpace 0

unsigned int rtty_mode = RTTY_LTRS;

//Useful RTTY Constants
int16_t rttySymbolTime;
int16_t rttySwitchTime;
	
//RTTY Decode state variables
int32_t rttyMarkTime, rttySpaceTime,
	rttyDecodeSymbol, rttyDam;
		
uint16_t rttyCurrentSymbol, rttyProcessMark, rttyProcessSpace,
	rttyCharacter, rttySymbolCount;

F16 lp_alpha, lp_beta;
F16 e_lp;

void rtty_init() {
	rttySymbolTime = (uint16_t)((((float)Fosc / 2.0) / 8.0) / 45.45); // The timer is driven by Fosc / 2 through a 256 prescaler and we are looking for 45.45 baud symbols
	rttySwitchTime = rttySymbolTime / 4;
	
	rttyMarkTime = 0;
	rttySpaceTime = 0;
	rttyDecodeSymbol = rttyMark;
	rttyDam = 0;
	
	rttyCurrentSymbol = rttyNone;
	rttyProcessMark = 0;
	rttyProcessSpace = 0;
	rttyCharacter = 0;
	rttySymbolCount = 0;
	
	//Initialize phase detector DSP
	//Low pass filter for RTTY
	float alpha_float = 1500.0f;
	float beta_float = expf(-1.0f * alpha_float * 1.0f / ((float)SAMPLE_RATE));
	
	lp_alpha = floatToF16(alpha_float * 1.0f / ((float)SAMPLE_RATE));
	lp_beta = floatToF16(beta_float);
}

/*F16 tmp[1024];
uint16 idxtmp = 0;*/

void rtty_process(F16 e) {
	uint16_t Telaps = baud_time_get();
	
	/*tmp[idxtmp] = e;
	if(idxtmp > 1023)
		idxtmp = 0;
	else
		idxtmp++;*/
		
	e_lp = F16add(F16unsafeMul(4, lp_beta, e_lp), F16unsafeMul(4, lp_alpha, e));
		
	if(e_lp <= 0)
		rttyCurrentSymbol = rttyMark;
	else if(e_lp > 0)
		rttyCurrentSymbol = rttySpace;
			
	//Watch for a symbol switch
	//  If an unexpected signal detection change occurs, track it to see if real
	if(rttyCurrentSymbol == rttyDecodeSymbol) {
		//If it turns out to be fake, do nothing (because time added to right bin anyway)
		rttyDam = 0;
	} else {
		//If it turns out to be real, evaluate other timer and switch decode symbol 
		rttyDam += Telaps;
		
		if(rttyDam > rttySwitchTime) {
			//If need to switch symbols, evaluate the old decodeSymbol before updating to new
			switch(rttyDecodeSymbol) {
				case rttyMark:
					if(rttyCurrentSymbol == rttySpace) rttySpaceTime = rttyDam - Telaps; //The '- Telaps' term avoids double adding
					rttyMarkTime -= rttyDam - Telaps;
					rttyProcessMark = 1;
					break;
				case rttySpace:
					if(rttyCurrentSymbol == rttyMark) rttyMarkTime = rttyDam - Telaps;
					rttySpaceTime -= rttyDam - Telaps;
					rttyProcessSpace = 1;
					break;
			}

			//Update to new symbol
			rttyDecodeSymbol = rttyCurrentSymbol;

			//Reset the dam
			rttyDam = 0;
		}	
	}
	
	switch(rttyDecodeSymbol) {
		case rttyMark:
			rttyMarkTime += Telaps;
			break;
		case rttySpace:
			rttySpaceTime += Telaps;
			break;
	}
			
	if(rttyProcessMark) {
		while(rttyMarkTime > (rttySymbolTime >> 1)) {
			rttyCharacter = (rttyCharacter << 1) | 0x0001;
					
			rttyMarkTime -= rttySymbolTime;
			rttySymbolCount++;
		}
		
		rttyMarkTime = 0;
		rttyProcessMark = 0;
	}
			
	if(rttyProcessSpace) {
		while(rttySpaceTime > (rttySymbolTime >> 1)) {
			rttyCharacter = (rttyCharacter << 1) & 0xFFFE;
					
			rttySpaceTime -= rttySymbolTime;
			rttySymbolCount++;
		}
		
		rttySpaceTime = 0;
		rttyProcessSpace = 0;
	}
	
	while(rttySymbolCount >= 8) {
		uint16_t overshoot = rttySymbolCount - 8;
		uint16_t adjustedCharacter = rttyCharacter >> overshoot;
		
		if(validateRTTY(adjustedCharacter)) {
			adjustedCharacter = bitrev((adjustedCharacter >> 2) & 0x1F, 5);
			
			rttySymbolCount -= 8;
			
			//printRTTY(adjustedCharacter);
		} else
			rttySymbolCount--;
	}			
}

uint16_t validateRTTY(uint16_t character) {
	uint16_t stopBits = character & 0x0003, startBit = (character & 0x0080) ^ 0x0080;
	
	if((startBit | stopBits) == 0x0083)
		return 1;
	else
		return 0;
}

void printRTTY(uint16_t character) {
	switch(rtty_mode) {
		case RTTY_LTRS:
			character = RTTY_LETTERS_TO_ASCII[character];
			break;
		case RTTY_FIGS:
			character = RTTY_FIGURES_TO_ASCII[character];
			break;
	}

	switch(character) {
		case (RTTY_NULL):
			//jxw printDisplay(0x20);
			break;
		case (RTTY_LF):
			break;
		case (RTTY_SP):
			//jxw printDisplay(0x20);
			break;
		case (RTTY_CR):
			break;
		case (RTTY_FIGS):
			rtty_mode = RTTY_FIGS;
			break;
		case (RTTY_LTRS):
			rtty_mode = RTTY_LTRS;
			break;
		case (RTTY_ENQ):
			//jxw printDisplay(0x20);
			//jxw printDisplay(0x65);
			//jxw printDisplay(0x20);
			break;
		case (RTTY_BEL):
			//jxw printDisplay(0x20);
			//jxw printDisplay(0x62);
			//jxw printDisplay(0x20);
			break;
		default:
			//jxw printDisplay(character);
			break;
	}
}
