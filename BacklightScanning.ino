// Each of the operating modes must provide some functions:
// Rule checking function
// Brightness calculation function
// Status printing function
// Rely on internal global variables

// Scanning uses timer1 and the pin interrupt
// Assumptions with the current implementation:
// Only one pulse will be active at any given time.  There must be a small margin of time between pulses for the interrupt to retrigger
// Scan segments are in the same order as pwm pins, starting with pwm 1 at the top
// The PULSE input goes high at the end of the valid frame and falls with the start of valid video data.
// The (falling) pin interrupt will turn off all outputs regardless of state, as will the overflow interrupt.  This is a safety measure.


uint16_t ScanPulseDurationMAX=OutputPulseDurationMAX; // units=microseconds
const uint16_t SafetyMarginScanPrePulse = 120; // Number of microseconds between the start of a frame (pulse input fall) and the first possible timer interrupt.  Time needed to send out serial commands
const uint16_t SafetyMarginScanPostPulse = 90; // Number of microseconds between strobe pulse falling and expected start of next frame

void PrintConfigScan(){
      SerialDebugln(F("SCAN"));
      SerialDebug(F("OffsetDelay   : "));  SerialDebug(ScanPulseDelayInitial);  SerialDebugln(F("us"));
      SerialDebug(F("PulseDuration : ")); SerialDebug(ScanPulseDuration);  SerialDebugln(F(" us"));
      SerialDebug(F("Current       : ")); SerialDebug(ADIM_LEVEL_TO_OUTPUT_CURRENT(CALCULATED_ADIM_SCAN));  SerialDebugln(F("mA"));
      SerialDebug(F("Brightness    : ")); SerialDebug(CalculateBrightnessScan());  SerialDebugln(F("nits"));
      SerialDebug(F("PowerOutput   : ")); SerialDebug(CalculatePowerScan());  SerialDebugln(F("mW"));
      SerialDebug(F("Time1+ : "));  SerialDebugln(GetScanTime0());
      SerialDebug(F("Time1- : "));  SerialDebugln(GetScanTime1());
      if((MyConfigLED.NumberStrings) <2) {return;}
      SerialDebug(F("Time2+ : "));  SerialDebugln(GetScanTime2());
      SerialDebug(F("Time2- : "));  SerialDebugln(GetScanTime3());
      if((MyConfigLED.NumberStrings) <3) {return;}
      SerialDebug(F("Time3+ : "));  SerialDebugln(GetScanTime4());
      SerialDebug(F("Time3- : "));  SerialDebugln(GetScanTime5());
      if((MyConfigLED.NumberStrings) <4) {return;}
      SerialDebug(F("Time4+ : "));  SerialDebugln(GetScanTime6());
      SerialDebug(F("Time4- : "));  SerialDebugln(GetScanTime7());
      if((MyConfigLED.NumberStrings) <5) {return;}
      SerialDebug(F("Time5+ : "));  SerialDebugln(GetScanTime8());
      SerialDebug(F("Time5- : "));  SerialDebugln(GetScanTime9());
      if((MyConfigLED.NumberStrings) <6) {return;}
      SerialDebug(F("Time6+ : "));  SerialDebugln(GetScanTime10());
      SerialDebug(F("Time6- : "));  SerialDebugln(GetScanTime11());  
}


void EnterScanning(){
    StopSyncInterrupt();
    POWER_LIMIT=INITIAL_POWER_LIMIT;
    adimWrite(CALCULATED_ADIM_OFF);
    WriteAllPWMsLOW();
    wdt_reset();
    adimWrite(CALCULATED_ADIM_SCAN);
    OUTPUT_MODE = OUTPUT_MODE_SCAN;
    ConfigureTimersScanning();
    StartSyncInterruptChange();
    IndicateNewState(); 
}


  
void ConfigureTimersScanning(){  
    SerialDebugln(F("Configuring timers for scanning mode"));
// Stop timer interrupts
    TIMSK0=0x00; // Note that the timer overflow will still be measured and accounted for in the HandleMillisInTimer0Overflow function
    TIMSK1=0x00; // Leave the overflow interrupt enabled
    TIMSK2=0x00; // Disable interrupts here

// Stop the output clocks, unconfigure the timers
    TCCR0B=0x00;
    TCCR1B=0x00;
    TCCR2B=0x00;
    TCCR0A=0x00;
    TCCR1A=0x00;
    TCCR2A=0x00;
    
    SetTimer0ClockDivider(DetermineCorrectTimer0Divider(64)); // For more correct timing with the builtin millis/micros stuff

    TIMSK1 =0x00;
    TCCR1A= 0; // turn off PWM
    TCNT1=0;
    SetTimer1ClockDivider(DetermineCorrectTimer1Divider(8)); // For 8:1 on timer1.  65ms rollover wtih 1us timer resolution
    BuildTimeLine();  
    OCR1A=GetScanTimeFirst();    
    TIMSK1 =0x00 | (1 << OCIE1A);    //Set interrupt A on compare match   
}



