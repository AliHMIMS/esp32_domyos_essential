#include "bike.h"

#include "settings.h"

namespace bike {

// A gap longer than this means the rider stopped.
static constexpr uint32_t STOP_MS = 3000;
// Cadence is averaged over the last few pulse intervals.
static constexpr int AVG_N = 3;

static volatile uint32_t isrPulses = 0;
static volatile uint32_t isrLastMs = 0;
static volatile uint32_t isrIntervals[AVG_N] = {};
static volatile uint8_t isrIntervalIdx = 0;
static volatile uint8_t isrIntervalCount = 0;
static volatile uint16_t isrDebounceMs = 60;

static BikeState st;
static uint32_t sessionStartPulses = 0;
static uint32_t lastUpdateMs = 0;
static uint32_t elapsedMs = 0;

static void IRAM_ATTR onPulse() {
  uint32_t now = millis();
  uint32_t dt = now - isrLastMs;
  if (dt < isrDebounceMs) return;
  isrLastMs = now;
  isrPulses++;
  if (dt < STOP_MS) {
    isrIntervals[isrIntervalIdx] = dt;
    isrIntervalIdx = (isrIntervalIdx + 1) % AVG_N;
    if (isrIntervalCount < AVG_N) isrIntervalCount++;
  } else {
    // First pulse after a stop: no valid interval yet.
    isrIntervalCount = 0;
  }
}

void begin(int pin) {
  isrDebounceMs = settings.debounceMs;
  pinMode(pin, INPUT_PULLUP);
  // The reed switch closes to GND when the magnet arrives.
  attachInterrupt(digitalPinToInterrupt(pin), onPulse, FALLING);
  lastUpdateMs = millis();
}

void update() {
  uint32_t now = millis();
  uint32_t dt = now - lastUpdateMs;
  if (dt < 100) return;
  lastUpdateMs = now;
  isrDebounceMs = settings.debounceMs;

  noInterrupts();
  uint32_t pulses = isrPulses;
  uint32_t lastMs = isrLastMs;
  uint8_t n = isrIntervalCount;
  uint32_t sum = 0;
  uint32_t newest = 0;
  for (int i = 0; i < n; i++) sum += isrIntervals[i];
  if (n > 0) newest = isrIntervals[(isrIntervalIdx + AVG_N - 1) % AVG_N];
  interrupts();

  uint32_t sinceLast = now - lastMs;
  bool moving = pulses > 0 && sinceLast < STOP_MS;

  float cadence = 0;
  if (moving && n > 0) {
    float avg = (float)sum / n;
    // While slowing down, the open interval is already longer than the last
    // one, so let it pull the cadence down instead of freezing the value.
    if (sinceLast > newest) avg = max(avg, (float)sinceLast);
    cadence = 60000.0f / (avg * settings.pulsesPerRev);
  }

  if (moving) elapsedMs += dt;

  // Leg-ergometer estimate: VO2 (ml/kg/min) = 7 + 10.8 * W/kg,
  // 1 L O2 ~ 5 kcal. powerK and the knob level's multiplier map cadence
  // to watts.
  float power = settings.powerK * settings.currentMult() * cadence * cadence;
  if (moving && cadence > 0) {
    float vo2 = 7.0f + 10.8f * power / settings.weightKg;
    float kcalPerMin = vo2 * settings.weightKg / 1000.0f * 5.0f;
    st.kcal += kcalPerMin * settings.calFactor * dt / 60000.0f;
  }

  st.sessionPulses = pulses - sessionStartPulses;
  st.totalPulses = pulses;
  st.cadenceRpm = cadence;
  st.speedKmh = cadence * settings.metersPerRev * 60.0f / 1000.0f;
  st.powerW = power;
  st.distanceKm = (float)st.sessionPulses / settings.pulsesPerRev *
                  settings.metersPerRev / 1000.0f;
  st.elapsedS = elapsedMs / 1000;
  st.moving = moving;
}

void resetSession() {
  noInterrupts();
  sessionStartPulses = isrPulses;
  interrupts();
  elapsedMs = 0;
  st.kcal = 0;
  st.sessionPulses = 0;
  st.distanceKm = 0;
  st.elapsedS = 0;
}

const BikeState& state() { return st; }

}  // namespace bike
