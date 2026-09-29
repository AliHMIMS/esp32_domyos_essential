// Domyos Essential exercise bike -> ESP32-C6
//
// Reads the bike's reed switch (3.5 mm TS jack: tip -> SENSOR_PIN,
// sleeve -> GND), computes speed/distance/time/calories, serves a live web
// UI at http://domyos.local and broadcasts Bluetooth FTMS for training apps.

#include <Arduino.h>

#include "bike.h"
#include "ftms.h"
#include "net.h"
#include "settings.h"

// ESP32-C6: GPIO2 is a plain I/O (not a strapping, USB or UART pin).
constexpr int SENSOR_PIN = 2;
constexpr const char* HOSTNAME = "domyos";
constexpr const char* BLE_NAME = "Domyos Essential";

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n[domyos] starting");

  settings.load();
  bike::begin(SENSOR_PIN);
  net::begin(HOSTNAME);
  ftms::begin(BLE_NAME);
}

void loop() {
  bike::update();
  net::update();
  ftms::update();

  // Log each new pulse on serial, useful when testing with a USB cable.
  static uint32_t lastLogged = 0;
  const BikeState& s = bike::state();
  if (s.totalPulses != lastLogged) {
    lastLogged = s.totalPulses;
    Serial.printf("[bike] pulse %lu  %.0f rpm  %.1f km/h  %.2f km\n",
                  (unsigned long)s.totalPulses, s.cadenceRpm, s.speedKmh,
                  s.distanceKm);
  }

  delay(2);
}
