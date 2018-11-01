void ClearTimer1InterruptFlags(){  
        TIFR1 = (1 << ICF1) | (1 << OCF1A) | (1 << OCF1B) | (1 << TOV1); // Clear timer 1 interrupts for a fresh start
}

void StartSyncInterruptFalling(){
//  attachInterrupt(digitalPinToInterrupt(DATA_ENABLE), BacklightInterruptStub, RISING); // This interrupt can be triggered by vsync or de
 EICRA = (1<<ISC01) | (1<<ISC00); //rising edge triggers the interrupt
 EIFR  = (1<<INTF0); // Clear the interrupt before we enable it
 EIMSK = (1<<INT0); //interrupt request enabled  
}

void StartSyncInterruptChange(){
//  attachInterrupt(digitalPinToInterrupt(DATA_ENABLE), BacklightInterruptStub, RISING); // This interrupt can be triggered by vsync or de
 EICRA = (1<<ISC00); // both edges trigger the interrupt
 EIFR  = (1<<INTF0); // Clear the interrupt before we enable it
 EIMSK = (1<<INT0); //interrupt request enabled  
}
void StopSyncInterrupt(){
 EIMSK =0x00; //interrupt request enabled
//  detachInterrupt(digitalPinToInterrupt(DATA_ENABLE)); // This interrupt can be triggered by vsync or de
}

void PrintTimer1ConfigurationRegisters(){
uint8_t my_TCCR1A = TCCR1A ;
uint8_t my_TCCR1B = TCCR1B ;
uint8_t my_TCNT1  = TCNT1  ;
uint8_t my_OCR1A  = OCR1A  ;
uint8_t my_OCR1B  = OCR1B  ;
uint8_t my_TIMSK1 = TIMSK1 ;
uint8_t my_TIFR1  = TIFR1  ;

  
  SerialDebug(F("TCCR1A : ")); SerialDebugln(my_TCCR1A);
  SerialDebug(F("TCCR1B : ")); SerialDebugln(my_TCCR1B);
  SerialDebug(F("TCNT1 : "));  SerialDebugln(my_TCNT1 );
  SerialDebug(F("OCR1A : "));  SerialDebugln(my_OCR1A );
  SerialDebug(F("OCR1B : "));  SerialDebugln(my_OCR1B );
  SerialDebug(F("TIMSK1 : ")); SerialDebugln(my_TIMSK1);
  SerialDebug(F("TIFR1 : "));  SerialDebugln(my_TIFR1 );
  
  SerialDebugln(F("Parsed results :"));

  uint8_t my_ICNC1=(GetBit(my_TCCR1B, ICNC1));
  uint8_t my_ICES1=(GetBit(my_TCCR1B, ICES1));
  uint8_t my_FOC1=(GetBit(my_TCCR1B, FOC1A)<<1)|(GetBit(my_TCCR1B, FOC1B));
  uint8_t my_COM1A=(GetBit(my_TCCR1A, COM1A1)<<1)|(GetBit(my_TCCR1A, COM1A0));
  uint8_t my_COM1B=(GetBit(my_TCCR1A, COM1B1)<<1)|(GetBit(my_TCCR1A, COM1B0));
  uint8_t my_WGM1=(GetBit(my_TCCR1B, WGM13)<<3)|(GetBit(my_TCCR1B, WGM12)<<2)|(GetBit(my_TCCR1A, WGM11)<<1)|(GetBit(my_TCCR1A, WGM10));
  uint8_t my_CS1=(GetBit(my_TCCR1B, CS12)<<2)|(GetBit(my_TCCR1B, CS11)<<1)|(GetBit(my_TCCR1B, CS10));

  
  SerialDebug(F("my_CS1 : "));  SerialDebugln(my_CS1 );
  SerialDebug(F("my_COM1A : "));  SerialDebugln(my_COM1A );
  SerialDebug(F("my_COM1B : "));  SerialDebugln(my_COM1B );
  SerialDebug(F("my_WGM1 : "));  SerialDebugln(my_WGM1 );
  SerialDebug(F("my_FOC1 : "));  SerialDebugln(my_FOC1 );
  SerialDebug(F("my_ICNC1 : "));  SerialDebugln(my_ICNC1 );
  SerialDebug(F("my_ICES1 : "));  SerialDebugln(my_ICES1 );
  
}

