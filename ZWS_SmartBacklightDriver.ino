/*
 * THIS CODE WILL FAIL TO COMPILE IN AN UNMODIFIED ARDUINO ENVIRONMENT! 
 * License for use is granted for use with ZisWorks hardware only.  Use of this code for any other purpose is prohibited.
 * TO AVOID DAMAGING YOUR BOARD AND/OR PANEL, CORRECTLY SET THE VALUES UNDER " CHANGE SYSTEM CONFIGURATION PARAMETERS HERE " IN "CONSTANTS.H"
 * This code is intended for use with the ZisWorks "smart backlight driver" board. 
 * It implements pwm, pwm-free, strobing, and scanning backlight modes.
 * Currently, only the "zws" control mode is implemented for use in the x28/x39 displays.  
 * In this mode, the backlight driver acts as the master of the system, reading button input, setting edids via softserial commands, and handling pulse control.
 * Timing in this application is absolutely critical, so some core files had to be modified to avoid the use of interrupts.  
 * Notably, serial buffer transfer and timer0 overflow handling.  These tasks have been pushed into the control loop.
 * 
 * This version is a work-in-progress, though it is in a fairly stable state and is perfectly usable.  
 * The remaining to-do items are as follows:
 * Add serial commands for LED parameter reconfiguraiton without a recompile
 * Add protection to raw input values
 * Add support for raw input values for scan mode
 * Additional testing of the raw-input feature
 * Improve brightness control during strobe/scan modes
 * Fix usage with overclocking library
 * 
 * MODIFICATIONS AND LIBRARIES NECESSARY
 * ... a bunch of them.  Will update this when posting the DP2LVDS firmware, many changes are shared.
 * 
 * 
 * Please forgive the coding.
 */


#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push

#include <Arduino.h>
#include <EEPROM.h>
#include <SendOnlySoftwareSerial.h>
#include <avr/wdt.h>
#include "arduino2.h"  // include the fast I/O 2 functions.  Info here: http://www.codeproject.com/Articles/732646/Fast-digital-I-O-for-Arduino
#include "constants.h"
#include "InputHandling.h"
#include "InfraredStereo.h"
#include <WS2812.h>

#if OVERCLOCKING==ENABLED
#include "Overclock.h"
#endif

#if OVERCLOCKING==ENABLED
Overclock my_Overclock_object = Overclock(OVERCLOCKED_SPEED, SERIAL_BAUD);
#endif


  #ifdef LED_RGB
#define LEDCount 1   // Number of LEDs in RGB string
WS2812 LED(LEDCount); 
cRGB value;
cRGB valuePrev;
#endif


SendOnlySoftwareSerial SerialToSlave(INPUT_DIM_OR_SDA_PIN);      // Note: only start the interface if in ZWS control mode


InputHandling ButtonBoard;
InfraredStereo InfraredStereoTransmitter;

// SAFETY LIMIT SETTINGS
const uint8_t MinimumBrightnessSetpoint = 5 ; // uinits=nits.  Extremely low values can be problematic in some calculations
const uint8_t MINIMUM_ONTIME = 10; // microseconds
const uint8_t MINIMUM_ONTIME_SCAN = 30; // microseconds.  Longer due to interrupt trigger timing requirements.
const uint8_t MINIMUM_OFFTIME = 10; // microseconds
const uint16_t VOLTAGE_MINIMUM = 30000; // millivolts
const uint16_t VOLTAGE_MAXIMUM = 60000; // millivolts
const uint16_t CURRENT_MINIMUM = 50;  // milliamps per channel
const uint16_t CURRENT_MAXIMUM = 395; // milliamps peak per channel // This should be 395mA, but for the sake of not blowing up more LED strips, use 100mA instead
const uint16_t CURRENT_MAXIMUM_SUTAINED = 200; // milliamps average per channel  // This should be 200mA, but for the sake of now blowing up more LED strips, set it to 75mA insteaad
const uint16_t POWER_MAXIMUM_SUTAINED = 30000; // milliwatts TOTAL OVER ALL CHANNELS
const uint16_t INITIAL_POWER_LIMIT = 5000; // milliwatts TOTAL OVER ALL CHANNELS
uint16_t POWER_LIMIT = INITIAL_POWER_LIMIT; // milliwatts TOTAL OVER ALL CHANNELS
uint16_t OLD_POWER_LIMIT = 0; // milliwatts TOTAL OVER ALL CHANNELS
uint16_t OutputPulseDurationMAX=2000; // units=microseconds

const uint8_t TimerDividerMinimum = 0x01; // The supported values happen to be in the timer0 and timer1 scaler range 1-5
const uint8_t TimerDividerMaximum = 0x05;
uint8_t TargetTimerDividerStable = 0x02;


// Scanning stuff
const uint8_t SEGMENT_1_ON  = 0;
const uint8_t SEGMENT_1_OFF = 1;
const uint8_t SEGMENT_2_ON  = 2;
const uint8_t SEGMENT_2_OFF = 3;
const uint8_t SEGMENT_3_ON  = 4;
const uint8_t SEGMENT_3_OFF = 5;
const uint8_t SEGMENT_4_ON  = 6;
const uint8_t SEGMENT_4_OFF = 7;
const uint8_t SEGMENT_5_ON  = 8;
const uint8_t SEGMENT_5_OFF = 9;
const uint8_t SEGMENT_6_ON  = 10;
const uint8_t SEGMENT_6_OFF = 11;
const uint8_t FIRST_SEGMENT_ON = SEGMENT_2_ON; //  Because the pin interrupt occurs as the valid frame starts, the first illuminated segment is the second segment, not the first.
#define GetScanTimeFirst() GetScanTime2()       //  Similarly, set the ScanningTimeFirst to be the ScanningTime associated with turing on the second segment

// CONFIGURATION REGISTERS
volatile uint8_t SuccessfulIIC = false;
volatile uint8_t RecentStrobe = false;
volatile boolean FrameTimingsBelongToNewFrame=true;
boolean DebugBlockSlaveSerialCommands = false;
uint8_t CONTROL_MODE = CONTROL_MODE_OFF;
uint8_t OUTPUT_MODE = OUTPUT_MODE_OFF;
uint8_t LED_STATE_INDICATOR = LED_STATE_INDICATOR_ERROR;
uint16_t TargetBrightnessStable = 0; // NITS
uint16_t TargetBrightnessStrobe = 0; // NITS
uint16_t TargetBrightnessScan = 0; // NITS
uint8_t TargetPowerSave;
uint8_t TargetMode;
uint8_t TargetEDID;
uint8_t TargetOSD;
uint16_t StrobePulseDuration = 1000; // microseconds
uint16_t StrobePulseDelay = 50; // microseconds

// LED CONFIGURATION REGISTERS
ConfigLED MyConfigLED;


// CALCULATED REGISTERS
uint8_t MAXIMUM_STABLE_ADIM = 0;
uint8_t CALCULATED_PWM_STABLE = 0;
uint8_t CALCULATED_ADIM_STABLE = 0;
uint8_t CALCULATED_ADIM_STROBE = 0;
uint8_t CALCULATED_ADIM_SCAN = 0;
uint8_t CALCULATED_ADIM_OFF = 0;
uint16_t CALCULATED_MINIMUM_BRIGHTNESS = 0;
uint16_t CALCULATED_MAXIMUM_BRIGHTNESS = 0;
uint8_t CALCULATED_PARTIAL_DUTYCYCLE_MAXIMUM = 0;
uint8_t CALCULATED_PARTIAL_DUTYCYCLE_MINIMUM = 0;
uint8_t STROBE_TRIGGER_POINT = 0;
uint16_t StrobeSendSoftSerial;
uint16_t StrobeSerial;
uint16_t StrobePulseHigh;
uint16_t StrobePulseLow;
volatile uint8_t ScanningState = 0;

// PWM MODE FILTER
uint16_t PostHysteresisInputPWM;
uint16_t SampledHigh = 0;
uint16_t SampledLow = 0;
uint16_t SampledPWM = 0;
// old method variables:
uint16_t RawInputPWM;
uint16_t FilteredInputPWM;