ISR(TIMER1_COMPA_vect) {
  // TODO: Fix this to compatible with the 6ch board.  Check the MyConfigLED.NumberStrings value to determine instead of assuming four strings
  switch(ScanningState){
    #ifdef BLDRIVER_PWM_2
      case SEGMENT_2_ON:  digitalWrite2(BLDRIVER_PWM_2, HIGH);  ScanningState=SEGMENT_2_OFF; OCR1A=GetScanTime3();   break;
      case SEGMENT_2_OFF: digitalWrite2(BLDRIVER_PWM_2, LOW );  ScanningState=SEGMENT_3_ON;  OCR1A=GetScanTime4();   break;
    #endif
    #ifdef BLDRIVER_PWM_3
      case SEGMENT_3_ON:  digitalWrite2(BLDRIVER_PWM_3, HIGH);  ScanningState=SEGMENT_3_OFF; OCR1A=GetScanTime5();   break;
      case SEGMENT_3_OFF: digitalWrite2(BLDRIVER_PWM_3, LOW );  ScanningState=SEGMENT_4_ON;  OCR1A=GetScanTime6();   break;    
    #endif
    #ifdef BLDRIVER_PWM_4
      case SEGMENT_4_ON:  digitalWrite2(BLDRIVER_PWM_4, HIGH);  ScanningState=SEGMENT_4_OFF; OCR1A=GetScanTime7();   break;
      case SEGMENT_4_OFF: digitalWrite2(BLDRIVER_PWM_4, LOW );  ScanningState=SEGMENT_1_ON;  OCR1A=GetScanTime0();   break;
   // case SEGMENT_4_OFF: digitalWrite2(BLDRIVER_PWM_4, LOW );  ScanningState=SEGMENT_5_ON;  OCR1A=GetScanTime8();   break;
    #endif
    #ifdef BLDRIVER_PWM_5
      case SEGMENT_5_ON:  digitalWrite2(BLDRIVER_PWM_5, HIGH);  ScanningState=SEGMENT_5_OFF; OCR1A=GetScanTime9();   break;
      case SEGMENT_5_OFF: digitalWrite2(BLDRIVER_PWM_5, LOW );  ScanningState=SEGMENT_6_ON;  OCR1A=GetScanTime10();  break;
    #endif
    #ifdef BLDRIVER_PWM_6
      case SEGMENT_6_ON:  digitalWrite2(BLDRIVER_PWM_6, HIGH);  ScanningState=SEGMENT_6_OFF; OCR1A=GetScanTime11();  break;
      case SEGMENT_6_OFF: digitalWrite2(BLDRIVER_PWM_6, LOW );  ScanningState=SEGMENT_1_ON;  OCR1A=GetScanTime0();   break;
    #endif
    #ifdef BLDRIVER_PWM_1
      case SEGMENT_1_ON:  digitalWrite2(BLDRIVER_PWM_1, HIGH);  ScanningState=SEGMENT_1_OFF; OCR1A=GetScanTime1();   break;
      case SEGMENT_1_OFF: digitalWrite2(BLDRIVER_PWM_1, LOW );  ScanningState=SEGMENT_2_ON;  OCR1A=GetScanTime2();   break;
      default :           digitalWrite2(BLDRIVER_PWM_1, HIGH);  ScanningState=SEGMENT_1_OFF; OCR1A=GetScanTime1();   break;
    #endif
  }
}


