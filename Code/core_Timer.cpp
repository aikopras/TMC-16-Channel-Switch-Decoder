//*****************************************************************************************************
//
// File:      core_Timer.cpp
// Author:    Aiko Pras
// History:   2022/07/19 AP Version 1.0
//            2025/12/01 AP Version 1.1: Changed from library to be used within the sketch
//                                       Filename changed
//
// Purpose:   Timer class
//
// This code is inspired and largely based on MoToTimer.h from the MoBaTools library:
// https://github.com/MicroBahner/MobaTools
// It was developed since the MoBaTools library did not compile on DxCore boards
//
// Technically the main difference between MoToTimer and DccTimer is that MoToTimer.h
// uses a status byte active, that includes two bit flags (RUNNING, NOTEXPIRED)
// whereas DccTimer uses a single status flag (notExpired).
//
// The "notExpired" status flag tells the "expired()" method to return TRUE only in the first call
// after the timer has expired. Earlier or subsequent calls to the "expired()" method will
// return FALSE. The "notExpired" flag is set immediately after the timer is started, either by
// "setTime()", "start()" or "restart". The "notExpired" flag is cleared by the "expired()",
// or by the "stop()" method.
//
// The "running()" method is used to determine if the timer has not been expired or stopped.
// The condition to determine expiry is: millis() - startTime >= runTime.
// If the timer was stopped before expiry, the "notExpired" flag was cleared by "stop()".
// The "running()" method checks both.
//
//
//*****************************************************************************************************
#include <Arduino.h>
#include "core_Timer.h"

void DccTimer::setTime(unsigned long value) {
  runTime = value;
  if (runTime > 0) {
    startTime = millis();
    notExpired = true;
  }
  else notExpired = false;
}

bool DccTimer::running() {
  if (notExpired) {
    if (millis() - startTime < runTime) return true;
  }
  return false;
}

bool DccTimer::expired() {
  // Only the first call after expiration returns true.
  // If stop() was called before expiration, false will be returned
  if (notExpired) {
    if (millis() - startTime >= runTime) {
      notExpired = false;
      return true;
    }
  }
  return false;
}

void DccTimer::start() {
  startTime = millis();
  notExpired = true;
}

void DccTimer::restart() {
  DccTimer::start();
}
    
void DccTimer::stop() {
  notExpired = false;
}

unsigned long DccTimer::getRuntime() {
  return runTime;
}

unsigned long DccTimer::getElapsed() {
  if (running()) return ((millis() - startTime) + 1);
  else return runTime;
}

unsigned long DccTimer::getRemain() {
  if (running()) return (runTime - (millis() - startTime));
  else return 0;
}