// MISC INTERNAL REGISTERS
boolean ShouldRecalculateParameters=true;
volatile uint8_t ShouldSendSerialCommand = false;
volatile uint8_t ShouldUpdateStatusLED = false;
uint8_t ShouldUpdateParameters = true;
uint8_t ShouldPrintUpdatedParameters = true;
volatile uint8_t RecentInputPulse = false;
uint32_t FinishedConfigurationTime = 0;
uint32_t currentMillis = 0;
uint32_t Task1ms_previousMillis = 0;
uint32_t Task10ms_previousMillis = 0;
uint32_t Task100ms_previousMillis = 0;
uint32_t Task1000ms_previousMillis = 0;
uint32_t Task10000ms_previousMillis = 0;
uint32_t EnterIntermediateStateMillis;
uint8_t myTemp = 0;
uint16_t SamplesPerSecondPWM = 0;

uint8_t PreviouslyPrintedMode = CONTROL_MODE_OFF;
uint8_t PreviouslyPrintedPWM = 0;
uint8_t PreviouslyPrintedADIM = 0;

// Variables related to pulse timing, filtering, and locking
static const uint8_t TIMER_1_CLOCK_DIVIDER = 0x02;
static const uint16_t TIMER1_CLOCK_DIVIDER_EQ = 8;
volatile uint16_t RAW_BLANKING      = 0;  // This one is modified by the interrupt
uint16_t SAMPLED_BLANKING  = 0;  // This one is pseudo-atomically buffered
uint16_t FILTERED_BLANKING = 0;  // This one is after the filter
int16_t  accumulated_error_BLANKING = 0;
uint16_t LOCKED_BLANKING   = 0;  // This one is the locked value (post hysteretic filter)
volatile uint16_t RAW_ACTIVE      = 0;
uint16_t SAMPLED_ACTIVE  = 0;
uint16_t FILTERED_ACTIVE = 0;
int16_t  accumulated_error_ACTIVE = 0;
uint16_t LOCKED_ACTIVE   = 0;
uint8_t FRAME_TIMINGS_LOCKED = false;
const    uint16_t FILTER_FAST_RATE               = 2;    // Use a power of 2 for efficient implementation
const    uint16_t FILTER_CONVERGENCE_HYSTERESIS  = 20    * (REAL_SPEED / 1000000) / TIMER1_CLOCK_DIVIDER_EQ;
const    uint16_t FILTER_LOCKLOSS_THRESHOLD      = 100   * (REAL_SPEED / 1000000) / TIMER1_CLOCK_DIVIDER_EQ; // Allowed deviation between the locked value and the current filtered value before attempting to re-lock
const    uint16_t BLANKING_TIME_MAXIMUM          = 40000 * (REAL_SPEED / 1000000) / TIMER1_CLOCK_DIVIDER_EQ;
const    uint16_t BLANKING_TIME_MINIMUM          = 30    * (REAL_SPEED / 1000000) / TIMER1_CLOCK_DIVIDER_EQ;
const    uint16_t ACTIVE_TIME_MAXIMUM            = 40000 * (REAL_SPEED / 1000000) / TIMER1_CLOCK_DIVIDER_EQ;
const    uint16_t ACTIVE_TIME_MINIMUM            = 1000  * (REAL_SPEED / 1000000) / TIMER1_CLOCK_DIVIDER_EQ;
const    uint16_t TIME_REQUIRED_FOR_SOFTSERIAL  = 440; // microseconds.  Allocated time for the softserial transaction.
const    uint16_t TIME_REQUIRED_FOR_RGBLED      = 150; // microseconds.  Allocated time for the softserial transaction.
const    uint16_t TIME_REQUIRED_FOR_TIMING_SENSITIVE_ACTIVITY = TIME_REQUIRED_FOR_SOFTSERIAL; // microseconds.  The longest time needed by any interrupt-sensitive activity.


uint32_t InterruptPreviousMicros=0;
ISR(INT0_vect) {
  noInterrupts();
  // Note: In scanning mode, the interrupt should only be enabled for the falling edge.  For other modes, both edges are used
  uint16_t myTCNT1 = TCNT1;
  switch (OUTPUT_MODE) {
    case OUTPUT_MODE_STROBE :
      if (digitalRead2(INPUT_PULSE) == HIGH) {
//        OCR1B = StrobePulseHigh;
//        WritePulseToAllPins(LOW);
        RAW_ACTIVE = myTCNT1;
//        Serial.disableSerialInterruptTX();
      } else {
        TCNT1 = 0;
        if(StrobeSerial<StrobePulseHigh) {
          OCR1B = StrobeSerial; 
        } else {
          OCR1B = StrobePulseHigh;          
        }
        RAW_BLANKING = myTCNT1; // Note: subtract RAW_ACTIVE later
        TIFR1 = (1 << ICF1) | (1 << OCF1A) | (1 << OCF1B) | (1 << TOV1); // Clear timer 1 interrupts for a fresh start
        FrameTimingsBelongToNewFrame=true;
        adimWrite();
        // Note that post-interrupt-activity has been moved until after the strobe falls (inside timer interrupt) for timing reasons.
        // InfraredStereoTransmitter.SendInfraredSyncToGlasses();
        // DoPeriodicTimingSensitiveActivity();     
        // Serial.automaticSerialInterruptTX();
      }
      break;
    case OUTPUT_MODE_SCAN :
      if (digitalRead2(INPUT_PULSE) == HIGH) {
        // Note: The specific time values of pulses should be chosen to avoid having a transition occur near this time.
        RAW_ACTIVE = myTCNT1;
//        Serial.disableSerialInterruptTX();
      } else {
        WritePulseToAllPins(LOW);
        ScanningState = FIRST_SEGMENT_ON;
        OCR1A = GetScanTimeFirst();
        TCNT1 = 0;
        RAW_BLANKING = myTCNT1;  // Note: subtract RAW_ACTIVE later
        TIFR1 = (1 << ICF1) | (1 << OCF1A) | (1 << OCF1B) | (1 << TOV1); // Clear timer 1 interrupts for a fresh start
        FrameTimingsBelongToNewFrame=true;
        adimWrite();
        DoPeriodicTimingSensitiveActivity();
//        Serial.automaticSerialInterruptTX();
      }
      break;   
    default :
    uint32_t myMicros=micros();    
    uint16_t myDelay=myMicros-InterruptPreviousMicros;
    InterruptPreviousMicros=myMicros;
      if (digitalRead2(INPUT_PULSE) == HIGH) {
        RAW_ACTIVE = myDelay;
      } else {
        RAW_BLANKING = myDelay;        
        FrameTimingsBelongToNewFrame=true;
        InfraredStereoTransmitter.SendInfraredSyncToGlasses_ContinuiousBacklight(); 
        DoPeriodicTimingSensitiveActivity();
      }
  }
  RecentInputPulse = true;
  wdt_reset();
  EIFR = (1 << INTF0); //Just in case the pin toggled while we were inside this interrupt, clear the flag.  Note: interrupt1 is not used
  interrupts();
}

ISR(WDT_vect) {
#if OVERCLOCKING==ENABLED
  my_Overclock_object.WatchdogFired();
#endif
  // Add your other watchdog code here.
  // TODO: figure out the 'best' way to handle this protection
  RecentInputPulse = false;
  OutputDisable();
  softReset();
}


