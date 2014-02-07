#ifndef FRACTIONALTYPES_H_
#define FRACTIONALTYPES_H_

//This should be a 16 bit signed integer type
#define F15 signed int

//This should be a 32 bit signed integer type
#define F16 long signed int

#define F15inc(x) ((x) ^ 0x8000)
#define F15dec(x) F15inc(x)
#define F15neg(x) (((x) ^ 0xFFFF) + 1)
#define F15add(x, y) ((x) + (y))
#define F15sub(x, y) ((x) - (y))
#define F15unsafeMul(s, x, y) (F15)((((x) >> s) * ((y) >> s)) >> (15 - 2 * s));
#define F15mul(x, y) ((F15)(((signed long int)(x) * (signed long int)(y)) >> 15))

#define F16inc(x) ((x) + 0x00010000)
#define F16dec(x) ((x) - 0x00010000)
#define F16neg(x) (((x) ^ 0xFFFFFFFF) + 1)
#define F16add(x, y) ((x) + (y))
#define F16sub(x, y) ((x) - (y))
#define F16unsafeMul(s, x, y) (F16)((((x) >> s) * ((y) >> s)) >> (16 - 2 * s))
#define F16mul(x, y) ((F16)(((signed long long int)(x) * (signed long long int)(y)) >> 16))

#define floatToF15(x) ((F15)((x) * 32768.0f))
#define floatToF16(x) ((F16)((x) * 65536.0f))
#define F15ToF16(x) (((F16)(x)) << 1)
#define F16ToF15(x) ((F15)((((x) >> 1) & 0x7FFF) | ((x) >> 16 & 0x8000)))
#define F16Toint(x) ((F15)(((x) < 0) ? (((x) >> 16) + 1) : (x) >> 16))

/* Useful test code
		F15 a = 0x4000, b = 0x4000, c, d, e, f;
		c = F15mul(a, b);
		d = F15inc(a);
		e = F15mul(d, d);
		f = F15mul(a, d);
		a = F15add(F15mul(e, e), f);
		
		c = F16neg(c);
		d = F16neg(c);
		
		F16 ta = 0x4000, tb = 0x4000, tc, td, te, tf;
		tc = F16mul(ta, tb);
		td = F16inc(ta);
		te = F16mul(td, td);
		tf = F16mul(ta, td);
		ta = F16add(F16mul(te, te), tf);
		
		ta = tb = tc = td = 0;
		ta = F15ToF16(a);
		tb = F15ToF16(b);
		a = 0;
		b = 0;
		a = F16ToF15(ta);
		b = F16ToF15(tb);
		
		tc = 0x40000000;
		td = 0x00004000;
		
		ta = F16mul(tc, td);
		tc = F16neg(tc);
		
		ta = F16mul(tc, td);
		td = F16neg(tc);
*/

#endif
