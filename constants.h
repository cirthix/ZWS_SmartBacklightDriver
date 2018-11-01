
#ifndef CONSTANTS_h
#define CONSTANTS_h

#include <Arduino.h>
#include <avr/wdt.h>


#ifndef ENABLED
#define ENABLED 1
#endif

#ifndef DISABLED
#define DISABLED 0
#endif

const uint8_t PWM_MAX = 255;
const uint8_t ADIM_MAX = 127;
const uint32_t SERIAL_BAUD = 9600;
const uint32_t SLAVESERIALSPEED = 38400;
const uint32_t OVERCLOCKED_SPEED = 8000000 ;

#define OVERCLOCKING DISABLED
#define SERIAL_INTERFACE ENABLED // If the serial interface is disabled, the D0 and D1 pins can be used for the buttonboard LEDs
#define SERIAL_DEBUGGING_OUTPUT ENABLED
#define SERIAL_COMMANDS_SIMPLE ENABLED // ENABLE THIS TO ALLOW VIRTUAL BUTTON PRESSES VIA SERIAL PORT (THIS FEATURE IS USEFUL FOR DEBUGGING)
#define SERIAL_COMMANDS_EXTENDED ENABLED // ENABLE THIS TO ALLOW COMPLEX SERIAL COMMANDS 



// conditional debugging
#if (SERIAL_DEBUGGING_OUTPUT == ENABLED)
#define SerialDebug(x)      Serial.print(x);   wdt_reset(); Serial.flush();
#define SerialDebugln(x)    Serial.println(x); wdt_reset(); Serial.flush();
#define SerialWrite(x)      Serial.write(x);   wdt_reset(); Serial.flush();
#define SerialFlush()       Serial.flush()
#else
#define SerialDebug(x)      ((void) 0)
#define SerialDebugln(x)    ((void) 0)
#define SerialWrite(x)      ((void) 0)
#define SerialFlush()       ((void) 0)
#endif




#define MY_WATCHDOG_TIMEOUT WDTO_60MS
#define I2C_TIMEOUT 10
#define MY_IIC_SPEED_DEFAULT 100 // Speed in KHz
#define MY_IIC_SPEED IIC_SPEED_DEFAULT
#define MY_IIC_ADDRESS 0x35

#define BOARD_IS_ZWS_SMART_BLDRIVER_REV1  1
#define BOARD_IS_ZWS_SMART_BLDRIVER_REV1_SCAN_HACK 2
#define BOARD_IS_ZWS_SMART_BLDRIVER_REV1_1  3

#define BUTTONBOARD_IS_ZWS_6PIN_9BUTTON_RG_LED     3
#define BUTTONBOARD_IS_ZWS_6PIN_8BUTTON_RGB_LED    4
#define BUTTONBOARD_IS_ZWS_7PIN_9BUTTON_RGB_LED    5

// Note: Quantum dot support has just been aded.  It has not been tested much.  Do not use this option yet
#define LED_IS_M280D_SAMSUNG_QUANTUMDOT 6
#define LED_IS_M280DGJ    7
#define LED_IS_V390DK     8
#define LED_IS_M270HHF    9

//////////////////////////////////////////////////////////////////////// CHANGE SYSTEM CONFIGURATION PARAMETERS HERE ////////////////////////////////////////////////////////////////////////
#define BOARD_VERSION BOARD_IS_ZWS_SMART_BLDRIVER_REV1_1
#define BUTTONBOARD_VERSION BUTTONBOARD_IS_ZWS_6PIN_9BUTTON_RGB_LED
#define LED_VERSION LED_IS_M280D_SAMSUNG_QUANTUMDOT
//////////////////////////////////////////////////////////////////////// CHANGE SYSTEM CONFIGURATION PARAMETERS HERE ////////////////////////////////////////////////////////////////////////

#include "SUPPORTED_BOARDS.h"
#include "SUPPORTED_BUTTONBOARDS.h"
#include "SUPPORTED_LEDS.h"


// Operational constants
const uint8_t TargetPowerSaveSHUTDOWN = 0; // Shutdown disables the dp recievers
const uint8_t TargetPowerSaveLOWPOWER = 1; // Lowpower mode disables the fpga, and by extension, the backlight
const uint8_t TargetPowerSaveFULLY_ON = 2; // System fully operational
const uint8_t CONTROL_MODE_OFF = 0; // The system is OFF
const uint8_t CONTROL_MODE_WAIT_ZWS_TURNON = 1; // The system is waiting for more pulses to avoid a spurious turnon
const uint8_t CONTROL_MODE_ZWS = 2; // The system is operating autonomously with the InputPulse as both keepalive and
const uint8_t OUTPUT_MODE_STROBE = 0;
const uint8_t OUTPUT_MODE_STABLE = 1;
const uint8_t OUTPUT_MODE_SCAN = 2;
const uint8_t OUTPUT_MODE_OFF = 3;
const uint16_t ModeTimeout = 50; // If no iic transaction within this many milliseconds of INPUT_ENABLE_OR_SCL_PIN rising, then we are not plugged into an IIC controller.  Go to PWM mode
const uint16_t ModeTimeoutZWS = 70; // If no additional pulses within this many milliseconds of the first pulse, assume that it was a spurious event and revert to the off state.
const float POWER_FILTER_CONVERGENCE_RATIO = 0.0625;
const uint16_t MaximumInputPWM = 16384;
const uint16_t HysteresisInputPWM = 64; // TODO: Implement a better filter to avoid such large hysteresis