void setup()
{
  wdt_disable();
  SetStaticPins();

  SerialFlush();
  Serial.begin(SERIAL_BAUD); // Note: not yet overclocked.
  SerialDebug(F("BUILD: ")); SerialDebugln(F("" __DATE__ " " __TIME__ "\n"));
  SerialDebug(F("PCB: ")); board_print_name();
  //  SerialDebug(F("BUTTONS: ")); buttonboard_print_name();
  
  #ifdef LED_RGB
  LED.setOutput(LED_RGB);
  LED.setColorOrderGRB();
  #endif

  if (!get_has_been_factory_programmed()) { RunFactoryProgramming(); }

  SerialDebugln(F("\nINIT/CONFIG"));
  SerialFlush(); // Flush serial output in case something goes horribly wrong, that way we guarantee seeing the last line of output.
  LoadParametersLED();
  LoadSavedParameters();
  SerialDebugln(F("\t->OK"));

  SerialDebug(F("LED: ")); SerialDebugln(MyConfigLED.Name);

#if OVERCLOCKING==ENABLED
  SerialDebugln(F("\nINIT/OC"));
  SerialFlush();
  my_Overclock_object.SetClockSpeedOC();
  SerialDebug(F("\t->Clock: ")); SerialDebug(my_Overclock_object.GetClockSpeed());   SerialDebugln(F("Hz"));
#endif

#ifdef SLAVESERIALSPEED
  SerialToSlaves.begin(SLAVESERIALSPEED);
#endif

  SerialDebugln(F("\nINIT/TIMERS"));
  SerialFlush(); // Flush serial output in case something goes horribly wrong, that way we guarantee seeing the last line of output.
  ConfigureTimersInitial();
  SerialDebugln(F("\t->OK"));

  SerialDebugln(F("\nINIT/INTERRUPT"));
  SerialFlush(); // Flush serial output in case something goes horribly wrong, that way we guarantee seeing the last line of output.
  StartSyncInterruptChange();
  interrupts();
  SerialDebugln(F("\t->OK"));

  SerialDebugln(F("\nINIT/DONE\n\n"));
  FinishedConfigurationTime = millisNoInterruptChanges();
  SerialFlush();
  EnterControlOFF();

  PostHysteresisInputPWM = MaximumInputPWM / 2;
  
  POWER_LIMIT = INITIAL_POWER_LIMIT;
  DetermineMaximumStableCurrent();
  CalculateCycleTimeLimits();

  PrintConfigLED();
Serial.flush();
}


const uint16_t OneMillisecond        = 1;
const uint16_t TenMilliseconds       = 10;
const uint16_t HundredMilliseconds   = 100;
const uint16_t OneSecond             = 1000;
const uint16_t TenSeconds            = 10000;

void loop() {
  currentMillis = millisNoInterruptChanges();
  HandleMillisInTimer0Overflow();
  wdt_reset();
  TaskFastest();
  if ((currentMillis - Task1ms_previousMillis) >= OneMillisecond) {
    Task1ms();
    Task1ms_previousMillis = currentMillis;
  }

  if ((currentMillis - Task10ms_previousMillis) >= TenMilliseconds) {
    Task10ms();
    Task10ms_previousMillis = currentMillis;
  }

  if ((currentMillis - Task100ms_previousMillis) >= HundredMilliseconds) {
    Task100ms();
    Task100ms_previousMillis = currentMillis;
  }

  if ((currentMillis - Task1000ms_previousMillis) >= OneSecond) {
    Task1000ms();
    Task1000ms_previousMillis = currentMillis;
  }

  if ((currentMillis - Task10000ms_previousMillis) >= TenSeconds) {
    Task10000ms();
    Task10000ms_previousMillis = currentMillis;
  }
}



void TaskFastest() {
  SampleInputPWM();
  SamplesPerSecondPWM = SamplesPerSecondPWM + 1;
}

void Task1ms() {
  RefilterFrameParameters();
  MaybeUpdateStatusLED();
  SoftAdjustAdim();  
}

void Task10ms() {
  ApplyParameters();
  ButtonBoard.ReadPhysicalInputs();
  ButtonBoard.RefilterInputState();
  HandleButtonBoardInput();
  MaybeSendSerialStateToSlaves();
  // SerialDebug(F("RecentPulse : "));  SerialDebugln(RecentInputPulse);
}

void Task100ms() {
  SoftAdjustPowerLimit();  // Gently ramps up the power limit to avoid tripping short-circuit protection on the input power supply
  ModeHandling();
  RecalculateParameters();
}

void Task1000ms() {
  PrintParametersIfChanged();
  //  SerialDebug(F(" Samples per second : ")); SerialDebugln(SamplesPerSecondPWM);  // This was around 3800 on the last check.
  SamplesPerSecondPWM = 0;
//  ButtonBoard.PrintButtons();
}

void Task10000ms() {
  PrintDebugSerialStateToSlaves();
//  PrintParameters();
  //  myTemp=myTemp+1;
  //  analogWrite(BUTTONBOARD_LED_G, myTemp);
  //  SerialDebug(F("Requesting new level : ")); SerialDebugln(myTemp);
}



uint8_t SafelyReadEnableInput() {
  return LOW;
  //return digitalRead(INPUT_ENABLE_OR_SCL_PIN);  // This function used to be significant when both INPUT_ENABLE and DIM inputs were used bidirectionally
}


uint8_t BufferedInputEnable = LOW;
uint8_t BufferedRecentInputPulse = false;

void ModeHandling() {
  BufferedInputEnable = SafelyReadEnableInput();
  BufferedRecentInputPulse = RecentInputPulse;
  RecentInputPulse = false;
  ModeHandlingControl();
  ModeHandlingOutput();
}

void ModeHandlingControl() {
  //SerialDebug(F("InPulse : ")); if(BufferedRecentInputPulse==false) { SerialDebugln(F("false"));} else { SerialDebugln(F("OK"));}
  // First handle the control mode
  //  SerialDebug(F("InputEnable : ")); SerialDebugln(InputEnable==HIGH);
  switch (CONTROL_MODE) {
    case CONTROL_MODE_OFF :
      if (BufferedRecentInputPulse == true) {
        CONTROL_MODE = CONTROL_MODE_WAIT_ZWS_TURNON;
        EnterIntermediateStateMillis = millisNoInterruptChanges();
      }
      break;
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
  }
}

void ModeHandlingOutput() {
  if (OUTPUT_MODE == OUTPUT_MODE_OFF ) {
    switch (CONTROL_MODE) {
      case CONTROL_MODE_OFF : break;
      case CONTROL_MODE_WAIT_ZWS_TURNON : break;
      case CONTROL_MODE_ZWS :
        if (TargetPowerSave == TargetPowerSaveFULLY_ON) {
          PowerON();
          return;
        }
        break;
    }
  }
  if (OUTPUT_MODE == OUTPUT_MODE_STABLE ) {
    switch (CONTROL_MODE) {
      case CONTROL_MODE_OFF : PowerOFF(); break;
      case CONTROL_MODE_WAIT_ZWS_TURNON : PowerOFF();  break;
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
    }
  }
  if (OUTPUT_MODE == OUTPUT_MODE_STROBE ) {
    switch (CONTROL_MODE) {
      case CONTROL_MODE_OFF : PowerOFF(); break;
      case CONTROL_MODE_WAIT_ZWS_TURNON : PowerOFF();  break;
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
    }
  }
  if (OUTPUT_MODE == OUTPUT_MODE_SCAN ) {
    switch (CONTROL_MODE) {
      case CONTROL_MODE_OFF : PowerOFF(); break;
      case CONTROL_MODE_WAIT_ZWS_TURNON : PowerOFF();  break;
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
    }
  }
}

void OutputEnable() {
  digitalWrite(BLDRIVER_ENABLE, HIGH);
}
void OutputDisable() {
  digitalWrite(BLDRIVER_ENABLE, LOW);
}


void PowerON() {
  SerialDebugln(F("Powering ON"));
  OutputEnable();
  EnterStable();
}

void PowerOFF() {
  SerialDebugln(F("Powering OFF"));
  OutputDisable();
  EnterOff();
  IndicateNewState();
}

void RotatePowerState(){
  // Don't support the fully off power state yet.
      if (TargetPowerSave != TargetPowerSaveFULLY_ON) {
        TargetPowerSave = TargetPowerSaveFULLY_ON;
      } else {
        TargetPowerSave = TargetPowerSaveLOWPOWER;
      }
      UserConfiguration_SaveShutdown(TargetPowerSave);  
}

void OutputModeRotate(){
switch (TargetMode) {
          case OUTPUT_MODE_STABLE : if(CheckCapableStrobing()== true) {OutputModeSetSave(OUTPUT_MODE_STROBE); return;} if(CheckCapableScanning()== true) {OutputModeSetSave(OUTPUT_MODE_SCAN); return; }  break;
          case OUTPUT_MODE_STROBE : if(CheckCapableScanning()== true) {OutputModeSetSave(OUTPUT_MODE_SCAN); return;}   OutputModeSetSave(OUTPUT_MODE_STABLE); return;
          case OUTPUT_MODE_SCAN   : OutputModeSetSave(OUTPUT_MODE_STABLE); return;
          default : TargetMode = OUTPUT_MODE_STABLE;
        }
}

