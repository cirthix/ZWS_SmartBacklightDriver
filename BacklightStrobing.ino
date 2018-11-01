// Each of the operating modes must provide some functions:
// Rule checking function
// Brightness calculation function
// Status printing function
// Rely on internal global variables

// Scanning uses timer1 and the pin interrupt
// Assumptions with the current implementation:
// Only one pulse will be active at any given time.  There must be a small margin of time between pulses for the interrupt to retrigger
// The pin interrupt will turn off all outputs regardless of state, as will the overflow interrupt.  This is a safety measure.


const uint16_t StrobePulseDurationMAX = OutputPulseDurationMAX; // units=microseconds
const uint16_t SafetyMarginPostPinInterrupt = 40; // Number of microseconds between the pin interrupt and the first allowed timer interrupt after it
const uint16_t SafetyMarginPostTimerInterrupt = 40; // Number of microseconds between the timer interrupt and the first allowed timer interrupt after it
const uint16_t SafetyMarginPrePulse         = 10; // Number of microseconds between blanking start and strobe start
const uint16_t SafetyMarginPostPulse        = 80; // Number of microseconds between strobe pulse falling and expected start of next frame
const uint16_t StrobeBlankingMinimum        = 500+SafetyMarginPrePulse + SafetyMarginPostPulse + MINIMUM_ONTIME; // units=microcseconds.  Dont' enable strobe mode unless an extended vblank signal is present.

void PrintConfigStrobe(){
      SerialDebugln(F("STROBE"));
      SerialDebug(F("OffsetDelay   : "));  SerialDebug(StrobePulseDelay);  SerialDebugln(F("us"));
      SerialDebug(F("PulseDuration : "));  SerialDebug(StrobePulseDuration);  SerialDebugln(F(" us"));
      SerialDebug(F("SerialStart   : "));  SerialDebug(StrobeSendSoftSerial);  SerialDebugln(F(" us"));
      SerialDebug(F("Current       : "));  SerialDebug(ADIM_LEVEL_TO_OUTPUT_CURRENT(CALCULATED_ADIM_STROBE));  SerialDebugln(F("mA"));
      SerialDebug(F("Brightness    : "));  SerialDebug(CalculateBrightnessStrobe());  SerialDebugln(F("nits"));
      SerialDebug(F("PowerOutput   : "));  SerialDebug(CalculatePowerStrobe());  SerialDebugln(F("mW"));
      SerialDebug(F("Time+ : "));  SerialDebug(StrobePulseHigh); SerialDebugln(F(" ticks"));
      SerialDebug(F("Time- : "));  SerialDebug(StrobePulseLow); SerialDebugln(F(" ticks"));
      SerialDebug(F("TimeS : "));  SerialDebug(StrobeSerial); SerialDebugln(F(" ticks"));
}

void EnterStrobing(){
    StopSyncInterrupt();
    POWER_LIMIT=INITIAL_POWER_LIMIT;
    adimWrite(CALCULATED_ADIM_OFF);
    WriteAllPWMsLOW();
    wdt_reset();
    adimWrite(CALCULATED_ADIM_STROBE);
    OUTPUT_MODE = OUTPUT_MODE_STROBE;
    ConfigureTimersStrobing();
    StartSyncInterruptChange();
    IndicateNewState(); 
}



  
void ConfigureTimersStrobing(){  
    SerialDebugln(F("Configuring timers for strobing mode"));
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
    AdjustTimerStrobing();  
    OCR1B=StrobePulseHigh;    
    TIMSK1 =0x00 | (1 << OCIE1B);    //Set interrupt B on compare match  
}

// This ISR is modified for visual inspection of flickering from vcom/vgamma imbalance

