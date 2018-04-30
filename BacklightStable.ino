// Each of the operating modes must provide some functions:
// Rule checking function
// Brightness calculation function
// Status printing function
// Rely on internal global variables

// TODO: to support other dividers, need to add logic to count overflows to keep the 65ms safety margin of timer1.


void PrintConfigStable(){
      SerialDebug(F("STABLE/PWM"));
      if (CALCULATED_PWM_STABLE < PWM_MAX) {
        SerialDebugln(F(""));
        SerialDebug(F("PWMfreq   :"));  SerialDebug(DetermineTimer2FrequencyPWM(TargetTimerDividerStable));  SerialDebugln(F("Hz"));
        SerialDebug(F("Dutycycle : "));  SerialDebug(100.0 * CALCULATED_PWM_STABLE / PWM_MAX);  SerialDebugln(F(""));
      } else {
        SerialDebugln(F("-FREE"));
      }
      SerialDebug(F("Current     : "));  SerialDebug(ADIM_LEVEL_TO_OUTPUT_CURRENT(CALCULATED_ADIM_STABLE));  SerialDebugln(F("mA"));
      SerialDebug(F("Brightness  : "));  SerialDebug(CalculateBrightnessStable());  SerialDebugln(F("nits"));
      SerialDebug(F("PowerOutput : "));  SerialDebug(CalculatePowerStable());  SerialDebugln(F("mW"));
}


void CalculateParametersStable() {
  uint8_t myStablePWM;
  uint8_t myStableADIM;
  uint16_t myCurrentMIN = CalculateMinimumCurrent();
  //  SerialDebug(F("Target brightness : "));  SerialDebug(TargetBrightnessStable); SerialDebugln(F("nits"));
  if (TargetBrightnessStable < current_to_brightness(myCurrentMIN)) {
    //  SerialDebug(F("NOTE: below minimum supported current, using mixed PWM mode.  MIN : ")); SerialDebug(myCurrentMIN);  SerialDebugln(F("mA"));
    //  SerialDebug(F("Minimum brightness : "));  SerialDebug(current_to_brightness(myCurrentMIN)); SerialDebugln(F("nits"));
    myStablePWM = PWM_MAX * 1.0 * TargetBrightnessStable / current_to_brightness(myCurrentMIN);
    myStableADIM = OUTPUT_CURRENT_TO_ADIM_LEVEL(myCurrentMIN);
  } else {
    myStablePWM = 255;
    myStableADIM = OUTPUT_CURRENT_TO_ADIM_LEVEL(brightness_to_current(TargetBrightnessStable));
  }
  uint8_t ModeConformsToRules = false;
  uint8_t RemainingIterations = 127;
  while ((RemainingIterations > 0) && (ModeConformsToRules == false)) {
    ModeConformsToRules = true;
    RemainingIterations = RemainingIterations - 1;
    //SerialDebug("pwm:"); SerialDebugln(myStablePWM);
    if ( current_to_voltage(ADIM_LEVEL_TO_OUTPUT_CURRENT(myStableADIM)) < VOLTAGE_MINIMUM) {
      //  SerialDebugln(F("NOTE: below minimum output voltage, adjusting to stay within specification."));
      ModeConformsToRules = false;
      myStableADIM = OUTPUT_CURRENT_TO_ADIM_LEVEL(voltage_to_current(VOLTAGE_MINIMUM));
    }
    if ( current_to_voltage(ADIM_LEVEL_TO_OUTPUT_CURRENT(myStableADIM)) > VOLTAGE_MAXIMUM) {
      //  SerialDebugln(F("NOTE: above maximum output voltage, adjusting to stay within specification."));
      ModeConformsToRules = false;
      myStableADIM = OUTPUT_CURRENT_TO_ADIM_LEVEL(voltage_to_current(VOLTAGE_MAXIMUM));
    }
    // Avoid minimum ontime / minimum offtime conditions
    if ((myStablePWM < CALCULATED_PARTIAL_DUTYCYCLE_MINIMUM) && (myStablePWM != 0)) {
      //  SerialDebugln(F("NOTE: below minimum on-time, using minimum brightness level."));
      ModeConformsToRules = false;
      myStablePWM = CALCULATED_PARTIAL_DUTYCYCLE_MINIMUM;
    }
    if ((myStablePWM > CALCULATED_PARTIAL_DUTYCYCLE_MAXIMUM) && (myStablePWM != PWM_MAX)) {
      //  SerialDebugln(F("NOTE: below minimum off-time, adjusting current to compensate."));
      ModeConformsToRules = false;
      myStablePWM = CALCULATED_PARTIAL_DUTYCYCLE_MAXIMUM;
      // This situation will occur just below the minimum constant current operating point.
      // Ensure a smooth transition by increasing led current to compensate for the minimum off time
      myStableADIM = OUTPUT_CURRENT_TO_ADIM_LEVEL(brightness_to_current(TargetBrightnessStable * PWM_MAX / CALCULATED_PARTIAL_DUTYCYCLE_MAXIMUM));
    }
    // Now that minimum ontime and offtime limits have been applied, need to apply maximum current/power limit
    if (myStableADIM > MAXIMUM_STABLE_ADIM) {
      ModeConformsToRules = false;
      myStableADIM = MAXIMUM_STABLE_ADIM;
    }
  }
  if ( ModeConformsToRules == true ) {
    CALCULATED_PWM_STABLE = myStablePWM;
    CALCULATED_ADIM_STABLE = myStableADIM;
  } else {
    CALCULATED_PWM_STABLE = 0;
    CALCULATED_ADIM_STABLE = 0;
  }
}



