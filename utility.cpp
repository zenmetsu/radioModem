#include "utility.h"

uint16 bitrev(uint16 in, uint8 bits) 
{
  uint16 out, marker;
  uint8 i;
	
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