void OutputModeSetSave(uint8_t myMode){
  TargetMode = myMode; 
  UserConfiguration_SaveStrobing(myMode);
}

      
 void extended_function(){
 #if SERIAL_COMMANDS_EXTENDED==DISABLED
    return;
 #endif
  const uint8_t buffersize = 16;
  char command_string[buffersize];
  uint8_t bufferindex=0;
  char* bufferpointer;
  char mychar;
  
  delay(50); // Give a small window from the first detection of the extended command character to recieve additional characters in the command
  
  while(Serial.available()) {
    mychar=Serial.read();
    Serial.write(mychar);
        if(bufferindex < buffersize-1) // One less than the size of the array
        {
            command_string[bufferindex] = mychar; // Read a character
            bufferindex++; // Increment where to write next
            command_string[bufferindex] = '\0'; // Null terminate the string
        }
        if (mychar!='\n') {delay(50);} // Extended command character recieve window
  }

  if (command_string[0] != '\0') {
    Serial.print("\nEcho:\"");
    Serial.print(command_string);
    Serial.println("\"");
  }
  
 // All commands are two letter acronyms, so numeric argument extraction can be done once here
    uint16_t my_value=0;
    bufferpointer=command_string+2;
    my_value=strtoul(bufferpointer, &bufferpointer, 10);

// COMMAND STRINGS ARE HANDLED HERE
// Sets the current brightness
  if (strncmp(command_string,"br",2)==0) {
  SetTargetBrightness(my_value);
    return;
  }

// Prints the current parameters
  if (strncmp(command_string,"pr",2)==0) {
  PrintParameters();
    return;
  }
// Run the scan test
  if (strncmp(command_string,"st",2)==0) {
  EnterScanningTest();
    return;
  }
// strobe configuration override
  if (strncmp(command_string,"po",2)==0) {
  if(my_value==0) { DisableStrobeOverride();} else {EnableStrobeOverride();}
    return;
  }
  
// scan configuration override
  if (strncmp(command_string,"so",2)==0) {
  if(my_value==0) { DisableScanOverride();} else {EnableScanOverride();}
    return;
  }

  
// strobe override configuration print
  if (strncmp(command_string,"pp",2)==0) {
    PrintStrobeOverride();
    return;
  }
  
// scan override configuration print
  if (strncmp(command_string,"sp",2)==0) {
    PrintScanOverride();
    return;
  }
  
// strobe current override
    if (strncmp(command_string,"pc",2)==0) {
      StrobeOverrideCurrent(my_value);
    return;
  }
  
// scan current override
    if (strncmp(command_string,"sc",2)==0) {
      ScanOverrideCurrent(my_value);
    return;
  }
  
// strobe duration override
    if (strncmp(command_string,"pw",2)==0) {
      StrobeOverrideDuration(my_value);
    return;
  }
  
// scan duration override
    if (strncmp(command_string,"sw",2)==0) {
      ScanOverrideDuration(my_value);
    return;
  }
  
// strobe delay override
    if (strncmp(command_string,"pd",2)==0) {
      StrobeOverrideDelay(my_value);
    return;
  }
  
// scan delay override
    if (strncmp(command_string,"sd",2)==0) {
      ScanOverrideDelay(my_value);
    return;
  }
  
 Serial.println(F("???"));
}


void HandleButtonBoardInput() {

  if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_SIMPLE_DEBUG_COMMAND) { EnterScanningTest(); }
   
  if(ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_SPECIAL_COMMANDS                                        ) { extended_function();                                               return; }   

    
  if (( CONTROL_MODE == CONTROL_MODE_ZWS) || (CONTROL_MODE == CONTROL_MODE_OFF)) {
    // Commands meant to be repeated when held
    if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_BRIGHTNESS_DECREASE) { BrightnessDecrement(); }
    if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_BRIGHTNESS_INCREASE) { BrightnessIncrement(); }
    // Commands meant to be run when released
    if ((ButtonBoard.GetPreviousFilteredInput() == COMMAND_CODE_FOR_BRIGHTNESS_INCREASE) && (ButtonBoard.GetCurrentFilteredInput() != COMMAND_CODE_FOR_BRIGHTNESS_INCREASE)) { BrightnessSave(); }
    if ((ButtonBoard.GetPreviousFilteredInput() == COMMAND_CODE_FOR_BRIGHTNESS_DECREASE) && (ButtonBoard.GetCurrentFilteredInput() != COMMAND_CODE_FOR_BRIGHTNESS_DECREASE)) { BrightnessSave(); }
    
    if ((ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_POWER_BUTTON) ) { SerialDebugBlockSlaveCommunication(); }
        
    if ((ButtonBoard.GetPreviousFilteredInput() == COMMAND_CODE_FOR_POWER_BUTTON) && ((ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_NOTHING) || (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_UNDEFINED))) {
      RotatePowerState();
      SerialDebugUnblockSlaveCommunication();
    }
    // commands meant to be run when pressed
    if ((ButtonBoard.GetPreviousFilteredInput() == COMMAND_CODE_FOR_NOTHING) || (ButtonBoard.GetPreviousFilteredInput() == COMMAND_CODE_FOR_UNDEFINED)) {
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_PWM_FREQ_DECREASE) {
        IncrementFrequencyPWM();
      }
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_PWM_FREQ_INCREASE) {
        DecrementFrequencyPWM();
      }
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_0) {
        TargetEDID = 0;
        UserConfiguration_SaveEDID(TargetEDID);
      }
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_1) {
        TargetEDID = 1;
        UserConfiguration_SaveEDID(TargetEDID);
      }
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_2) {
        TargetEDID = 2;
        UserConfiguration_SaveEDID(TargetEDID);
      }
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_3) {
        TargetEDID = 3;
        UserConfiguration_SaveEDID(TargetEDID);
      }
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_4) {
        TargetEDID = 4;
        UserConfiguration_SaveEDID(TargetEDID);
      }
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_STROBE_ROTATE) { OutputModeRotate(); } 
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_PANEL_OSD) {
        if (TargetOSD == false) {
          TargetOSD = true;
        } else {
          TargetOSD = false;
        }  UserConfiguration_SaveOSD(TargetOSD);
      }
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_TOGGLE_STEREO_EYE) {
        InfraredStereoTransmitter.SwapEye();
      }
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_STEREO_ENABLE) {
        InfraredStereoTransmitter.SetDisabled();
      }
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_STEREO_ENABLE) {
        InfraredStereoTransmitter.SetEnabled();
      }
      if (ButtonBoard.GetCurrentFilteredInput() == COMMAND_CODE_FOR_FACTORY_PROGRAM) {
        RunFactoryProgramming();
      }
    }
  }
}

void BrightnessIncrement(){
  ShouldPrintUpdatedParameters=true;
      if (GetTargetBrightness() < CALCULATED_MAXIMUM_BRIGHTNESS) {
        SetTargetBrightness(GetTargetBrightness() + 1);
        ShouldRecalculateParameters = true;
      }
  
}

void BrightnessDecrement(){
  ShouldPrintUpdatedParameters=true;
      if ((GetTargetBrightness() > CALCULATED_MINIMUM_BRIGHTNESS) && (GetTargetBrightness() > MinimumBrightnessSetpoint)) {
        SetTargetBrightness(GetTargetBrightness() - 1);
        ShouldRecalculateParameters = true;
      }
}

void IncrementFrequencyPWM() {
  return;
  ShouldPrintUpdatedParameters=true;
  if (TargetTimerDividerStable > TimerDividerMinimum) {
    TargetTimerDividerStable = TargetTimerDividerStable - 1;
    CalculateCycleTimeLimits();
    UpdateTargetBrightness();
    ShouldRecalculateParameters = true;
  }
}

void DecrementFrequencyPWM() {
  return;
  ShouldPrintUpdatedParameters=true;
  if (TargetTimerDividerStable > TimerDividerMinimum) {
    TargetTimerDividerStable = TargetTimerDividerStable - 1;
    CalculateCycleTimeLimits();
    UpdateTargetBrightness();
    ShouldRecalculateParameters = true;
  }
}