//volatile boolean EvenOdd = true;
//volatile boolean DisplayingEvenOdd = true;
//volatile uint8_t FrameCounter = 0;
//const uint8_t FramesPerField = 240; // swap displayed polarity every second (at 240Hz)
//ISR(TIMER1_COMPB_vect) {  
//  if(OCR1B==StrobePulseHigh) {
//    OCR1B=StrobePulseLow;       
//    if(EvenOdd==true) { EvenOdd=false; } else {EvenOdd=true;}
//    if(FrameCounter<FramesPerField) { FrameCounter=FrameCounter+1; } else {FrameCounter=0; if(DisplayingEvenOdd==true) { DisplayingEvenOdd=false; } else {DisplayingEvenOdd=true;}}
//    if(EvenOdd!=DisplayingEvenOdd) { return;}
//    WritePulseToAllPins(HIGH);
//    }  
//  else {WritePulseToAllPins(LOW); OCR1B=StrobePulseHigh;   }
//}
// This is the unmodified ISR 
ISR(TIMER1_COMPB_vect) {
  if(OCR1B==StrobePulseHigh) {
    WritePulseToAllPins(HIGH);
    OCR1B=StrobePulseLow;
  } else {
    if(OCR1B==StrobePulseLow) {
      WritePulseToAllPins(LOW);
      OCR1B=StrobeSerial;
    } else {
      OCR1B=StrobePulseHigh;  
      InfraredStereoTransmitter.SendInfraredSyncToGlasses_PulsedBacklight();
      DoPeriodicTimingSensitiveActivity();
    }
  }
}


void AdjustTimerStrobing(){
  StrobePulseHigh = Timer1MicrosecondsToTicks(StrobePulseDelay);
  StrobePulseLow  = Timer1MicrosecondsToTicks(StrobePulseDuration) + StrobePulseHigh;
  StrobeSerial    = Timer1MicrosecondsToTicks(StrobeSendSoftSerial);
}


boolean CheckCapableStrobing(){
  if( FRAME_TIMINGS_LOCKED != true ) {return false;}     
  if( LOCKED_BLANKING < StrobeBlankingMinimum ) { return false; }
  return true;  
}

uint16_t CalculateMaxDurationStrobe() {
  uint16_t myStrobePulseDuration = LOCKED_BLANKING-(SafetyMarginPrePulse+SafetyMarginPostPulse);
  if(myStrobePulseDuration>StrobePulseDurationMAX) {myStrobePulseDuration=StrobePulseDurationMAX;}
  return myStrobePulseDuration;  
}




boolean StrobeConfigOverrideDesired = false;
uint16_t StrobeConfigOverrideCurrent = 0;
uint16_t StrobeConfigOverrideDuration = 0;
uint16_t StrobeConfigOverrideDelay = 0;
uint16_t StrobeSendSoftSerialOverride = 0;


void PrintStrobeOverride(){  
      SerialDebug(F("OverrideDesired  : "));  if(StrobeConfigOverrideDesired!=true) { SerialDebugln(F("FALSE"));  } else { SerialDebugln(F("TRUE"));  }
      SerialDebug(F("OverrideValid    : "));  if(CheckStrobeOverrideConfigurationValid()!=true) { SerialDebugln(F("FALSE"));  } else { SerialDebugln(F("TRUE"));  }
      SerialDebug(F("OverrideCurrent  : "));  SerialDebug(StrobeConfigOverrideCurrent);  SerialDebugln(F("mA"));
      SerialDebug(F("OverrideDelay    : "));  SerialDebug(StrobeConfigOverrideDelay);  SerialDebugln(F("us"));
      SerialDebug(F("OverrideDuration : "));  SerialDebug(StrobeConfigOverrideDuration);  SerialDebugln(F(" us"));
  }

uint8_t CheckStrobeOverrideConfigurationValid(){ 
   // Until the output current is not ignored, all values should be safe
if(StrobeConfigOverrideDesired!=true) { return false;}
   StrobeOverrideDuration(StrobeConfigOverrideDuration);
   StrobeOverrideDelay(StrobeConfigOverrideDelay); // This line is needed in case the duraton was adjusted after the delay.
   if(StrobeSendSoftSerial==0) {return false;}
   StrobeOverrideCurrent(StrobeConfigOverrideCurrent); // An extra safety measure
  return true;
}


void ApplyStrobeOverrideConfiguration(){
  CALCULATED_ADIM_STROBE = OUTPUT_CURRENT_TO_ADIM_LEVEL(StrobeConfigOverrideCurrent);
  StrobePulseDuration    = StrobeConfigOverrideDuration;
  StrobePulseDelay       = StrobeConfigOverrideDelay;
  StrobeSendSoftSerial   = StrobeSendSoftSerialOverride;
  AdjustTimerStrobing();
}


void DisableStrobeOverride(){StrobeConfigOverrideDesired = false;}
void EnableStrobeOverride() {StrobeConfigOverrideDesired = true;}

