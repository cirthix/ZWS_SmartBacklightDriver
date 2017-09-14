#ifndef SUPPORTEDBOARDS_H
#define SUPPORTEDBOARDS_H

#if BOARD_VERSION==BOARD_IS_ZWS_SMART_BLDRIVER_REV1_1
inline void board_print_name(){ SerialDebugln(F("ZisWorks smart backlight driver V1.1"));}
#define BUTTONBOARD_BUTTON_A_ANALOG  A0
#define BUTTONBOARD_BUTTON_B_ANALOG  A1
#define BUTTONBOARD_BUTTON_C_ANALOG  A2 // For 6pwms, change button board to 8 buttons with a ws2812 style smart rgb led here
// For the other led pin on the button board connector, use the serial TX pin
#define BLDRIVER_ENABLE  A3   // For 6 pwms, change this to bldriver_enable
#define INPUT_DIM_OR_SDA_PIN  A4
#define INPUT_ENABLE_OR_SCL_PIN  A5 
#define BUTTONBOARD_LED_R  0 // Shared with the serial output
#define BUTTONBOARD_LED_G  1 // Shared with the serial output
#define INPUT_PULSE  2
#define BLDRIVER_PWM_1  3
#define BLDRIVER_ADIM_0  4
#define BLDRIVER_PWM_6  5
#define BLDRIVER_PWM_5  6
#define BLDRIVER_ADIM_1  7 
#define BLDRIVER_ADIM_3  8
#define BLDRIVER_PWM_4  9  // For 6 pwms, change this to pwm5
#define BLDRIVER_PWM_3  10  // for 6 pwms, change this to pwm6
#define BLDRIVER_PWM_2  11
#define BLDRIVER_ADIM_5  12
#define BLDRIVER_ADIM_6  13
#define BLDRIVER_ADIM_2  14
#define BLDRIVER_ADIM_4  15
#define USE_PORTB_FULL_ADIM_WRITE_METHOD false

#elif BOARD_VERSION==BOARD_IS_ZWS_SMART_BLDRIVER_REV1_SCAN_HACK
inline void board_print_name(){ SerialDebugln(F("ZisWorks smart backlight driver V1.0 MODDED FOR SCANNING"));}
#define BUTTONBOARD_BUTTON_A_ANALOG  A0
#define BUTTONBOARD_BUTTON_B_ANALOG  A1
#define BUTTONBOARD_BUTTON_C_ANALOG  A2 // For 6pwms, change button board to 8 buttons with a ws2812 style smart rgb led here
// For the other led pin on the button board connector, use the serial TX pin
#define ONBOARD_LED  A3   // For 6 pwms, change this to bldriver_enable
#define INPUT_DIM_OR_SDA_PIN  A4
#define INPUT_ENABLE_OR_SCL_PIN  A5 
#define BUTTONBOARD_LED_R  0 // Shared with the serial output
#define BUTTONBOARD_LED_G  1 // Shared with the serial output
#define INPUT_PULSE  2
#define BLDRIVER_PWM_1  3
#define BLDRIVER_ENABLE  4  // For 6 pwms, change this to adim3
#define BLDRIVER_PWM_4  5
#define BLDRIVER_PWM_3  6
#define BLDRIVER_FAULT  7  // For 6 pwms, change this to adim4
#define BLDRIVER_ADIM_2  8
#define BLDRIVER_ADIM_3  9  // For 6 pwms, change this to pwm5
#define BLDRIVER_ADIM_4  10  // for 6 pwms, change this to pwm6
#define BLDRIVER_PWM_2  11
#define BLDRIVER_ADIM_5  12
#define BLDRIVER_ADIM_6  13
#define BLDRIVER_ADIM_0  14
#define BLDRIVER_ADIM_1  15
#define USE_PORTB_FULL_ADIM_WRITE_METHOD true

#elif BOARD_VERSION==BOARD_IS_ZWS_SMART_BLDRIVER_REV1
inline void board_print_name(){ SerialDebugln(F("ZisWorks smart backlight driver V1.0"));}
#define BUTTONBOARD_BUTTON_A_ANALOG  A0
#define BUTTONBOARD_BUTTON_B_ANALOG  A1
#define BUTTONBOARD_BUTTON_C_ANALOG  A2 // NOTE: this pin is shared with led_b on the first-gen board
#define ONBOARD_LED  A3
#define INPUT_DIM_OR_SDA_PIN  A4
#define INPUT_ENABLE_OR_SCL_PIN  A5 
#define INPUT_PULSE  2
#define BLDRIVER_PWM  3
#define BLDRIVER_ENABLE  4
#define BUTTONBOARD_LED_G  5
#define BUTTONBOARD_LED_B  6
#define BLDRIVER_FAULT  7
#define BLDRIVER_ADIM_2  8
#define BLDRIVER_ADIM_3  9
#define BLDRIVER_ADIM_4  10
#define BUTTONBOARD_LED_R  11
#define BLDRIVER_ADIM_5  12
#define BLDRIVER_ADIM_6  13
#define BLDRIVER_ADIM_0  14
#define BLDRIVER_ADIM_1  15
#define USE_PORTB_FULL_ADIM_WRITE_METHOD true
#else
  #error "Unsupported BOARD_VERSION"
#endif

#ifndef USE_PORTB_FULL_ADIM_WRITE_METHOD
  #define USE_PORTB_FULL_ADIM_WRITE_METHOD false
#endif

#endif