//void CalculateParametersScan() {
//  const uint8_t ScanDutyCycleMax = 255;
//  const uint8_t ScanDutyCycleEffective = 100; // NOTE: this is only the duty cycle within the potentially active time of the segment.  The actual dutycycle is this value divided by the number of segments.
//  uint16_t ScanTime = ( LOCKED_ACTIVE ) / MyConfigLED.NumberStrings;
//  uint16_t myScanPulseDuration = 1.0 * ScanTime * ScanDutyCycleEffective / ScanDutyCycleMax ;
//  uint16_t myScanPulseDelay    = ScanTime - myScanPulseDuration;
//  uint8_t  myScanADIM     = MAXIMUM_STABLE_ADIM; // TODO: add safety to avoid SafetyMarginScanPrePulse
//  // Calculate optimal parameters here  
//    // TODO: IMPLEMENT THIS
//  // Update the scan configuration
//  CALCULATED_ADIM_SCAN           = myScanADIM;
//  ScanPulseDuration  = myScanPulseDuration;
//  ScanPulseDelay     = myScanPulseDelay;
//  BuildTimeLine();  
//}

boolean CheckCapableScanning(){
  if( FRAME_TIMINGS_LOCKED != true ) {return false;}     
  if( MyConfigLED.SupportsScanning != true ) {return false;}
  if( LOCKED_ACTIVE  < (SafetyMarginScanPrePulse + SafetyMarginScanPostPulse + MINIMUM_ONTIME_SCAN) * MyConfigLED.NumberStrings ) {return false;}
  return true;  
}

uint16_t CalculateSegmentDurationScan() {
  uint16_t myScanSegmentDuration = (( LOCKED_ACTIVE ) / MyConfigLED.NumberStrings);
  return myScanSegmentDuration;  
}

uint16_t CalculateMaxDurationScan() {
  uint16_t myScanPulseDuration = (( LOCKED_ACTIVE ) / MyConfigLED.NumberStrings) - SafetyMarginScanPrePulse;
  if(myScanPulseDuration>ScanPulseDurationMAX) {myScanPulseDuration=ScanPulseDurationMAX;}
  return myScanPulseDuration;  
}


uint8_t CheckScanOverrideConfigurationValid(){return false;}
void PrintScanOverride(){}
void ApplyScanOverrideConfiguration(){}
void DisableScanOverride(){}
void EnableScanOverride(){}
void ScanOverrideCurrent(uint16_t myValue){}
void ScanOverrideDuration(uint16_t myValue){}
void ScanOverrideDelay(uint16_t myValue){}


