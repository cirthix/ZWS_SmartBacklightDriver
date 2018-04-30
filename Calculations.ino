





void PseudoAtomicReadTimings(){
  // This function is written a bit strangely with two captures having to be equal to compare.  
  // The reason for this is to emulate an atomic read, because the target value is volatile and updated in an interrupt.
  uint16_t CaptureBlankingA=RAW_BLANKING;
  uint16_t CaptureActiveA=RAW_ACTIVE;
  uint16_t CaptureBlankingB=RAW_BLANKING;
  uint16_t CaptureActiveB=RAW_ACTIVE;
  while((CaptureBlankingA!=CaptureBlankingB) || (CaptureActiveA!=CaptureActiveB)) {
    CaptureBlankingA=RAW_BLANKING;
    CaptureActiveA=RAW_ACTIVE;
    CaptureBlankingB=RAW_BLANKING;
    CaptureActiveB=RAW_ACTIVE;
  }
  if(OUTPUT_MODE!=OUTPUT_MODE_STABLE) {  
    SAMPLED_ACTIVE = Timer1TicksToMicrocseconds(CaptureActiveA) ;
    SAMPLED_BLANKING = Timer1TicksToMicrocseconds(CaptureBlankingA) - SAMPLED_ACTIVE; // Because we don't want to reset timer1 twice during the pin change interrupt
  } else {
    SAMPLED_BLANKING = CaptureBlankingA ;
    SAMPLED_ACTIVE = CaptureActiveA ;
  }
}

void ResetFrameParameters(){
  SerialDebugln(F("Reset frame parameters"));
  PrintInputPulseTiming();
      FILTERED_BLANKING = 0;
      SAMPLED_BLANKING  = 0;
      LOCKED_BLANKING   = 0;
      FILTERED_ACTIVE = 0;
      SAMPLED_ACTIVE  = 0;
      LOCKED_ACTIVE   = 0;
      FRAME_TIMINGS_LOCKED = false;
}



// In the filter function, we switch between a quickly convergent but possibly noisy function and a noise-rejecting slowly converging filter.
void RefilterFrameParameters(){     
  wdt_reset();   
  if(FrameTimingsBelongToNewFrame==false) {return;}
  FrameTimingsBelongToNewFrame=false;
    PseudoAtomicReadTimings();
  
  if(FRAME_TIMINGS_LOCKED != true){
      FILTERED_BLANKING=FILTERED_BLANKING-(FILTERED_BLANKING/FILTER_FAST_RATE)+(SAMPLED_BLANKING/FILTER_FAST_RATE);
      FILTERED_ACTIVE=FILTERED_ACTIVE-(FILTERED_ACTIVE/FILTER_FAST_RATE)+(SAMPLED_ACTIVE/FILTER_FAST_RATE);
  if( 
    (abs(FILTERED_BLANKING-SAMPLED_BLANKING)<FILTER_CONVERGENCE_HYSTERESIS) &&
    (abs(FILTERED_ACTIVE-SAMPLED_ACTIVE)<FILTER_CONVERGENCE_HYSTERESIS) ) {

      // Now we have timings which are supposedly 'locked'.  Do they make sense?
//      if(     (FILTERED_BLANKING<BLANKING_TIME_MINIMUM )||
//              (FILTERED_BLANKING>BLANKING_TIME_MAXIMUM )||
//              (FILTERED_ACTIVE<ACTIVE_TIME_MINIMUM )||
//              (FILTERED_ACTIVE>ACTIVE_TIME_MAXIMUM ))
//      {ResetFrameParameters(); return;}
      
    
      SerialDebugln(F("Got lock"));    
      ShouldPrintUpdatedParameters=true; 
      FRAME_TIMINGS_LOCKED = true;
      LOCKED_BLANKING=FILTERED_BLANKING;   
      LOCKED_ACTIVE=FILTERED_ACTIVE;  
      accumulated_error_BLANKING=0;
      accumulated_error_ACTIVE=0; 
      RecalculateParameters();
    } 
  } else {
      
    accumulated_error_BLANKING+=SAMPLED_BLANKING-FILTERED_BLANKING;
    if(abs(accumulated_error_BLANKING)>FILTER_CONVERGENCE_HYSTERESIS){
      if(accumulated_error_BLANKING>0){  FILTERED_BLANKING++;}
      else {    FILTERED_BLANKING--;}  
      accumulated_error_BLANKING=0;
    }
    accumulated_error_ACTIVE+=SAMPLED_ACTIVE-FILTERED_ACTIVE;
    if(abs(accumulated_error_ACTIVE)>FILTER_CONVERGENCE_HYSTERESIS){
      if(accumulated_error_ACTIVE>0){  FILTERED_ACTIVE++;}
      else {    FILTERED_ACTIVE--;}  
      accumulated_error_ACTIVE=0;
    }



int16_t DEVIATION_BLANKING=FILTERED_BLANKING-LOCKED_BLANKING;
int16_t DEVIATION_ACTIVE=FILTERED_ACTIVE-LOCKED_ACTIVE;

  if( (abs(DEVIATION_BLANKING)>FILTER_LOCKLOSS_THRESHOLD) || (abs(DEVIATION_ACTIVE)>FILTER_LOCKLOSS_THRESHOLD) ){
    
    SerialDebugln(F("Lost lock"));    
    ShouldPrintUpdatedParameters=true; 
    FRAME_TIMINGS_LOCKED = false;
  }
    
}

}


