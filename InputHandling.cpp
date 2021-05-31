#include "InputHandling.h"

InputHandling::InputHandling(){
InputHandling::ResetInputHistory();
}

void  InputHandling::ReadPhysicalInputs() {     
  if (input_index < FILTERDEPTH_INPUT-1) { input_index ++;  } else { input_index=0; }
filter_is_dirty=1;
  #if BUTTONBOARD_VERSION==BUTTONBOARD_IS_ZISWORKS || BUTTONBOARD_VERSION==BUTTONBOARD_IS_ZISWORKS_WITH_OLD_REV1_PIN_MAPPING
   input_history[input_index] = InputHandling::ReadPhysicalInputsZisworks();
  #endif
  #if BUTTONBOARD_VERSION==BUTTONBOARD_IS_SAMSUNG || BUTTONBOARD_VERSION==BUTTONBOARD_IS_SAMSUNG_WITH_RGBLED
   input_history[input_index] = InputHandling::ReadPhysicalInputsSamsung();
  #endif
}


uint8_t InputHandling::ReadPhysicalInputsSamsung(){
  #if BUTTONBOARD_VERSION==BUTTONBOARD_IS_SAMSUNG || BUTTONBOARD_VERSION==BUTTONBOARD_IS_SAMSUNG_WITH_RGBLED
uint8_t button_combinations=0;
const uint8_t BUTTON_U_MASK   =  0 ;
const uint8_t BUTTON_D_MASK   =  1 ;
const uint8_t BUTTON_L_MASK   =  2 ;
const uint8_t BUTTON_R_MASK   =  3 ;
const uint8_t BUTTON_P_MASK  =  4 ;

uint16_t adc_key_in = analogRead(BUTTONBOARD_C_BUTTON);   
  if ((adc_key_in >= cutoff_zero_low)  && (adc_key_in <= cutoff_zero_high))  { button_combinations|=(0x01<<BUTTON_U_MASK);}
  if ((adc_key_in >= cutoff_one_low)   && (adc_key_in <= cutoff_one_high))   { button_combinations|=(0x01<<BUTTON_D_MASK);}
  if ((adc_key_in >= cutoff_two_low)   && (adc_key_in <= cutoff_two_high))   { button_combinations|=(0x01<<BUTTON_L_MASK);}
  if ((adc_key_in >= cutoff_three_low) && (adc_key_in <= cutoff_three_high)) { button_combinations|=(0x01<<BUTTON_R_MASK);}
 
  if(digitalRead(BUTTONBOARD_POWER_BUTTON)==LOW) {  button_combinations|=(0x01<<BUTTON_P_MASK);}
  
   switch (button_combinations) {
      case (0x01<<BUTTON_P_MASK)|(0x00<<BUTTON_U_MASK)|(0x00<<BUTTON_D_MASK)|(0x00<<BUTTON_L_MASK)|(0x00<<BUTTON_R_MASK) : return COMMAND_CODE_FOR_POWER_BUTTON ;
      case (0x01<<BUTTON_P_MASK)|(0x01<<BUTTON_U_MASK)|(0x00<<BUTTON_D_MASK)|(0x00<<BUTTON_L_MASK)|(0x00<<BUTTON_R_MASK) : return COMMAND_CODE_FOR_EDID_0 ;
      case (0x01<<BUTTON_P_MASK)|(0x00<<BUTTON_U_MASK)|(0x00<<BUTTON_D_MASK)|(0x00<<BUTTON_L_MASK)|(0x01<<BUTTON_R_MASK) : return COMMAND_CODE_FOR_EDID_1 ;
      case (0x01<<BUTTON_P_MASK)|(0x00<<BUTTON_U_MASK)|(0x01<<BUTTON_D_MASK)|(0x00<<BUTTON_L_MASK)|(0x00<<BUTTON_R_MASK) : return COMMAND_CODE_FOR_EDID_2 ;
      case (0x01<<BUTTON_P_MASK)|(0x00<<BUTTON_U_MASK)|(0x00<<BUTTON_D_MASK)|(0x01<<BUTTON_L_MASK)|(0x00<<BUTTON_R_MASK) : return COMMAND_CODE_FOR_EDID_3 ;
      case (0x00<<BUTTON_P_MASK)|(0x01<<BUTTON_U_MASK)|(0x00<<BUTTON_D_MASK)|(0x00<<BUTTON_L_MASK)|(0x00<<BUTTON_R_MASK) : return COMMAND_CODE_FOR_BRIGHTNESS_INCREASE ;
      case (0x00<<BUTTON_P_MASK)|(0x00<<BUTTON_U_MASK)|(0x00<<BUTTON_D_MASK)|(0x00<<BUTTON_L_MASK)|(0x01<<BUTTON_R_MASK) : return COMMAND_CODE_FOR_STROBE_ROTATE ;
      case (0x00<<BUTTON_P_MASK)|(0x00<<BUTTON_U_MASK)|(0x01<<BUTTON_D_MASK)|(0x00<<BUTTON_L_MASK)|(0x00<<BUTTON_R_MASK) : return COMMAND_CODE_FOR_BRIGHTNESS_DECREASE ;
      case (0x00<<BUTTON_P_MASK)|(0x00<<BUTTON_U_MASK)|(0x00<<BUTTON_D_MASK)|(0x01<<BUTTON_L_MASK)|(0x00<<BUTTON_R_MASK) : return COMMAND_CODE_FOR_CROSSHAIR ;
      default : ;
   } 
  #endif
  return COMMAND_CODE_FOR_NOTHING;
}

