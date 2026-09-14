/**
 * Robot-specific dwell potentiometer input.
 * Keeps the custom dwell timing logic compact and readable.
 */
#pragma once

#include "../inc/MarlinConfig.h"

extern int DwPot;

class DwellPotSensor {
public:
  DwellPotSensor() { init(); }
  static void init();

  static void fpaccumulate(const int fpadc) {
    DwPot = int(fpadc);
  }
};

extern DwellPotSensor dwpot;

