const uint16_t ScanTestOntime=1000;  // milliseconds
const uint16_t ScanTestOfftime=1000; // microseconds

void EnterScanningTest(){
  if( MyConfigLED.SupportsScanning != true ) { return ;}
  SerialDebugln(F("Scanning test - check order of illumination"));
    adimWrite(CALCULATED_ADIM_STABLE);  // enable this by setting to CALCULATED_ADIM_SCAN after the timing is verified good
    OUTPUT_MODE = OUTPUT_MODE_OFF;
    IndicateNewState(); 
    ConfigureTimersOFF();
    RunScanningTest();
    EnterStable();
    delay(1000);
}
  

void RunScanningTest(){
    #ifdef BLDRIVER_PWM_1
       pinMode(BLDRIVER_PWM_1, OUTPUT);
       digitalWrite(BLDRIVER_PWM_1, LOW );
    #endif
    #ifdef BLDRIVER_PWM_2
       pinMode(BLDRIVER_PWM_2, OUTPUT);
       digitalWrite(BLDRIVER_PWM_2, LOW );
    #endif
    #ifdef BLDRIVER_PWM_3
       pinMode(BLDRIVER_PWM_3, OUTPUT);
       digitalWrite(BLDRIVER_PWM_3, LOW );
    #endif
    #ifdef BLDRIVER_PWM_4
       pinMode(BLDRIVER_PWM_4, OUTPUT);
       digitalWrite(BLDRIVER_PWM_4, LOW );
    #endif
    #ifdef BLDRIVER_PWM_5
       pinMode(BLDRIVER_PWM_5, OUTPUT);
       digitalWrite(BLDRIVER_PWM_5, LOW );
    #endif
    #ifdef BLDRIVER_PWM_6
       pinMode(BLDRIVER_PWM_6, OUTPUT);
       digitalWrite(BLDRIVER_PWM_6, LOW );
    #endif
       delay(ScanTestOfftime);
    #ifdef BLDRIVER_PWM_1
       digitalWrite2(BLDRIVER_PWM_1, HIGH);
       delay(ScanTestOntime);
       digitalWrite2(BLDRIVER_PWM_1, LOW );
       delay(ScanTestOfftime);
    #endif
    #ifdef BLDRIVER_PWM_2
       digitalWrite2(BLDRIVER_PWM_2, HIGH);
       delay(ScanTestOntime);
       digitalWrite2(BLDRIVER_PWM_2, LOW );
       delay(ScanTestOfftime);
    #endif
    #ifdef BLDRIVER_PWM_3
       digitalWrite2(BLDRIVER_PWM_3, HIGH);
       delay(ScanTestOntime);
       digitalWrite2(BLDRIVER_PWM_3, LOW );
       delay(ScanTestOfftime);
    #endif
    #ifdef BLDRIVER_PWM_4
       digitalWrite2(BLDRIVER_PWM_4, HIGH);
       delay(ScanTestOntime);
       digitalWrite2(BLDRIVER_PWM_4, LOW );
       delay(ScanTestOfftime);
    #endif
    #ifdef BLDRIVER_PWM_5
       digitalWrite2(BLDRIVER_PWM_5, HIGH);
       delay(ScanTestOntime);
       digitalWrite2(BLDRIVER_PWM_5, LOW );
       delay(ScanTestOfftime);
    #endif
    #ifdef BLDRIVER_PWM_6
       digitalWrite2(BLDRIVER_PWM_6, HIGH);
       delay(ScanTestOntime);
       digitalWrite2(BLDRIVER_PWM_6, LOW );
       delay(ScanTestOfftime);
    #endif
}