uint32_t CalculateDeratedPowerLimit(uint16_t myNumerator, uint16_t myDenominator){
    float myEffectiveDutyCycle=myNumerator/myDenominator;
    float myDeratedBoost=1+(((myDenominator/myNumerator)-1)/MyConfigLED.DeratingFactor);
    uint32_t myCalculatedPowerLimit=(MyConfigLED.PowerLimit)/myDeratedBoost;
    
// Maybe add safety checks here?
    return myCalculatedPowerLimit;
}


void CalculateCycleTimeLimits(){
  DeterminePartialDutyCycleLimits();
  DetermineDimmingAdjustBrightnessRange();
//  SerialDebug(F("Min brightness : "));  SerialDebug(CALCULATED_MINIMUM_BRIGHTNESS);  SerialDebugln(F(" nits")); 
//  SerialDebug(F("Max brightness : "));  SerialDebug(CALCULATED_MAXIMUM_BRIGHTNESS);  SerialDebugln(F(" nits"));   
}

void DeterminePartialDutyCycleLimits(){ // Note: this calculation is determined for pwm mode operation.  Instead of using the real divider, use the intended divider.
  uint16_t myCycleTime=1.0*1000000/DetermineTimer2FrequencyPWM(TargetTimerDividerStable); 
  CALCULATED_PARTIAL_DUTYCYCLE_MINIMUM = PWM_MAX*1.0*MINIMUM_ONTIME/myCycleTime;
  CALCULATED_PARTIAL_DUTYCYCLE_MAXIMUM = PWM_MAX*1.0*(myCycleTime-MINIMUM_OFFTIME)/myCycleTime;  
//  SerialDebug(F("CycleTime : "));  SerialDebug(myCycleTime); SerialDebugln(F(" us"));
//  SerialDebug(F("Dutycycle minimum : "));  SerialDebugln(CALCULATED_PARTIAL_DUTYCYCLE_MINIMUM);
//  SerialDebug(F("Dutycycle maximum : "));  SerialDebugln(CALCULATED_PARTIAL_DUTYCYCLE_MAXIMUM);
}

void DetermineDimmingAdjustBrightnessRange(){
CALCULATED_MINIMUM_BRIGHTNESS = CalculateBrightness(OUTPUT_CURRENT_TO_ADIM_LEVEL(CalculateMinimumCurrent()), CALCULATED_PARTIAL_DUTYCYCLE_MINIMUM);
CALCULATED_MAXIMUM_BRIGHTNESS = CalculateBrightness(MAXIMUM_STABLE_ADIM, PWM_MAX);
}





uint16_t ADIM_LEVEL_TO_OUTPUT_CURRENT(uint8_t adim_level){
  return CURRENT_MINIMUM + ((CURRENT_MAXIMUM-CURRENT_MINIMUM)*1.0*adim_level/ADIM_MAX);
}

uint8_t OUTPUT_CURRENT_TO_ADIM_LEVEL(uint16_t current){
  if(current>=CURRENT_MAXIMUM) { return ADIM_MAX;} 
  if(current<=CURRENT_MINIMUM) { return 0;} 
  return ADIM_MAX*(1.0*(current-CURRENT_MINIMUM))/(CURRENT_MAXIMUM-CURRENT_MINIMUM);
}
