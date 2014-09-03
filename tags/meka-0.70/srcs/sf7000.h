//-----------------------------------------------------------------------------
// MEKA - sf7000.h
// SF-7000 (Super Control Station) Emulation - Headers
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

#define IPL_Enabled             (!(SF7000.Port_E6 & 0x40))
#define IPL_Disabled            (SF7000.Port_E6 & 0x40)

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

void    SF7000_Reset (void);
void    SF7000_IPL_Mapping_Update (void);

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------

typedef struct  s_sf7000
{
 byte           Port_E4, Port_E5, Port_E6, Port_E7;
 byte           Port_E8, Port_E9;
}               t_sf7000;

t_sf7000        SF7000;

// SF-7000 BIOS Image
byte *          BIOS_ROM_SF7000;

//-----------------------------------------------------------------------------