void PrintTimer2ConfigurationRegisters(){
uint8_t my_TCCR2A = TCCR2A ;
uint8_t my_TCCR2B = TCCR2B ;
uint8_t my_TCNT2  = TCNT2  ;
uint8_t my_OCR2A  = OCR2A  ;
uint8_t my_OCR2B  = OCR2B  ;
uint8_t my_TIMSK2 = TIMSK2 ;
uint8_t my_TIFR2  = TIFR2  ;
uint8_t my_GTCCR  = GTCCR  ;

  
  SerialDebug(F("TCCR2A : ")); SerialDebugln(my_TCCR2A);
  SerialDebug(F("TCCR2B : ")); SerialDebugln(my_TCCR2B);
  SerialDebug(F("TCNT2 : "));  SerialDebugln(my_TCNT2 );
  SerialDebug(F("OCR2A : "));  SerialDebugln(my_OCR2A );
  SerialDebug(F("OCR2B : "));  SerialDebugln(my_OCR2B );
  SerialDebug(F("TIMSK2 : ")); SerialDebugln(my_TIMSK2);
  SerialDebug(F("TIFR2 : "));  SerialDebugln(my_TIFR2 );
  SerialDebug(F("GTCCR : "));  SerialDebugln(my_GTCCR );
  
  SerialDebugln(F("Parsed results :"));

  uint8_t my_FOC2=(GetBit(my_TCCR2B, FOC2A)<<1)|(GetBit(my_TCCR2B, FOC2B));
  uint8_t my_COM2A=(GetBit(my_TCCR2A, COM2A1)<<1)|(GetBit(my_TCCR2A, COM2A0));
  uint8_t my_COM2B=(GetBit(my_TCCR2A, COM2B1)<<1)|(GetBit(my_TCCR2A, COM2B0));
  uint8_t my_WGM2=(GetBit(my_TCCR2B, WGM22)<<2)|(GetBit(my_TCCR2A, WGM21)<<1)|(GetBit(my_TCCR2A, WGM20));
  uint8_t my_CS2=(GetBit(my_TCCR2B, CS22)<<2)|(GetBit(my_TCCR2B, CS21)<<1)|(GetBit(my_TCCR2B, CS20));

  
  SerialDebug(F("my_CS2 : "));  SerialDebugln(my_CS2 );
  SerialDebug(F("my_COM2A : "));  SerialDebugln(my_COM2A );
  SerialDebug(F("my_COM2B : "));  SerialDebugln(my_COM2B );
  SerialDebug(F("my_WGM2 : "));  SerialDebugln(my_WGM2 );
  SerialDebug(F("my_FOC2 : "));  SerialDebugln(my_FOC2 );
  
}

uint8_t GetBit(uint8_t myBits, uint8_t myBit){
  if((myBits&(0x01<<myBit))==0) return 0; else return 1;  
}





void WritePulseToAllPins(uint8_t TargetValue){  
    #ifdef BLDRIVER_PWM
        digitalWrite2(BLDRIVER_PWM, TargetValue);   
    #endif
    #ifdef BLDRIVER_PWM_1
        digitalWrite2(BLDRIVER_PWM_1, TargetValue);
    #endif
    #ifdef BLDRIVER_PWM_2
        digitalWrite2(BLDRIVER_PWM_2, TargetValue);  
    #endif
    #ifdef BLDRIVER_PWM_3
        digitalWrite2(BLDRIVER_PWM_3, TargetValue);   
    #endif
    #ifdef BLDRIVER_PWM_4
        digitalWrite2(BLDRIVER_PWM_4, TargetValue);  
    #endif
    #ifdef BLDRIVER_PWM_5
        digitalWrite2(BLDRIVER_PWM_5, TargetValue);   
    #endif
    #ifdef BLDRIVER_PWM_6
        digitalWrite2(BLDRIVER_PWM_6, TargetValue);   
    #endif    
}


