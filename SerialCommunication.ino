

struct ParsedSerialCommand {
bool Valid;
bool OSD;
uint8_t EDID;
uint8_t PowerSave;
};


uint8_t SerialCommandGenerate(){
  uint8_t mySerialCommand=0x00;
  mySerialCommand|=(1<<7);
  if (TargetOSD == true) {   mySerialCommand|=(1<<6);  }
  uint8_t myFourBits = 0x00;
  if((TargetEDID == 0) && (TargetPowerSave==TargetPowerSaveSHUTDOWN) ) { myFourBits=1; }
  if((TargetEDID == 0) && (TargetPowerSave==TargetPowerSaveLOWPOWER) ) { myFourBits=2; }
  if((TargetEDID == 0) && (TargetPowerSave==TargetPowerSaveFULLY_ON) ) { myFourBits=3; }
  if((TargetEDID == 1) && (TargetPowerSave==TargetPowerSaveSHUTDOWN) ) { myFourBits=4; }
  if((TargetEDID == 1) && (TargetPowerSave==TargetPowerSaveLOWPOWER) ) { myFourBits=5; }
  if((TargetEDID == 1) && (TargetPowerSave==TargetPowerSaveFULLY_ON) ) { myFourBits=6; }
  if((TargetEDID == 2) && (TargetPowerSave==TargetPowerSaveSHUTDOWN) ) { myFourBits=7; }
  if((TargetEDID == 2) && (TargetPowerSave==TargetPowerSaveLOWPOWER) ) { myFourBits=8; }
  if((TargetEDID == 2) && (TargetPowerSave==TargetPowerSaveFULLY_ON) ) { myFourBits=9; }
  if((TargetEDID == 3) && (TargetPowerSave==TargetPowerSaveSHUTDOWN) ) { myFourBits=10; }
  if((TargetEDID == 3) && (TargetPowerSave==TargetPowerSaveLOWPOWER) ) { myFourBits=11; }
  if((TargetEDID == 3) && (TargetPowerSave==TargetPowerSaveFULLY_ON) ) { myFourBits=12; }
  if((TargetEDID == 4) && (TargetPowerSave==TargetPowerSaveSHUTDOWN) ) { myFourBits=13; }
  if((TargetEDID == 4) && (TargetPowerSave==TargetPowerSaveLOWPOWER) ) { myFourBits=14; }
  if((TargetEDID == 4) && (TargetPowerSave==TargetPowerSaveFULLY_ON) ) { myFourBits=15; }
  mySerialCommand|=(myFourBits<<2);

  mySerialCommand|=SerialCommandCalculateChecksum(mySerialCommand);  

  return mySerialCommand;
}

uint8_t SerialCommandExtractFixedBit(uint8_t myCommand){ return ((myCommand & (1<<7))>>7);}
uint8_t SerialCommandExtractOSD(uint8_t myCommand){ return ((myCommand & (1<<6))>>6);}
uint8_t SerialCommandExtractFourBits(uint8_t myCommand){ return (0x0f&(myCommand>>2));}
uint8_t SerialCommandExtractChecksum(uint8_t myCommand){ return (0x03&myCommand);}
uint8_t SerialCommandCalculateChecksum(uint8_t myCommand){
  // The last two bits are the last two bits of the number of ones in the other five bits.
  uint8_t numberOnes=0;
  if( (myCommand & (1<<7))>0) {   numberOnes++;  }
  if( (myCommand & (1<<6))>0) {   numberOnes++;  }
  if( (myCommand & (1<<5))>0) {   numberOnes++;  }
  if( (myCommand & (1<<4))>0) {   numberOnes++;  }
  if( (myCommand & (1<<3))>0) {   numberOnes++;  }
  if( (myCommand & (1<<2))>0) {   numberOnes++;  }  
  return (numberOnes&0x03);  
}