void EnterStable(){
    StopSyncInterrupt();
    POWER_LIMIT=INITIAL_POWER_LIMIT;
    adimWrite(CALCULATED_ADIM_OFF);
    WriteAllPWMsLOW();
    wdt_reset();
    adimWrite(CALCULATED_ADIM_STABLE);
    OUTPUT_MODE = OUTPUT_MODE_STABLE;
    ConfigureTimersPWM();
    StartSyncInterruptChange();
    IndicateNewState(); 
}



void ConfigureTimersPWM(){  
    SerialDebugln(F("Configuring timers for stable mode"));
WriteAllPWMsLOW(); 
// To avoid a glitch when disabling the timer clocks, disable the outputs temporarily

// Stop timer interrupts
TIMSK0=0x00|(1<<TOIE0); // To count millis and micros properly while in stable mode
TIMSK1=0x00; // Leave the overflow interrupt enabled
TIMSK2=0x00; // Disable interrupts here

// Stop the output clocks, unconfigure the timers
TCCR0B=0x00;
TCCR1B=0x00;
TCCR2B=0x00;
TCCR0A=0x00;
TCCR1A=0x00;
TCCR2A=0x00;

TIFR1 = (1 << ICF1) | (1 << OCF1A) | (1 << OCF1B) | (1 << TOV1); // Clear timer 1 interrupts for a fresh start

//  Set fastPWM 8bit mode for each timer
TCCR0A=0x00|(1<<WGM01)|(1<<WGM00);
TCCR1A=0x00|(1<<WGM10); // Set WGM20 for fast mode
TCCR1B=0x00|(1<<WGM12);
TCCR2A=0x00|(1<<WGM21)|(1<<WGM20);

// Add the phase offsets (offset phase reduces peak currents, improves efficiency, and reduces audible noise)
// If using two or four strings
TCNT0=0;
TCNT1=0;
TCNT2=PWM_MAX/2;

// If using three or six strings
//TCNT0=0;
//TCNT1=1*PWM_MAX/3;
//TCNT2=2*PWM_MAX/3;

// Turn on the clocks
SetTimer0ClockDivider(DetermineCorrectTimer0Divider(DetermineTimer0Prescaler(TargetTimerDividerStable))); // NOTE: Requires a modified version of wiring.c that is capable of operating with different timer0 clock prescalers.
SetTimer1ClockDivider(DetermineCorrectTimer1Divider(DetermineTimer0Prescaler(TargetTimerDividerStable))); // Also note: TargetTimerDividerStable refers to the divider of timer0 prescaler, which is then used for all timers.
SetTimer2ClockDivider(DetermineCorrectTimer2Divider(DetermineTimer0Prescaler(TargetTimerDividerStable)));



// I'm not sure why, but there is a bug here, i think withink the hardware, which causes some channels to turn off until the led driver is enable-cycled.  The patch is to always enable-cycle the led driver when entering pwm mode.
wdt_reset();
OutputDisable();
delay(50);
wdt_reset();
// Enable the output PWMs
WriteAllPWMs(CALCULATED_PWM_STABLE); 
delay(50);
OutputEnable();
wdt_reset();


}


