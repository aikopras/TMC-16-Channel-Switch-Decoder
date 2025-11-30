//*****************************************************************************************************
//
// File:      core_CvValues.cpp
// Author:    Aiko Pras
// History:   2021/06/14 AP V1.0
//            2025/12/01 AP V2.0 Made specific for the TMC 16 Channel switch decoder
//
// Purpose:   C++ file that implements the methods to read and modify CV values stored in EEPROM,  
//            as well as the default values for all CVs.
//
// Origin: This code was taken from the AP_DCC_Decoder_Core library and rewritten to be used as
// part of the sketch for the TMC 16 Channel switch decoder. Everything related to other type of 
// decoders was removed, as well as RS-Bus specifics.
//
//*****************************************************************************************************
#include <Arduino.h>
#include <EEPROM.h>
#include "core_CvValues.h"


//*****************************************************************************************************
// Relation between default values and CVs
//
//                   RAM                            EEPROM
//                 defaults                           CVs
//                +--------+                      +--------+
//                |        |                      |        |
// constructor -> |        |     setDefaults()    |        | -> init()
//                |        |  ------------------> |        |
//     init() - > |        |       long push      |        | <- SM / PoM
//                |        |          CV8         |        |
//                +--------+                      +--------+
//
//*****************************************************************************************************

// Instantiate the cvValues object, which will be initialised in the main sketch
// The class CvValues is defined in CvValues.h
CvValues cvValues;

//*****************************************************************************************************
// Fill the `defaults` array with initial values, depending on the decoder type
void CvValues::init(uint8_t decoderType, uint8_t softwareVersion) {
  // Make all values of the defaults array 0, except the first
  for (uint8_t i = 1; i <= max_cvs; i++) defaults[i] = 0;
  defaults[0] = 0b01010101;
  //
  // Decoder type and Software version and Vendor IDs
  defaults[DecType] = decoderType;      // See const definitions from core_CvValues.h
  defaults[version] = softwareVersion;  // Any value is acceptable, 
  defaults[VID] = 0x0D;                 // Do It Yourself (DIY) decoder
  defaults[VID_2] = 0x0D;               // Used by my PoM software to detect these are my decoders
  //
  // Addresses:
  // - MyAddrL and MyAddrH will often be combined to create the Accessory Decoder address
  //   Lowest address is 1 (not 0!). myAddrH == 0x80 means undefined.
  // - myRSAddr will often be equivalent to the decoder address, and will for feedback decoders be the main address
  //   myRSAddr == 0 means undefined. 128 is reserved for RS-bus feedback to PoM messages
  defaults[myAddrL] = 0x01;             // Decoder address, low order bits (1..64) 
  defaults[myAddrH] = 0x80;             // Decoder address, high order bits (0..3)
  //
  // Accessory Decoder configuration
  // Bit 7: ‘0’ = Multi-function (Loco) Decoder / ‘1’= Accessory Decoder
  // Bit 6: Addressing Method: ‘0’= Decoder Address; ‘1’ = Output Address
  // Bit 5: Type: ‘0’ = Basic Accessory; ‘1’ = Extended Accessory
  // Bit 4: Reserved for future use.
  // Bit 3: "0" = no RailCom support, "1" = RailCom support
  // Bit 0..2 = Reserved for future use.
  // Most DIY decoders are Basic Accessory Decoders with multiple outputs (decoder addressing)
  // For Accessory Decoders that have a single output only, it would be better to set bit 6
  defaults[Config] = 0b10000000;        // Setting fits for most DIY decoders
  //
  // Generic settings for most decoders
  defaults[RailCom] = 0;                // 0..1 - We don't support RailCom
  defaults[CmdStation] = 1;             // 1 = LENZ LZV100 with Xpressnet V3.6; the default value
  defaults[DccQuality] = 0;             // Counts the number of DCC message checksum errors since last restart
  //
  // Specific settings for the TMC output shortcut protection
  defaults[Shortcut] = 64;              // AVR measurement that indicates an output shortcut (40..80)
  //
  // print every accessory command to the serial interface?
  defaults[PrintDetails] = 0;          // 0: no, 1: yes
  
}