void CalculateParametersScan() {
  if( FRAME_TIMINGS_LOCKED != true ) {return;}   
  uint16_t myScanPulseDuration = CalculateMaxDurationScan();  
  uint8_t  myScanADIM = MAXIMUM_STABLE_ADIM;
  // Calculate optimal parameters here  

  myScanADIM=OUTPUT_CURRENT_TO_ADIM_LEVEL(CalculateMaximumCurrent()); 
  uint8_t ModeConformsToRules = false;
  uint8_t RemainingIterations = myScanADIM;
  while ((RemainingIterations > 0) && (ModeConformsToRules == false)) {
    ModeConformsToRules = true;
    RemainingIterations = RemainingIterations - 1;
    myScanADIM=RemainingIterations;    
//    SerialDebug("Iteration:"); SerialDebugln(RemainingIterations);
    if ( current_to_voltage(ADIM_LEVEL_TO_OUTPUT_CURRENT(myScanADIM)) < VOLTAGE_MINIMUM) {
//        SerialDebugln(F("NOTE: below minimum output voltage, adjusting to stay within specification."));
      ModeConformsToRules = false;
      myScanADIM = OUTPUT_CURRENT_TO_ADIM_LEVEL(voltage_to_current(VOLTAGE_MINIMUM));
    }
    if ( current_to_voltage(ADIM_LEVEL_TO_OUTPUT_CURRENT(myScanADIM)) > VOLTAGE_MAXIMUM) {
//        SerialDebugln(F("NOTE: above maximum output voltage, adjusting to stay within specification."));
      ModeConformsToRules = false;
      myScanADIM = OUTPUT_CURRENT_TO_ADIM_LEVEL(voltage_to_current(VOLTAGE_MAXIMUM));
    }
    if ( ADIM_LEVEL_TO_OUTPUT_CURRENT(myScanADIM) < CalculateMinimumCurrent()) {
//        SerialDebugln(F("NOTE: below minimum output current, adjusting to stay within specification."));
      ModeConformsToRules = false;
      myScanADIM = OUTPUT_CURRENT_TO_ADIM_LEVEL(CalculateMinimumCurrent());
    }
    // At or below max current by design due to the loop starting point  
  // Now that current and voltage limits have been considered, calculate the ontime needed for the desired brightness
  uint16_t  myFullBrightness = CalculateBrightness(myScanADIM, PWM_MAX);
  myScanPulseDuration = 1.0 * CalculateMaxDurationScan() * TargetBrightnessScan / myFullBrightness ; 
//  SerialDebug(F("PeakBrightness : ")); SerialDebug(myFullBrightness); SerialDebugln(F(" nits")); 
//  SerialDebug(F("TargetBrightness : ")); SerialDebug(TargetBrightnessScan); SerialDebugln(F(" nits")); 
//  SerialDebug(F("myScanCycleTime : ")); SerialDebug(myScanCycleTime); SerialDebugln(F(" us")); 
//  SerialDebug(F("myScanPulseDuration : ")); SerialDebug(myScanPulseDuration); SerialDebugln(F(" us")); 
  

     
  // Apply ontime/offtime limitations.  Correct the values but don't cause another loop iteration, the situation will only get worse.
    if ( myScanPulseDuration < MINIMUM_ONTIME_SCAN) {
//        SerialDebugln(F("NOTE: below minimum pulse duration, adjusting to stay within specification."));
//      ModeConformsToRules = false;
      myScanPulseDuration = MINIMUM_ONTIME_SCAN;
    }
    if ( myScanPulseDuration > CalculateMaxDurationScan()) {
//        SerialDebugln(F("NOTE: above maximum pulse duration, adjusting to stay within specification."));
//      ModeConformsToRules = false;
      myScanPulseDuration = CalculateMaxDurationScan();
    }
  // Apply derated power limit
  // Note: may be able to exceed the board power limit because we can guarantee that no more than one string will be active at any given time
    if ( CalculatePower(myScanADIM, myScanPulseDuration, CalculateSegmentDurationScan()) > CalculateDeratedPowerLimit(myScanPulseDuration, CalculateSegmentDurationScan())) {
//        SerialDebugln(F("NOTE: above maximum output power, adjusting to stay within specification."));
      ModeConformsToRules = false;
    }

    if ( CalculatePower(myScanADIM, myScanPulseDuration, CalculateSegmentDurationScan()) > POWER_LIMIT) { ModeConformsToRules = false; }
    
//    SerialDebugln(F("Scan mode calculations:"));
//    SerialDebug(F("Delay    : ")); SerialDebug(myScanPulseDelay); SerialDebugln(F(" us")); 
//    SerialDebug(F("Duration : ")); SerialDebug(myScanPulseDuration); SerialDebugln(F(" us")); 
//    SerialDebug(F("Current  : "));  SerialDebug(ADIM_LEVEL_TO_OUTPUT_CURRENT(myScanADIM));  SerialDebugln(F("mA"));    
  }
    
  // Update the scan configuration
  CALCULATED_ADIM_SCAN     = myScanADIM;
  ScanPulseDuration = myScanPulseDuration;
  ScanPulseDelay    = CalculateSegmentDurationScan() - myScanPulseDuration;
}






