#include "arctanapprox.h"
#include "math.h"

F16 F16pi, F16pi1By2, atanLookupTable[64];



void atan_init() 
{
  int i;
	
  const float pi = 3.14159265f;
  F16pi = floatToF16(pi);
  F16pi1By2 = floatToF16(pi / 2.0f);

  for(i = 0; i < 64; i++) atanLookupTable[i] = floatToF16(atanf(((float)i) / 4.0f));
}



uint8 count_leading_unused_bits(uint16 x) 
{
  uint8 i = 0;	
  uint8 sign = (x >> 15) & 0x1;
	
  if(sign == 0x1) x = ~x;
	
  x <<= 1;
	
  while((x & 0x8000) == 0 && i < 16)
  {
    i++;
    x <<= 1;
  }	
  return i;
}



uint8 count_trailing_unused_bits(uint16 x) 
{
  uint8 i = 0;	
  uint8 sign = (x >> 15) & 0x1;
	
  if(sign == 0x1) x = ~x;
	
  while((x & 0x0001) == 0 && i < 16) 
  {
    i++;
    x >>= 1;
  }	
  return i;
}



uint16 remove_leading_bits(uint8 bitcount, uint16 x) 
{
  return (x & 0x8000) | ((x << bitcount) & 0x7FFF);
}

F16 atan_lookup(F15 xi, F15 yi) 
{
  F15 _x, _y;
	
  //Handle the zeroes here
  if(yi == 0)
  {
    if(xi >= 0) 
    {
      return F16pi;
    } 
    else 
    {
      return F16neg(F16pi);
    }
  }	
        
  if(xi == 0)
  {
    if(yi > 0) 
    {
      return F16pi1By2;
    } 
    else
    {
      return F16neg(F16pi1By2);
    }
  }	
	
  _x = (xi < 0) ? F15neg(xi) : xi;
  _y = (yi < 0) ? F15neg(yi) : yi;
	
  F16 val = 0, valp = 0;
	
  uint16 i = _y / _x;
  uint16 r = _y % _x;
	
  uint8 us = count_leading_unused_bits(r);
  uint8 ls = count_leading_unused_bits(_x);
	
  _x >>= (15 - ls - 7);
	
  uint16 c = 0xFFFF;
	
  if(_x != 0) c = (r << us) / _x;
		
  c = (c << (15 - (us + (15 - ls - 7)))) - 1;
	
  F15 alpha = c, alphap;
	
  i = (i << 2) | ((alpha & 0x6000) >> 13);
  alpha = (alpha << 2) & 0x7FFF;
	
  alphap = F15neg(F15inc(alpha));
	
  if(i >= 64) 
  {
    val = F16pi1By2;
  }
  else
  {
    val = atanLookupTable[i];
    if(i + 1 < 64) valp = atanLookupTable[i + 1];
    else valp = F16pi1By2;
    val = F16add(F16unsafeMul(2, val, F15ToF16(alphap)), F16unsafeMul(2, valp, F15ToF16(alpha)));
  }
	
  if(xi > 0 && yi > 0) 
  {
    return val;
  }
  else if(xi > 0 && yi < 0)
  {
    return F16neg(val);
  }
  else if(xi < 0 && yi > 0)
  {
    return F16add(val, F16pi1By2);
  }
  else if(xi < 0 && yi < 0)
  {
    return F16neg(F16add(val, F16pi1By2));
  }
  return 0;
}
