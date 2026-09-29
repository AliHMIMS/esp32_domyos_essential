#include "net.h"

#include <ESPmDNS.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>

#include "bike.h"
#include "ftms.h"
#include "secrets.h"
#include "settings.h"
#include "web_ui.h"

namespace net {

// Fallback hotspot, opened when the home network can't be reached.
static constexpr const char* AP_SSID = "Domyos-Setup";
static constexpr const char* AP_PASSWORD = "domyos123";
static constexpr uint32_t STA_TIMEOUT_MS = 15000;

static WebServer server(80);
static bool apActive = false;
static uint32_t staStartMs = 0;
static bool restartPending = false;
static uint32_t restartAtMs = 0;

static void startAp() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  apActive = true;
  Serial.printf("[net] hotspot \"%s\" up, http://%s\n", AP_SSID,
                WiFi.softAPIP().toString().c_str());
}

static void stopAp() {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  apActive = false;
  Serial.println("[net] hotspot off");
}

static void sendJson(const String& body) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", body);
}

static void handleState() {
  const BikeState& s = bike::state();
  char buf[320];
  snprintf(buf, sizeof(buf),
           "{\"speed\":%.2f,\"cadence\":%.1f,\"power\":%.0f,"
           "\"distance\":%.3f,\"kcal\":%.1f,\"time\":%lu,\"moving\":%s,"
           "\"pulses\":%lu,\"totalPulses\":%lu,\"ble\":%s,\"rssi\":%d}",
           s.speedKmh, s.cadenceRpm, s.powerW, s.distanceKm, s.kcal,
           (unsigned long)s.elapsedS, s.moving ? "true" : "false",
           (unsigned long)s.sessionPulses, (unsigned long)s.totalPulses,
           ftms::connected() ? "true" : "false",
           WiFi.isConnected() ? WiFi.RSSI() : 0);
  sendJson(buf);
}

static void handleGetSettings() {
  char buf[320];
  snprintf(buf, sizeof(buf),
           "{\"mpr\":%.3f,\"weight\":%.1f,\"powerK\":%.4f,\"calF\":%.3f,"
           "\"ppr\":%u,\"debounce\":%u,\"ssid\":\"%s\",\"ip\":\"%s\","
           "\"build\":\"%s %s\"}",
           settings.metersPerRev, settings.weightKg, settings.powerK,
           settings.calFactor, settings.pulsesPerRev, settings.debounceMs,
           settings.wifiSsid.c_str(), WiFi.localIP().toString().c_str(),
           __DATE__, __TIME__);
  sendJson(buf);
}

static float argFloat(const char* name, float current, float lo, float hi) {
  if (!server.hasArg(name)) return current;
  float v = server.arg(name).toFloat();
  return (v >= lo && v <= hi) ? v : current;
}

static void handlePostSettings() {
  settings.metersPerRev = argFloat("mpr", settings.metersPerRev, 0.1f, 50);
  settings.weightKg = argFloat("weight", settings.weightKg, 20, 250);
  settings.powerK = argFloat("powerK", settings.powerK, 0, 1);
  settings.calFactor = argFloat("calF", settings.calFactor, 0.1f, 10);
  settings.pulsesPerRev =
      (uint8_t)argFloat("ppr", settings.pulsesPerRev, 1, 20);
  settings.debounceMs =
      (uint16_t)argFloat("debounce", settings.debounceMs, 5, 500);
  settings.save();
  handleGetSettings();
}

static void handleWifi() {
  String ssid = server.arg("ssid");
  if (ssid.isEmpty()) {
    server.send(400, "text/plain", "ssid required");
    return;
  }
  settings.wifiSsid = ssid;
  settings.wifiPassword = server.arg("pass");
  settings.save();
  sendJson("{\"ok\":true}");
  restartPending = true;
  restartAtMs = millis() + 1000;
}

static void handleReset() {
  bike::resetSession();
  sendJson("{\"ok\":true}");
}

// Firmware update: POST multipart "firmware" to /update with basic auth
// admin:OTA_PASSWORD. Unlike ArduinoOTA, the ESP32 never connects back to
// the PC, so no firewall rule is needed.
static constexpr const char* OTA_USER = "admin";
static bool updateAuthorized = false;

static void handleUpdateUpload() {
  HTTPUpload& up = server.upload();
  if (up.status == UPLOAD_FILE_START) {
    updateAuthorized = server.authenticate(OTA_USER, OTA_PASSWORD);
    if (!updateAuthorized) return;
    Serial.printf("[ota] receiving %s\n", up.filename.c_str());
    Update.begin(UPDATE_SIZE_UNKNOWN);
  } else if (!updateAuthorized) {
    return;
  } else if (up.status == UPLOAD_FILE_WRITE) {
    Update.write(up.buf, up.currentSize);
  } else if (up.status == UPLOAD_FILE_END) {
    Update.end(true);
    Serial.printf("[ota] %u bytes, %s\n", up.totalSize,
                  Update.hasError() ? "FAILED" : "ok");
  } else if (up.status == UPLOAD_FILE_ABORTED) {
    Update.abort();
  }
}

static void handleUpdateDone() {
  if (!updateAuthorized) {
    server.requestAuthentication();
    return;
  }
  if (Update.hasError()) {
    server.send(500, "text/plain", String("update failed: ") +
                                       Update.errorString() + "\n");
    return;
  }
  server.send(200, "text/plain", "update ok, rebooting\n");
  restartPending = true;
  restartAtMs = millis() + 500;
}

void begin(const char* hostname) {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostname);
  WiFi.setAutoReconnect(true);
  WiFi.begin(settings.wifiSsid.c_str(), settings.wifiPassword.c_str());
  staStartMs = millis();
  Serial.printf("[net] connecting to \"%s\"...\n", settings.wifiSsid.c_str());

  MDNS.begin(hostname);
  MDNS.addService("http", "tcp", 80);

  server.on("/", HTTP_GET, [] {
    server.send_P(200, "text/html", WEB_UI_HTML);
  });
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/settings", HTTP_GET, handleGetSettings);
  server.on("/api/settings", HTTP_POST, handlePostSettings);
  server.on("/api/wifi", HTTP_POST, handleWifi);
  server.on("/api/reset", HTTP_POST, handleReset);
  server.on("/update", HTTP_POST, handleUpdateDone, handleUpdateUpload);
  server.onNotFound([] {
    // Send hotspot clients (captive-portal checks) to the UI.
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });
  server.begin();
}

void update() {
  server.handleClient();

  static bool wasConnected = false;
  bool connected = WiFi.isConnected();
  if (connected && !wasConnected) {
    Serial.printf("[net] connected, http://%s  (http://domyos.local)\n",
                  WiFi.localIP().toString().c_str());
    if (apActive) stopAp();
  }
  wasConnected = connected;

  if (!connected && !apActive && millis() - staStartMs > STA_TIMEOUT_MS) {
    startAp();  // keeps retrying the home network in the background
  }

  if (restartPending && (int32_t)(millis() - restartAtMs) > 0) ESP.restart();
}

}  // namespace net
