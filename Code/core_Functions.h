//*****************************************************************************************************
//
// File:      core_Functions.h
// Author:    Aiko Pras
// History:   2021/06/14 AP V1.0
//            2021/12/30 AP V1.2
//            2022/08/02 AP V1.3 Restructure of the library
//            2025/12/01 AP V1.4 Allow usage without the RS-Bus library
//            2025/12/01 AP V1.5 changed from library to "local" code. Filename changed
//               Anything related to RS-Bus / feedback messages removed, as well as GBM specific code 
//
// Purpose:   Header file for the core function for the (TMC switch) DCC accessory decoder.
//
// This code takes care of many of the common accesory decoder functions, such as:
// - accessing the CV values,
// - instantiating the onboard LED and Button,
// - filling the EEPROM with initial values,
// - resetting the EEPROM with initial values,
// - rebooting the decoder,
// - reacting on general CV operations (such as reset decoder, find decoder, reboot decoder),
// - initialisation and the initial assignment of the decoder address.
//
// Main => core_Functions +-> AP_DCC_Lib          - Needed to detach the ISR
//                        +-> core_CvValues       - To access the CV EEPROM values
//                        +-> core_LEDs           - the LED object is instatiated here
//                        +-> core_ProgButton     - the Button object is instatiated here
//                        +-> core_Timer          - the timer is used for the programming button / LED
//
//*****************************************************************************************************
#pragma once

#include <Arduino.h>                  // For general definitions
#include <AP_DCC_library.h>           // Interface to DCC input and DCC Ack pin
#include "hardware.h"                 // Pins and USART being used for this board
#include "core_CvValues.h"            // To define and access cvValue
#include "core_LEDs.h"                // For the programming LED
#include "core_ProgButton.h"          // For the onboard Button
#include "core_Timer.h"               // Allows timers to be used


//*****************************************************************************************************
class Processor {
  public:
    void reboot(void);                            // Restarts the decoder, using the latest EEPROM CV values
};


class CvProgramming {
  public:
    void initPoM(void);                           // Set the Loco address for PoM messages and the RS-Pom address
    void processMessage(Dcc::CmdType_t cmdType);  // Called if we have a PoM or SM message

  private:
    bool LedShouldFlash;                          // Local copy of CV23 (search)
};


class CommonDecHwFunctions {
  public:
    void init(void);                              // Should be called from init() in the main sketch.
    void update(void);                            // Should be called from main as often as possible.
    unsigned long TLast;                          // Some elements of update() are called every 20ms
};


//*****************************************************************************************************
// Objects defined the DCC_Library and in the files beginning with core_
//*****************************************************************************************************
// A number of objects are instantiated in the DCC library, the file "core_Functions.cpp" and the file
// "core_CvValues.cpp". These objects must be made known to the compiler and linker by "repeating" 
// their instantiation again, preceeded by the extern keyword.
// Therefore the main sketch does not need to repeat these object declarations again.

// The following objects are used for decoding received DCC messages. The main object is dcc,
// which manages the layer 1 (hardware) specific parts of the DCC interface. The other objects make
// available to the main sketch the contents of respectively accessory and loco (multi-function) commands.
extern Dcc dcc;                              // Instantiated in AP_DCC_library.cpp
extern Accessory accCmd;                     // Instantiated in AP_DCC_library.cpp
extern Loco locoCmd;                         // Instantiated in AP_DCC_library.cpp

// The following objects are used for storing and retrieving CV values from EEPROM,
// and respond to DCC CV-access commands
extern CvAccess cvCmd;                       // Instantiated in AP_DCC_library.cpp
extern CvValues cvValues;                    // Instantiated in core_CvValues.cpp
extern CvProgramming cvProgramming;          // Instantiated in core_Functions.cpp

// The following objects provide access to the onboard LED, as well as the
// onboard programming button
extern DccLed programmingLed;                // Instantiated in core_Functions.cpp
extern CommonDecHwFunctions decoderHardware; // Instantiated in core_Functions.cpp