// Note that the backlight driver board has weak (100K) pulldowns present on these pins.  It is preferable, but probably not necessary, to turn off the pullups on the recieving devices.
// Note: connect all reciever boards to the same pin.
void SlaveSerialBegin() {
  pinMode(INPUT_DIM_OR_SDA_PIN, OUTPUT);
  SerialToSlave.begin(SLAVESERIALSPEED);
}

void SlaveSerialEnd() {
  SerialToSlave.end(); // Note: this function does nothing
  pinMode(INPUT_DIM_OR_SDA_PIN, INPUT_PULLUP);
}

void EnterControlOFF() {
  if(DebugBlockSlaveSerialCommands==true) { return;}
  SlaveSerialBegin();
}
void ExitControlOFF() {
  SlaveSerialEnd();
}
void EnterControlZWS() {
  if(DebugBlockSlaveSerialCommands==true) { return;}
  SlaveSerialBegin();
}
void ExitControlZWS() {
  SlaveSerialEnd();
}


void DoPeriodicTimingSensitiveActivity(){
        if(ShouldUpdateStatusLED == true) {UpdateStateLED(); return;}  
        if(ShouldSendSerialCommand == true) {SendSerialStateToSlaves(); return;}
}

void SerialDebugBlockSlaveCommunication(){
  ShouldSendSerialCommand=false;
  DebugBlockSlaveSerialCommands=true;
  SlaveSerialEnd();
}


void SerialDebugUnblockSlaveCommunication(){
  DebugBlockSlaveSerialCommands=false;  
  if((CONTROL_MODE == CONTROL_MODE_OFF) || (CONTROL_MODE == CONTROL_MODE_ZWS)) { SlaveSerialBegin(); }
}

volatile uint8_t TXvalue;
uint8_t SkipSameTXCount=0;
const uint8_t ResendTX = 100; // Reduce same-state retransmits by this ratio

void MaybeSendSerialStateToSlaves(){
//  if(DebugBlockSlaveSerialCommands==true) { return;} // Note: use SlaveSerialEnd(); and SlaveSerialBegin(); when changing DebugBlockSlaveSerialCommands
  if((TXvalue==SerialCommandGenerate()) && (SkipSameTXCount >0)) {
    SkipSameTXCount=SkipSameTXCount-1;  
  } else {
    SkipSameTXCount=ResendTX;    
    if(CONTROL_MODE == CONTROL_MODE_OFF) { SendSerialStateToSlaves(); return;}
    if(CONTROL_MODE == CONTROL_MODE_ZWS) { ShouldSendSerialCommand=true; return; }// Handle this in the interrupt now
  }
}

volatile uint8_t SendSerialStateToSlavesPhase=0;
void SendSerialStateToSlaves() { // Note: sending newlines can increase error rate, it is better to just send one byte
if(SendSerialStateToSlavesPhase == 0){
  SendSerialStateToSlavesPhase = 1;
  TXvalue=SerialCommandGenerate();
} else {
  SendSerialStateToSlavesPhase = 0;  
  switch (OUTPUT_MODE) {
    case OUTPUT_MODE_STABLE : if (CALCULATED_PWM_STABLE < PWM_MAX) {TXvalue=ASCII_CODE_FOR_BL_MODE_IS_PWM;}
                              else {TXvalue=ASCII_CODE_FOR_BL_MODE_IS_NOPWM;}
                              break; 
    case OUTPUT_MODE_STROBE : TXvalue=ASCII_CODE_FOR_BL_MODE_IS_STROBE; break;
    case OUTPUT_MODE_SCAN   : TXvalue=ASCII_CODE_FOR_BL_MODE_IS_SCAN; break;
    default                 : TXvalue=ASCII_CODE_FOR_BL_MODE_IS_INVALID; break;
  };  
}  
  SerialToSlave.write(TXvalue);
  ShouldSendSerialCommand=false;
}




void PrintDebugSerialStateToSlaves() {PrintParsedSerialCommand(SerialCommandGenerate());}


void SoftSerialTest() {
  // This function helps verify that the software serial is working OK (timer0 prescaler and interrupt OK)
  //SetTimer0ClockDivider(DetermineCorrectTimer0Divider(64));
  //TIMSK0=0x00|(1<<TOIE0); // To count millis and micros properly while in stable mode
  SendOnlySoftwareSerial SerialTest(1); // Assume TX is on D1 (always the case on atmega328p)
  SerialDebug(F("Sending..."));
  Serial.end();
  SerialTest.begin(SERIAL_BAUD);
  SerialTest.println(F("SoftSending OK"));
  SerialTest.end();
  Serial.begin(SERIAL_BAUD);

}


void ApplyParameters() {
  switch (OUTPUT_MODE) {
    case OUTPUT_MODE_STABLE : adimWrite(CALCULATED_ADIM_STABLE); WriteAllPWMs(CALCULATED_PWM_STABLE); break;
    case OUTPUT_MODE_STROBE : AdjustTimerStrobing(); break;
    case OUTPUT_MODE_SCAN   : BuildTimeLine(); break;
    default    : adimWrite(0); WriteAllPWMsLOW(); break;
  }  
}

void SampleInputPWM() { return;
//  if ( CONTROL_MODE == CONTROL_MODE_PWM) {
//    // This method may be less elegant, but it is self-filtering and tolerant of jitter, interrupts, and noise.
//    const uint16_t SampleDepth = 4096;
//    if ((digitalRead2(INPUT_DIM_OR_SDA_PIN)) == LOW) {
//      SampledLow = SampledLow + 1;
//    } else {
//      SampledHigh = SampledHigh + 1;
//    }
//    if ((SampledHigh == SampleDepth) || (SampledLow == SampleDepth)) {
//      SampledHigh = SampledHigh / 2;
//      SampledLow = SampledLow / 2;
//    }
//    SampledPWM = 1.0 * MaximumInputPWM * SampledHigh / (SampledHigh + SampledLow);
//    //  SerialDebug(F("SampledHigh: ")); SerialDebugln(SampledHigh);
//    //  SerialDebug(F("SampledLow : ")); SerialDebugln(SampledLow);
//    //  SerialDebug(F("SampledPWM : ")); SerialDebugln(SampledPWM);
//    if ( abs(SampledPWM - PostHysteresisInputPWM) > HysteresisInputPWM) {
//      PostHysteresisInputPWM = SampledPWM ;
//    }
//  }
}

void UpdateTargetBrightness() {
  SetTargetBrightness(CALCULATED_MINIMUM_BRIGHTNESS + 1.0 * (CALCULATED_MAXIMUM_BRIGHTNESS - CALCULATED_MINIMUM_BRIGHTNESS) * (1.0 * PostHysteresisInputPWM / MaximumInputPWM));
}





uint16_t ScanPulseDelayInitial=15; // microseconds
uint16_t ScanPulseDuration=150; // microseconds
uint16_t ScanPulseDelay=500; // microseconds

void RecalculateParameters() {
  ShouldRecalculateParameters = false;
  CalculateParametersStable(); wdt_reset();
  CalculateParametersStrobe(); wdt_reset();
  CalculateParametersScan(); wdt_reset();  
}


// Recalculate either when getting a lock on the video signal or when changing brightness.  To avoid sluggishness, don't necessarily recalculate for every new brighntess value
void MaybeRecalculateParameters() {
  if( ShouldRecalculateParameters == false ) {return;}
  RecalculateParameters();  
}


uint16_t CalculateMinimumCurrent() {
  if ( CURRENT_MINIMUM < MyConfigLED.CurrentMinimum ) {
    return MyConfigLED.CurrentMinimum ;
  } else {
    return CURRENT_MINIMUM ;
  }
}


uint16_t CalculateMaximumCurrent() {
  if ( CURRENT_MAXIMUM > MyConfigLED.CurrentMaximum ) {
    return MyConfigLED.CurrentMaximum ;
  } else {
    return CURRENT_MAXIMUM ;
  }
}

uint32_t CalculatePowerStable(){  return CalculatePower(CALCULATED_ADIM_STABLE, CALCULATED_PWM_STABLE);  }

