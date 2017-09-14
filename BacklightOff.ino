// Each of the operating modes must provide some functions:
// Rule checking function
// Brightness calculation function
// Status printing function
// Rely on internal global variables

void EnterOff(){ 
    StopSyncInterrupt();
    WriteAllPWMs(0);
    adimWrite(CALCULATED_ADIM_OFF);
    ConfigureTimersOFF();        
    OUTPUT_MODE = OUTPUT_MODE_OFF;
    StartSyncInterruptChange();
    IndicateNewState(); 
}

void ConfigureTimersOFF(){  
  WriteAllPWMs(0); 
  // To avoid a glitch when disabling the timer clocks, disable the outputs temporarily  
  
  // Stop timer interrupts
  TIMSK0=0x00|(1<<TOIE0); // To count millis and micros properly while in off mode
  TIMSK1=0x00; // Disable interrupts here
  TIMSK2=0x00; // Disable interrupts here

  // Stop the output clocks, unconfigure the timers
  TCCR0B=0x00;
  TCCR1B=0x00;
  TCCR2B=0x00;
  TCCR0A=0x00;
  TCCR1A=0x00;
  TCCR2A=0x00;

  // Turn the clocks
  SetTimer0ClockDivider(DetermineCorrectTimer0Divider(64)); // Set the default (64) divider for timer0 to keep millis and micros operating
}

