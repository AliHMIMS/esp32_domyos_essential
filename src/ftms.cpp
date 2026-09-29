#include "ftms.h"

#include <NimBLEDevice.h>

#include "bike.h"

namespace ftms {

static constexpr uint16_t SVC_FTMS = 0x1826;
static constexpr uint16_t CHR_FEATURE = 0x2ACC;
static constexpr uint16_t CHR_INDOOR_BIKE_DATA = 0x2AD2;
static constexpr uint16_t CHR_CONTROL_POINT = 0x2AD9;

static NimBLECharacteristic* bikeData = nullptr;
static NimBLECharacteristic* controlPoint = nullptr;
static volatile bool isConnected = false;
static uint32_t lastSendMs = 0;

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer*, NimBLEConnInfo&) override {
    isConnected = true;
  }
  void onDisconnect(NimBLEServer*, NimBLEConnInfo&, int) override {
    isConnected = false;  // advertising restarts automatically
  }
};

// Apps expect the control point to exist and acknowledge basic commands,
// even though the bike's resistance can't be changed electronically.
class ControlPointCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c, NimBLEConnInfo&) override {
    NimBLEAttValue v = c->getValue();
    if (v.size() < 1) return;
    uint8_t op = v[0];
    uint8_t result;
    switch (op) {
      case 0x00:  // request control
      case 0x07:  // start / resume
      case 0x08:  // stop / pause
        result = 0x01;
        break;
      case 0x01:  // reset
        bike::resetSession();
        result = 0x01;
        break;
      default:
        result = 0x02;  // op code not supported
    }
    uint8_t resp[3] = {0x80, op, result};
    c->setValue(resp, sizeof(resp));
    c->indicate();
  }
};

static void put16(uint8_t*& p, uint16_t v) {
  *p++ = v & 0xFF;
  *p++ = v >> 8;
}

void begin(const char* deviceName) {
  NimBLEDevice::init(deviceName);
  NimBLEServer* server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  NimBLEService* svc = server->createService(NimBLEUUID(SVC_FTMS));

  // Features: cadence, total distance, expended energy, elapsed time, power.
  uint8_t feature[8] = {0};
  uint32_t f = (1 << 1) | (1 << 2) | (1 << 9) | (1 << 12) | (1 << 14);
  memcpy(feature, &f, 4);
  NimBLECharacteristic* feat = svc->createCharacteristic(
      NimBLEUUID(CHR_FEATURE), NIMBLE_PROPERTY::READ);
  feat->setValue(feature, sizeof(feature));

  bikeData = svc->createCharacteristic(NimBLEUUID(CHR_INDOOR_BIKE_DATA),
                                       NIMBLE_PROPERTY::NOTIFY);

  controlPoint = svc->createCharacteristic(
      NimBLEUUID(CHR_CONTROL_POINT),
      NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE);
  controlPoint->setCallbacks(new ControlPointCallbacks());

  svc->start();

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->setName(deviceName);
  adv->addServiceUUID(NimBLEUUID(SVC_FTMS));
  // FTMS service data: machine available, type = indoor bike.
  const uint8_t svcData[] = {0x01, 0x20, 0x00};
  adv->setServiceData(NimBLEUUID(SVC_FTMS), svcData, sizeof(svcData));
  adv->start();
}

void update() {
  if (!isConnected || !bikeData) return;
  uint32_t now = millis();
  if (now - lastSendMs < 1000) return;
  lastSendMs = now;

  const BikeState& s = bike::state();

  // Flags: speed (implicit), cadence, total distance, power, energy, time.
  const uint16_t flags = (1 << 2) | (1 << 4) | (1 << 6) | (1 << 8) | (1 << 11);
  uint8_t buf[18];
  uint8_t* p = buf;
  put16(p, flags);
  put16(p, (uint16_t)(s.speedKmh * 100.0f));  // 0.01 km/h
  put16(p, (uint16_t)(s.cadenceRpm * 2.0f));  // 0.5 rpm
  uint32_t meters = (uint32_t)(s.distanceKm * 1000.0f);
  *p++ = meters & 0xFF;
  *p++ = (meters >> 8) & 0xFF;
  *p++ = (meters >> 16) & 0xFF;
  put16(p, (uint16_t)(int16_t)s.powerW);  // W
  put16(p, (uint16_t)s.kcal);             // total kcal
  put16(p, 0xFFFF);                       // kcal/h: not available
  *p++ = 0xFF;                            // kcal/min: not available
  put16(p, (uint16_t)min<uint32_t>(s.elapsedS, 0xFFFF));

  bikeData->setValue(buf, p - buf);
  bikeData->notify();
}

bool connected() { return isConnected; }

}  // namespace ftms