void WriteAllPWMs(uint8_t TargetValue){
  // TODO: Add code to invert PWM signal and PWM value on every other output to further reduce peak current and audible noise.
  // Note: this function makes use of "analogWriteOkFF", which is a modified version of analogWrite().  The modification is to skip the "else if (val == 255) {digitalWrite(HIGH);}" section.

uint8_t TargetValueB=TargetValue;
const boolean USE_OPPOSITE_PWM_TIMER_MODE = true;
if(USE_OPPOSITE_PWM_TIMER_MODE==true){
  TargetValueB=255-TargetValue;
  TCCR0A|=(1<<COM0B0);
  TCCR1A|=(1<<COM1B0);
  TCCR2A|=(1<<COM2B0);
}

    #ifdef BLDRIVER_PWM
    if((digitalPinToTimer(BLDRIVER_PWM)==TIMER0B)||(digitalPinToTimer(BLDRIVER_PWM)==TIMER1B)||(digitalPinToTimer(BLDRIVER_PWM)==TIMER2B)){
      if(TargetValueB==0) {
        digitalWrite(BLDRIVER_PWM, HIGH);
      } else {   
        analogWriteOkFF(BLDRIVER_PWM, TargetValueB); 
      }  
    } else {
      analogWriteOkFF(BLDRIVER_PWM, TargetValue);  
    }
    #endif
    #ifdef BLDRIVER_PWM_1
    if((digitalPinToTimer(BLDRIVER_PWM_1)==TIMER0B)||(digitalPinToTimer(BLDRIVER_PWM_1)==TIMER1B)||(digitalPinToTimer(BLDRIVER_PWM_1)==TIMER2B)){
      if(TargetValueB==0) {
        digitalWrite(BLDRIVER_PWM_1, HIGH);
      } else {   
        analogWriteOkFF(BLDRIVER_PWM_1, TargetValueB); 
      }  
    } else {
      analogWriteOkFF(BLDRIVER_PWM_1, TargetValue);  
    }
    #endif
    #ifdef BLDRIVER_PWM_2
    if((digitalPinToTimer(BLDRIVER_PWM_2)==TIMER0B)||(digitalPinToTimer(BLDRIVER_PWM_2)==TIMER1B)||(digitalPinToTimer(BLDRIVER_PWM_2)==TIMER2B)){
      if(TargetValueB==0) {
        digitalWrite(BLDRIVER_PWM_2, HIGH);
      } else {   
        analogWriteOkFF(BLDRIVER_PWM_2, TargetValueB); 
      }  
    } else {
      analogWriteOkFF(BLDRIVER_PWM_2, TargetValue);  
    }
    #endif
    #ifdef BLDRIVER_PWM_3
    if((digitalPinToTimer(BLDRIVER_PWM_3)==TIMER0B)||(digitalPinToTimer(BLDRIVER_PWM_3)==TIMER1B)||(digitalPinToTimer(BLDRIVER_PWM_3)==TIMER2B)){
      if(TargetValueB==0) {
        digitalWrite(BLDRIVER_PWM_3, HIGH);
      } else {   
        analogWriteOkFF(BLDRIVER_PWM_3, TargetValueB); 
      }  
    } else {
      analogWriteOkFF(BLDRIVER_PWM_3, TargetValue);  
    }   
    #endif
    #ifdef BLDRIVER_PWM_4
    if((digitalPinToTimer(BLDRIVER_PWM_4)==TIMER0B)||(digitalPinToTimer(BLDRIVER_PWM_4)==TIMER1B)||(digitalPinToTimer(BLDRIVER_PWM_4)==TIMER2B)){
      if(TargetValueB==0) {
        digitalWrite(BLDRIVER_PWM_4, HIGH);
      } else {   
        analogWriteOkFF(BLDRIVER_PWM_4, TargetValueB); 
      }  
    } else {
      analogWriteOkFF(BLDRIVER_PWM_4, TargetValue);  
    }  
    #endif
    #ifdef BLDRIVER_PWM_5
    if((digitalPinToTimer(BLDRIVER_PWM_5)==TIMER0B)||(digitalPinToTimer(BLDRIVER_PWM_5)==TIMER1B)||(digitalPinToTimer(BLDRIVER_PWM_5)==TIMER2B)){
      if(TargetValueB==0) {
        digitalWrite(BLDRIVER_PWM_5, HIGH);
      } else {   
        analogWriteOkFF(BLDRIVER_PWM_5, TargetValueB); 
      }  
    } else {
      analogWriteOkFF(BLDRIVER_PWM_5, TargetValue);  
    }  
    #endif
    #ifdef BLDRIVER_PWM_6
    if((digitalPinToTimer(BLDRIVER_PWM_6)==TIMER0B)||(digitalPinToTimer(BLDRIVER_PWM_6)==TIMER1B)||(digitalPinToTimer(BLDRIVER_PWM_6)==TIMER2B)){
      if(TargetValueB==0) {
        digitalWrite(BLDRIVER_PWM_6, HIGH);
      } else {   
        analogWriteOkFF(BLDRIVER_PWM_6, TargetValueB); 
      }  
    } else {
      analogWriteOkFF(BLDRIVER_PWM_6, TargetValue);  
    }   
    #endif    
}




void WriteAllPWMsLOW(){
  // Note: digitalwrite disconnects from PWM output!

    #ifdef BLDRIVER_PWM
        digitalWrite(BLDRIVER_PWM, LOW);   
    #endif
    #ifdef BLDRIVER_PWM_1
        digitalWrite(BLDRIVER_PWM_1, LOW);
    #endif
    #ifdef BLDRIVER_PWM_2
        digitalWrite(BLDRIVER_PWM_2, LOW);  
    #endif
    #ifdef BLDRIVER_PWM_3
        digitalWrite(BLDRIVER_PWM_3, LOW);   
    #endif
    #ifdef BLDRIVER_PWM_4
        digitalWrite(BLDRIVER_PWM_4, LOW);  
    #endif
    #ifdef BLDRIVER_PWM_5
        digitalWrite(BLDRIVER_PWM_5, LOW);   
    #endif
    #ifdef BLDRIVER_PWM_6
        digitalWrite(BLDRIVER_PWM_6, LOW);   
    #endif    
}


