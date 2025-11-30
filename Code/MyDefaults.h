//*****************************************************************************************************
//
// File:      MyDefaults.h
// Author:    Aiko Pras
// History:   2024/04/27 AP V1.0
//            2025/12/01 AP V1.1: Changed, to be used with the TMC Switch-16 decoder.
//                                Filename capitalized, to show-up first in the IDE
//
// Purpose:   Each switch-decoder gets a unique default DCC addresses.
//            Although this address may be changed using normal procedure (programming button, 
//            PoM messages), after a factory reset (long button push, wrinting to CV8) the new values
//            will be the default address.
//
// The TMC 16 chennel switch decoder board represents the equivalent of 4 DCC decoders.
// CV1 should therefore have values in increments of 4.
// Possible values for CV1 are therefore 1, 5, 9, 13, 17, ..., 61 
// Possible values for CV9 are 0, 1, 2 and 3 
//  
// Make sure you understand the relation between Accessory addresses and CV1/CV9 values
// See the general discussion on https://github.com/aikopras/AP_DCC_library, as well as the addressing
// explanation in https://github.com/aikopras/AP_DCC_library/blob/main/src/sup_acc.cpp for details
//         
//******************************************************************************************************
#pragma once

// Make the DECODER number unique for each switch decoder board
// 1 = switch addresses 529..544
// 2 = switch addresses 545..560
// 3 = switch addresses 561..576
// 4 = switch addresses 577..592
// 5 = switch addresses 593..608
// 6 = switch addresses 609..624
// 7 = switch addresses 625..640
// 8 = switch addresses 641..656
// 9 = switch addresses 657..672  

#define DECODER 1

//******************************************************************************************************
// Do not edit below this line
// CV1: Decoder address, low order bits (1..64)
// CV9: Decoder address, high order bits (0..3)
// This for the first 15 decoders the following formula / value holds:
#define MY_CV1 (1 + (DECODER) * 4)
#define MY_CV9     2     

// This results into the following switch addresses:
// 529: CV1=5  / CV2=2
// 545: CV1=9  / CV2=2
// 561: CV1=13 / CV2=2
// 577: CV1=17 / CV2=2
// 593: CV1=21 / CV2=2
// 609: CV1=25 / CV2=2
// 625: CV1=29 / CV2=2
// 641: CV1=33 / CV2=2
// 657: CV1=37 / CV2=2
// 673: CV1=41 / CV2=2
// 689: CV1=45 / CV2=2
// 705: CV1=49 / CV2=2
// ...


class myDefaults_class {
  public:
    void init();
};
