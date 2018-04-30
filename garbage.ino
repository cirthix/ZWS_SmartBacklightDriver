/*
  
// This file contains old functions which may be useful as reference but should not be in the code any longer






void ModeHandlingControl() {
  //SerialDebug(F("InPulse : ")); if(BufferedRecentInputPulse==false) { SerialDebugln(F("false"));} else { SerialDebugln(F("OK"));}
  // First handle the control mode
  //  SerialDebug(F("InputEnable : ")); SerialDebugln(InputEnable==HIGH);
  switch (CONTROL_MODE) {
    case CONTROL_MODE_OFF :
      if (BufferedInputEnable == HIGH) {
        CONTROL_MODE = CONTROL_MODE_WAIT_IIC_PWM;
        ExitControlOFF();
        EnterIntermediateStateMillis = millisNoInterruptChanges();
      } else {
        if (BufferedRecentInputPulse == true) {
          CONTROL_MODE = CONTROL_MODE_WAIT_ZWS_TURNON;
          EnterIntermediateStateMillis = millisNoInterruptChanges();
        }
      }
      break;
    case CONTROL_MODE_IIC : break; // Do nothing, stay in this mode until power loss;
    case CONTROL_MODE_PWM : if (BufferedInputEnable == LOW) {
        CONTROL_MODE = CONTROL_MODE_OFF;
        break;
      }
    case CONTROL_MODE_WAIT_ZWS_TURNON :
      if (BufferedRecentInputPulse == false) {
        CONTROL_MODE = CONTROL_MODE_OFF;
      }
      else {
        if ((millisNoInterruptChanges() - EnterIntermediateStateMillis) >= ModeTimeoutZWS) {
          CONTROL_MODE = CONTROL_MODE_ZWS;
          EnterControlZWS();
        }
      }
      break;
    case CONTROL_MODE_ZWS : if (BufferedRecentInputPulse == false) {
        CONTROL_MODE = CONTROL_MODE_OFF;
        ExitControlZWS();
        EnterControlOFF();
        SerialDebugln("EXITING STATE "); // Remove this line when satisfied with mode stability
      }
      break;
    case CONTROL_MODE_WAIT_IIC_PWM : if (SuccessfulIIC == true)  {
        CONTROL_MODE = CONTROL_MODE_IIC;
      }  if ((millisNoInterruptChanges() - EnterIntermediateStateMillis) >= ModeTimeout) {
        CONTROL_MODE = CONTROL_MODE_PWM;
        break;
      }
  }
}

void ModeHandlingOutput() {
  if (OUTPUT_MODE == OUTPUT_MODE_OFF ) {
    switch (CONTROL_MODE) {
      case CONTROL_MODE_OFF : break;
      case CONTROL_MODE_IIC :
        if (TargetPowerSave == TargetPowerSaveFULLY_ON) {
          PowerON();
          return;
        }
        break;
      case CONTROL_MODE_PWM :
        PowerON(); return;
        break;
      case CONTROL_MODE_ZWS :
        if (TargetPowerSave == TargetPowerSaveFULLY_ON) {
          PowerON();
          return;
        }
        break;
      case CONTROL_MODE_WAIT_IIC_PWM : break;
      case CONTROL_MODE_WAIT_ZWS_TURNON : break;
    }
  }
  if (OUTPUT_MODE == OUTPUT_MODE_STABLE ) {
    switch (CONTROL_MODE) {
      case CONTROL_MODE_OFF : PowerOFF(); break;
      case CONTROL_MODE_IIC :
        if (TargetPowerSave != TargetPowerSaveFULLY_ON) {
          PowerOFF();
          return;
        }
        if ((CheckCapableStrobing() == true) && (TargetMode == OUTPUT_MODE_STROBE )) {
          EnterStrobing();
        }
        if ((CheckCapableScanning() == true) && (TargetMode == OUTPUT_MODE_SCAN   )) {
          EnterScanning();
        }
        break;
      case CONTROL_MODE_PWM :
        if (CheckCapableStrobing() == true) {
          EnterStrobing();
        }
        // There is no input which can select between strobing and scanning in this mode
        // Only support strobe mode with the pwm control scheme
        break;
      case CONTROL_MODE_ZWS :
        if (TargetPowerSave != TargetPowerSaveFULLY_ON) {
          PowerOFF();
          return;
        }
        if (TargetMode == OUTPUT_MODE_STROBE ) {
          EnterStrobing();
        }
        if (TargetMode == OUTPUT_MODE_SCAN   ) {
          EnterScanning();
        }
        return;
      case CONTROL_MODE_WAIT_IIC_PWM : PowerOFF();  break;
      case CONTROL_MODE_WAIT_ZWS_TURNON : PowerOFF();  break;
    }
  }
  if (OUTPUT_MODE == OUTPUT_MODE_STROBE ) {
    switch (CONTROL_MODE) {
      case CONTROL_MODE_OFF : PowerOFF(); break;
      case CONTROL_MODE_IIC :
        if (TargetPowerSave != TargetPowerSaveFULLY_ON) {
          PowerOFF();
          return;
        }
        if ((CheckCapableStrobing() == false) && (TargetMode == OUTPUT_MODE_STROBE )) {
          EnterStable();
        }
        if ((CheckCapableScanning() == true) && (TargetMode == OUTPUT_MODE_SCAN   )) {
          EnterScanning();
        }
        break;
      case CONTROL_MODE_PWM :
        if (CheckCapableStrobing() == false) {
          EnterStable();
        }
        // There is no input which can select between strobing and scanning in this mode
        // Only support strobe mode with the pwm control scheme
        break;
      case CONTROL_MODE_ZWS :
        if (TargetPowerSave != TargetPowerSaveFULLY_ON) {
          PowerOFF();
          return;
        }
        if (TargetMode == OUTPUT_MODE_STABLE ) {
          EnterStable();
        }
        if (TargetMode == OUTPUT_MODE_SCAN   ) {
          EnterScanning();
        }
        break;
      case CONTROL_MODE_WAIT_IIC_PWM : PowerOFF(); break;
      case CONTROL_MODE_WAIT_ZWS_TURNON : PowerOFF();  break;
    }
  }
  if (OUTPUT_MODE == OUTPUT_MODE_SCAN ) {
    switch (CONTROL_MODE) {
      case CONTROL_MODE_OFF : PowerOFF(); break;
      case CONTROL_MODE_IIC :
        if (TargetPowerSave != TargetPowerSaveFULLY_ON) {
          PowerOFF();
          return;
        }
        if ((CheckCapableStrobing() == false) && (TargetMode == OUTPUT_MODE_STROBE )) {
          EnterStable();
        }
        if ((CheckCapableStrobing() == true) && (TargetMode == OUTPUT_MODE_STROBE   )) {
          EnterStrobing();
        }
        break;
      case CONTROL_MODE_PWM :
        EnterStable();
        // There is no input which can select between strobing and scanning in this mode
        // Only support strobe mode with the pwm control scheme
        break;
      case CONTROL_MODE_ZWS :
        if (TargetPowerSave != TargetPowerSaveFULLY_ON) {
          PowerOFF();
          return;
        }
        if (TargetMode == OUTPUT_MODE_STABLE ) {
          EnterStable();
        }
        if (TargetMode == OUTPUT_MODE_STROBE ) {
          EnterStrobing();
        }
        break;
      case CONTROL_MODE_WAIT_IIC_PWM : PowerOFF(); break;
      case CONTROL_MODE_WAIT_ZWS_TURNON : PowerOFF();  break;
    }
  }
}
























void DebugUART(){
  uint8_t myUCSR0B = UCSR0B;
  uint8_t myRXCIE0 = myUCSR0B & (1<<RXCIE0);
  uint8_t myTXCIE0 = myUCSR0B & (1<<TXCIE0);
  uint8_t myUDRIE0 = myUCSR0B & (1<<UDRIE0);

  SerialDebug(F("UCSR0B : ")); SerialDebugln(myUCSR0B); 
  SerialDebug(F("RXCIE0 : ")); SerialDebugln(myRXCIE0); 
  SerialDebug(F("TXCIE0 : ")); SerialDebugln(myTXCIE0); 
  SerialDebug(F("UDRIE0 : ")); SerialDebugln(myUDRIE0); 

  //  SerialDebug("Four seconds of delay here ..."); SerialDebug("4.."); delay(1000); SerialDebug("3.."); delay(1000); SerialDebug("2.."); delay(1000); SerialDebug("1.."); delay(1000); SerialDebugl


const uint8_t TIMING_SOURCE_MICROS = 0;
const uint8_t TIMING_SOURCE_TIMER1 = 1;
volatile uint8_t TimingSource=TIMING_SOURCE_MICROS;
volatile uint32_t InterruptPreviousMicros=0;
ISR(INT0_vect) {
  noInterrupts();
  // Note: In scanning mode, the interrupt should only be enabled for the falling edge.  For other modes, both edges are used
  
  uint16_t myTCNT1 = TCNT1;
  uint8_t myPulsePolarity=digitalRead2(INPUT_PULSE);
  
  switch (OUTPUT_MODE) {
    case OUTPUT_MODE_STROBE :
      if (myPulsePolarity == HIGH) {
        WritePulseToAllPins(LOW);
        OCR1B = StrobePulseHigh;
      } else {
        WritePulseToAllPins(LOW);
        OCR1B = StrobePulseHigh;
      }
      break;
    case OUTPUT_MODE_SCAN :
      if (myPulsePolarity == LOW) {
        WritePulseToAllPins(LOW);
        ScanningState = FIRST_SEGMENT_ON;
        OCR1A = ScanningTimeFirst;
      }
      break;   
    default : // Don't do anything special, only record the timestamps
  }

  switch (TimingSource) {
    case TIMING_SOURCE_MICROS :
      uint32_t myMicros=micros();    
      uint16_t myDelay=myMicros-InterruptPreviousMicros;
      InterruptPreviousMicros=myMicros;
      if (myPulsePolarity == HIGH) {
        RAW_ACTIVE = myDelay;
      } else {
        RAW_BLANKING = myDelay;
      }
      break;
    default:
      if (myPulsePolarity == HIGH) {
        // Note: Timer interrupts be chosen to avoid having a transition occur near this time.
        RAW_ACTIVE = myTCNT1;
      } else {
        RAW_BLANKING = myTCNT1;  // Note: subtract RAW_ACTIVE later
        TCNT1 = 0;
      }
  }




























uint8_t CalculateScanEffectiveDutyCycle(){
  return = PWM_MAX*1.0*ScanPulseDurationReal/FILTERED_ACTIVE;
}

uint8_t CalculateStrobeEffectiveDutyCycle(){
  return = PWM_MAX*1.0*StrobeDurationReal/FILTERED_ACTIVE;
}


// MODIFICATIONS TO WIRING.C:



void HandleMillisInTimer0Overflow(){
  // This function is nearly the same as the one in wiring.c which updates the millis counters appropriately.
  // The idea with having this function called manually in a polled state is to avoid an unnecessary interrupt
  if(!(TIFR0&(0x01<<TOV0))) {return;} // The timer0 overflow interrupt is enabled.  Don't run this version.
// Overflow rates: 1x prescaler => 32us, 8x prescaler => 256us, 64x prescaler => 2048us

  // Clear the overflow flag
TIFR0=(0x01<<TOV0); 



// the prescaler is set so that timer0 ticks every 64 clock cycles, and the
// the overflow handler is called every 256 ticks.
//#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))

// the whole number of milliseconds per timer0 overflow
//#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)

// the fractional number of milliseconds per timer0 overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
//#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
//#define FRACT_MAX (1000 >> 3)
  
  // copy these to local variables so they can be stored in registers
  // (volatile variables must be read from memory on every access)
  unsigned long m = timer0_millis;
  unsigned char f = timer0_fract;

  m += MILLIS_INC;
  f += FRACT_INC;
  if (f >= FRACT_MAX) {
    f -= FRACT_MAX;
    m += 1;
  }
// note that this nonatomic multibyte data may be read incorrectly if millis is called from an interrupt while the update occurs.
  timer0_fract = f;
  timer0_millis = m;
  timer0_overflow_count++; 
}


// This function does the same job as millis(), but does not interfere with interrupt handling. 
unsigned long millisNoInterruptChanges(){
  uint8_t TriesRemaining=255;
  uint32_t CaptureOne;
  uint32_t CaptureTwo;

  // Read this volatile value twice to avoid inconsistency (e.g. in the middle of a write to timer0_millis)
  while(TriesRemaining>0) {
    CaptureOne=timer0_millis;
    CaptureTwo=timer0_millis;
    if(CaptureOne == CaptureTwo) {return CaptureTwo;}
    TriesRemaining=TriesRemaining-1;
  }
  return CaptureTwo; // It might be wrong, but this will only ever occur in a situation where interrupts are totally swamping out regular program flow.  Such a system probably wouldn't work properly anyway.
}






void UpdateLoopDelays(){
OneMillisecond = 1 * 1.0*(64/DetermineTimer0Prescaler(GetCurrentTimer0ClockDivider()));
TenMilliseconds = 10 * 1.0*(64/DetermineTimer0Prescaler(GetCurrentTimer0ClockDivider()));
HundredMilliseconds = 100 * 1.0*(64/DetermineTimer0Prescaler(GetCurrentTimer0ClockDivider()));
OneSecond = 1000 * 1.0*(64/DetermineTimer0Prescaler(GetCurrentTimer0ClockDivider()));
TenSeconds = 10000 * 1.0*(64/DetermineTimer0Prescaler(GetCurrentTimer0ClockDivider()));
}








    currentMillis = millis();
  wdt_reset();
  TaskFastest();
  if ((currentMillis - Task10ms_previousMillis) >= 1) {
    Task1ms();
    Task1ms_previousMillis=currentMillis;
  }
  
  if ((currentMillis - Task10ms_previousMillis) >= 10) {
    Task10ms();
    Task10ms_previousMillis=currentMillis;
  }

  if ((currentMillis - Task100ms_previousMillis) >= 100) {
    Task100ms();
    Task100ms_previousMillis=currentMillis;
  }

  if ((currentMillis - Task1000ms_previousMillis) >= 1000) {
    Task1000ms();
    Task1000ms_previousMillis=currentMillis;
  }  
  
  if ((currentMillis - Task10000ms_previousMillis) >= 10000) {
    Task10000ms();
    Task10000ms_previousMillis=currentMillis;
  }  


















const float FILTER_CONVERGENCE_RATIO = 0.25;
const uint32_t TimeoutPWM = 10000; // Inputs below 100Hz will be ignored.
void CaptureInputPWM(){
  // This method is not usable under some conditions (eg:rising edge aligns with InputPulse)
  if( CONTROL_MODE == CONTROL_MODE_PWM){
    uint32_t highTime = pulseIn(INPUT_DIM_OR_SDA_PIN, HIGH, TimeoutPWM);
    uint32_t lowTime = pulseIn(INPUT_DIM_OR_SDA_PIN, LOW, TimeoutPWM);
    uint32_t cycleTime = highTime + lowTime;
    if(cycleTime>0) {
      RawInputPWM = MaximumInputPWM*(float)highTime / float(cycleTime);
    } else{
      RawInputPWM = 0;
    }
    RawInputPWM=MaximumInputPWM;
//    SerialDebug(F("RAW input PWM : ")); SerialDebugln(RawInputPWM);  
    uint16_t OldFilteredInputPWM=FilteredInputPWM;
    FilteredInputPWM = (1-FILTER_CONVERGENCE_RATIO)*FilteredInputPWM + FILTER_CONVERGENCE_RATIO*RawInputPWM;
    if(( abs(OldFilteredInputPWM-FilteredInputPWM) < HysteresisInputPWM ) && ( abs(FilteredInputPWM-PostHysteresisInputPWM) > HysteresisInputPWM)) {
      PostHysteresisInputPWM = FilteredInputPWM ;
//      SerialDebug(F("GOOD input PWM : ")); SerialDebugln(PostHysteresisInputPWM);  
    }         
  }
}











  

void fixed_brightness(){
    analogWrite(BLDRIVER_PWM, PWM_LEVEL); 
    adimWrite();
}

void StrobeSimulator_SHITTY(){
// Test for 144Hz, 500us ontime each
  const uint16_t TEST_POINT_TIME = 6944; // units = microseconds
  const uint32_t TIME_TO_BE_ON=500;
  const uint32_t TIME_TO_BE_OFF=TEST_POINT_TIME-TIME_TO_BE_ON;
  digitalWrite(BLDRIVER_PWM, LOW);
  ADIM_LEVEL=255; adimWrite();
  delay(100);  
  while(1){
    TCNT1=0;
    digitalWrite2(BLDRIVER_PWM, HIGH);
    delayMicroseconds(TIME_TO_BE_ON);
    digitalWrite2(BLDRIVER_PWM, LOW);
    delayMicroseconds(TIME_TO_BE_OFF);
  }      
}



static const uint8_t TIMER_1_CLOCK_DIVIDER = 0x02;
static const uint16_t TIMER1_CLOCK_DIVIDER_EQ=8;  // At 8MHz, each timer tick is 1us

void StrobeSimulator_directpwm(){
// Test for 122Hz, 500us ontime each
    TCCR2B = 0x05; ckdivider=128; // set clock scaler to 1 /128
    PWM_LEVEL = 10; 
    analogWrite(BLDRIVER_PWM, PWM_LEVEL); 
    ADIM_LEVEL=255;
    adimWrite();
  }      

void StrobeSimulator(){
// Test for 120Hz, 500us ontime each
    analogWrite(BLDRIVER_PWM, 255); 
ConfigureTimers();
    ADIM_LEVEL=42;
    adimWrite();

while(1) {delayMicroseconds(1000000/240);  OSP_FIRE(); }
  }    



void ramp_adim_up_then_down(){
  const uint8_t BR_PWM = 41; //41; // 10us ontime minimum
  const uint8_t BRMIN = 0;
  const uint8_t BRMAX = ADIM_MAX;
  const uint8_t STEPSIZE = 1;

  const uint16_t TEST_POINT_TIME = 10; // units = milliseconds
  const float ONRATIO=1;
  const uint32_t TIME_TO_BE_ON=(1.0*ONRATIO)*TEST_POINT_TIME;
  const uint32_t TIME_TO_BE_OFF=TEST_POINT_TIME-TIME_TO_BE_ON;
  uint16_t myBright=BRMIN;

//SerialDebug(F("ONTIME : ")); SerialDebugln(TIME_TO_BE_ON);
//SerialDebug(F("OFFTIME : ")); SerialDebugln(TIME_TO_BE_OFF);

  while( myBright<BRMAX) {   
    PWM_LEVEL=BR_PWM; analogWrite(BLDRIVER_PWM, PWM_LEVEL); 
    ADIM_LEVEL=myBright; adimWrite();
    SerialDebugln(myBright);  SerialFlush();
    myBright=myBright+STEPSIZE;
    zdelay(TIME_TO_BE_ON);  
    if(TIME_TO_BE_OFF>0){
      PWM_LEVEL=0; analogWrite(BLDRIVER_PWM, PWM_LEVEL); 
      zdelay(TIME_TO_BE_OFF);  
    }
  }    
  
  while( myBright>BRMIN) {
    PWM_LEVEL=BR_PWM; analogWrite(BLDRIVER_PWM, PWM_LEVEL); 
    ADIM_LEVEL=myBright; adimWrite();
    SerialDebugln(myBright);  SerialFlush();
    myBright=myBright-STEPSIZE;
    zdelay(TIME_TO_BE_ON);    
    if(TIME_TO_BE_OFF>0){
      PWM_LEVEL=0; analogWrite(BLDRIVER_PWM, PWM_LEVEL); 
      zdelay(TIME_TO_BE_OFF);  
    }
  }    
}










 
 */
