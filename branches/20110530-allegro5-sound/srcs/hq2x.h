//-----------------------------------------------------------------------------
// MEKA - hq2x.h
// HQ2X filter initialization and interface.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------

extern "C"
{
extern unsigned int   HQ2X_LUT16to32[65536];
extern unsigned int   HQ2X_RGBtoYUV[65536];
}

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

extern "C"
{
void    hq2x_16(unsigned char*, unsigned char*, int, int, int);
void    hq2x_32(unsigned char*, unsigned char*, int, int, int);
}

void    HQ2X_Init(void);

//-----------------------------------------------------------------------------
