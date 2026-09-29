# Domyos Essential → ESP32-C6

Replaces the Domyos Essential exercise bike's display with an ESP32-C6: live web
UI plus Bluetooth FTMS for training apps.

## Guides

1. [Identifying the sensor signal](docs/1-identify-signal.md): what the jack carries, and how it was measured
2. [Setting up at the bike](docs/2-at-the-bike.md): wiring, power, phone, pulse check, Bluetooth apps, troubleshooting
3. [Calibrating against the old display](docs/3-calibration.md): speed, distance, calories and knob levels

## Wiring

The bike's 3.5 mm TS jack is a reed switch (1 pulse per pedal revolution):

| Jack         | ESP32-C6 |
|--------------|----------|
| Tip (T)      | GPIO2    |
| Sleeve (S)   | GND      |

Optional: 10 kΩ GPIO2→3V3 and 100 nF GPIO2→GND for a cleaner signal.

## Using it

- Web UI: <http://domyos.local> (or the IP shown in Settings)
- Bluetooth: pair "Domyos Essential" in any FTMS app (Kinomap, Zwift, ...)
- If the home Wi-Fi can't be reached, the board opens the hotspot
  `Domyos-Setup` (password `domyos123`) → <http://192.168.4.1>

Speed, distance and calories use the constants in the Settings page
(metres per revolution, weight, power factor, calorie multiplier); they are
stored on the board.

The bike can't report its tension knob, so select the level (1–8) on the
dashboard when you turn it. It scales the power and calorie estimates, and is
sent to Bluetooth apps as the resistance level. **Settings → Knob levels →
Calibrate levels** walks you through calibrating each level.

## Building

Needs PlatformIO. Copy `secrets.ini.example` → `secrets.ini` and
`include/secrets.example.h` → `include/secrets.h`.

```sh
pio run -e usb -t upload     # first flash, CH343 USB-C port
pio run -e ota -t upload     # later updates over Wi-Fi
```

## Code

- `src/bike.cpp` – pulse interrupt, cadence/speed/distance/time/calories
- `src/net.cpp` – Wi-Fi + fallback hotspot, mDNS, HTTP API, firmware update
- `src/ftms.cpp` – Bluetooth LE Fitness Machine Service (Indoor Bike Data)
- `src/web_ui.h` – the single-page web UI
- `src/settings.cpp` – settings persisted in NVS
