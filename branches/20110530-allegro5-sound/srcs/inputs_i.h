//-----------------------------------------------------------------------------
// MEKA - inputs_i.h
// Inputs Initialization - Headers
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

void    Inputs_Init             (void);
void    Inputs_Close            (void);

#ifdef MEKA_JOY
void    Inputs_Joystick_Init    (void);
void    Inputs_Joystick_Close   (void);
#endif

//-----------------------------------------------------------------------------

