#ifndef SUPPORTEDBUTTONBOARDS_H
#define SUPPORTEDBUTTONBOARDS_H


#if BUTTONBOARD_VERSION==BUTTONBOARD_IS_ZISWORKS
#define BUTTONBOARD_POWER_BUTTON  BUTTONBOARD_BUTTON_A_ANALOG
#define BUTTONBOARD_A_BUTTON  BUTTONBOARD_BUTTON_B_ANALOG
#define BUTTONBOARD_B_BUTTON  BUTTONBOARD_BUTTON_C_ANALOG
#define LED_RGB  1 // Shared with the serial output of UART
// These values are for a system with a 1kohm pullup and pulldowns in series with buttons, having values of 0, 4.02k, 2k, and 1k.
const uint16_t abserror          = 15;
const float relerror             = 0.10;
const uint16_t cutoff_zero       = 0;
const uint16_t cutoff_zero_low   = cutoff_zero;
const uint16_t cutoff_zero_high  = cutoff_zero+(abserror+cutoff_zero*relerror);
const uint16_t cutoff_one        = 512;
const uint16_t cutoff_one_low    = cutoff_one-(abserror+cutoff_one*relerror);
const uint16_t cutoff_one_high   = cutoff_one+(abserror+cutoff_one*relerror);
const uint16_t cutoff_two        = 682;
const uint16_t cutoff_two_low    = cutoff_two-(abserror+cutoff_two*relerror);
const uint16_t cutoff_two_high   = cutoff_two+(abserror+cutoff_two*relerror);
const uint16_t cutoff_three      = 819;
const uint16_t cutoff_three_low  = cutoff_three-(abserror+cutoff_three*relerror);
const uint16_t cutoff_three_high = cutoff_three+(abserror+cutoff_three*relerror);

#elif BUTTONBOARD_VERSION==BUTTONBOARD_IS_SAMSUNG
#define BUTTONBOARD_POWER_BUTTON  BUTTONBOARD_BUTTON_A_ANALOG
#define BUTTONBOARD_C_BUTTON  BUTTONBOARD_BUTTON_C_ANALOG
#define LED_STATUS  BUTTONBOARD_BUTTON_B_ANALOG
// These values are for a system with a 1kohm pullup and pulldowns in series with buttons, having values of 22, 3.264k, 9.06k, and 32.75k.
const uint16_t abserror          = 11;
const float relerror             = 0;
const uint16_t cutoff_zero       = 22;
const uint16_t cutoff_zero_low   = cutoff_zero-(abserror+cutoff_zero*relerror);;
const uint16_t cutoff_zero_high  = cutoff_zero+(abserror+cutoff_zero*relerror);
const uint16_t cutoff_one        = 783;
const uint16_t cutoff_one_low    = cutoff_one-(abserror+cutoff_one*relerror);
const uint16_t cutoff_one_high   = cutoff_one+(abserror+cutoff_one*relerror);
const uint16_t cutoff_two        = 921;
const uint16_t cutoff_two_low    = cutoff_two-(abserror+cutoff_two*relerror);
const uint16_t cutoff_two_high   = cutoff_two+(abserror+cutoff_two*relerror);
const uint16_t cutoff_three      = 993;
const uint16_t cutoff_three_low  = cutoff_three-(abserror+cutoff_three*relerror);
const uint16_t cutoff_three_high = cutoff_three+(abserror+cutoff_three*relerror);

#elif BUTTONBOARD_VERSION==BUTTONBOARD_IS_SAMSUNG_WITH_RGBLED
#define BUTTONBOARD_POWER_BUTTON  BUTTONBOARD_BUTTON_A_ANALOG
#define BUTTONBOARD_C_BUTTON  BUTTONBOARD_BUTTON_C_ANALOG
#define LED_RGB  1 // Shared with the serial output of UART
// These values are for a system with a 1kohm pullup and pulldowns in series with buttons, having values of 22, 3.264k, 9.06k, and 32.75k.
const uint16_t abserror          = 11;
const float relerror             = 0;
const uint16_t cutoff_zero       = 22;
const uint16_t cutoff_zero_low   = cutoff_zero-(abserror+cutoff_zero*relerror);;
const uint16_t cutoff_zero_high  = cutoff_zero+(abserror+cutoff_zero*relerror);
const uint16_t cutoff_one        = 783;
const uint16_t cutoff_one_low    = cutoff_one-(abserror+cutoff_one*relerror);
const uint16_t cutoff_one_high   = cutoff_one+(abserror+cutoff_one*relerror);
const uint16_t cutoff_two        = 921;
const uint16_t cutoff_two_low    = cutoff_two-(abserror+cutoff_two*relerror);
const uint16_t cutoff_two_high   = cutoff_two+(abserror+cutoff_two*relerror);
const uint16_t cutoff_three      = 993;
const uint16_t cutoff_three_low  = cutoff_three-(abserror+cutoff_three*relerror);
const uint16_t cutoff_three_high = cutoff_three+(abserror+cutoff_three*relerror);
#else
  #error "Unsupported BUTTONBOARD_VERSION"
#endif



#endif


