//*******************************************************************************************
//
// file:      core_ProgButton.h
// author:    Jack Christensen / Modified by Aiko Pras
// history:   2021-06-27 V1.1   The code is originally written by Jack Christensen
//                              https://github.com/JChristensen/JC_Button
//                              Small modifications are made by Aiko Pras to make the class
//                              interface resemble the other AP_DCC and RSBus libraries
//            2021-12-31 V1.2   Reading from the button input pin has been made faster
//                              digitalRead() is replaced by a register pointer and mask
//            2025/12/01 V1.3   Changed from library to be used within the sketch
//                              Filename changed
//
// purpose:   Reads the status of (debounced) buttons
//
// Copyright: Jack Christensen (2018) and licensed under
//            GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
//*******************************************************************************************
#pragma once
#include <Arduino.h>

class DccButton {
  
public:
  // attach(pin, dbTime, puEnable, invert) initialises a button object.
  //
  // Required parameter:
  // pin      The Arduino pin the button is connected to
  //
  // Optional parameters:
  // dbTime   Debounce time in milliseconds (default 25ms)
  // puEnable true to enable the AVR internal pullup resistor (default true)
  // invert   true to interpret a low logic level as pressed (default true)
  
  // Initialize a Button object and the pin it's connected to
  void attach(uint8_t pin, unsigned long dbTime=25, bool puEnable=true, bool invert=true);
  
  // Returns the current debounced button state, true for pressed,
  // false for released. Call this function frequently to ensure
  // the sketch is responsive to user input.
  bool read();
  
  // Returns true if the button state was pressed at the last call to read().
  // Does not cause the button to be read.
  bool isPressed();
  
  // Returns true if the button state was released at the last call to read().
  // Does not cause the button to be read.
  bool isReleased();
  
  // Returns true if the button state at the last call to read() was pressed,
  // and this was a change since the previous read.
  bool wasPressed();
  
  // Returns true if the button state at the last call to read() was released,
  // and this was a change since the previous read.
  bool wasReleased();
  
  // Returns true if the button state at the last call to read() was pressed,
  // and has been in that state for at least the given number of milliseconds.
  bool pressedFor(unsigned long ms);
  
  // Returns true if the button state at the last call to read() was released,
  // and has been in that state for at least the given number of milliseconds.
  bool releasedFor(unsigned long ms);
  
  // Returns the time in milliseconds (from millis) that the button last
  // changed state.
  unsigned long lastChange();
  
private:
  uint8_t m_pin;                    // arduino pin number connected to button
  unsigned long m_dbTime;           // debounce time (ms)
  bool m_puEnable;                  // internal pullup resistor enabled
  bool m_invert;                    // if true, interpret logic low as pressed,
                                    // else interpret logic high as pressed
  bool m_state;                     // current button state, true=pressed
  bool m_lastState;                 // previous button state
  bool m_changed;                   // state changed since last read
  unsigned long m_time;             // time of current state (ms from millis)
  unsigned long m_lastChange;       // time of last state change (ms)
  
  // The following was added in december 2021. Instead of using the standard and
  // slow Arduino digitalRead() function, we use a pointer to the inpu port the
  // button is connected to, plus a bitmask.
  uint8_t m_port;                   // PA=1, PB=2, PC=3, PD=4 etc.
  uint8_t m_bit;                    // Bitmask for reading the input Port
  volatile uint8_t *m_portRegister; // Example: PINA=$39, PINB=$36, PINC=$34, PIND=$30
};


//*******************************************************************************************
// ToggleButton
// A derived class for a "push-on, push-off" (toggle) type button.
// initial state can be given, default is off (false).
//*******************************************************************************************
class ToggleButton : public DccButton {
  
public:
  
  // attach is similar to Button, but includes the initial state for the toggle.
  void attach(uint8_t pin, unsigned long dbTime=25, bool puEnable=true,
              bool invert=true, bool initialState=false) {
    DccButton::attach(pin, dbTime, puEnable, invert);
    m_toggleState = initialState;
  }
  
  
  // read the button and return its state.
  // should be called frequently.
  bool read() {
    DccButton::read();
    if (wasPressed()) {
      m_toggleState = !m_toggleState;
      m_changed = true;
    }
    else {
      m_changed = false;
    }
    return m_toggleState;
  }
  
  // has the state changed?
  bool changed() {return m_changed;}
  
  // return the current state
  bool toggleState() {return m_toggleState;}
  
private:
  bool m_toggleState;
  bool m_changed;
};
