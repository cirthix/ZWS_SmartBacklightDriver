#include "InfraredStereo.h"


InfraredStereo::InfraredStereo(){
  pinMode(INFRARED_STEREO_PIN, OUTPUT);
  current_eye = LEFT_EYE ;
  InfraredStereo::SetEnabled();
}


uint8_t InfraredStereo::GetEye() { 
  return current_eye ;   
}


void  InfraredStereo::SwapEye() {
  if(current_eye == LEFT_EYE) {
    current_eye = RIGHT_EYE ;
  } else {
    current_eye = LEFT_EYE ;
  }
}


void  InfraredStereo::SetEnabled() {
  infrared_stereo_is_enabled = true;
}


void  InfraredStereo::SetDisabled() {
  infrared_stereo_is_enabled = false;
}


uint8_t InfraredStereo::GetEnabled() { 
  return infrared_stereo_is_enabled ;   
}



void InfraredStereo::SendInfraredSyncToGlasses_ContinuiousBacklight(){
  if(InfraredStereo::GetEnabled() == false) { return ; }
  if(current_eye == LEFT_EYE) {
    InfraredStereo::CloseLeftEye();
    delayMicroseconds(557);
    InfraredStereo::OpenRightEye();
    current_eye = RIGHT_EYE ;
  } else {
    InfraredStereo::CloseRightEye();
    delayMicroseconds(500);
    InfraredStereo::OpenLeftEye();
    current_eye = LEFT_EYE ;
  }
}


void InfraredStereo::SendInfraredSyncToGlasses_PulsedBacklight(){
  if(InfraredStereo::GetEnabled() == false) { return ; }
  if(current_eye == LEFT_EYE) {
    InfraredStereo::CloseLeftEye();
    delayMicroseconds(157);
    InfraredStereo::OpenRightEye();
    current_eye = RIGHT_EYE ;
  } else {
    InfraredStereo::CloseRightEye();
    delayMicroseconds(100);
    InfraredStereo::OpenLeftEye();
    current_eye = LEFT_EYE ;
  }
}


void InfraredStereo::CloseLeftEye(){
  digitalWrite2(INFRARED_STEREO_PIN, HIGH); 
  delayMicroseconds(43);
  digitalWrite2(INFRARED_STEREO_PIN, LOW);  
}


void InfraredStereo::OpenRightEye(){
  digitalWrite2(INFRARED_STEREO_PIN, HIGH); 
  delayMicroseconds(23);
  digitalWrite2(INFRARED_STEREO_PIN, LOW);
  delayMicroseconds(78); 
  digitalWrite2(INFRARED_STEREO_PIN, HIGH); 
  delayMicroseconds(40);
  digitalWrite2(INFRARED_STEREO_PIN, LOW);    
}


void InfraredStereo::CloseRightEye(){
// 23.25us high, 78.25us low, 39.75us high
  digitalWrite2(INFRARED_STEREO_PIN, HIGH); 
  delayMicroseconds(23);
  digitalWrite2(INFRARED_STEREO_PIN, LOW);
  delayMicroseconds(46); 
  digitalWrite2(INFRARED_STEREO_PIN, HIGH); 
  delayMicroseconds(31);
  digitalWrite2(INFRARED_STEREO_PIN, LOW);  
}  


void InfraredStereo::OpenLeftEye(){
  digitalWrite2(INFRARED_STEREO_PIN, HIGH); 
  delayMicroseconds(23);
  digitalWrite2(INFRARED_STEREO_PIN, LOW);
  delayMicroseconds(21); 
  digitalWrite2(INFRARED_STEREO_PIN, HIGH); 
  delayMicroseconds(23);
  digitalWrite2(INFRARED_STEREO_PIN, LOW);  
}