void SetStaticPins(){
  #ifdef INPUT_DIM_OR_SDA_PIN
    pinMode(INPUT_DIM_OR_SDA_PIN , INPUT_PULLUP);
  #endif
  #ifdef INPUT_ENABLE_OR_SCL_PIN
//    pinMode(INPUT_ENABLE_OR_SCL_PIN , INPUT); #disabled for 3d glasses support
  #endif
  #ifdef INPUT_PULSE
    pinMode(INPUT_PULSE  , INPUT);
  #endif
  
  #ifdef BLDRIVER_ENABLE
    pinMode(BLDRIVER_ENABLE , OUTPUT); digitalWrite(BLDRIVER_ENABLE, LOW);
  #endif
  #ifdef BLDRIVER_PWM
    pinMode(BLDRIVER_PWM  , OUTPUT); digitalWrite(BLDRIVER_PWM, LOW);
  #endif
  #ifdef BLDRIVER_PWM_1
    pinMode(BLDRIVER_PWM_1  , OUTPUT); digitalWrite(BLDRIVER_PWM_1, LOW);
  #endif
  #ifdef BLDRIVER_PWM_2
    pinMode(BLDRIVER_PWM_2  , OUTPUT); digitalWrite(BLDRIVER_PWM_2, LOW);
  #endif
  #ifdef BLDRIVER_PWM_3
    pinMode(BLDRIVER_PWM_3  , OUTPUT); digitalWrite(BLDRIVER_PWM_3, LOW);
  #endif
  #ifdef BLDRIVER_PWM_4
    pinMode(BLDRIVER_PWM_4  , OUTPUT); digitalWrite(BLDRIVER_PWM_4, LOW);
  #endif
  #ifdef BLDRIVER_PWM_5
    pinMode(BLDRIVER_PWM_5  , OUTPUT); digitalWrite(BLDRIVER_PWM_5, LOW);
  #endif
  #ifdef BLDRIVER_PWM_6
    pinMode(BLDRIVER_PWM_6  , OUTPUT); digitalWrite(BLDRIVER_PWM_6, LOW);
  #endif
  #ifdef BLDRIVER_FAULT
    pinMode(BLDRIVER_FAULT , INPUT);
  #endif

  
  #ifdef BLDRIVER_ADIM_0
    pinMode(BLDRIVER_ADIM_0 , OUTPUT); digitalWrite(BLDRIVER_ADIM_0, LOW);
  #endif
  #ifdef BLDRIVER_ADIM_1
    pinMode(BLDRIVER_ADIM_1 , OUTPUT); digitalWrite(BLDRIVER_ADIM_1, LOW);
  #endif
  #ifdef BLDRIVER_ADIM_2
    pinMode(BLDRIVER_ADIM_2 , OUTPUT); digitalWrite(BLDRIVER_ADIM_2, LOW);
  #endif
  #ifdef BLDRIVER_ADIM_3
    pinMode(BLDRIVER_ADIM_3 , OUTPUT); digitalWrite(BLDRIVER_ADIM_3, LOW);
  #endif
  #ifdef BLDRIVER_ADIM_4
    pinMode(BLDRIVER_ADIM_4 , OUTPUT); digitalWrite(BLDRIVER_ADIM_4, LOW);
  #endif
  #ifdef BLDRIVER_ADIM_5
    pinMode(BLDRIVER_ADIM_5 , OUTPUT); digitalWrite(BLDRIVER_ADIM_5, LOW);
  #endif
  #ifdef BLDRIVER_ADIM_6
    pinMode(BLDRIVER_ADIM_6 , OUTPUT); digitalWrite(BLDRIVER_ADIM_6, LOW);
  #endif
  
  #ifdef ONBOARD_LED
    pinMode(ONBOARD_LED , OUTPUT); digitalWrite(ONBOARD_LED, LOW);
  #endif

  #if !((SERIAL_INTERFACE==ENABLED) && ((LED_R == D0) || (LED_R == D1)) )
    pinMode(LED_R , OUTPUT); digitalWrite(BUTTONBOARD_LED_R, LOW);
  #endif
  
  #if !((SERIAL_INTERFACE==ENABLED) && ((LED_G == D0) || (LED_G == D1)) )
    pinMode(LED_G , OUTPUT); digitalWrite(BUTTONBOARD_LED_G, LOW);
  #endif

  #ifdef BUTTON_A_ANALOG
    pinMode(BUTTON_A_ANALOG , INPUT);
  #endif
  #ifdef BUTTON_B_ANALOG
    pinMode(BUTTON_B_ANALOG , INPUT);
  #endif
  #ifdef BUTTON_C_ANALOG
    pinMode(BUTTON_C_ANALOG , INPUT);
  #endif 

  
  #if (SERIAL_INTERFACE==ENABLED)
    return;
  #endif

}




void SetTimer0ClockDivider(uint8_t myDivider){ TCCR0B=(TCCR0B&0b11111000)|myDivider; }
void SetTimer1ClockDivider(uint8_t myDivider){ TCCR1B=(TCCR1B&0b11111000)|myDivider; }
void SetTimer2ClockDivider(uint8_t myDivider){ TCCR2B=(TCCR2B&0b11111000)|myDivider; }

uint8_t GetCurrentTimer0ClockDivider(){ return (TCCR0B&0b00000111); }
uint8_t GetCurrentTimer1ClockDivider(){ return (TCCR1B&0b00000111); }
uint8_t GetCurrentTimer2ClockDivider(){ return (TCCR2B&0b00000111); }


