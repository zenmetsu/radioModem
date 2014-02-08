#include "fractionaltypes.h"

#include "utility.h"
#include "float.h"
#include "math.h"

//Frontend bandpass filter centered around 1360Hz

//X coeffs: [n - 0] (0.000000, 0x0), [n - 1] (0.007846, 0x101), [n - 2] (-0.007253, 0xFF12), [n - 3] (0.007165, 0xEB), [n - 4] (0.000000, 0x0), 
//Y coeffs: [n - 0] (-0.691730, 0x588B), [n - 1] (1.000000, 0x8000), [n - 2] (-0.631693, 0x50DB), [n - 3] (0.313409, 0xD7E2), 

//X coeffs: [n - 0] (0.000000, 0x0), [n - 1] (0.009815, 0x142), [n - 2] (0.000000, 0x0), 
//Y coeffs: [n - 0] (1.000000, 0x8000), [n - 1] (-0.874318, 0x9016), [n - 2] (0.807649, 0x6761), 

//F15 xd1, xd2, xd3, yd1, yd2, yd3, yd4;
F15 xd1, yd1, yd2;	
//F15 xc1, xc2, xc3, yc1, yc2, yc3, yc4;
F15 xc1, yc1, yc2;

void frontend_init() {
	//xd1 = xd2 = xd3 = yd1 = yd2 = yd3 = yd4 = 0;
        xd1 = yd1 = yd2 = 0;
	
	xc1 = floatToF15(0.009815f);
	
	//yc1 = floatToF15(0.874318f);
	yc1 = floatToF15(0.915618f);
	//yc2 = floatToF15(-0.807649f);
	yc2 = floatToF15(-0.898693f);
}

F15 frontend_filter(F15 x) {
	F15 y = F15mul(xd1, xc1) + F15mul(yd1, yc1) + F15mul(yd2, yc2) ;
		
	xd1 = x;
	yd2 = yd1;
	yd1 = y;
        return y;
}
