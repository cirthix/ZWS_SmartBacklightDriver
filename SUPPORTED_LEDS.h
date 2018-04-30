#ifndef SUPPORTEDLEDS_H
#define SUPPORTEDLEDS_H




struct ConfigLED {
char Name[16];
boolean SupportsScanning; // True if the LEDs are physically arranged to support scanning mode
uint16_t PowerLimit; // milliwatts, total including all strings
uint16_t CurrentMinimum;  // milliamps
uint16_t CurrentMaximum; // milliamps
uint8_t NumberStrings; // number of strings
float DeratingFactor; // Above the maximum continuious current, additional current 'costs' n-times more
float CurrentToBrightnessIntercept ;
float CurrentToBrightnessSlope  ;
float CurrentToVoltageIntercept  ;
float CurrentToVoltageSlope  ;
};





const ConfigLED ConfigLED_LED_IS_M280DGJ = {
"ExtraSafe M280D",
true,
22500,
50,
200, // Can safely crank this up to 350mA after additonal testing.  Peak pulse value is 450mA from v390dk1 datasheet.  v390dk1 uses the same LEDs but with eight strings instead of four.
4,
3,
39.929,
1.6,  // If this is over 1.6, it breaks the system.  wtf???
32190,
26.6
};


const ConfigLED ConfigLED_LED_IS_V390DK = {
"INNOLUX V390DK",
true,
45000,
50,  // 145*2 is the maximum 100% point
200, // Can safely crank this up to 350mA after additonal testing.  Peak pulse value is 9000mA from v390dk1 datasheet (two parallel strings)
4, // 8 strings, but put two in paralell due to TPS6196 only supporting up to 6.
2,  // Don't need to do accelerated derating since we can't push the leds to their limits anyway
39.929,
0.838345,
32190,
13.3
};

const ConfigLED ConfigLED_LED_IS_M270HHF = {
"INNOLUX M270HHF",
false,
20000,
50,
320,
4,
3,
39.929,
1.67669,
32190,
26.6
};



#if LED_VERSION==LED_IS_M280DGJ
  #define TargetConfigLED ConfigLED_LED_IS_M280DGJ
#elif LED_VERSION==LED_IS_V390DK
  #define TargetConfigLED ConfigLED_LED_IS_V390DK
#elif LED_VERSION==LED_IS_M270HHF
  #define TargetConfigLED ConfigLED_LED_IS_M270HHF
#else
#error "Unsupported LED_VERSION"
#endif

#endif
