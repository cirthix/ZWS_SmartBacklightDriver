#include "InputHandling.h"

InputHandling::InputHandling(){
InputHandling::ResetInputHistory();
}

void  InputHandling::ReadPhysicalInputs() {     
  if (input_index < FILTERDEPTH_INPUT-1) {    input_index ++;  } 
  else {    input_index=0;  }
  
  input_history[input_index] = COMMAND_CODE_FOR_NOTHING;
  
  #if(SERIAL_COMMANDS_SIMPLE == ENABLED)
  // If the serial port has button input, use that and ignore the real button board.  Also, virtual buttons bypass filtering.
                int incomingByte = Serial.read();      
                   if (incomingByte > 0) {
  //              SerialDebug("Got byte: ");                SerialDebug(incomingByte&0xff, HEX);      
                uint8_t myChar = incomingByte & 0xff ;
                if(myChar>0x7f) { InputHandling::SetInputHistory( myChar ); return;} // All bytes with values above 127 are considered serial commands.  Let the software handle it.
                switch (myChar) {
                  #if(SERIAL_COMMANDS_EXTENDED == ENABLED)
                  case ASCII_CODE_FOR_SPECIAL_COMMANDS      : InputHandling::SetInputHistory( COMMAND_CODE_FOR_SPECIAL_COMMANDS      );  break;
                  #endif
                  case ASCII_CODE_FOR_SLAVE_ON              : InputHandling::SetInputHistory( COMMAND_CODE_FOR_SLAVE_ON              );  break;
                  case ASCII_CODE_FOR_SLAVE_OFF             : InputHandling::SetInputHistory( COMMAND_CODE_FOR_SLAVE_OFF             );  break;
                  case ASCII_CODE_FOR_POWER_BUTTON          : InputHandling::SetInputHistory( COMMAND_CODE_FOR_POWER_BUTTON          );  break;
                  case ASCII_CODE_FOR_BRIGHTNESS_INCREASE   : InputHandling::SetInputHistory( COMMAND_CODE_FOR_BRIGHTNESS_INCREASE   );  break;
                  case ASCII_CODE_FOR_BRIGHTNESS_DECREASE   : InputHandling::SetInputHistory( COMMAND_CODE_FOR_BRIGHTNESS_DECREASE   );  break;
                  case ASCII_CODE_FOR_FACTORY_PROGRAM       : InputHandling::SetInputHistory( COMMAND_CODE_FOR_FACTORY_PROGRAM       );  break;
                  case ASCII_CODE_FOR_PWM_FREQ_DECREASE     : InputHandling::SetInputHistory( COMMAND_CODE_FOR_PWM_FREQ_DECREASE     );  break;
                  case ASCII_CODE_FOR_PWM_FREQ_INCREASE     : InputHandling::SetInputHistory( COMMAND_CODE_FOR_PWM_FREQ_INCREASE     );  break;
                  case ASCII_CODE_FOR_EDID_ROTATE           : InputHandling::SetInputHistory( COMMAND_CODE_FOR_EDID_ROTATE           );  break;
                  case ASCII_CODE_FOR_EDID_0                : InputHandling::SetInputHistory( COMMAND_CODE_FOR_EDID_0                );  break;
                  case ASCII_CODE_FOR_EDID_1                : InputHandling::SetInputHistory( COMMAND_CODE_FOR_EDID_1                );  break;
                  case ASCII_CODE_FOR_EDID_2                : InputHandling::SetInputHistory( COMMAND_CODE_FOR_EDID_2                );  break;
                  case ASCII_CODE_FOR_EDID_3                : InputHandling::SetInputHistory( COMMAND_CODE_FOR_EDID_3                );  break;
                  case ASCII_CODE_FOR_EDID_4                : InputHandling::SetInputHistory( COMMAND_CODE_FOR_EDID_4                );  break;
                  case ASCII_CODE_FOR_EDID_5                : InputHandling::SetInputHistory( COMMAND_CODE_FOR_EDID_5                );  break;
                  case ASCII_CODE_FOR_EDID_6                : InputHandling::SetInputHistory( COMMAND_CODE_FOR_EDID_6                );  break;
                  case ASCII_CODE_FOR_EDID_7                : InputHandling::SetInputHistory( COMMAND_CODE_FOR_EDID_7                );  break;
                  case ASCII_CODE_FOR_STROBE_ROTATE         : InputHandling::SetInputHistory( COMMAND_CODE_FOR_STROBE_ROTATE         );  break;
                  case ASCII_CODE_FOR_PANEL_OSD             : InputHandling::SetInputHistory( COMMAND_CODE_FOR_PANEL_OSD             );  break;
                  case ASCII_CODE_FOR_OCTESTMODE_ON         : InputHandling::SetInputHistory( COMMAND_CODE_FOR_OCTESTMODE_ON         );  break;
                  case ASCII_CODE_FOR_OCTESTMODE_OFF        : InputHandling::SetInputHistory( COMMAND_CODE_FOR_OCTESTMODE_OFF        );  break;
                  case ASCII_CODE_FOR_OSD_ON                : InputHandling::SetInputHistory( COMMAND_CODE_FOR_OSD_ON                );  break;
                  case ASCII_CODE_FOR_OSD_OFF               : InputHandling::SetInputHistory( COMMAND_CODE_FOR_OSD_OFF               );  break;
                  case ASCII_CODE_FOR_POWER_ON              : InputHandling::SetInputHistory( COMMAND_CODE_FOR_POWER_ON              );  break;
                  case ASCII_CODE_FOR_POWER_OFF             : InputHandling::SetInputHistory( COMMAND_CODE_FOR_POWER_OFF             );  break;
                  case ASCII_CODE_FOR_SIMPLE_DEBUG_COMMAND  : InputHandling::SetInputHistory( COMMAND_CODE_FOR_SIMPLE_DEBUG_COMMAND  );  break;
                }
                return;
        }
#endif

filter_is_dirty=1;
 uint16_t adc_key_in; 

uint8_t ButtonStateA=0x00;
#ifdef BUTTON_A_ANALOG  
adc_key_in = analogRead(BUTTON_A_ANALOG); 
//SerialDebug(F("keyA : ")); SerialDebugln(adc_key_in);
 if ((adc_key_in >= cutoff_zero_low)  && (adc_key_in <= cutoff_zero_high))  { ButtonStateA|=BUTTON_0_MASK;}
 if ((adc_key_in >= cutoff_one_low)   && (adc_key_in <= cutoff_one_high))   { ButtonStateA|=BUTTON_1_MASK;}
 if ((adc_key_in >= cutoff_two_low)   && (adc_key_in <= cutoff_two_high))   { ButtonStateA|=BUTTON_2_MASK;}
 if ((adc_key_in >= cutoff_three_low) && (adc_key_in <= cutoff_three_high)) { ButtonStateA|=BUTTON_3_MASK;}
#endif
//SerialDebug(F("keyA : ")); SerialDebugln(ButtonStateA, HEX);

uint8_t ButtonStateB=0x00;
#ifdef BUTTON_B_ANALOG
adc_key_in = analogRead(BUTTON_B_ANALOG);   
//SerialDebug(F("keyB : ")); SerialDebugln(adc_key_in);
 if ((adc_key_in >= cutoff_zero_low)  && (adc_key_in <= cutoff_zero_high))  { ButtonStateB|=BUTTON_0_MASK;}
 if ((adc_key_in >= cutoff_one_low)   && (adc_key_in <= cutoff_one_high))   { ButtonStateB|=BUTTON_1_MASK;}
 if ((adc_key_in >= cutoff_two_low)   && (adc_key_in <= cutoff_two_high))   { ButtonStateB|=BUTTON_2_MASK;}
 if ((adc_key_in >= cutoff_three_low) && (adc_key_in <= cutoff_three_high)) { ButtonStateB|=BUTTON_3_MASK;}
#endif
//SerialDebug(F("keyB : ")); SerialDebugln(ButtonStateB, HEX);

uint8_t ButtonStateC=0x00;
#ifdef BUTTON_C_ANALOG
adc_key_in = analogRead(BUTTON_C_ANALOG);   
//SerialDebug(F("keyC : ")); SerialDebugln(adc_key_in);
 if ((adc_key_in >= cutoff_zero_low)  && (adc_key_in <= cutoff_zero_high))  { ButtonStateC|=BUTTON_0_MASK;}
 if ((adc_key_in >= cutoff_one_low)   && (adc_key_in <= cutoff_one_high))   { ButtonStateC|=BUTTON_1_MASK;}
 if ((adc_key_in >= cutoff_two_low)   && (adc_key_in <= cutoff_two_high))   { ButtonStateC|=BUTTON_2_MASK;}
 if ((adc_key_in >= cutoff_three_low) && (adc_key_in <= cutoff_three_high)) { ButtonStateC|=BUTTON_3_MASK;}
#endif
//SerialDebug(F("keyC : ")); SerialDebugln(ButtonStateC, HEX);

// The button boards supported all have the same voltage dividers on the buttons, but differ in the number of buttons, pins, and leds
// For now, only support the manufactured board and ignore the future ones.
#if (BUTTONBOARD_VERSION==BUTTONBOARD_IS_ZWS_6PIN_9BUTTON_RG_LED) || (BUTTONBOARD_VERSION==BUTTONBOARD_IS_ZWS_6PIN_9BUTTON_RGB_LED)
 if ((ButtonStateA&BUTTON_0_MASK) && (!ButtonStateB) && (!ButtonStateC))   { input_history[input_index]=COMMAND_CODE_FOR_POWER_BUTTON; return;}
 if ((!ButtonStateA) && (ButtonStateB&BUTTON_0_MASK) && (!ButtonStateC))   { input_history[input_index]=COMMAND_CODE_FOR_BRIGHTNESS_INCREASE; return;}
 if ((!ButtonStateA) && (ButtonStateB&BUTTON_1_MASK) && (!ButtonStateC))   { input_history[input_index]=COMMAND_CODE_FOR_EDID_2; return;}
 if ((!ButtonStateA) && (ButtonStateB&BUTTON_2_MASK) && (!ButtonStateC))   { input_history[input_index]=COMMAND_CODE_FOR_EDID_1; return;}
 if ((!ButtonStateA) && (ButtonStateB&BUTTON_3_MASK) && (!ButtonStateC))   { input_history[input_index]=COMMAND_CODE_FOR_EDID_0; return;}
 if ((!ButtonStateA) && (!ButtonStateB) && (ButtonStateC&BUTTON_0_MASK))   { input_history[input_index]=COMMAND_CODE_FOR_BRIGHTNESS_DECREASE; return;}
 if ((!ButtonStateA) && (!ButtonStateB) && (ButtonStateC&BUTTON_1_MASK))   { input_history[input_index]=COMMAND_CODE_FOR_STROBE_ROTATE; return;}
 if ((!ButtonStateA) && (!ButtonStateB) && (ButtonStateC&BUTTON_2_MASK))   { input_history[input_index]=COMMAND_CODE_FOR_EDID_4; return;}
 if ((!ButtonStateA) && (!ButtonStateB) && (ButtonStateC&BUTTON_3_MASK))   { input_history[input_index]=COMMAND_CODE_FOR_EDID_3; return;}
 if ((!ButtonStateA) && (ButtonStateB&BUTTON_0_MASK)  && (ButtonStateC&BUTTON_0_MASK))   { input_history[input_index]=COMMAND_CODE_FOR_PANEL_OSD; return;}
 if ((ButtonStateA&BUTTON_0_MASK) && (ButtonStateB&BUTTON_0_MASK)  && (ButtonStateC&BUTTON_0_MASK))   { input_history[input_index]=ASCII_CODE_FOR_SIMPLE_DEBUG_COMMAND; return;}
 if ((ButtonStateA&BUTTON_0_MASK) && (ButtonStateB&BUTTON_0_MASK)  && (!ButtonStateC))   { input_history[input_index]=COMMAND_CODE_FOR_PWM_FREQ_INCREASE; return;}
 if ((ButtonStateA&BUTTON_0_MASK) && (!ButtonStateB)  && (ButtonStateC&BUTTON_0_MASK))   { input_history[input_index]=COMMAND_CODE_FOR_PWM_FREQ_DECREASE; return;}
#endif

}



void  InputHandling::ResetInputHistory() {
  InputHandling::SetInputHistory(COMMAND_CODE_FOR_UNDEFINED);
}

void  InputHandling::SetInputHistory(uint8_t state) {
  current_filtered_input = state;
  previous_filtered_input = COMMAND_CODE_FOR_SERIAL;
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
  if (myButtonState == COMMAND_CODE_FOR_PANEL_OSD) {
    SerialDebugln(F("COMMAND_CODE_FOR_PANEL_OSD"));
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




