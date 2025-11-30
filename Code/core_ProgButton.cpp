//*******************************************************************************************
//
// file:      core_ProgButton.cpp
// author:    Jack Christensen / Modified by Aiko Pras
// history:   2021-06-27 V1.1   The code is originally written by Jack Christensen
//                              https://github.com/JChristensen/JC_Button
//                              Small modifications are made by Aiko Pras to make the class
//                              interface resemble the other AP_DCC and RSBus libraries
//            2025/12/01 V1.2   Changed from library to be used within the sketch.
//                              Filename changed
//
// purpose:   Reads the status of (debounced) buttons
//
// Copyright: Jack Christensen (2018) and licensed under
//            GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
//
// Updated: 30 december 2021
// The routine that reads the value of the button pin should be fast, certainly  if we call
// that routine at each cycle of the main loop.
// The traditional Arduino digitalRead() function is relatively slow, certainly when compared
// to direct port reading, which can (for example) be done using digitalReadfast() or directly:
// BitVal = !(PIND & (1<<PD3);
// However, direct port reading has as disadvantage that the port and mask should be hardcoded,
// and can not be set by the main sketch, thus the user of this library.
// Therefore we take a slightly slower approach, and use a variable called "portRegister",
// which points to the right input port, and "bit", which masks the selected input port.
// To use "portRegister" and "bit", we basically split the Arduino digitalRead() function into:
// 1) an initialisation part, which maps dccPin to "portInputRegister" and "bit".
//    This part may be slow
// 2) The actual reading from the port, which is fast.
// Comparison between approaches:               Flash  RAM  Time  Delta
//   value = (PINC & bit);                        6     1   1,09    -
//   value = (*portRegister & bit);              14     1   1,54   0,45
//   value = (*portRegister(port) & bit);        28     2   2,37 . 1,28
//   value = digitalRead(pin);                   88     0   3,84   2,75
//           Note: Flash & RAM in bytes / Time & Delta in microseconds

//*******************************************************************************************
#include "core_ProgButton.h"


//*******************************************************************************************
// attach and initialize a DccButton object and the pin it's connected to.
//*******************************************************************************************
void DccButton::attach(uint8_t pin, unsigned long dbTime, bool puEnable, bool invert) {
  m_pin = pin;
  m_dbTime = dbTime;
  m_puEnable = puEnable;
  m_invert = invert;
  // Note that the question mark `?` in `m_puEnable ? INPUT_PULLUP : INPUT`
  // is commonly referred to as the conditional operator. Its meaning is:
  // if m_puEnable is true, `INPUT_PULLUP` is used, otherwise `INPUT`.
  pinMode(m_pin, m_puEnable ? INPUT_PULLUP : INPUT);
  // Since the Arduino standard digitalRead is too slow, we use a something faster.
  // Map from the button pin to a pointer for the port, as well as a bitmask
  m_port = digitalPinToPort(m_pin);
  m_bit = digitalPinToBitMask(m_pin);
  m_portRegister = portInputRegister(m_port);
  m_state = (*m_portRegister & m_bit);
  // The old code was:
  // m_state = digitalRead(m_pin);
  if (m_invert) m_state = !m_state;
  m_time = millis();
  m_lastState = m_state;
  m_changed = false;
  m_lastChange = m_time;
}

//*******************************************************************************************
// returns the state of the button, true if pressed, false if released.
// does debouncing, captures and maintains times, previous state, etc.
//*******************************************************************************************
bool DccButton::read() {
  unsigned long ms = millis();
  bool pinVal = (*m_portRegister & m_bit);
  // bool pinVal = (PIND & (1<<PD3);      // Direct port access: fast but hardcoded
  // bool pinVal = digitalRead(m_pin);    // Standard Arduino, flexible but slow
  if (m_invert) pinVal = !pinVal;
  if (ms - m_lastChange < m_dbTime)  {
    m_changed = false;
  }
  else {
    m_lastState = m_state;
    m_state = pinVal;
    m_changed = (m_state != m_lastState);
    if (m_changed) m_lastChange = ms;
  }
  m_time = ms;
  return m_state;
}

//*******************************************************************************************
// isPressed() and isReleased() check the button state when it was last read, and return
// false (0) or true (!=0) accordingly.
// These functions do not cause the button to be read.
//*******************************************************************************************
bool DccButton::isPressed() {
  return m_state;
}

bool DccButton::isReleased() {
  return !m_state;
}

//*******************************************************************************************
// wasPressed() and wasReleased() check the button state to see if it changed between the
// last two reads and return false (0) or true (!=0) accordingly.
// These functions do not cause the button to be read.
//*******************************************************************************************
bool DccButton::wasPressed() {
  return m_state && m_changed;
}

bool DccButton::wasReleased() {
  return !m_state && m_changed;
}

//*******************************************************************************************
// pressedFor(ms) and releasedFor(ms) check to see if the button is pressed (or released),
// and has been in that state for the specified time in milliseconds.
// Returns false (0) or true (!=0) accordingly.
// These functions do not cause the button to be read.
//*******************************************************************************************
bool DccButton::pressedFor(unsigned long ms) {
  return m_state && m_time - m_lastChange >= ms;
}

bool DccButton::releasedFor(unsigned long ms) {
  return !m_state && m_time - m_lastChange >= ms;
}

//*******************************************************************************************
// lastChange() returns the time the button last changed state, in milliseconds.                                                     *
//*******************************************************************************************
unsigned long DccButton::lastChange() {
  return m_lastChange;
}
