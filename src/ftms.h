#pragma once

// Bluetooth LE Fitness Machine Service (FTMS) "Indoor Bike", so training
// apps (Kinomap, Zwift, ...) can read speed, cadence, distance and power.
namespace ftms {

void begin(const char* deviceName);
void update();  // sends Indoor Bike Data once per second when connected
bool connected();

}  // namespace ftms
