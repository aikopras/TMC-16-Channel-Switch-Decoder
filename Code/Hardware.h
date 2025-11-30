// *******************************************************************************************************
// File:      Hardware.h
// Author:    Aiko Pras
// History:   2025/12/01 AP Version 1.0
// 
// Purpose:   Pin definitions for the TMC 16-Channel AVR32DA48 Switch Decoder board
//            Header file for the hardware initialisation and the ADC functions
//
// The following Timers are used:
// TCB0: AP_DCC_LIB
// TCB2: DxCore default for millis()
//
// ******************************************************************************************************
#pragma once

// DCC pins
#define dccPin            PIN_PA1    // DCC input pin
#define ackPin            PIN_PA2    // DCC Acknowlegement, for Service Mode programming

// LED pins
#define LED_DCC           PIN_PF5    // indicates valid DCC signal (PWM)
#define LED_ACC           PIN_PF4    // an accessory (switch) command for this decoder is received (PWM)
#define LED_PROG          PIN_PA0    // to indicate we are in programming mode
#define LED_ERROR         PIN_PA3    // An error has occured. Possible errors: output shortcut

// Button pin
#define buttonPin         PIN_PF6    // The onboard button to set the DCC address

// Monitoring Pins
#define MON_TXD           PIN_PA4    // Serial
#define MON_RXD           PIN_PA5    // 

// Map the 16 pins to switch the relays to AVR output pins
// Note: we could have made a more iniuitive mapping, 
// but that would have made PCB design more difficult
#define RELAY1   PIN_PB5
#define RELAY2   PIN_PB4
#define RELAY3   PIN_PB3
#define RELAY4   PIN_PB2
#define RELAY5   PIN_PB1
#define RELAY6   PIN_PB0
#define RELAY7   PIN_PA7
#define RELAY8   PIN_PA6
#define RELAY9   PIN_PC4
#define RELAY10  PIN_PC5
#define RELAY11  PIN_PC6
#define RELAY12  PIN_PC7
#define RELAY13  PIN_PC3
#define RELAY14  PIN_PC2
#define RELAY15  PIN_PC1
#define RELAY16  PIN_PC0


// Map the 16 pins to measure the current flowing through the relays to AVR ADC pins
#define ADC_RELAY1   ADC_MUXPOS_AIN11_gc  // PIN_PE3
#define ADC_RELAY2   ADC_MUXPOS_AIN10_gc  // PIN_PE2
#define ADC_RELAY3   ADC_MUXPOS_AIN9_gc   // PIN_PE1
#define ADC_RELAY4   ADC_MUXPOS_AIN8_gc   // PIN_PE0
#define ADC_RELAY5   ADC_MUXPOS_AIN7_gc   // PIN_PD7
#define ADC_RELAY6   ADC_MUXPOS_AIN6_gc   // PIN_PD6
#define ADC_RELAY7   ADC_MUXPOS_AIN5_gc   // PIN_PD5
#define ADC_RELAY8   ADC_MUXPOS_AIN4_gc   // PIN_PD4
#define ADC_RELAY9   ADC_MUXPOS_AIN19_gc  // PIN_PF3
#define ADC_RELAY10  ADC_MUXPOS_AIN18_gc  // PIN_PF2
#define ADC_RELAY11  ADC_MUXPOS_AIN17_gc  // PIN_PF1
#define ADC_RELAY12  ADC_MUXPOS_AIN16_gc  // PIN_PF0
#define ADC_RELAY13  ADC_MUXPOS_AIN3_gc   // PIN_PD3
#define ADC_RELAY14  ADC_MUXPOS_AIN2_gc   // PIN_PD2
#define ADC_RELAY15  ADC_MUXPOS_AIN1_gc   // PIN_PD1
#define ADC_RELAY16  ADC_MUXPOS_AIN0_gc   // PIN_PD0


// ******************************************************************************************************
class IO_Pin_class {
  public:
    void init();
  private:
    void init_serial();
    void init_leds();
    void init_relays_pins();
};


class adc_class {
  public:
    uint8_t maxValue;
    void init(uint8_t shortcutValueFromCV);
    bool shortcut(uint8_t muxpos);
  private:
    void init_adc_pins();
    void init_adc_logic();
};
