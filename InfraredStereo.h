#ifndef INFRAREDSTEREO_h
#define INFRAREDSTEREO_h
#include "constants.h"
#include "arduino2.h"  // include the fast I/O 2 functions.  Info here: http://www.codeproject.com/Articles/732646/Fast-digital-I-O-for-Arduino

// This code is intended for use with an external infrared transmitter which takes a logic level input.  This can be as simple as a string of IR LEDs and an N-Mosfet as a low-side switch
// It is intended for use with Nvidia 3dvision shutterglasses, but can be adapted for others.  Shutterglass support is not yet well tested.  Use at your own risk.

#ifndef INPUT_ENABLE_OR_SCL_PIN
  #error "InfraredStereo class failed - no pin set"
#endif

// The "INPUT_ENABLE_OR_SCL_PIN" is present on all boards, but does not serve any real purpose.  Let's use it for infrared shutterglass sync instead.
#define INFRARED_STEREO_PIN INPUT_ENABLE_OR_SCL_PIN


class InfraredStereo
{
public:
  InfraredStereo() ;
  const uint8_t LEFT_EYE = 0 ;
  const uint8_t RIGHT_EYE = 1 ;
  uint8_t GetEye() ;
  uint8_t GetEnabled() ;
  void SetEnabled() ;
  void SetDisabled() ;
  void SwapEye();
  void SendInfraredSyncToGlasses_PulsedBacklight();
  void SendInfraredSyncToGlasses_ContinuiousBacklight();
private:
  volatile uint8_t current_eye;
  uint8_t infrared_stereo_is_enabled = true ;
  void CloseLeftEye();
  void CloseRightEye();
  void OpenLeftEye();
  void OpenRightEye();
};

#endif
