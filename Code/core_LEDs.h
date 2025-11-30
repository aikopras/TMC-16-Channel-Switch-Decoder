//******************************************************************************************************
//
// file:      core_LEDs.h
// author:    Aiko Pras
// history:   2019-02-24 V1.0.2 ap initial version, changed into Arduino library
//            2021-06-26 V1.2   ap extended with fade out
//            2022-07-20 V1.3   ap split into multiple objects, to save RAM if methods are not needed
//            2022-08-02 V1.4   ap const static uint8_t replaced by #defines
//            2025/12/01 V1.4   ap changed from library to be used within the sketch. Filename changed
//
// purpose:   LED object. LED can be switched on, switched off, put in flashing mode or fade out.
//            Next to these basic modes, additional functions are defined for some common tasks,
//            such as the decoder (re)starts, performs some activity or sends feedback
//
// Kind of LED objects:
// - BasicLed:    these are simple on/off LEDs. No update() is needed for such LEDs
// - FlashLed:    extends BasicLed with flashing
// - DccLed:      extends FlashLed with DCC decoder specific functions (start_up, activity, feedback)
// - FadeOutLed:  extends BasicLed with fadeOut (not recommeded: is expensive regarding RAM and CPU)
//
// RAM required per object:
// - BasicLed:     2
// - FlashLed:    11
// - DccLed:      11
// - FadeOutLed:  23
//
//******************************************************************************************************
#pragma once


//******************************************************************************************************
// BasicLed
//******************************************************************************************************
class BasicLed {
public:
  void attach (uint8_t pin, bool invert=false);     // logic may be inverted: turn_on() => LOW
  bool ledIsOn(void);                               // true if the LED is on
  void turn_on(void);
  void turn_off(void);
  void toggle(void);

protected:
  uint8_t _pin;                                    // Hardware pin to which the LED is connected
  bool _LED_ON;                                    // Default for _LED_ON is HIGH, but may be inverted
};


//******************************************************************************************************
// FlashLed
//******************************************************************************************************
// If the LED is put in flashing mode, the following attributes  can be set:
// - mode:              if the flash led should (continously) flash, or be (permanently) on / off
// - flashOntime:       (0..255) time the LED is on (in 100 msec steps)
// - flashOfftime:      (0..255) time between to flashes
// - flashCount:        (0..255) the total number of flashes (within a single serie)
// - flashPause:        (0..255) time between two series of flashes (in case of continuous flashing)
// All times are in steps of 100 msec.
//                                                 flashCount
//                 flashOntime
//     ----+          +----+         +----+         +----+                                    +----+
//         |          |    |         |    |         |    |                                    |    |
//         |          |    |         |    |         |    |                                    |    |
//         +----------+    +---------+    +---------+    +------------------------------------+    +----
//                        flashOfftime                                      flashPause
//
//                                                                               mode: neverStopFlashing
//******************************************************************************************************
class FlashLed: public BasicLed {
public:
  // Attributes to set the LED's flashing behaviour
  uint8_t flashOntime;            // For flashing: time the LED should be ON (in 100ms steps)
  uint8_t flashOfftime;           // For flashing: time the LED should be OFF (in 100ms steps)
  uint8_t flashPause;             // For flashing: LED off time between a series of flashes
  uint8_t flashCount;             // Number of flashes before a pause
  // Constants and Flash mode attribute
  #define alwaysOn            1
  #define alwaysOff           2
  #define singleFlashSerie    3
  #define neverStopFlashing   4
  uint8_t mode;

  // The Basic::attach / on / off will be extended
  void attach (uint8_t pin, bool invert=false);
  void turn_off(void);
  void turn_on(void);

  void flash(void);               // Add flash to the basic functions
  void flashSlow(void);           // Continuous series of slow flashes
  void flashFast(void);           // Continuous series of fast flashes
  void update(void);              // Should be called from main as often as possible

protected:
  unsigned long last_flash_time;  // time in msec since we last updated the LEDs
  uint8_t flash_number_now;       // Number of flashes thusfar
  uint8_t flash_time_remain;      // Remaining time before LED status changes. In Ticks (100ms)
};


//******************************************************************************************************
// DccLed
//******************************************************************************************************
// The DccLed class extends the FlashLed class with methods to indicate specific decoder states
class DccLed: public FlashLed {
  public:
    void start_up(void);            // Decoder (re)started
    void activity(void);            // Single very short flash, to indicate a switch command
    void feedback(void);            // A (RS-Bus) feedback message is send
};


//******************************************************************************************************
// FadeOutLed
//******************************************************************************************************
//
//            +--------+
//                     +---------+
//     ^                         +---------+
//     |                                   +---------+
//     |                                             +---------+
// fadeSteps                                                     fadeTime (100ms) --->
//                               [         ]
//                              fadeStepTime
//                               (microsec)
//
//  fadeStepTime =  100000 / fadeSteps * fadeTime
//  Note1: fadeStepTime is uint16_t, so maximum is 65535. To avoid overflow, division should come first
//  Note2: We divide 100000 (instead of 1000000), since fadeTime is in 100ms (and not seconds)
//
//******************************************************************************************************
//
//                  pwmOnTime              pwmOffTime                    pwmInterval     (microsec)
//                    <--->               [           ]               [               ]
//    +---+           +---+           +---+           +---+           +---+           +
//        |           |   |           |   |           |   |           |   |           |
//        |           |   |           |   |           |   |           |   |           |
//        +-----------+   +-----------+   +-----------+   +-----------+   +-----------+
//    <------------------------------------------------------------------------------->
//    0                                                                               1 sec
//                                  pwmFrequency (Hz)
//
//  pwmInterval = 1000000 / pwmFrequency
//  pwmOnTime = pwmInterval * brightnessLevel / 100.  brightnessLevel is between 0..100)
//  pwmOffTime = pwmInterval - pwmOnTime
//
//******************************************************************************************************
class FadeOutLed: public BasicLed {
public:
  
  // Attributes to set the LED's fading behaviour
  uint8_t fadeTime;             // In 100ms steps
  uint8_t fadeSteps;            // Number of steps between LED is 100% and 0%
  uint8_t pwmFrequency;         // PWM frequency in Herz (preferably 50 or higher)
  
  // The Basic::attach will be extended with some initialisation functions
  // Note that only fadeOut is implemented; fadeIn has not (yet??) been implemented
  void attach (uint8_t pin, bool invert=false);
  void fadeOut(void);
  void update(void);              // Schould be called from main as often as possible

private:
    // While fading, the following variables are used to calculate PWM and fade times
    uint16_t fadeStepTime;          // in microseconds, see figures above
    uint16_t pwmInterval;           // in microseconds, see figures above
    uint16_t pwmOnTime;             // in microseconds, see figures above
    uint16_t pwmOffTime;            // in microseconds, see figures above
    unsigned long last_fade_time;   // time (ms) since we last updated the fade settings
    unsigned long last_pwm_time;    // time (ms) since we last updated the PWM settings
    bool fadeLedIsOn;               // for performance reasons we don't use BasicLed::ledIsOn()
    uint8_t brightnessLevel;        // Current LED level
};