uint32_t CalculatePowerStrobe(){  return CalculatePower(CALCULATED_ADIM_STROBE, StrobePulseDuration, FILTERED_ACTIVE);}

uint32_t CalculatePowerScan(){  return CalculatePower(CALCULATED_ADIM_SCAN, ScanPulseDuration, FILTERED_ACTIVE);  }

uint32_t CalculatePower(uint8_t myADIM, uint8_t myPWM) {  return CalculatePower(myADIM, myPWM,  PWM_MAX);}

uint32_t CalculatePower(uint8_t myADIM, uint16_t myNumerator, uint16_t myDenominator) {
  uint16_t myCurrent = ADIM_LEVEL_TO_OUTPUT_CURRENT(myADIM);
  uint16_t myVoltage = current_to_voltage(myCurrent);
  float myDutyCycle = 1.0 * myNumerator / myDenominator;
  return MyConfigLED.NumberStrings * myDutyCycle * myCurrent * myVoltage / 1000; // Divided by 1000 because myVoltage is in mA
}

uint16_t CalculateBrightnessStable() {  return CalculateBrightness(CALCULATED_ADIM_STABLE, CALCULATED_PWM_STABLE);}

uint16_t CalculateBrightnessStrobe() {  return CalculateBrightness(CALCULATED_ADIM_STROBE, StrobePulseDuration, FILTERED_ACTIVE);}

uint16_t CalculateBrightnessScan() {  return CalculateBrightness(CALCULATED_ADIM_SCAN, ScanPulseDuration, FILTERED_ACTIVE);}

uint16_t CalculateBrightness(uint8_t myADIM, uint8_t myPWM) {  return CalculateBrightness(myADIM, myPWM,  PWM_MAX);}


uint16_t CalculateBrightness(uint8_t myADIM, uint16_t myNumerator, uint16_t myDenominator) {
  uint16_t myCurrent = ADIM_LEVEL_TO_OUTPUT_CURRENT(myADIM);
  uint16_t myPeakBrightness = current_to_brightness(myCurrent);
  float myDutyCycle = 1.0 * myNumerator / myDenominator;
  return myDutyCycle * myPeakBrightness;
}

void DetermineMaximumStableCurrent() {
  uint8_t myTempADIM = ADIM_MAX;
  uint32_t myPowerMAX=POWER_LIMIT;
  uint32_t myPower;

  //SerialDebug(F("LIMIT IS : ")); SerialDebug(myPowerMAX);  SerialDebugln(F("mW"));
  while ( myTempADIM > 0 ) {
    myPower = CalculatePower(myTempADIM, PWM_MAX);
    //  SerialDebug(F("DEBUGPOWER is "));  SerialDebug(myPower);  SerialDebugln(F("mW"));
    //  SerialDebug(F("DEBUGCurrent : ")); SerialDebug(ADIM_LEVEL_TO_OUTPUT_CURRENT(myTempADIM));  SerialDebugln(F("mA"));
    //  SerialDebug(F("DEBUGADIM : ")); SerialDebugln(myTempADIM);
    if ( myPower > myPowerMAX ) {
      myTempADIM = myTempADIM - 1;
    } else {
      MAXIMUM_STABLE_ADIM = myTempADIM;
      myTempADIM = 0;
    }
  }
  myTempADIM = MAXIMUM_STABLE_ADIM;

  // Now that minimum ontime and offtime limits have been applied, need to apply maximum current limits
  uint16_t myCurrentMAX;
  uint16_t myCurrent;
  if ( CURRENT_MAXIMUM_SUTAINED > MyConfigLED.CurrentMaximum ) {
    myCurrentMAX = MyConfigLED.CurrentMaximum ;
  } else {
    myCurrentMAX = CURRENT_MAXIMUM_SUTAINED ;
  }
  //SerialDebug(F("LIMIT IS : ")); SerialDebug(myCurrentMAX);  SerialDebugln(F("mA"));
  while ( myTempADIM > 0 ) {
    myCurrent = ADIM_LEVEL_TO_OUTPUT_CURRENT(myTempADIM);
    //  SerialDebug(F("DEBUGCurrent is "));  SerialDebug(myCurrent);  SerialDebugln(F("mA"));
    //  SerialDebug(F("DEBUGADIM : ")); SerialDebugln(myTempADIM);
    if ( myCurrent > myCurrentMAX ) {
      myTempADIM = myTempADIM - 1;
    } else {
      MAXIMUM_STABLE_ADIM = myTempADIM;
      return;
    }
  }
}




void PrintParametersIfChanged() {
  if(ShouldPrintUpdatedParameters==true) { PrintParameters(); }
  ShouldPrintUpdatedParameters=false;  
}





void PrintParameters() {
#if (SERIAL_DEBUGGING_OUTPUT == DISABLED)
  return;
#endif
  SerialDebugln(F(""));
  SerialDebugln(F(""));
  PrintSomeDebugCrap();
  PrintInputPulseTiming();
  PrintControlScheme();
  
  SerialDebug(F("TargetBrightness : ")); SerialDebug(GetTargetBrightness()); SerialDebugln(F(" nits"));
      
  SerialDebug(F("BACKLIGHT MODE IS : "));
  switch (OUTPUT_MODE) {
    case OUTPUT_MODE_OFF :
        SerialDebugln(F("OFF"));
        return;
    case OUTPUT_MODE_STABLE :
        PrintConfigStable();
        break;
    case OUTPUT_MODE_STROBE :  
        PrintConfigStrobe();
        break;
    case OUTPUT_MODE_SCAN :   
        PrintConfigScan();
        break;
    default :                 SerialDebugln(F("???"));
  }

//  SerialDebugln(F(""));
//  SerialDebugln(F(""));
//  PrintTimer1ConfigurationRegisters();
//  PrintTimer2ConfigurationRegisters();
//  SerialDebugln(F("")); PrintConfigStable();
//  SerialDebugln(F("")); PrintConfigStrobe();
//  SerialDebugln(F("")); PrintConfigScan();

  SerialDebugln(F(""));
  SerialDebugln(F(""));
}




void PrintSomeDebugCrap(){  
  //  SerialDebug(F("BrightnessTarget is "));  SerialDebug(GetTargetBrightness());  SerialDebugln(F("nits"));
  //  SerialDebug(F("Brightness@MIN is "));  SerialDebug(current_to_brightness(LED_CURRENT_MINIMUM));  SerialDebugln(F("nits"));
  //  SerialDebug(F("Brightness@MAX is "));  SerialDebug(current_to_brightness(LED_CURRENT_MAXIMUM));  SerialDebugln(F("nits"));
  //  SerialDebug(F("ADIM_LEVEL is "));  SerialDebug(CALCULATED_ADIM_STABLE);  SerialDebugln(F(""));

  //  SerialDebug(F("RECALCULATED CURRENT is "));  SerialDebug(ADIM_LEVEL_TO_OUTPUT_CURRENT(CALCULATED_ADIM_STABLE));  SerialDebugln(F("mA"));
  //  SerialDebug(F("RECALCULATED CURRENT is "));  SerialDebug(voltage_to_current(current_to_voltage(ADIM_LEVEL_TO_OUTPUT_CURRENT(CALCULATED_ADIM_STABLE))));  SerialDebugln(F("mA"));
  //  SerialDebug(F("VOLTAGE is "));  SerialDebug(current_to_voltage(ADIM_LEVEL_TO_OUTPUT_CURRENT(CALCULATED_ADIM_STABLE)));  SerialDebugln(F("mV"));
  //  SerialDebug(F("I_effectived is "));  SerialDebug(brightness_to_current(GetTargetBrightness()));  SerialDebugln(F("mA"));
}


void PrintControlScheme(){
  SerialDebug(F("CONTROL SCHEME : "));
  switch (CONTROL_MODE) {
    case CONTROL_MODE_OFF :   SerialDebugln(F("OFF")); break;
    case CONTROL_MODE_WAIT_ZWS_TURNON : SerialDebugln(F("ZWAIT")); break;
    case CONTROL_MODE_ZWS :   SerialDebugln(F("ZWS")); break;
    default :                 SerialDebugln(F("???"));
  } 
}


