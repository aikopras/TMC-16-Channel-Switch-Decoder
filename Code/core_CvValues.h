//*****************************************************************************************************
//
// File:      core_CvValues.h
// Author:    Aiko Pras
// History:   2021/06/14 AP V1.0
//            2025/12/01 AP V2.0 Made specific for the TMC 16 Channel switch decoder
//
// Purpose:   Header file that defines the methods to read and modify CV values stored in EEPROM,
//            as well as the default values for all
//
// Origin: This code was taken from the AP_DCC_Decoder_Core library and rewritten to be used as
// part of the sketch for the TMC 16 Channel switch decoder. Everything related to other type of 
// decoders was removed, as well as RS-Bus specifics.
//
//******************************************************************************************************
#pragma once
#include <Arduino.h>


//*****************************************************************************************************
// CV values are stored in EEPROM and can be accessed via methods like read(), write(), and verify().
// CV values can be modified via PoM or SM messages. A restart is generally needed to take these
// new values into effect.
//
// EEPROM default values
// The CV default values are written to EEPROM by the setDefaults() method.
// setDefaults() simply copies to EEPROM the contents of the `defaults` array, which holds
// all default values for all CVs. These values may vary for different types of decoders.
// The `defaults` array will initially be filled by the class constructor, but after instatiation
// the main sketch may override values whenever needed.
//
// Depending on the further decoder software, there are three ways to call setDefaults():
// 1) Long (>5 sec.) press of the programming button on the board.
// 2) Sending a PoM or SM message to CV 8 (the  Vendor ID) with the value 0x0D.
// 3) In case the EEPROM has not yet been initialised, the setup() method in main will call setDefaults().
//
// CommonDecHwFunctions() should call notInitialised() to check if the EEPROM is already filled.
// notInitialised() reads the first EEPROM element (EEPROM.read(0)), which is for uninitialised
// EEPROMs generally 00 or FF.
// If the EEPROM is uninitialised, begin() writes the value 0b01010101 to the first EEPROM element
// and copies immediately after the contents of the `defaults` array.
// Note that the first EEPROM element will not be used by any CV, since the first CV has
// number 1, and (for simplicity) is stored at EEPROM location 1 (EEPROM.write(1, "value CV1")).
//
// Depending on the board that is being used and the specific Arduino IDE settings, EEPROM values
// may not be erased when a new sketch is being uploaded. For example, the standard Arduino AVR Uno
// board does not allow the EEPROM to be erased during sketch upload. The MiniCore board for the
// ATmega328, however, like all other boards from MCUdude (MegaCore, MightyCore, MiniCore...) has an
// option (`EEPROM not retained`) to erase all EEPROM contents (see for details MCUdude's github pages).
//
// The `defaults` array
// As opposed to the original OpenDecoder software, the default CV values will not be stored in
// PROGMEM but in RAM, since RAM sizes have increased over the years and storing them into RAM allows
// the main sketch to modify some of these values. In this way it is possible to have most
// parts of the CV handling in a library, but still allow the main sketch to make modifications to
// default values in case that is considered necessary.
//
// A description of CV values can be found in RCN-225.
// For CV1-CV30 we follow that description, but with a number of exceptions:
// - CV2: not implemented here
// - CV17-18: not implemented here
// - CV19-27: These CVs are reserved by the NMRA, but some are used here for different purposes.
// - CV30:    This CV is reserved by the NMRA, but is used here for the second vendor ID.
// - CV31-32: Should be "pointers to extended area", but not implemented here.
//
//*****************************************************************************************************
// Predefined values for the Decoder Type CV (CV27).
const uint8_t SwitchDecoder                     = 0b00010000;   // Switch decoder
const uint8_t SwitchDecoderWithEmergency        = 0b00010001;   // Switch decoder with Emergency board
const uint8_t ServoDecoder                      = 0b00010100;   // Decoder for 2 Servo's
const uint8_t Servo3Decoder                     = 0b00010101;   // Decoder for 3 Servo's
const uint8_t LiftDecoder                       = 0b00011000;   // Lift Decoder
const uint8_t Relays4Decoder                    = 0b00100000;   // Relays decoder for 4 relays
const uint8_t Relays16Decoder                   = 0b00100001;   // Relays decoder for 16 relays
const uint8_t TrackOccupancyDecoder             = 0b00110000;   // Track Occupancy decoder
const uint8_t TrackOccupancyDecoderWithReverser = 0b00110001;   // Track Occupancy decoder with reverser
const uint8_t TrackOccupancyDecoderWithRelays   = 0b00110010;   // Track Occupancy decoder with relays
const uint8_t TrackOccupancyDecoderWithSpeed    = 0b00110100;   // Track Occupancy decoder with speed measurement
const uint8_t FunctionDecoder                   = 0b01000000;   // Function Decoder
const uint8_t SafetyDecoder                     = 0b10000000;   // Watchdog and safety decoder
const uint8_t TMC24ChannelIODecoder             = 0b11000001;   // 24 Channel IO decoder for the TMC
const uint8_t TMC16ChannelSwitchDecoder         = 0b11000010;   // 16 Channel switch decoder for the TMC