struct ParsedSerialCommand SerialCommandParser(uint8_t myCommand){
  struct ParsedSerialCommand myParsedSerialCommand;
  if((SerialCommandExtractFixedBit(myCommand)!=0) && (SerialCommandExtractChecksum(myCommand) == SerialCommandCalculateChecksum(myCommand))) {myParsedSerialCommand.Valid=true; } else {myParsedSerialCommand.Valid=false; } 
  if(SerialCommandExtractOSD(myCommand)) {myParsedSerialCommand.OSD=true; } else {myParsedSerialCommand.OSD=false; } 
  switch (SerialCommandExtractFourBits(myCommand)) {
    case 1  : myParsedSerialCommand.EDID = 0 ; myParsedSerialCommand.PowerSave = TargetPowerSaveSHUTDOWN ; break;
    case 2  : myParsedSerialCommand.EDID = 0 ; myParsedSerialCommand.PowerSave = TargetPowerSaveLOWPOWER ; break;
    case 3  : myParsedSerialCommand.EDID = 0 ; myParsedSerialCommand.PowerSave = TargetPowerSaveFULLY_ON ; break;
    case 4  : myParsedSerialCommand.EDID = 1 ; myParsedSerialCommand.PowerSave = TargetPowerSaveSHUTDOWN ; break;
    case 5  : myParsedSerialCommand.EDID = 1 ; myParsedSerialCommand.PowerSave = TargetPowerSaveLOWPOWER ; break;
    case 6  : myParsedSerialCommand.EDID = 1 ; myParsedSerialCommand.PowerSave = TargetPowerSaveFULLY_ON ; break;
    case 7  : myParsedSerialCommand.EDID = 2 ; myParsedSerialCommand.PowerSave = TargetPowerSaveSHUTDOWN ; break;
    case 8  : myParsedSerialCommand.EDID = 2 ; myParsedSerialCommand.PowerSave = TargetPowerSaveLOWPOWER ; break;
    case 9  : myParsedSerialCommand.EDID = 2 ; myParsedSerialCommand.PowerSave = TargetPowerSaveFULLY_ON ; break;
    case 10 : myParsedSerialCommand.EDID = 3 ; myParsedSerialCommand.PowerSave = TargetPowerSaveSHUTDOWN ; break;
    case 11 : myParsedSerialCommand.EDID = 3 ; myParsedSerialCommand.PowerSave = TargetPowerSaveLOWPOWER ; break;
    case 12 : myParsedSerialCommand.EDID = 3 ; myParsedSerialCommand.PowerSave = TargetPowerSaveFULLY_ON ; break;
    case 13 : myParsedSerialCommand.EDID = 4 ; myParsedSerialCommand.PowerSave = TargetPowerSaveSHUTDOWN ; break;
    case 14 : myParsedSerialCommand.EDID = 4 ; myParsedSerialCommand.PowerSave = TargetPowerSaveLOWPOWER ; break;
    case 15 : myParsedSerialCommand.EDID = 4 ; myParsedSerialCommand.PowerSave = TargetPowerSaveFULLY_ON ; break;
    default:  myParsedSerialCommand.EDID = 0 ; myParsedSerialCommand.PowerSave = TargetPowerSaveFULLY_ON ; // Maybe set a special flag here?
  }
  return myParsedSerialCommand;
}



void PrintParsedSerialCommand(uint8_t myCommand){
  struct ParsedSerialCommand myParsedSerialCommand=SerialCommandParser(myCommand);
  Serial.print(F("ZWS : "));  Serial.print(myCommand);   Serial.print(F("\t"));
  uint8_t ChecksumExtracted = SerialCommandExtractChecksum(myCommand);
  uint8_t ChecsumCalculated = SerialCommandCalculateChecksum(myCommand);  
  Serial.print(F("BIT :")); Serial.print(SerialCommandExtractFixedBit(myCommand));     Serial.print(F("\t"));
  Serial.print(F("SUM : ")); Serial.print(ChecksumExtracted); Serial.print(F(" / ")); Serial.print(ChecsumCalculated);     Serial.print(F("\t"));
  if(myParsedSerialCommand.Valid==true ) {Serial.print(F("VALID"));} else {  Serial.print(F("INVALID"));}     Serial.print(F("\t"));
  if(myParsedSerialCommand.OSD==true ) {Serial.print(F("OK OSD"));} else {  Serial.print(F("NO OSD"));}     Serial.print(F("\t")); 
  Serial.print(F("4BITs :")); Serial.print(SerialCommandExtractFourBits(myCommand));     Serial.print(F("\t"));
  Serial.print(F("EDID :")); Serial.print(myParsedSerialCommand.EDID);     Serial.print(F("\t"));
  Serial.print(F("PowerSave :")); Serial.print(myParsedSerialCommand.PowerSave);     Serial.println(F(""));  
}

