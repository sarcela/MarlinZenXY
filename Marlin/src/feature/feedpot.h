/**
 * Robot-specific feedrate potentiometer input.
 * Keeps the custom feed-rate control logic compact and readable.
 */
#pragma once

#include "../inc/MarlinConfig.h"

extern int feedRate;

class FeedratePotSensor {
public:
  FeedratePotSensor() { init(); }
  static void init();

  static void fpaccumulate(const int fpadc) {
    feedRate = int(fpadc);
  }
};

extern FeedratePotSensor feedpot;

