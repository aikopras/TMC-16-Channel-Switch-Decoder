//*****************************************************************************************************
//
// File:      core_Timer.h
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
// The differences between MoToTimer.h and this code are small:
// - The class name was changed from MoToTimer to DccTimer
// - The "runTime" variable has been made public, to allow initialising the "runTime" without
//   immediately starting the timer.
// - "start()" has been added, which is equivalent to "restart()".
// - The code has been simplified and has become shorter.
//   In particular the 2 flags that were used in the original MoToTimer.h (RUNNING, NOTEXPIRED)
//   have been replaced by a single flag (notExpired).
//
//*****************************************************************************************************
#include <Arduino.h>
#pragma once

class DccTimer {
  public:
    unsigned long runTime = 0;

    void setTime(unsigned long value);
    bool running();
    bool expired();
    void start();
    void restart();
    void stop();
    unsigned long getRuntime();
    unsigned long getElapsed();
    unsigned long getRemain();

  private:
    bool notExpired = false;             // can be set by stop() / expired()
    unsigned long startTime = 0;
};