void StrobeOverrideCurrent(uint16_t myValue){
  StrobeConfigOverrideCurrent=ADIM_LEVEL_TO_OUTPUT_CURRENT(MAXIMUM_STABLE_ADIM); // Until the safety validation function on the override inputs is done, ignore requested output current and use the max 100% dutycycle value.
  }
void StrobeOverrideDuration(uint16_t myValue){
  if(myValue < MINIMUM_ONTIME) {StrobeConfigOverrideDuration=MINIMUM_ONTIME; return;}
  if(myValue > StrobePulseDurationMAX) {StrobeConfigOverrideDuration=StrobePulseDurationMAX; return;}
  StrobeConfigOverrideDuration=myValue;
  }
void StrobeOverrideDelay(uint16_t myValue){  
  StrobeConfigOverrideDelay=myValue;  
uint16_t MaxDelay=LOCKED_ACTIVE+LOCKED_BLANKING-(StrobeConfigOverrideDuration+SafetyMarginPostPulse);
uint16_t MinDelay=SafetyMarginPostPinInterrupt;

  if(myValue < MinDelay) {StrobeConfigOverrideDelay=MinDelay;}  
  if(myValue > MaxDelay) {StrobeConfigOverrideDelay=MaxDelay;}


uint16_t CutoffForEarliestPulse=SafetyMarginPostPinInterrupt+TIME_REQUIRED_FOR_TIMING_SENSITIVE_ACTIVITY+SafetyMarginPostTimerInterrupt;
uint16_t EarlySerialTime=0 + SafetyMarginPostPinInterrupt;
uint16_t CutoffForLatestPulse=LOCKED_ACTIVE+LOCKED_BLANKING-(TIME_REQUIRED_FOR_TIMING_SENSITIVE_ACTIVITY+SafetyMarginPostTimerInterrupt+StrobeConfigOverrideDuration+SafetyMarginPostPulse);
uint16_t LateSerialTime= StrobeConfigOverrideDelay + StrobeConfigOverrideDuration + SafetyMarginPostTimerInterrupt ;

  
  if(StrobeConfigOverrideDelay > CutoffForEarliestPulse) {StrobeSendSoftSerialOverride = EarlySerialTime; }
  else { 
    if(StrobeConfigOverrideDelay<CutoffForLatestPulse) {StrobeSendSoftSerialOverride = LateSerialTime; }
   else { StrobeSendSoftSerialOverride=0;}
  }  
  }