uint8_t InputHandling::ReadPhysicalInputsZisworks(){
#if BUTTONBOARD_VERSION==BUTTONBOARD_IS_ZISWORKS || BUTTONBOARD_VERSION==BUTTONBOARD_IS_ZISWORKS_WITH_OLD_REV1_PIN_MAPPING
  uint8_t button_combinations=0;
  uint16_t adc_key_in; 
 
const uint8_t BUTTON_EDID_MASK   =  0 ;
const uint8_t BUTTON_UP_MASK     =  1 ;
const uint8_t BUTTON_DOWN_MASK   =  2 ;
const uint8_t BUTTON_POWER_MASK  =  3 ;

adc_key_in = analogRead(BUTTONBOARD_A_BUTTON); 
 if ((adc_key_in >= cutoff_zero_low)  && (adc_key_in <= cutoff_zero_high))  { button_combinations|=(0x01<<BUTTON_UP_MASK);}
 if ((adc_key_in >= cutoff_one_low)   && (adc_key_in <= cutoff_one_high))   { return COMMAND_CODE_FOR_EDID_2; }
 if ((adc_key_in >= cutoff_two_low)   && (adc_key_in <= cutoff_two_high))   { return COMMAND_CODE_FOR_EDID_1; }
 if ((adc_key_in >= cutoff_three_low) && (adc_key_in <= cutoff_three_high)) { return COMMAND_CODE_FOR_EDID_0; }
 
adc_key_in = analogRead(BUTTONBOARD_B_BUTTON);   
 if ((adc_key_in >= cutoff_zero_low)  && (adc_key_in <= cutoff_zero_high))  { button_combinations|=(0x01<<BUTTON_DOWN_MASK);}
 if ((adc_key_in >= cutoff_one_low)   && (adc_key_in <= cutoff_one_high))   { return COMMAND_CODE_FOR_STROBE_ROTATE; }
 
 #if BUTTONBOARD_VERSION==BUTTONBOARD_IS_ZISWORKS_WITH_OLD_REV1_PIN_MAPPING
 if ((adc_key_in >= cutoff_two_low)   && (adc_key_in <= cutoff_two_high))   { return COMMAND_CODE_FOR_EDID_4; }
 #else
 if ((adc_key_in >= cutoff_two_low)   && (adc_key_in <= cutoff_two_high))   { return COMMAND_CODE_FOR_CROSSHAIR; }
 #endif
  
 if ((adc_key_in >= cutoff_three_low) && (adc_key_in <= cutoff_three_high)) { return COMMAND_CODE_FOR_EDID_3; }
 
if(digitalRead(BUTTONBOARD_POWER_BUTTON)==LOW) {  button_combinations|=(0x01<<BUTTON_POWER_MASK);}
            
const uint8_t COMBO_FOR_POWER_BUTTON     = 0x01<<BUTTON_POWER_MASK                                                        ;
const uint8_t COMBO_BRIGHTNESS_INCREASE  = (0x01<<BUTTON_UP_MASK)                                                         ;
const uint8_t COMBO_BRIGHTNESS_DECREASE  = (0x01<<BUTTON_DOWN_MASK)                                                       ;
//const uint8_t COMBO_FACTORY_PROGRAM      = (0x01<<BUTTON_POWER_MASK) | (0x01<<BUTTON_UP_MASK) | (0x01<<BUTTON_DOWN_MASK)  ;
//const uint8_t COMBO_PWM_FREQ_DECREASE    = (0x01<<BUTTON_POWER_MASK) | (0x01<<BUTTON_DOWN_MASK)                           ;
//const uint8_t COMBO_PWM_FREQ_INCREASE    = (0x01<<BUTTON_POWER_MASK) | (0x01<<BUTTON_UP_MASK)                             ;
//const uint8_t COMBO_CONDITIONAL_ROTATE   = (0x01<<BUTTON_UP_MASK) | (0x01<<BUTTON_DOWN_MASK)                              ;

   switch (button_combinations) {
      case COMBO_FOR_POWER_BUTTON      : return COMMAND_CODE_FOR_POWER_BUTTON ;
      case COMBO_BRIGHTNESS_INCREASE   : return COMMAND_CODE_FOR_BRIGHTNESS_INCREASE ;
      case COMBO_BRIGHTNESS_DECREASE   : return COMMAND_CODE_FOR_BRIGHTNESS_DECREASE ;
 //     case COMBO_FACTORY_PROGRAM       : return COMMAND_CODE_FOR_SPECIAL_COMMAND ;
      default : ;
   } 
   #endif
   return COMMAND_CODE_FOR_NOTHING;
}

