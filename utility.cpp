#include "utility.h"
#include <stdint.h>

uint16_t bitrev(uint16_t in, uint8_t bits) 
{
  uint16_t out, marker;
  uint8_t i;
	
  if(bits == 0) return 0;
	
  out = 0;
  marker = 0x1 << (bits - 1);
	
  for(i = 0; i < bits; i++) 
  {
    out >>= 1;
    out = out | ((in & marker) << i);
    marker >>= 1;
  }	
  return out;
}