void CalculateParametersStrobe() {
  if( FRAME_TIMINGS_LOCKED != true ) {return;} 
  if(CheckStrobeOverrideConfigurationValid()==true) { ApplyStrobeOverrideConfiguration(); return;}
  
  uint16_t myStrobeCycleTime   = LOCKED_ACTIVE+LOCKED_BLANKING;
  uint16_t myStrobePulseDelay    = LOCKED_ACTIVE+SafetyMarginPrePulse;  
  uint16_t myStrobePulseDuration = LOCKED_BLANKING-(SafetyMarginPrePulse+SafetyMarginPostPulse);
  uint8_t  myStrobeADIM = MAXIMUM_STABLE_ADIM;
  // Calculate optimal parameters here  
    myStrobePulseDuration=CalculateMaxDurationStrobe();

    myStrobeADIM=OUTPUT_CURRENT_TO_ADIM_LEVEL(CalculateMaximumCurrent());

  uint8_t ModeConformsToRules = false;
  uint8_t RemainingIterations = myStrobeADIM;
  while ((RemainingIterations > 0) && (ModeConformsToRules == false)) {
    ModeConformsToRules = true;
    RemainingIterations = RemainingIterations - 1;
    myStrobeADIM=RemainingIterations;    
//    SerialDebug("Iteration:"); SerialDebugln(RemainingIterations);
    if ( current_to_voltage(ADIM_LEVEL_TO_OUTPUT_CURRENT(myStrobeADIM)) < VOLTAGE_MINIMUM) {
//        SerialDebugln(F("NOTE: below minimum output voltage, adjusting to stay within specification."));
      ModeConformsToRules = false;
      myStrobeADIM = OUTPUT_CURRENT_TO_ADIM_LEVEL(voltage_to_current(VOLTAGE_MINIMUM));
    }
    if ( current_to_voltage(ADIM_LEVEL_TO_OUTPUT_CURRENT(myStrobeADIM)) > VOLTAGE_MAXIMUM) {
//        SerialDebugln(F("NOTE: above maximum output voltage, adjusting to stay within specification."));
      ModeConformsToRules = false;
      myStrobeADIM = OUTPUT_CURRENT_TO_ADIM_LEVEL(voltage_to_current(VOLTAGE_MAXIMUM));
    }
    if ( ADIM_LEVEL_TO_OUTPUT_CURRENT(myStrobeADIM) < CalculateMinimumCurrent()) {
//        SerialDebugln(F("NOTE: below minimum output current, adjusting to stay within specification."));
      ModeConformsToRules = false;
      myStrobeADIM = OUTPUT_CURRENT_TO_ADIM_LEVEL(CalculateMinimumCurrent());
    }
    // At or below max current by design due to the loop starting point  
    
  // Now that current and voltage limits have been considered, calculate the ontime needed for the desired brightness
  uint16_t  myFullBrightness = CalculateBrightness(myStrobeADIM, PWM_MAX);
  myStrobePulseDuration = 1.0 * myStrobeCycleTime * TargetBrightnessStrobe / myFullBrightness ; 
//  SerialDebug(F("PeakBrightness : ")); SerialDebug(myFullBrightness); SerialDebugln(F(" nits")); 
//  SerialDebug(F("TargetBrightness : ")); SerialDebug(TargetBrightnessStrobe); SerialDebugln(F(" nits")); 
//  SerialDebug(F("myStrobeCycleTime : ")); SerialDebug(myStrobeCycleTime); SerialDebugln(F(" us")); 
//  SerialDebug(F("myStrobePulseDuration : ")); SerialDebug(myStrobePulseDuration); SerialDebugln(F(" us")); 
  

     
  // Apply ontime/offtime limitations.  Correct the values but don't cause another loop iteration, the situation will only get worse.
    if ( myStrobePulseDuration < MINIMUM_ONTIME) {
//        SerialDebugln(F("NOTE: below minimum pulse duration, adjusting to stay within specification."));
//      ModeConformsToRules = false;
      myStrobePulseDuration = MINIMUM_ONTIME;
    }
    if ( myStrobePulseDuration > CalculateMaxDurationStrobe()) {
//        SerialDebugln(F("NOTE: above maximum pulse duration, adjusting to stay within specification."));
//      ModeConformsToRules = false;
      myStrobePulseDuration = CalculateMaxDurationStrobe();
    }
  // Apply derated power limit
    if ( CalculatePower(myStrobeADIM, myStrobePulseDuration, myStrobeCycleTime) > CalculateDeratedPowerLimit(myStrobePulseDuration, myStrobeCycleTime)) {
//        SerialDebugln(F("NOTE: above maximum output power, adjusting to stay within specification."));
      ModeConformsToRules = false;
    }
    
    if ( CalculatePower(myStrobeADIM, myStrobePulseDuration, myStrobeCycleTime) > POWER_LIMIT) { ModeConformsToRules = false; }
//    SerialDebugln(F("Strobe mode calculations:"));
//    SerialDebug(F("Delay    : ")); SerialDebug(myStrobePulseDelay); SerialDebugln(F(" us")); 
//    SerialDebug(F("Duration : ")); SerialDebug(myStrobePulseDuration); SerialDebugln(F(" us")); 
//    SerialDebug(F("Current  : "));  SerialDebug(ADIM_LEVEL_TO_OUTPUT_CURRENT(myStrobeADIM));  SerialDebugln(F("mA"));    
  }
    
  // Update the strobe configuration
  CALCULATED_ADIM_STROBE     = myStrobeADIM;
  StrobeSendSoftSerial = 0 + SafetyMarginPostPinInterrupt ; 
  StrobePulseDuration = myStrobePulseDuration;
  StrobePulseDelay    = LOCKED_ACTIVE+LOCKED_BLANKING-(myStrobePulseDuration+SafetyMarginPostPulse);
}





