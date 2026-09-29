#pragma once

// Wi-Fi (home network, with a fallback hotspot), mDNS, OTA and the web UI.
namespace net {

void begin(const char* hostname);
void update();  // call often from loop()

}  // namespace net