void BuildTimeLine(){
  if( FRAME_TIMINGS_LOCKED != true ) { 
    SetScanTime0(  0 );
    SetScanTime1(  0 );
    SetScanTime2(  0 );
    SetScanTime3(  0 );
    SetScanTime4(  0 );
    SetScanTime5(  0 );
    SetScanTime6(  0 );
    SetScanTime7(  0 );
    SetScanTime8(  0 );
    SetScanTime9(  0 );
    SetScanTime10( 0 );
    SetScanTime11( 0 );
  } else {
    uint16_t myTempON  = Timer1MicrosecondsToTicks(ScanPulseDelayInitial+ScanPulseDelay)+0;
    uint16_t myTempOFF = Timer1MicrosecondsToTicks(ScanPulseDuration)+myTempON;
    SetScanTime2(myTempON); 
    SetScanTime3(myTempOFF);    
    if((MyConfigLED.NumberStrings) >1) { myTempON = Timer1MicrosecondsToTicks(ScanPulseDelay)+myTempOFF; } 
    myTempOFF = Timer1MicrosecondsToTicks(ScanPulseDuration)+myTempON;
    SetScanTime4(myTempON); 
    SetScanTime5(myTempOFF); 
    if((MyConfigLED.NumberStrings) >2) { myTempON = Timer1MicrosecondsToTicks(ScanPulseDelay)+myTempOFF; }
    myTempOFF = Timer1MicrosecondsToTicks(ScanPulseDuration)+myTempON;
    SetScanTime6(myTempON); 
    SetScanTime7(myTempOFF); 
    if((MyConfigLED.NumberStrings) >3) { myTempON = Timer1MicrosecondsToTicks(ScanPulseDelay)+myTempOFF; } 
    myTempOFF = Timer1MicrosecondsToTicks(ScanPulseDuration)+myTempON;
    SetScanTime8(myTempON); 
    SetScanTime9(myTempOFF); 
    if((MyConfigLED.NumberStrings) >4) { myTempON = Timer1MicrosecondsToTicks(ScanPulseDelay)+myTempOFF; } 
    myTempOFF = Timer1MicrosecondsToTicks(ScanPulseDuration)+myTempON;
    SetScanTime10(myTempON); 
    SetScanTime11(myTempOFF); 
    if((MyConfigLED.NumberStrings) >5) { myTempON = Timer1MicrosecondsToTicks(ScanPulseDelay)+myTempOFF; } 

    // The last pulse can be delayed longer than most, since it aligns to the beginning of the next frame
    boolean UseExtaDelayOnFirstScanZone = true;
    if( UseExtaDelayOnFirstScanZone == false ) {
      myTempOFF = Timer1MicrosecondsToTicks(ScanPulseDuration)+myTempON;
    } else {
      myTempOFF = Timer1MicrosecondsToTicks( LOCKED_ACTIVE + LOCKED_BLANKING - SafetyMarginScanPostPulse);
      myTempON = myTempOFF - Timer1MicrosecondsToTicks(ScanPulseDuration);
    }
    SetScanTime0(myTempON); 
    SetScanTime1(myTempOFF); 
    if((MyConfigLED.NumberStrings) <6) { SetScanTime10(myTempON); SetScanTime11(myTempOFF); } 
    if((MyConfigLED.NumberStrings) <5) { SetScanTime8( myTempON); SetScanTime9( myTempOFF); } 
    if((MyConfigLED.NumberStrings) <4) { SetScanTime6( myTempON); SetScanTime7( myTempOFF); } 
    if((MyConfigLED.NumberStrings) <3) { SetScanTime4( myTempON); SetScanTime5( myTempOFF); } 
    if((MyConfigLED.NumberStrings) <2) { SetScanTime2( myTempON); SetScanTime3( myTempOFF); }   
  }
  SwapScanTimeValueBuffers();    
}



uint8_t CurrentScanTimeValues=0;
uint16_t ScanningTime0_0  =0;   
uint16_t ScanningTime0_1  =0;   
uint16_t ScanningTime0_2  =0;   
uint16_t ScanningTime0_3  =0;   
uint16_t ScanningTime0_4  =0;   
uint16_t ScanningTime0_5  =0;   
uint16_t ScanningTime0_6  =0;   
uint16_t ScanningTime0_7  =0;   
uint16_t ScanningTime0_8  =0;   
uint16_t ScanningTime0_9  =0;   
uint16_t ScanningTime0_10 =0;  
uint16_t ScanningTime0_11 =0; 
uint16_t ScanningTime1_0  =0;   
uint16_t ScanningTime1_1  =0;   
uint16_t ScanningTime1_2  =0;   
uint16_t ScanningTime1_3  =0;   
uint16_t ScanningTime1_4  =0;   
uint16_t ScanningTime1_5  =0;   
uint16_t ScanningTime1_6  =0;   
uint16_t ScanningTime1_7  =0;   
uint16_t ScanningTime1_8  =0;   
uint16_t ScanningTime1_9  =0;   
uint16_t ScanningTime1_10 =0;  
uint16_t ScanningTime1_11 =0; 


