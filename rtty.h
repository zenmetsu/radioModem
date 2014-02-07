#ifndef RTTY_H_
#define RTTY_H_

#include "utility.h"
#include "fractionaltypes.h"

#define RTTY_NULL 0b10000000
#define RTTY_LF 0b10000001
#define RTTY_SP 0b10000010
#define RTTY_CR 0b10000011
#define RTTY_FIGS 0b10000100
#define RTTY_LTRS 0b10000101
#define RTTY_ENQ 0b10000110
#define RTTY_BEL 0b10000111

//RTTY decode utility functions
void rtty_init();
void rtty_process(F16 e);

#endif
