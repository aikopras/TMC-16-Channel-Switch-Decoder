//*****************************************************************************************************
//
// File:      core_Functions.cpp
// Author:    Aiko Pras
// History:   2021/06/14 AP V1.0
//            2021/12/30 AP V1.2
//            2022/08/02 AP V1.3
//            2025/12/01 AP V1.4: Changed, to be used within a sketch. Filename changed
//               Anything related to RS-Bus / feedback messages removed, as well as GBM specific code 
//
// Purpose:   C++ file that implements the methods to act on DCC CV-messages and pushes on the 
//            programming button. It can make changes to the LED.
//           
//*****************************************************************************************************
#include "core_Functions.h"                       // Header file for this C++ file

class ProgButton {
  public:
    void attach(uint8_t pin);                     // Attach the onboard programming button
    void checkForNewDecoderAddress(void);         // Is the onboard programming button pushed?
    void addressProgramming(void);                // Store the new decoder / RS-Bus address in EEPROM
};


//*****************************************************************************************************
// Objects instatiated / used in this file
//*****************************************************************************************************
DccLed programmingLed;                // The DccLed class is defined in core_LEDs
DccButton onBoardButton;              // The DccButton class is defined in core_ProgButton

// Classes defined in here
ProgButton progButton;                // The onBoardButton is used as programming button
CvProgramming cvProgramming;          // For actions after a CV-access commandis received
Processor processor;                  // Instantiate the Processor's object, which is used for reboot
CommonDecHwFunctions decoderHardware; // Common inits(), and update of the LED interface


//*****************************************************************************************************
// Reset the software / Reboot
//*****************************************************************************************************
void Processor::reboot(void) {
  // Restarts the program from the beginning
  // We first need to disable the DCC interrupt. If we wouldn't disable this, an external
  // event will cause a jump to the ISR. After an ISR is ready, it takes the return address and returns
  // to the calling routine. It is probably this bahavior that causes problems with interrupts if we
  // don't detach them before we jump to the reset vector at address 0.
  // Note that various Internet fora propose using the Watchdog Timer (see AVR data sheets) to do a
  // software reset. Using WDT has as advantage that all IO Registers will be set to their initial
  // value, but requires the use of (optiboot) bootloaders or special code during initialization.
  // A JMP to zero seems much simpler.
  noInterrupts();
  dcc.detach();
  asm volatile("  jmp 0");
  interrupts();
}


//*****************************************************************************************************
// Programming button 
//*****************************************************************************************************
// The ProgButton class is basically a small wrapper around the DccButton class (see AP_DccButton)
void ProgButton::attach(uint8_t pin) {
  onBoardButton.attach(pin);
  delay(500);
  onBoardButton.read();
}


void ProgButton::checkForNewDecoderAddress() {
  // Check if the decoder programming button is pushed
  // If the button is pushed for 5 seconds, restore all EEPROM data with default values.
  // If it is just pushed shortly, enter address programming
  onBoardButton.read();
  if (onBoardButton.isPressed()) programmingLed.turn_on();
  if (onBoardButton.pressedFor(5000)) {
      programmingLed.turn_off();
      cvValues.setDefaults();
      delay(500);
      processor.reboot();
  }    
  if (onBoardButton.wasReleased()) addressProgramming();
}


void ProgButton::addressProgramming() {
  // Set the decoder addresses:
  // CV1/CV9 (myAddrL/myAddrH): We store the output or decoder address 
  programmingLed.flashFast();
  do {
    programmingLed.update();
    if (dcc.input()) {
      uint8_t cv29 = cvValues.read(Config);
      bool accDecoder = bitRead(cv29,7);       // Are we an accessory decoder?
      bool outputAddr = bitRead(cv29,6);       // Do we want output (or decoder) addressing?
      // Only act if we are an accessory decoder and receive an accessory commands 
      if (accDecoder && ((dcc.cmdType == Dcc::MyAccessoryCmd) || (dcc.cmdType == Dcc::AnyAccessoryCmd))) {  
        // Store the Output address or the Decoder address
        // According to RCN213, for the first handheld address (switch = 1) CV1 should become 1.
        // - the valid range for CV1 is 1..63 (if CV9 == 0) or 0..63 (if CV9 !=0)
        // - the valid range for CV9 is 0..3  (or 128, if the decoder has not been initialised)
        if (outputAddr) {
          // Store the output address:
          // The range of the received output address is 1..1024 (LZV100) / 1..2048 (NMRA)
          uint8_t my_cv1 =  (accCmd.outputAddress & 0b11111111);
          uint8_t my_cv9 = ((accCmd.outputAddress >> 8) & 0b00000111);
          cvValues.write(myAddrL, my_cv1);
          cvValues.write(myAddrH, my_cv9);
        }      
        else {
          // Store the decoder address:
          // The range of the received decoder address is 0..255 (LZV100) / 511 (NMRA)
          // We therefore have to add 1 
          uint16_t tempAddress = accCmd.decoderAddress + 1;
          uint8_t my_cv1 =  (tempAddress & 0b00111111);
          uint8_t my_cv9 = ((tempAddress >> 6) & 0b00000111);
          cvValues.write(myAddrL, my_cv1);
          cvValues.write(myAddrH, my_cv9);
        }
        delay(100);
        processor.reboot(); // we got reprogrammed -> forget everthing running and restart decoder!
      }
    }
    onBoardButton.read();
  } while (!onBoardButton.isPressed());
  // Button was pushed again, but no DCC accessory decoder message was received.
  // No need to reboot().
  programmingLed.turn_off();
  delay(500);
  onBoardButton.read();;
}