uint16_t GetScanTime0() { if(CurrentScanTimeValues==0) {return ScanningTime0_0 ;} else {return ScanningTime1_0 ;} }    // At this time, turn output 1 ON
uint16_t GetScanTime1() { if(CurrentScanTimeValues==0) {return ScanningTime0_1 ;} else {return ScanningTime1_1 ;} }    // At this time, turn output 1 OFF
uint16_t GetScanTime2() { if(CurrentScanTimeValues==0) {return ScanningTime0_2 ;} else {return ScanningTime1_2 ;} }    // At this time, turn output 2 ON
uint16_t GetScanTime3() { if(CurrentScanTimeValues==0) {return ScanningTime0_3 ;} else {return ScanningTime1_3 ;} }    // At this time, turn output 2 OFF
uint16_t GetScanTime4() { if(CurrentScanTimeValues==0) {return ScanningTime0_4 ;} else {return ScanningTime1_4 ;} }    // At this time, turn output 3 ON
uint16_t GetScanTime5() { if(CurrentScanTimeValues==0) {return ScanningTime0_5 ;} else {return ScanningTime1_5 ;} }    // At this time, turn output 3 OFF
uint16_t GetScanTime6() { if(CurrentScanTimeValues==0) {return ScanningTime0_6 ;} else {return ScanningTime1_6 ;} }    // At this time, turn output 4 ON
uint16_t GetScanTime7() { if(CurrentScanTimeValues==0) {return ScanningTime0_7 ;} else {return ScanningTime1_7 ;} }    // At this time, turn output 4 OFF
uint16_t GetScanTime8() { if(CurrentScanTimeValues==0) {return ScanningTime0_8 ;} else {return ScanningTime1_8 ;} }    // At this time, turn output 5 ON
uint16_t GetScanTime9() { if(CurrentScanTimeValues==0) {return ScanningTime0_9 ;} else {return ScanningTime1_9 ;} }    // At this time, turn output 5 OFF
uint16_t GetScanTime10(){ if(CurrentScanTimeValues==0) {return ScanningTime0_10;} else {return ScanningTime1_10;} }    // At this time, turn output 6 ON
uint16_t GetScanTime11(){ if(CurrentScanTimeValues==0) {return ScanningTime0_11;} else {return ScanningTime1_11;} }    // At this time, turn output 6 OFF

void SetScanTime0( uint16_t myValue){ if(CurrentScanTimeValues==0) {ScanningTime1_0 =myValue;} else {ScanningTime0_0 =myValue;} }
void SetScanTime1( uint16_t myValue){ if(CurrentScanTimeValues==0) {ScanningTime1_1 =myValue;} else {ScanningTime0_1 =myValue;} }
void SetScanTime2( uint16_t myValue){ if(CurrentScanTimeValues==0) {ScanningTime1_2 =myValue;} else {ScanningTime0_2 =myValue;} }
void SetScanTime3( uint16_t myValue){ if(CurrentScanTimeValues==0) {ScanningTime1_3 =myValue;} else {ScanningTime0_3 =myValue;} }
void SetScanTime4( uint16_t myValue){ if(CurrentScanTimeValues==0) {ScanningTime1_4 =myValue;} else {ScanningTime0_4 =myValue;} }
void SetScanTime5( uint16_t myValue){ if(CurrentScanTimeValues==0) {ScanningTime1_5 =myValue;} else {ScanningTime0_5 =myValue;} }
void SetScanTime6( uint16_t myValue){ if(CurrentScanTimeValues==0) {ScanningTime1_6 =myValue;} else {ScanningTime0_6 =myValue;} }
void SetScanTime7( uint16_t myValue){ if(CurrentScanTimeValues==0) {ScanningTime1_7 =myValue;} else {ScanningTime0_7 =myValue;} }
void SetScanTime8( uint16_t myValue){ if(CurrentScanTimeValues==0) {ScanningTime1_8 =myValue;} else {ScanningTime0_8 =myValue;} }
void SetScanTime9( uint16_t myValue){ if(CurrentScanTimeValues==0) {ScanningTime1_9 =myValue;} else {ScanningTime0_9 =myValue;} }
void SetScanTime10(uint16_t myValue){ if(CurrentScanTimeValues==0) {ScanningTime1_10=myValue;} else {ScanningTime0_10=myValue;} }
void SetScanTime11(uint16_t myValue){ if(CurrentScanTimeValues==0) {ScanningTime1_11=myValue;} else {ScanningTime0_11=myValue;} }

void SwapScanTimeValueBuffers(){if(CurrentScanTimeValues==0) {CurrentScanTimeValues=1;} else {CurrentScanTimeValues=0;}}