uint8_t DetermineCorrectTimer0Divider(uint16_t myTargetDivision){
  switch (myTargetDivision) {
  case     1 : return 0x01;
  case     8 : return 0x02;
  case    64 : return 0x03;
  case   256 : return 0x04;
  case  1024 : return 0x05;
  default    : return 0x00;
  }
}
uint8_t DetermineCorrectTimer1Divider(uint16_t myTargetDivision){
  switch (myTargetDivision) {
  case     1 : return 0x01;
  case     8 : return 0x02;
  case    64 : return 0x03;
  case   256 : return 0x04;
  case  1024 : return 0x05;
  default    : return 0x00;
  }
}
uint8_t DetermineCorrectTimer2Divider(uint16_t myTargetDivision){
  switch (myTargetDivision) {
  case     1 : return 0x01;
  case     8 : return 0x02;
  case    32 : return 0x03;
  case    64 : return 0x04;
  case   128 : return 0x05;
  case   256 : return 0x06;
  case  1024 : return 0x07;
  default    : return 0x00;
  }
}

uint16_t DetermineTimer0Prescaler(uint8_t myDivider){
  switch (myDivider) {
  case 0x01:                return 1;
  case 0x02:                return 8;
  case 0x03:                return 64;
  case 0x04:                return 256;
  case 0x05:                return 1024;
  default:                  return 0;  
  }
}

uint16_t DetermineTimer1Prescaler(uint8_t myDivider){
  switch (myDivider) {
  case 0x01:                return 1;
  case 0x02:                return 8;
  case 0x03:                return 64;
  case 0x04:                return 256;
  case 0x05:                return 1024;
  default:                  return 0;  
  }
}

uint16_t DetermineTimer2Prescaler(uint8_t myDivider){
  switch (myDivider) {
  case 0x01:                return 1;
  case 0x02:                return 8;
  case 0x03:                return 32;
  case 0x04:                return 64;
  case 0x05:                return 128;
  case 0x06:                return 256;
  case 0x07:                return 1024;
  default:                  return 0;  
  }
}
// Note: common speeds between the three timers are 1,8,64,256, and 1024.  Most used will be 8x
uint32_t DetermineTimer0TickRate(){ // Returns ticks per second
  return REAL_SPEED / DetermineTimer0Prescaler(GetCurrentTimer0ClockDivider());
}

uint32_t DetermineTimer1TickRate(){ // Returns ticks per second
  return REAL_SPEED / DetermineTimer1Prescaler(GetCurrentTimer1ClockDivider());
}

uint32_t DetermineTimer2TickRate(uint8_t myDivider){ // Returns ticks per second
  return REAL_SPEED / DetermineTimer2Prescaler(myDivider);
}

uint32_t Timer1TicksToMicrocseconds(uint16_t myTicks){
 return 1.0*myTicks*1000000/DetermineTimer1TickRate();
}

uint16_t Timer1MicrosecondsToTicks(uint16_t myMicroseconds){
 return 1.0*myMicroseconds*DetermineTimer1TickRate()/1000000;
}

uint32_t Timer2TicksToMicrocseconds(uint8_t myDivider, uint16_t myTicks){
 return 1.0*myTicks*1000000/DetermineTimer2TickRate(myDivider);
}

uint16_t DetermineTimer2FrequencyPWM(uint8_t myDivider){ // Returns total pwm frequency.  /256 in fast mode, /510 in phase-correct mode
  // Note: fast mode allows the timer overflow interrupt to still be used.  phase-correct mode does not.  This is important for timer0 and the millis/micros/delay functions.
return ((DetermineTimer2TickRate(myDivider))/256);
}



uint16_t current_to_brightness(uint16_t myCurrent){     return MyConfigLED.CurrentToBrightnessIntercept + MyConfigLED.CurrentToBrightnessSlope*myCurrent;}
uint16_t brightness_to_current(uint16_t myBrightness){  return -(MyConfigLED.CurrentToBrightnessIntercept/MyConfigLED.CurrentToBrightnessSlope)+(1/MyConfigLED.CurrentToBrightnessSlope)*myBrightness;}
//uint16_t brightness_to_current(uint16_t myBrightness){  return MyConfigLED.BrightnessToCurrentIntercept+MyConfigLED.BrightnessToCurrentSlope*myBrightness;}
uint16_t current_to_voltage(uint16_t myCurrent){     return MyConfigLED.CurrentToVoltageIntercept+MyConfigLED.CurrentToVoltageSlope*myCurrent;}
uint16_t voltage_to_current(uint16_t myVoltage){  return -(MyConfigLED.CurrentToVoltageIntercept/MyConfigLED.CurrentToVoltageSlope)+(1/MyConfigLED.CurrentToVoltageSlope)*myVoltage;}
//uint16_t voltage_to_current(uint16_t myVoltage){  return MyConfigLED.VoltageToCurrentIntercept+MyConfigLED.VoltageToCurrentSlope*myVoltage;}

void DisableUnusedPeripherals(){
  // disable ADC
  ADCSRA = 0;  

// Disable the analog comparator by setting the ACD bit
// (bit 7) of the ACSR register to one.
ACSR = B10000000;  
}
