// *******************************************************************************************************
// File:      Hardware.cpp
// Author:    Aiko Pras
// History:   2025/12/01 AP Version 1.0
// 
// Purpose:   Initialisation of the hardware
//
// The "novel" code in this file is the code related to ADC. The ADC operates at 2Mhz (prescaler = 12)
// and with 10 bit resolution. At 2 Mhz a conversion typically takes 7,25 microseconds. That should be
// fast enough to protect the resistors and transistor against shortcuts.
//
// Below are some measured ADC values from resistors that were connected to the switch output connector
// of the TMC switch decoder board:
// 270 Ohm: 110
// 680 Ohm:  59
// 1 KOhm:   42
// 3,3 KOhm: 11 
//
// If a TMC switch (telephone) relais gets connected, the following values are measured:
// Immediate: 0 or 4
// after 1 ms: 12
// after 10 ms: 31
// after 100 ms: 48
// after 1 second: 48
// => To cope with differences in relais and other components, a value of 64 seems reasonable
//    This value can be changed using CV33
//
// The adc_class object checks for shortcuts, using this value.
//
// ******************************************************************************************************
#include <Arduino.h>
#include "Hardware.h"

void IO_Pin_class::init() {
  init_serial();
  init_leds();
  init_relays_pins();
}


void adc_class::init(uint8_t shortcutValueFromCV) {
  maxValue = shortcutValueFromCV;
  init_adc_pins();
  init_adc_logic();
}

void IO_Pin_class::init_serial() {
  // Serial monitor is used for debugging
  Serial.swap(1);      // use alternative pins = PA4/PA5
  delay(100);
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("TMC 16 Channel Switch Decoder");
  delay(100);
}


void IO_Pin_class::init_leds() {
  pinMode(LED_DCC, OUTPUT);
  digitalWrite(LED_DCC, LOW);          // indicates valid DCC signal
  pinMode(LED_ACC, OUTPUT);
  digitalWrite(LED_ACC, LOW);          // an accessory (switch) command for this decoder is received
  pinMode(LED_ERROR, OUTPUT);
  digitalWrite(LED_ERROR, LOW);        // To indicate output shortcut
}


void IO_Pin_class::init_relays_pins() {
  pinMode(RELAY1, OUTPUT);
  digitalWrite(RELAY1, LOW);
  pinMode(RELAY2, OUTPUT);
  digitalWrite(RELAY2, LOW);
  pinMode(RELAY3, OUTPUT);
  digitalWrite(RELAY3, LOW);
  pinMode(RELAY4, OUTPUT);
  digitalWrite(RELAY4, LOW);
  pinMode(RELAY5, OUTPUT);
  digitalWrite(RELAY5, LOW);
  pinMode(RELAY6, OUTPUT);
  digitalWrite(RELAY6, LOW);
  pinMode(RELAY7, OUTPUT);
  digitalWrite(RELAY7, LOW);
  pinMode(RELAY8, OUTPUT);
  digitalWrite(RELAY8, LOW);  
  pinMode(RELAY9, OUTPUT);
  digitalWrite(RELAY9, LOW);
  pinMode(RELAY10, OUTPUT);
  digitalWrite(RELAY10, LOW);
  pinMode(RELAY11, OUTPUT);
  digitalWrite(RELAY11, LOW);
  pinMode(RELAY12, OUTPUT);
  digitalWrite(RELAY12, LOW);  
  pinMode(RELAY13, OUTPUT);
  digitalWrite(RELAY13, LOW);
  pinMode(RELAY14, OUTPUT);
  digitalWrite(RELAY14, LOW);
  pinMode(RELAY15, OUTPUT);
  digitalWrite(RELAY15, LOW);
  pinMode(RELAY16, OUTPUT);
  digitalWrite(RELAY16, LOW);  
}


void adc_class::init_adc_pins() {
  // Ensure all pins for ADC are used as inputs, and not for outputs
  // PORT D: all pins as analog input
  PORTD.PINCONFIG  = PORT_ISC_INPUT_DISABLE_gc; // digital input off, pull-up off
  PORTD.PINCTRLUPD = 0xFF;                      // select PD0..PD7
  PORTD.DIRCLR     = 0xFF;                      // all input
  // PORT E: all pins as analog input
  PORTE.PINCONFIG  = PORT_ISC_INPUT_DISABLE_gc;
  PORTE.PINCTRLUPD = 0xFF;                      // PE0..PE7
  PORTE.DIRCLR     = 0xFF;
  // PORT F: only PF0..PF3 as analog input
  PORTF.PINCONFIG  = PORT_ISC_INPUT_DISABLE_gc;
  PORTF.PINCTRLUPD = 0x0F;                      // PF0..PF3
  PORTF.DIRCLR     = 0x0F;                      // PF0..PF3 input
}


void adc_class::init_adc_logic() {
  // Select the reference voltage of 1,024 V
  VREF.ADC0REF = VREF_REFSEL_1V024_gc;
  // CTRLA
  ADC0.CTRLA = ADC_RESSEL_10BIT_gc              // 10-bit resolution
             | ADC_ENABLE_bm;                   // ADC enabled     
  // CTRLB
  ADC0.CTRLB = ADC_SAMPNUM_NONE_gc;             // No sampling desired for shortcut detection    
  // CTRLC: Prescaler
  ADC0.CTRLC = ADC_PRESC_DIV12_gc;              // CLK_PER / 12  => 24 MHz gives 2 MHz ADC clock
  ADC0.INTFLAGS = ADC_RESRDY_bm;                // clear flag
  //
  // One dummy conversion seems to be necessary during start up
  ADC0.MUXPOS = ADC_RELAY16;                   // select arbitrary ADC pin
  ADC0.COMMAND = ADC_STCONV_bm;                // start single conversion
  while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));    // busy wait till result
  ADC0.INTFLAGS = ADC_RESRDY_bm;               // clear flag
}


bool adc_class::shortcut(uint8_t muxpos) {
  ADC0.MUXPOS = muxpos;                        // select ADC pin
  ADC0.COMMAND = ADC_STCONV_bm;                // start single conversion
  while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));    // busy wait till result
  ADC0.INTFLAGS = ADC_RESRDY_bm;               // clear flag
  return (ADC0.RES > maxValue);                // set the boolean return value
}
