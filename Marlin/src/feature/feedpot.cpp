/**
 * Robot-specific feedrate potentiometer support.
 */

#include "../inc/MarlinConfig.h"

#if ENABLED(FEEDRATE_POT_SENSOR)
  #include "feedpot.h"

  FeedratePotSensor feedpot;

  void FeedratePotSensor::init() {}
#endif