//*****************************************************************************************************
const uint8_t max_cvs = 63;        // Maximum number of Generic CVs (that are initialised by setDefaults)

// CV Names
const uint8_t myAddrL      = 1;    // 0..63 / 0..255 - Decoder Address low. First address = 1.
const uint8_t version      = 7;    // 8..255 - Version should be 8 or higher
const uint8_t VID          = 8;    // 0x0D   - Vendor ID (0x0D = DIY Decoder) / If set to 0x0D, the decoder resets to default settings
const uint8_t myAddrH      = 9;    // 0..3   - Decoder Address high (3 bits)
const uint8_t myRSAddr     = 10;   // 1..128 - RS-bus address (Main address of the Feedback decoder). 0 = undefined
const uint8_t CmdStation   = 19;   // 0..2   - Master Station: 0 = Roco/Multimouse, 1 = Lenz, 2 = OpenDCC Z1 (Xpressnet V3.6)
const uint8_t Search       = 23;   // 0..1   - If 1: decoder LED blinks
const uint8_t Restart      = 25;   // 0..1   - To restart (as opposed to reset) the decoder
const uint8_t DccQuality   = 26;   // 0..255 - DCC Signal Quality
const uint8_t DecType      = 27;   // ...    - Decoder Type / See below for acceptable values. Parameter to the class constructor
const uint8_t RailCom      = 28;   // 0      - Bi-Directional (RailCom) Communication Config
const uint8_t Config       = 29;   // ...    - Accessory Decoder configuration
const uint8_t VID_2        = 30;   // 0x0D   - Second Vendor ID (Used by my PoM software to detect these are my decoders)
const uint8_t Shortcut     = 33;   // 40..80 - Value that indicate an output shortcut
const uint8_t PrintDetails = 34;   // 0..1   - 1: print every accessory command to the serial interface


//*****************************************************************************************************
class CvValues {
  public:
    uint8_t defaults[max_cvs + 1];                 // Default values for CVs

    // Fills the defaults array
    void init(uint8_t decoderType, uint8_t softwareVersion = 10);

    // Functions to ensure the EEPROM is being filled
    bool notInitialised(void);                     // Checks if the EEPROM has been initialised
    void setDefaults(void);                        // Fills the decoder with the default values


    // Generic CV functions
    uint8_t read(uint16_t number);                 // Can read every byte in EEPROM
    void write(uint16_t number, uint8_t value);    // Can write every byte in EEPROM


    unsigned int storedAddress(void);              // From CV1 and CV9 we get the decoder address
    bool addressNotSet(void);                      // Check if the decoder / RS-Bus address has been set

  private:

};