//*****************************************************************************************************
// CvProgramming 
//*****************************************************************************************************
void CvProgramming::initPoM() {
  // For CV PoM (as well as F1..F4) messages we also listen to a loco address.   
  // This loco address is equal to the (first) decoder address, plus an offset.
  // For all accessory decoders, this offset is 7000.
  // If the decoder address is not programmed, the decoder listens for PoM messages at 'pomAddress - 1'.
  const int offset4Pom = 7000;
  uint16_t pomAddress;
  if (cvValues.addressNotSet()) {pomAddress = offset4Pom - 1;}
    else {pomAddress = offset4Pom + cvValues.storedAddress();} 
  locoCmd.setMyAddress(pomAddress);
}       


//*****************************************************************************************************
// CvProgramming::processMessage
//*****************************************************************************************************
void CvProgramming::processMessage(Dcc::CmdType_t cmdType) {
  // Create some local variables 
  unsigned int RecCvNumber = cvCmd.number;
  uint8_t RecCvData = cvCmd.value;
  uint8_t CurrentEEPROMValue = cvValues.read(RecCvNumber);
  bool SM  = (cmdType == Dcc::SmCmd);
  bool PoM = (cmdType == Dcc::MyPomCmd);
  // 2025/05/06 AP: Modified, to allow using the entire EEPROM size
  // Ensure we stay within the range supported by this decoder
  // if (RecCvNumber < max_cvs) {
  // if (RecCvNumber <  EEPROM_SIZE) {
  // 2025/10/18 AP: Modified, since EEPROM_SIZE is not always defined.
  if (RecCvNumber <= E2END) {
    switch(cvCmd.operation) {
      case CvAccess::verifyByte :
        if (SM) {
          // In SM we send back a DCC-ACK signal
          // if the value of the received byte matches the CV value in EEPROM
          if (CurrentEEPROMValue == RecCvData) {dcc.sendAck();}
        }
        if (PoM) {
          // In PoM mode a railcom reply should be returned, but since we don't support Railcom
          // we do nothing
        }
      break;
      case CvAccess::writeByte :
        // A number of CVs have a special meaning, and can not directly be written
        switch (RecCvNumber) {
          case version:
            // CV7 (version): should not be writeable
          break;
          case VID: 
            //CV8 (VID): Reset decoder data to initial values if we'll write to CV8 the value 0x0D
            if (RecCvData == 0x0D) {
              cvValues.setDefaults();
              if (SM) dcc.sendAck();
              processor.reboot();
            }
          break;
          case Restart:
            // CV25: Restart the decoder if we write a value of 1 or higher, but do not reset the EEPROM data (cvValues)
            // Use this function after PoM has changed CV values and new values should take effect now
            if (RecCvData) processor.reboot();
          break;
          case Search:
            // Search function: blink the decoder's LED if CV23 is set to 1. 
            // Continue blinking until CV23 is set to 0
            if (RecCvData) {
              LedShouldFlash = true; 
              programmingLed.flashFast();
            } 
            else {
              LedShouldFlash = false; 
              programmingLed.turn_off();
            }
          break;
          default:
            cvValues.write(RecCvNumber, RecCvData);
            if (SM) dcc.sendAck();
          break;
        }
      break;
      case CvAccess::bitManipulation :
      // Note: CV Bit Operation is only implemented for Service Mode (not for PoM)
        if (cvCmd.writecmd) {
          uint8_t NewEEPROMValue = cvCmd.writeBit(CurrentEEPROMValue);
          cvValues.write(RecCvNumber, NewEEPROMValue);
          if (SM) dcc.sendAck();
        }
        else { // verify if bits are equal
          if (cvCmd.verifyBit(CurrentEEPROMValue)) {
            if (SM) dcc.sendAck();
          }
        }
      break;
      default:
      break;
    }
  }
}


//*****************************************************************************************************
// Common functions for the Decoder hardware (DCC, RS-Bus, LED, Button)
//*****************************************************************************************************
void CommonDecHwFunctions::init(void) {
  // Should be called from setup() in the main sketch.
  // Initialise the EEPROM (cvValues) if it has been erased. 
  if (cvValues.notInitialised()) cvValues.setDefaults();
  // attach input pins to the objects below 
  dcc.attach(dccPin, ackPin);
  programmingLed.attach(LED_PROG);
  progButton.attach(buttonPin);
  // Set the Loco address for PoM messages 
  cvProgramming.initPoM();
  // Light the LED to indicate the decoder has started and if the address is set
  programmingLed.start_up();
  if (cvValues.addressNotSet()) programmingLed.flashSlow();
  else programmingLed.start_up();
  // Set the Accessory address and the type of Command Station
  accCmd.setMyAddress(cvValues.storedAddress());
  accCmd.myMaster = cvValues.read(CmdStation);
  // initializes the 20ms timer that reduces the CPU load of update()
  TLast = millis();
}  


void CommonDecHwFunctions::update(void) {
  // Should be called from main as often as possible.
  unsigned long TNow = millis();            // millis() is expensive, so call it only once
  if ((TNow - TLast) >= 20) {               // 20ms passed?
    TLast = TNow;
    progButton.checkForNewDecoderAddress(); // Is the decoder programming button pushed?
    programmingLed.update();                    // Control LED flashing
  }
}                            