void  InputHandling::ResetInputHistory() {
  InputHandling::SetInputHistory(COMMAND_CODE_FOR_UNDEFINED);
}

void  InputHandling::SetInputHistory(uint8_t state) {
  current_filtered_input = state;
  for (uint8_t i = 0; i < FILTERDEPTH_INPUT; i++) {
    input_history[i] = state;
  }
  input_index = 0; 
  filter_is_dirty=0;
}



void InputHandling::RefilterInputState()    {
//InputHandling::PrintState();  
  if(filter_is_dirty==0){return;}
  uint8_t found_filtered_input = input_history[0];
  uint8_t myindex = 0;
  while (myindex < FILTERDEPTH_INPUT) {
    if (input_history[myindex] == found_filtered_input) {
      myindex++;
    } else {
      previous_filtered_input = current_filtered_input;
      current_filtered_input = COMMAND_CODE_FOR_UNDEFINED;
      return ;
    }
  }
  previous_filtered_input = current_filtered_input;
  current_filtered_input = found_filtered_input;
    filter_is_dirty=0;
}


  uint8_t InputHandling::GetCurrentFilteredInput(){return current_filtered_input;}
  uint8_t InputHandling::GetPreviousFilteredInput(){return previous_filtered_input;}
  uint8_t InputHandling::GetCurrentUnFilteredInput(){return input_history[input_index];}


void InputHandling::PrintState()    {
SerialDebug(F("Filter state:"));
  if(filter_is_dirty==0){SerialDebug(F("DIRTY"));} else {SerialDebug(F("CLEAN"));}
  SerialDebug(F(" ["));
  uint8_t myindex = 0;
  while (myindex < FILTERDEPTH_INPUT) {
      SerialDebug(input_history[myindex]);
      if(myindex < FILTERDEPTH_INPUT-1) {      SerialDebug(F(", ")); }
      myindex++;
    } 
  SerialDebug(F("] ("));
  SerialDebug(input_index);
  SerialDebug(F(") Filtered: "));
  SerialDebug( InputHandling::GetCurrentFilteredInput());
  
  SerialDebugln(F(""));  
}



void InputHandling::PrintButtons() {
  SerialDebugln(F(""));
  SerialDebugln(F(""));
  uint8_t myButtonState = InputHandling::GetCurrentFilteredInput();

  SerialDebug(F("ButtonState : "));
  if (myButtonState == COMMAND_CODE_FOR_POWER_BUTTON) {
    SerialDebugln(F("COMMAND_CODE_FOR_POWER_BUTTON"));
    return;
  }
  if (myButtonState == COMMAND_CODE_FOR_BRIGHTNESS_DECREASE) {
    SerialDebugln(F("COMMAND_CODE_FOR_BRIGHTNESS_DECREASE"));
    return;
  }
  if (myButtonState == COMMAND_CODE_FOR_BRIGHTNESS_INCREASE) {
    SerialDebugln(F("COMMAND_CODE_FOR_BRIGHTNESS_INCREASE"));
    return;
  }
  if (myButtonState == COMMAND_CODE_FOR_PWM_FREQ_DECREASE) {
    SerialDebugln(F("COMMAND_CODE_FOR_PWM_FREQ_DECREASE"));
    return;
  }
  if (myButtonState == COMMAND_CODE_FOR_PWM_FREQ_INCREASE) {
    SerialDebugln(F("COMMAND_CODE_FOR_PWM_FREQ_INCREASE"));
    return;
  }
  if (myButtonState == COMMAND_CODE_FOR_EDID_0) {
    SerialDebugln(F("COMMAND_CODE_FOR_EDID_0"));
    return;
  }
  if (myButtonState == COMMAND_CODE_FOR_EDID_1) {
    SerialDebugln(F("COMMAND_CODE_FOR_EDID_1"));
    return;
  }
  if (myButtonState == COMMAND_CODE_FOR_EDID_2) {
    SerialDebugln(F("COMMAND_CODE_FOR_EDID_2"));
    return;
  }
  if (myButtonState == COMMAND_CODE_FOR_EDID_3) {
    SerialDebugln(F("COMMAND_CODE_FOR_EDID_3"));
    return;
  }
  if (myButtonState == COMMAND_CODE_FOR_STROBE_ROTATE) {
    SerialDebugln(F("COMMAND_CODE_FOR_STROBE_ROTATE"));
    return;
  }
  if (myButtonState == COMMAND_CODE_FOR_CROSSHAIR) {
    SerialDebugln(F("COMMAND_CODE_FOR_PANEL_XHAIR"));
    return;
  }
  if (myButtonState == COMMAND_CODE_FOR_FACTORY_PROGRAM) {
    SerialDebugln(F("COMMAND_CODE_FOR_FACTORY_PROGRAM"));
    return;
  }
  SerialDebugln(F("COMMAND_CODE_FOR_NOTHING/OTHER"));

  SerialDebugln(F(""));
  SerialDebugln(F(""));
  SerialFlush();
}




