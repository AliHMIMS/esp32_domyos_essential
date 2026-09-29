#include "settings.h"

#include <Preferences.h>

#include "secrets.h"

Settings settings;

static constexpr const char* NS = "domyos";

void Settings::load() {
  Preferences p;
  p.begin(NS, true);
  metersPerRev = p.getFloat("mpr", metersPerRev);
  weightKg = p.getFloat("weight", weightKg);
  powerK = p.getFloat("powerK", powerK);
  calFactor = p.getFloat("calF", calFactor);
  pulsesPerRev = p.getUChar("ppr", pulsesPerRev);
  debounceMs = p.getUShort("debounce", debounceMs);
  wifiSsid = p.getString("ssid", WIFI_SSID);
  wifiPassword = p.getString("pass", WIFI_PASSWORD);
  p.end();

  if (pulsesPerRev == 0) pulsesPerRev = 1;
}

void Settings::save() const {
  Preferences p;
  p.begin(NS, false);
  p.putFloat("mpr", metersPerRev);
  p.putFloat("weight", weightKg);
  p.putFloat("powerK", powerK);
  p.putFloat("calF", calFactor);
  p.putUChar("ppr", pulsesPerRev);
  p.putUShort("debounce", debounceMs);
  p.putString("ssid", wifiSsid);
  p.putString("pass", wifiPassword);
  p.end();
}