void PrintInputPulseTiming(){
    SerialDebug(F("Video timings "));  if( FRAME_TIMINGS_LOCKED != true ) {SerialDebug(F("not "));} SerialDebugln(F("locked"));   
    SerialDebug(F("LockedPulseRate   : "));  SerialDebug(LOCKED_ACTIVE);  SerialDebugln(F(" us"));   
    SerialDebug(F("LockedBlanking    : "));  SerialDebug(LOCKED_BLANKING);  SerialDebugln(F(" us"));   
}


void PrintVerboseInputPulseTiming(){
    SerialDebug(F("Video timings "));  if( FRAME_TIMINGS_LOCKED != true ) {SerialDebug(F("not "));} SerialDebugln(F("locked"));   
    SerialDebug(F("RawPulseRate      : "));  SerialDebug(RAW_ACTIVE);  SerialDebugln(F(" us"));
    SerialDebug(F("SampledPulseRate  : "));  SerialDebug(SAMPLED_ACTIVE);  SerialDebugln(F(" us"));
    SerialDebug(F("FilteredPulseRate : "));  SerialDebug(FILTERED_ACTIVE);  SerialDebugln(F(" us"));
    SerialDebug(F("LockedPulseRate   : "));  SerialDebug(LOCKED_ACTIVE);  SerialDebugln(F(" us"));   
    SerialDebug(F("RawBlanking       : "));  SerialDebug(RAW_BLANKING);  SerialDebugln(F(" us"));
    SerialDebug(F("SampledBlanking   : "));  SerialDebug(SAMPLED_BLANKING);  SerialDebugln(F(" us"));
    SerialDebug(F("FilteredBlanking  : "));  SerialDebug(FILTERED_BLANKING);  SerialDebugln(F(" us"));
    SerialDebug(F("LockedBlanking    : "));  SerialDebug(LOCKED_BLANKING);  SerialDebugln(F(" us"));   
}


void PrintConfigLED() {
  SerialDebugln(F(""));
  SerialDebugln(F(""));
  SerialDebug(F("Name : ")); SerialDebugln(MyConfigLED.Name);
  SerialDebug(F("PowerLimit : ")); SerialDebug(MyConfigLED.PowerLimit); SerialDebugln(F(" mW"));
  SerialDebug(F("CurrentMinimum : ")); SerialDebug(MyConfigLED.CurrentMinimum); SerialDebugln(F(" mA"));
  SerialDebug(F("CurrentMaximum : ")); SerialDebug(MyConfigLED.CurrentMaximum); SerialDebugln(F(" mA"));
  SerialDebug(F("NumberStrings : ")); SerialDebugln(MyConfigLED.NumberStrings);
  SerialDebug(F("DeratingFactor : ")); SerialDebugln(MyConfigLED.DeratingFactor);
  SerialDebug(F("CurrentToBrightnessIntercept : ")); SerialDebug(MyConfigLED.CurrentToBrightnessIntercept); SerialDebugln(F(" nits"));
  SerialDebug(F("CurrentToBrightnessSlope : ")); SerialDebug(MyConfigLED.CurrentToBrightnessSlope); SerialDebugln(F(" nits/mA"));
  SerialDebug(F("CurrentToVoltageIntercept : ")); SerialDebug(MyConfigLED.CurrentToVoltageIntercept); SerialDebugln(F(" mv"));
  SerialDebug(F("CurrentToVoltageSlope : ")); SerialDebug(MyConfigLED.CurrentToVoltageSlope); SerialDebugln(F(" mv/mA"));
  
  SerialDebugln(F(""));
  SerialDebugln(F(""));
  SerialFlush();
}






uint16_t GetTargetBrightness(){
  switch (OUTPUT_MODE) {
    case OUTPUT_MODE_OFF     : return 0;
    case OUTPUT_MODE_STABLE  : return TargetBrightnessStable;
    case OUTPUT_MODE_STROBE  : return TargetBrightnessStrobe;
    case OUTPUT_MODE_SCAN    : return TargetBrightnessScan;
    default                  : return 0;
  }
}

void SetTargetBrightness(uint16_t myBrightness){
  switch (OUTPUT_MODE) {
    case OUTPUT_MODE_OFF     : return;
    case OUTPUT_MODE_STABLE  : TargetBrightnessStable=myBrightness; return;
    case OUTPUT_MODE_STROBE  : TargetBrightnessStrobe=myBrightness; return;
    case OUTPUT_MODE_SCAN    : TargetBrightnessScan=myBrightness; return;
    default                  : return;
  }  
}


void BrightnessSave(){
  switch (OUTPUT_MODE) {
    case OUTPUT_MODE_OFF     : return;
    case OUTPUT_MODE_STABLE  : UserConfiguration_SaveBrightnessStable(TargetBrightnessStable); return;
    case OUTPUT_MODE_STROBE  : UserConfiguration_SaveBrightnessStrobe(TargetBrightnessStrobe); return;
    case OUTPUT_MODE_SCAN    : UserConfiguration_SaveBrightnessScan(  TargetBrightnessScan  ); return;
    default                  : return;
  }  
}



void ConfigureTimersInitial() {
  ConfigureTimersOFF();
  configure_watchdog_timer();
}



void configure_watchdog_timer() {
  wdt_enable(MY_WATCHDOG_TIMEOUT);
  wdt_reset();
}

boolean SoftStartupPowerLimitIsInEffect(){  
  if((POWER_LIMIT <  POWER_MAXIMUM_SUTAINED) && (POWER_LIMIT <  MyConfigLED.PowerLimit) ) { return true; } else {return false;}
}

// Note that it is assumed in the power ramping function will be called 10x per second

void SoftAdjustPowerLimit(){
  if(SoftStartupPowerLimitIsInEffect()==false ) { return; } else { POWER_LIMIT = POWER_LIMIT+(POWER_RAMP_RATE*100); }
  if(POWER_LIMIT >  POWER_MAXIMUM_SUTAINED) { POWER_LIMIT = POWER_MAXIMUM_SUTAINED; }
  if(POWER_LIMIT >  MyConfigLED.PowerLimit) { POWER_LIMIT = MyConfigLED.PowerLimit; }  
  if(OLD_POWER_LIMIT!=POWER_LIMIT) {
    DetermineMaximumStableCurrent();
    CalculateCycleTimeLimits();
    RecalculateParameters();
    ApplyParameters();  
    OLD_POWER_LIMIT=POWER_LIMIT;
  }
}




uint8_t TARGET_ADIM = 0;
uint8_t OUTPUT_ADIM = 0;

void adimWrite(){
  switch (OUTPUT_MODE) {
    case OUTPUT_MODE_STABLE : adimWrite(CALCULATED_ADIM_STABLE); break;
    case OUTPUT_MODE_STROBE : adimWrite(CALCULATED_ADIM_STROBE); break;
    case OUTPUT_MODE_SCAN   : adimWrite(CALCULATED_ADIM_SCAN);   break;
    default :                 adimWrite(CALCULATED_ADIM_OFF);    break;
  }
}

void adimWrite(uint8_t ADIM_LEVEL) {
  TARGET_ADIM = ADIM_LEVEL ;
  SoftAdjustAdim();
}


void SoftAdjustAdim(){
  if(TARGET_ADIM == OUTPUT_ADIM) { return; }
  if(TARGET_ADIM <  OUTPUT_ADIM) { OUTPUT_ADIM = TARGET_ADIM; }
  if(TARGET_ADIM >  OUTPUT_ADIM) { OUTPUT_ADIM = OUTPUT_ADIM +1; }
  adimWritePins(OUTPUT_ADIM);
}


