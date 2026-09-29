#pragma once
#include <Arduino.h>

// User-tunable values, persisted in NVS and editable from the web UI.
struct Settings {
  float metersPerRev = 5.0f;   // virtual distance per pedal revolution
  float weightKg = 70.0f;      // rider weight, used for calories
  float powerK = 0.020f;       // estimated watts = powerK * rpm^2
  float calFactor = 1.0f;      // final multiplier to match the old display
  uint8_t pulsesPerRev = 1;    // magnet passes per pedal revolution
  uint16_t debounceMs = 60;    // ignore contact bounce shorter than this

  String wifiSsid;             // empty -> use secrets.h defaults
  String wifiPassword;

  void load();
  void save() const;
};

extern Settings settings;
