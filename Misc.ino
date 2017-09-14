














// Replacement for 'delay' which uses the CLOCK_RATIO parameter for overclocked mode and will also touch the watchdog periodically
void zdelay(uint32_t delayvalue){  
const uint8_t Real_DELAY_TIME = 10;
const uint8_t SCALED_DELAY_TIME=(Real_DELAY_TIME*1.0)*CLOCK_RATIO;
uint32_t remdel=delayvalue*CLOCK_RATIO;
while(remdel>SCALED_DELAY_TIME) {
  wdt_reset();
  delay(SCALED_DELAY_TIME);
  remdel=remdel-SCALED_DELAY_TIME; 
}
  wdt_reset();
  delay(remdel);
  wdt_reset();
}




void softReset() {  
  SerialDebugln(F("\nRst!"));  SerialFlush();
  noInterrupts();
  PowerOFF();
//  power_down_board();
  zdelay(500); // Ensure that the board is powered down sufficiently long to cause a full reset.  
#if OVERCLOCKING==ENABLED
  my_Overclock_object.SetClockSpeedStock();
#endif
  Serial.end();  
wdt_enable(WDTO_15MS); 
 while(1){}
//  asm volatile ("  jmp 0"); // Note: the jmp 0 does not load the bootloader, so we might not get the startup state that we expect.  Using watchdog is cleaner.
}