//*****************************************************************************************************
// Checks if the EEPROM and the decoder address have been initialised
//*****************************************************************************************************
bool CvValues::notInitialised(void) {
  return (EEPROM.read(0) != 0b01010101);
}


bool CvValues::addressNotSet(void) {
  // We check CV9 (decoder address High)
  return (EEPROM.read(myAddrH) == 0x80);
}


//*****************************************************************************************************
// Restore all EEPROM content to default
//*****************************************************************************************************
void CvValues::setDefaults(void) {
  // Note that defaults[0] contains the value that indicates the EEPROM has been initialised
  // Note also that the decoder type as well as software version will be overwritten.
  for (uint8_t i = 0; i <= max_cvs; i++) EEPROM.update(i, defaults[i]);
}


//*****************************************************************************************************
// Read and Write
//*****************************************************************************************************
uint8_t CvValues::read(uint16_t number){
  return EEPROM.read(number);
}

void CvValues::write(uint16_t number, uint8_t value){
  // We do not do any sanity check regarding the value that is entered!
  EEPROM.update(number, value) ;
}


//*****************************************************************************************************
// Retrieve the decoder address, as stored in the EEPROM
// For Accessory Decoders this is either the decoder address or the output address
// For Multi-function Decoders this is the (short or long) loco address
//*****************************************************************************************************
unsigned int CvValues::storedAddress(void) {
  // The decoder configuration, and thus the address mode, is stored in CV29 (Config)
  // Bit 7: ‘0’ = Multi-function (Loco) Decoder / ‘1’= Accessory Decoder
  // Bit 6: Accessory addressing Method: ‘0’= Decoder Address; ‘1’ = Output Address
  // Bit 5: Loco addressing mode: '0' = Use short loco address from CV1, 
  //                              '1' = Use long loco address from CV17/18

  unsigned int address;
  uint8_t cv1;
  uint8_t cv9;
  uint8_t cv17;
  uint8_t cv18;
  uint8_t cv29 = cvValues.read(Config);
  bool accDecoder = bitRead(cv29,7);
  bool outputAddr = bitRead(cv29,6);
  bool longLocoAddr = bitRead(cv29,5);
  if (accDecoder) {
    // RCN-213 and RCN-225 describe the relationship between CV1, CV9 and the address.
    // According to RCN213, for the first handheld address (switch = 1) CV1 should become 1.
    // - the valid range for CV1 is 1..63 (if CV9 == 0) or 0..63 (if CV9 !=0)
    // - the valid range for CV9 is 0..3  (or 128, if the decoder has not been initialised)
    // If the decoder address has not yet been initialised, the highest order bit 
    // of `address` will be set.
    if (outputAddr) {
      cv1 = cvValues.read(myAddrL);
      cv9 = cvValues.read(myAddrH) & 0b00000111;
      // CV1 starts from 1, thus the lowest output address becomes 1
      address = (cv9 << 8) + cv1;
    }
    else {  // decoder addressing
      cv1 = cvValues.read(myAddrL) & 0b00111111;
      cv9 = cvValues.read(myAddrH) & 0b00000111;
      // CV1 starts from 1, but the lowest decoder address should be 0
      address = (cv9 << 6) + cv1 - 1;
    }    
    // If the decoder address has not yet been initialised (the highest order bit 
    // of myAddrH is set), `address` will be set to maxint.
    if (cvValues.read(myAddrH) >= 128) {address = 65535;}
  }
  else {  // Loco (multifunction) address
  // CV1 stores the basic (short) address. The range is between 1 and 127
  // CV17 and CV18 store the extended (14 bit long) address. CV17 is the MSB.
  // The default address is 3
    if (longLocoAddr) {
      cv17 = cvValues.read(17) & 0b00111111;
      cv18 = cvValues.read(18);
      address = (cv17 << 8) + cv18;
    }
    else {  // Use short loco address from CV1
      cv1 = cvValues.read(myAddrL) & 0b01111111;
      address = cv1;
    }
    // Address 0 is invalid. In that case enter the default address
    if (address == 0) address = 3;
  }
  return address;
}
