// *******************************************************************************************************
// File:      MyDefaults.cpp
// Author:    Aiko Pras
// History:   2024/05/05 AP Version 1.0
//            2025/12/01 AP V1.1: Changed, to be used with the TMC Switch-16 decoder.
//                                Filename capitalized, to show-up first in the IDE
// 
// Purpose:   To set a default address value in CV1 abd CV9
//
// ******************************************************************************************************
#include "core_Functions.h"      // To include the cvValues object
#include "MyDefaults.h"


void myDefaults_class::init() {
  cvValues.defaults[myAddrL]       = MY_CV1;
  cvValues.defaults[myAddrH]       = MY_CV9;
  // cvValues.defaults[CmdStation] = OpenDCC;

}