const uint16_t ADDRESS_MAGIC_BYTE               = 0;
const uint16_t ADDRESS_TARGET_POWERSAVE         = 1;
const uint16_t ADDRESS_SAVED_MODE               = 2;
const uint16_t ADDRESS_SAVED_OSD                = 3;
const uint16_t ADDRESS_TARGET_EDID              = 4;
const uint16_t ADDRESS_SAVED_BRIGHTNESS_STABLE  = 5;
const uint16_t ADDRESS_SAVED_BRIGHTNESS_STROBE  = 7;
const uint16_t ADDRESS_SAVED_BRIGHTNESS_SCAN    = 9;
const uint16_t ADDRESS_LED_STRUCTURE            = 11;

const uint8_t DefaultPowerSave = TargetPowerSaveFULLY_ON;
const uint8_t DefaultMode = OUTPUT_MODE_STABLE;
const uint8_t DefaultOSD = false;
const uint8_t DefaultEDID = 2; // Change this back to edid #0 after testing
const uint16_t DefaultBrightnessStable = 125;   // nits
const uint16_t DefaultBrightnessStrobe = 125;   // nits
const uint16_t DefaultBrightnessScan   = 125;   // nits


const uint8_t LED_STATE_INDICATOR_MAX_PWM_VALUE  = 32;  // The LED is too bright for indicating status!
const uint8_t LED_STATE_INDICATOR_ERROR       = 0;
const uint8_t LED_STATE_INDICATOR_POWERSAVE   = 1;
const uint8_t LED_STATE_INDICATOR_NO_INPUT    = 2;
const uint8_t LED_STATE_INDICATOR_PWM         = 3;
const uint8_t LED_STATE_INDICATOR_PWMFREE     = 4;
const uint8_t LED_STATE_INDICATOR_STROBING    = 5;
const uint8_t LED_STATE_INDICATOR_SCANNING    = 6;
const uint8_t LED_STATE_INDICATOR_SOFTADJUST  = 7;

const uint8_t POWER_RAMP_RATE = 15; // Units of watts per second. This limits the system to turn on slowly to avoid tripping the short-circuit protection in some power supplies

struct LED_STATE_INDICATOR_COLOR {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};



const LED_STATE_INDICATOR_COLOR LED_STATE_INDICATOR_COLORS[] = {
  { .r = LED_STATE_INDICATOR_MAX_PWM_VALUE, .g = LED_STATE_INDICATOR_MAX_PWM_VALUE, .b = LED_STATE_INDICATOR_MAX_PWM_VALUE }, // WHITE  // LED_STATE_INDICATOR_ERROR      = 0
  { .r =                0                 , .g = LED_STATE_INDICATOR_MAX_PWM_VALUE, .b =               0                  }, // GREEN  // LED_STATE_INDICATOR_POWERSAVE  = 1
  { .r = LED_STATE_INDICATOR_MAX_PWM_VALUE, .g =               0                 , .b =               0                  }, // RED    // LED_STATE_INDICATOR_NO_INPUT   = 2
  { .r = LED_STATE_INDICATOR_MAX_PWM_VALUE, .g = LED_STATE_INDICATOR_MAX_PWM_VALUE, .b =               0                  }, // YELLOW // LED_STATE_INDICATOR_PWM        = 3
  { .r =                0                 , .g = LED_STATE_INDICATOR_MAX_PWM_VALUE, .b = LED_STATE_INDICATOR_MAX_PWM_VALUE }, // CYAN   // LED_STATE_INDICATOR_PWMFREE    = 4
  { .r =                0                 , .g =               0                 , .b = LED_STATE_INDICATOR_MAX_PWM_VALUE }, // BLUE   // LED_STATE_INDICATOR_STROBING   = 5
  { .r = LED_STATE_INDICATOR_MAX_PWM_VALUE, .g =               0                 , .b = LED_STATE_INDICATOR_MAX_PWM_VALUE }, // PURPLE // LED_STATE_INDICATOR_SCANNING   = 6
  { .r = LED_STATE_INDICATOR_MAX_PWM_VALUE, .g = LED_STATE_INDICATOR_MAX_PWM_VALUE * 165 / 255    , .b = 0}                // ORANGE // LED_STATE_INDICATOR_SOFTADJUST = 7
};



#if SERIAL_COMMANDS_SIMPLE!=ENABLED
#if SERIAL_COMMANDS_EXTENDED==ENABLED
#error "SIMPLE SERIAL COMMANDS MUST BE ENABLED IF EXTENDED ONES ARE ENABLED"
#endif
#if SERIAL_COMMANDS_SIMPLE!=DISABLED
#error "SERIAL_COMMANDS_SIMPLE not specified"
#endif
#endif

#if OVERCLOCKING!=ENABLED
#if OVERCLOCKING!=DISABLED
#error "OVERCLOCKING not specified"
#endif
#endif


#if OVERCLOCKING==ENABLED
const uint32_t REAL_SPEED = OVERCLOCKED_SPEED;
#else
const uint32_t REAL_SPEED = F_CPU;
#endif
const float CLOCK_RATIO = 1.0 * REAL_SPEED / F_CPU;

#endif

