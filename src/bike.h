#pragma once
#include <Arduino.h>

// Live ride metrics derived from the reed-switch pulses.
struct BikeState {
  float speedKmh = 0;
  float cadenceRpm = 0;
  float powerW = 0;       // estimate, the bike has no power sensor
  float distanceKm = 0;
  float kcal = 0;
  uint32_t elapsedS = 0;  // moving time only
  uint32_t sessionPulses = 0;
  uint32_t totalPulses = 0;  // since boot, for calibration
  bool moving = false;
};

namespace bike {

void begin(int pin);
void update();  // call often from loop()
void resetSession();
const BikeState& state();

}  // namespace bike
