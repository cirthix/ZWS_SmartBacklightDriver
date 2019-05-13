const uint16_t ScanTestOntime=1000;  // milliseconds
const uint16_t ScanTestOfftime=1000; // milliseconds

void EnterScanningTest(){
  if( MyConfigLED.SupportsScanning != true ) { return ;}
//  Serial.println(F("Scanning test - check order of illumination"));
//  Serial.print(F("StringMapping: "));
//  for(uint8_t LEDStringID=0; LEDStringID<TargetConfigLED.NumberStrings; LEDStringID++){ Serial.print(LEDStringID); Serial.print(F("->")); Serial.print(GetAbstractedPin(TargetConfigLED.StringOrdering[0])); Serial.print(F("      "));}
//  Serial.println("");  
    adimWrite(CALCULATED_ADIM_STABLE);  // enable this by setting to CALCULATED_ADIM_SCAN after the timing is verified good
    OUTPUT_MODE = OUTPUT_MODE_OFF;
    IndicateNewState(); 
    ConfigureTimersOFF();
    RunScanningTest();
    EnterStable();
    delay(1000);
}
  
void RunScanningTest(){    
  for(uint8_t LEDStringID=0; LEDStringID<TargetConfigLED.NumberStrings; LEDStringID++){
    if(GetAbstractedPin(TargetConfigLED.StringOrdering[LEDStringID]) != MY_INVALID_PIN) { pinMode(GetAbstractedPin(TargetConfigLED.StringOrdering[LEDStringID]), OUTPUT); digitalWrite(GetAbstractedPin(TargetConfigLED.StringOrdering[LEDStringID]), LOW );}
  }    
    delay(ScanTestOfftime);    
  for(uint8_t LEDStringID=0; LEDStringID<TargetConfigLED.NumberStrings; LEDStringID++){
    if(GetAbstractedPin(TargetConfigLED.StringOrdering[LEDStringID]) != MY_INVALID_PIN) { digitalWrite2(GetAbstractedPin(TargetConfigLED.StringOrdering[LEDStringID]), HIGH); delay(ScanTestOntime); digitalWrite2(GetAbstractedPin(TargetConfigLED.StringOrdering[LEDStringID]), LOW ); delay(ScanTestOfftime);}
  }
}

