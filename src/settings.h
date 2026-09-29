#pragma once
#include <Arduino.h>

// User-tunable values, persisted in NVS and editable from the web UI.
struct Settings {
  float metersPerRev = 5.0f;   // virtual distance per pedal revolution
  float weightKg = 70.0f;      // rider weight, used for calories
  float powerK = 0.020f;       // watts = powerK * levelMult * rpm^2
  float calFactor = 1.0f;      // final multiplier to match the old display
  uint8_t pulsesPerRev = 1;    // magnet passes per pedal revolution
  uint16_t debounceMs = 60;    // ignore contact bounce shorter than this

  // Tension knob. The bike can't report it, so the rider sets it in the UI.
  // Multipliers are relative to the reference level (the one at 1.0); the
  // defaults are a guess until the levels are calibrated.
  static constexpr uint8_t LEVELS = 8;
  uint8_t level = 4;
  float levelMult[LEVELS] = {0.55f, 0.67f, 0.82f, 1.0f,
                             1.22f, 1.49f, 1.82f, 2.22f};

  String wifiSsid;             // empty -> use secrets.h defaults
  String wifiPassword;

  void load();
  void save() const;
  void saveLevel() const;  // the level changes often, so it saves alone

  float currentMult() const { return levelMult[level - 1]; }
};

extern Settings settings;
