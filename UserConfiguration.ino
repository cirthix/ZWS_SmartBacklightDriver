
void UserConfiguration_SaveShutdown(uint8_t myValue){ EEPROM.put(ADDRESS_TARGET_POWERSAVE, myValue); }

void UserConfiguration_SaveStrobing(uint8_t myValue){ EEPROM.put(ADDRESS_SAVED_MODE, myValue); }

void UserConfiguration_SaveOSD(uint8_t myValue){ EEPROM.put(ADDRESS_SAVED_OSD, myValue); }

void UserConfiguration_SaveEDID(uint8_t myValue){ EEPROM.put(ADDRESS_TARGET_EDID, myValue); }

void UserConfiguration_SaveBrightnessStable(uint16_t myValue){ EEPROM.put(ADDRESS_SAVED_BRIGHTNESS_STABLE, myValue); }

void UserConfiguration_SaveBrightnessStrobe(uint16_t myValue){ EEPROM.put(ADDRESS_SAVED_BRIGHTNESS_STROBE, myValue); }

void UserConfiguration_SaveBrightnessScan(  uint16_t myValue){ EEPROM.put(ADDRESS_SAVED_BRIGHTNESS_SCAN  , myValue); }

void set_has_been_factory_programmed(){ EEPROM.put(ADDRESS_MAGIC_BYTE, MagicByte()); }

uint8_t get_has_been_factory_programmed(){ return (EEPROM.read(ADDRESS_MAGIC_BYTE) == (MagicByte())); }

void LoadParametersLED(){
EEPROM.get(ADDRESS_LED_STRUCTURE, MyConfigLED);
}

void LoadSavedParameters(){
EEPROM.get(ADDRESS_TARGET_POWERSAVE, TargetPowerSave);
EEPROM.get(ADDRESS_SAVED_MODE, TargetMode);
EEPROM.get(ADDRESS_SAVED_OSD, TargetOSD);
EEPROM.get(ADDRESS_TARGET_EDID, TargetEDID);
EEPROM.get(ADDRESS_SAVED_BRIGHTNESS_STABLE, TargetBrightnessStable);
EEPROM.get(ADDRESS_SAVED_BRIGHTNESS_STROBE, TargetBrightnessStrobe);
EEPROM.get(ADDRESS_SAVED_BRIGHTNESS_SCAN, TargetBrightnessScan);
}

void RunFactoryProgramming() {
  SerialDebugln(F("Fac"));  
  SerialFlush();
  // Do the factory programming here
  EEPROM.put(ADDRESS_TARGET_POWERSAVE, DefaultPowerSave);
  EEPROM.put(ADDRESS_SAVED_MODE, DefaultMode);
  EEPROM.put(ADDRESS_SAVED_OSD, DefaultOSD);
  EEPROM.put(ADDRESS_TARGET_EDID, DefaultEDID);
  EEPROM.put(ADDRESS_SAVED_BRIGHTNESS_STABLE, DefaultBrightnessStable);
  EEPROM.put(ADDRESS_SAVED_BRIGHTNESS_STROBE, DefaultBrightnessStrobe);
  EEPROM.put(ADDRESS_SAVED_BRIGHTNESS_SCAN, DefaultBrightnessScan);
  EEPROM.put(ADDRESS_LED_STRUCTURE, TargetConfigLED);
  set_has_been_factory_programmed();
  softReset();
}

// This function is entirely compiled away into a single byte value
uint8_t MagicByte(){
const uint8_t array_size=24;
const uint8_t compile_date_time[array_size] = "" __DATE__ " " __TIME__ ""; // Example string: "Apr  3 2016 01:44:37"
uint8_t hashed_value= 
compile_date_time[0]+
compile_date_time[1]+
compile_date_time[2]+
compile_date_time[3]+
compile_date_time[4]+
compile_date_time[5]+
compile_date_time[6]+
compile_date_time[7]+
compile_date_time[8]+
compile_date_time[9]+
compile_date_time[10]+
compile_date_time[11]+
compile_date_time[12]+
compile_date_time[13]+
compile_date_time[14]+
compile_date_time[15]+
compile_date_time[16]+
compile_date_time[17]+
compile_date_time[18]+
compile_date_time[19];
  
  if(hashed_value== 0xFF || hashed_value==0x00) {return 0xa9;}
return hashed_value;
}