// This old code using the one-shot pulse generation method of the timer only works for the 'b' output of each timer, so only a few pins could be used.
// If this limitation is acceptable, the one-shot pulse method is very safe and reliable
/*
// Each of the operating modes must provide some functions:
// Rule checking function
// Brightness calculation function
// Status printing function
// Rely on internal global variables

uint8_t TargetTimer2DividerStrobe=0x04; // Default to 4ms max


void EnterStrobing(){
    WriteAllPWMs(0);
    ConfigureTimersStrobing();
    adimWrite(CALCULATED_ADIM_STROBE);  
    AdjustTimerStrobing();
    OUTPUT_MODE = OUTPUT_MODE_STROBE;
    IndicateNewState(); 
}



#define OSP_SET_WIDTH(cycles) (OCR2B = 0xff-(cycles-1))
#define OSP_FIRE() (TCNT2 = STROBE_TRIGGER_POINT)
#define OSP_FORCE_OFF() (TCNT2 = 0)


  // As of now, this function works but does not support variable length.  These should be added at a later date
void InterruptGotPulse(){
  if(digitalRead2(INPUT_PULSE) == HIGH){
    if(OUTPUT_MODE == OUTPUT_MODE_STROBE) {
      OSP_FIRE();    
    }
    RecentInputPulse=true;
    PulseInputTime=TCNT1; 
    TCNT1=0;  
  } else {
    PulseInputTimeHigh=TCNT1; 
//    if(OUTPUT_MODE == OUTPUT_MODE_STROBE) {
//      OSP_FORCE_OFF();    
//    }
//    PulseInputTimeLOW=TCNT1; 
//    TCNT1=0;
  }    
  
}

void ConfigureTimersStrobing(){  
  if((StrobeDelay+StrobeDuration)< 8000 ) { TargetTimer2DividerStrobe=0x06; HysteresisInputPulseHigh=64; } // Using the 1:256 clock divider for 8ms max duration wtih 32us resolution
  if((StrobeDelay+StrobeDuration)< 4000 ) { TargetTimer2DividerStrobe=0x05; HysteresisInputPulseHigh=32; } // Using the 1:128 clock divider for 4ms max duration wtih 16us resolution
  if((StrobeDelay+StrobeDuration)< 2000 ) { TargetTimer2DividerStrobe=0x04; HysteresisInputPulseHigh=16; } // Using the 1:64  clock divider for 2ms max duration wtih 8us resolution
  if((StrobeDelay+StrobeDuration)< 1000 ) { TargetTimer2DividerStrobe=0x03; HysteresisInputPulseHigh=8; } // Using the 1:32  clock divider for 1ms max duration wtih 4us resolution

  TCCR2B =  0;      // Halt counter by setting clock select bits to 0 (No clock source).
              // This keeps anything from happeneing while we get set up

  TCNT2 = 0x00;     // Start counting at bottom. 
  OCR2A = 0;      // Set TOP to 0. This effectively keeps us from counting becuase the counter just keeps reseting back to 0.
          // We break out of this by manually setting the TCNT higher than 0, in which case it will count all the way up to MAX and then overflow back to 0 and get locked up again.
          
  AdjustTimerStrobing();  // This part of the function is broken off to allow for updates to the duration and delay without fully reconfiguring the timer.
  
  TCCR2A = _BV(COM2B0) | _BV(COM2B1) | _BV(WGM20) | _BV(WGM21); // OC2B=Set on Match, clear on BOTTOM. Mode 7 Fast PWM.
  TCCR2B = _BV(WGM22)| TargetTimer2DividerStrobe;         // Start counting now. WGM22=1 to select Fast PWM mode 7
}

void AdjustTimerStrobing(){
  // If not careful, updating this while actively strobing could create a pulse on the pwm output that violates the minimum ontime and minimum offtime.
  // Avoid this by only updating while the timer is idle, but make sure that it is actually counting to avoid a lockup situation
  while((TCNT2!=0) && (GetCurrentTimer2ClockDivider()!=0x00))  { ; } 
  uint8_t myDuration= 1.0*(StrobeDuration*DetermineTimer2TickRate(TargetTimer2DividerStrobe))/1000000;
  StrobeDurationReal=Timer2TicksToMicrocseconds(TargetTimer2DividerStrobe,myDuration);
  uint8_t myDelay= 1.0*(StrobeDelay*DetermineTimer2TickRate(TargetTimer2DividerStrobe))/1000000;
//  SerialDebug(F("DEBUGDURATION: ")); SerialDebugln(myDuration);
//  SerialDebug(F("DEBUGDELAY: ")); SerialDebugln(myDelay);
//  SerialDebug(F("DEBUGDURATION2: ")); SerialDebugln(StrobeDurationReal);
  OSP_SET_WIDTH(myDuration);
  STROBE_TRIGGER_POINT=OCR2B-1-myDelay;
}

*/
