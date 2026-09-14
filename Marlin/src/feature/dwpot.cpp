/**
 * Robot-specific dwell potentiometer support.
 */

#include "../inc/MarlinConfig.h"

#if ENABLED(DWEL_POT_SENSOR)
  #include "dwpot.h"

  DwellPotSensor dwpot;

  void DwellPotSensor::init() {}
#endif
