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
uint8_t StringOrdering[6] ;
};




const ConfigLED ConfigLED_LED_IS_M280D_SAMSUNG_QUANTUMDOT = {
"QuantumDotM280",
true,
18000, // Samsung U28H750 config uses up 15.5w.  However, there is some error on the output current setting, so this should be ok.
50,
160, // Samsung U28H750 uses 102mA maximum 100% dutycycle, set conservative max at +60% limit
4,
3,
1.80905243888213,
2.85128739427457,
36978.3217866044,
13.9148883455194,
{1,2,3,4,0,0}
};

const ConfigLED ConfigLED_LED_IS_M280D_SAMSUNG_QUANTUMDOT_REVERSED = {
"QuantumDotM280",
true,
18000, // Samsung U28H750 config uses up 15.5w.  However, there is some error on the output current setting, so this should be ok.
50,
160, // Samsung U28H750 uses 102mA maximum 100% dutycycle, set conservative max at +60% limit
4,
3,
1.80905243888213,
2.85128739427457,
36978.3217866044,
13.9148883455194,
{4,3,2,1,0,0}
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
26.6,
{1,2,3,4,0,0}
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
13.3,
{1,2,3,4,0,0}
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
26.6,
{1,2,3,4,0,0}
};



#if LED_VERSION==LED_IS_M280DGJ
  #define TargetConfigLED ConfigLED_LED_IS_M280DGJ
#elif LED_VERSION==LED_IS_M280D_SAMSUNG_QUANTUMDOT
  #define TargetConfigLED ConfigLED_LED_IS_M280D_SAMSUNG_QUANTUMDOT
#elif LED_VERSION==LED_IS_M280D_SAMSUNG_QUANTUMDOT_REVERSED
  #define TargetConfigLED ConfigLED_LED_IS_M280D_SAMSUNG_QUANTUMDOT_REVERSED
#elif LED_VERSION==LED_IS_V390DK
  #define TargetConfigLED ConfigLED_LED_IS_V390DK
#elif LED_VERSION==LED_IS_M270HHF
  #define TargetConfigLED ConfigLED_LED_IS_M270HHF
#else
#error "Unsupported LED_VERSION"
#endif

#endif