void adimWritePins(uint8_t ADIM_LEVEL) {
  if (ADIM_LEVEL > ADIM_MAX) {
    ADIM_LEVEL = ADIM_MAX;
  }

#if USE_PORTB_FULL_ADIM_WRITE_METHOD == true
  // NOTE: This technique writes to the entirety of portB simultaneously.
  // The extra pin on portB will not be affected if and only if it is either an input or a PWM output.
  uint8_t myportB = 0x00;
  if (ADIM_LEVEL & (0x01 << 0)) {
    myportB |= digitalPinToBitMask(BLDRIVER_ADIM_0);
  }
  if (ADIM_LEVEL & (0x01 << 1)) {
    myportB |= digitalPinToBitMask(BLDRIVER_ADIM_1);
  }
  if (ADIM_LEVEL & (0x01 << 2)) {
    myportB |= digitalPinToBitMask(BLDRIVER_ADIM_2);
  }
  if (ADIM_LEVEL & (0x01 << 3)) {
    myportB |= digitalPinToBitMask(BLDRIVER_ADIM_3);
  }
  if (ADIM_LEVEL & (0x01 << 4)) {
    myportB |= digitalPinToBitMask(BLDRIVER_ADIM_4);
  }
  if (ADIM_LEVEL & (0x01 << 5)) {
    myportB |= digitalPinToBitMask(BLDRIVER_ADIM_5);
  }
  if (ADIM_LEVEL & (0x01 << 6)) {
    myportB |= digitalPinToBitMask(BLDRIVER_ADIM_6);
  }
  PORTB = myportB;
 #else
  // If for some reason, this is not desired, the pins can be written individually at the expense of possibly some bad behavior (such as being interrupted and a potentially unsafe value used temporarily) during changes.
    if(ADIM_LEVEL&(0x01<<0)) {digitalWrite2(BLDRIVER_ADIM_0, HIGH);} else {digitalWrite2(BLDRIVER_ADIM_0, LOW);}
    if(ADIM_LEVEL&(0x01<<1)) {digitalWrite2(BLDRIVER_ADIM_1, HIGH);} else {digitalWrite2(BLDRIVER_ADIM_1, LOW);}
    if(ADIM_LEVEL&(0x01<<2)) {digitalWrite2(BLDRIVER_ADIM_2, HIGH);} else {digitalWrite2(BLDRIVER_ADIM_2, LOW);}
    if(ADIM_LEVEL&(0x01<<3)) {digitalWrite2(BLDRIVER_ADIM_3, HIGH);} else {digitalWrite2(BLDRIVER_ADIM_3, LOW);}
    if(ADIM_LEVEL&(0x01<<4)) {digitalWrite2(BLDRIVER_ADIM_4, HIGH);} else {digitalWrite2(BLDRIVER_ADIM_4, LOW);}
    if(ADIM_LEVEL&(0x01<<5)) {digitalWrite2(BLDRIVER_ADIM_5, HIGH);} else {digitalWrite2(BLDRIVER_ADIM_5, LOW);}
    if(ADIM_LEVEL&(0x01<<6)) {digitalWrite2(BLDRIVER_ADIM_6, HIGH);} else {digitalWrite2(BLDRIVER_ADIM_6, LOW);}
  #endif
}



















uint8_t StatusLEDState = 0;
void UpdateStatusLEDBuffer(){
  if(TargetPowerSave != TargetPowerSaveFULLY_ON ) {
    LED_STATE_INDICATOR = LED_STATE_INDICATOR_POWERSAVE ;
  }  else  {
    switch (OUTPUT_MODE) {
      case OUTPUT_MODE_STABLE : if (CALCULATED_PWM_STABLE < PWM_MAX) { LED_STATE_INDICATOR = LED_STATE_INDICATOR_PWM; } else { LED_STATE_INDICATOR = LED_STATE_INDICATOR_PWMFREE; } break;
      case OUTPUT_MODE_STROBE : LED_STATE_INDICATOR = LED_STATE_INDICATOR_STROBING; break;
      case OUTPUT_MODE_SCAN   : LED_STATE_INDICATOR = LED_STATE_INDICATOR_SCANNING; break;
      case OUTPUT_MODE_OFF    : LED_STATE_INDICATOR = LED_STATE_INDICATOR_NO_INPUT; break;
      default                 : LED_STATE_INDICATOR = LED_STATE_INDICATOR_ERROR ;      
    }
  }
  if( (TARGET_ADIM!=OUTPUT_ADIM) || (SoftStartupPowerLimitIsInEffect()==true)) {LED_STATE_INDICATOR = LED_STATE_INDICATOR_SOFTADJUST;}
  #ifdef LED_RGB
  value.r=LED_STATE_INDICATOR_COLORS[LED_STATE_INDICATOR].r;
  value.g=LED_STATE_INDICATOR_COLORS[LED_STATE_INDICATOR].g;
  value.b=LED_STATE_INDICATOR_COLORS[LED_STATE_INDICATOR].b;
  LED.set_crgb_at(0, value); // Set value at LED found at index 0  
  #endif  
  #if defined(LED_R) && defined(LED_G)
    #if ((SERIAL_INTERFACE==ENABLED) &&((LED_R == D0) || (LED_G == D0) || (LED_R == D1) || (LED_G == D1)) )
      return;
    #endif
    if (StatusLEDState == 0) {StatusLEDState = 1;} else {StatusLEDState = 0; }  
    switch (LED_STATE_INDICATOR) {
      case LED_STATE_INDICATOR_POWERSAVE   : if (StatusLEDState == 0) {digitalWrite(LED_R, LOW  ); digitalWrite(LED_G, LOW  );} else {digitalWrite(LED_R, LOW  ); digitalWrite(LED_G, HIGH );} break;  // Flashing green
      case LED_STATE_INDICATOR_NO_INPUT    : if (StatusLEDState == 0) {digitalWrite(LED_R, HIGH ); digitalWrite(LED_G, LOW  );} else {digitalWrite(LED_R, LOW  ); digitalWrite(LED_G, LOW  );} break;  // Flashing red
      case LED_STATE_INDICATOR_PWM         : if (StatusLEDState == 0) {digitalWrite(LED_R, LOW  ); digitalWrite(LED_G, HIGH );} else {digitalWrite(LED_R, LOW  ); digitalWrite(LED_G, HIGH );} break;  // Stready green
      case LED_STATE_INDICATOR_PWMFREE     : if (StatusLEDState == 0) {digitalWrite(LED_R, LOW  ); digitalWrite(LED_G, HIGH );} else {digitalWrite(LED_R, HIGH ); digitalWrite(LED_G, HIGH );} break;  // Green+yellow
      case LED_STATE_INDICATOR_STROBING    : if (StatusLEDState == 0) {digitalWrite(LED_R, HIGH ); digitalWrite(LED_G, LOW  );} else {digitalWrite(LED_R, HIGH ); digitalWrite(LED_G, LOW  );} break;  // Red
      case LED_STATE_INDICATOR_SCANNING    : if (StatusLEDState == 0) {digitalWrite(LED_R, HIGH ); digitalWrite(LED_G, LOW  );} else {digitalWrite(LED_R, HIGH ); digitalWrite(LED_G, HIGH );} break;  // Red+yellow
      case LED_STATE_INDICATOR_ERROR       : // fall through to the default case
      default                              : if (StatusLEDState == 0) {digitalWrite(LED_R, LOW  ); digitalWrite(LED_G, HIGH );} else {digitalWrite(LED_R, HIGH ); digitalWrite(LED_G, LOW  );} break;  // Flashing green+red
    }
  #endif  
  }

void IndicateNewState(){}//UpdateStatusLEDBuffer();}


uint8_t SkipSameLEDCount=0;
const uint8_t ResendLED = 250; // Reduce same-state retransmits by this ratio

void MaybeUpdateStatusLED(){
  UpdateStatusLEDBuffer();
  if((value.r==valuePrev.r) && (value.g==valuePrev.g) && (value.b==valuePrev.b) && (SkipSameLEDCount >0)) {
    SkipSameLEDCount=SkipSameLEDCount-1;  
  } else {
    valuePrev=value;
    SkipSameLEDCount=ResendLED;
    if(CONTROL_MODE == CONTROL_MODE_OFF) { UpdateStateLED(); return;}
    ShouldUpdateStatusLED=true;    
  }     
}



uint8_t IsSerialPortBusy(){
  return 0;
  return ((UCSR0A & (1<<TXC0))>0) ;  
}

void UpdateStateLED(){
  #ifdef LED_RGB
    if(IsSerialPortBusy()==0){      
     UCSR0B = UCSR0B & ~(1<<TXEN0); 
     delayMicroseconds(50); 
     LED.sync(); // Sends the data to the LEDs
     delayMicroseconds(50);
     UCSR0B |= (1<<TXEN0);
     ShouldUpdateStatusLED=false;
   }
   #endif
}


