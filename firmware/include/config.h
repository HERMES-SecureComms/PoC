#pragma once

#include <Arduino.h>

namespace hermes {
namespace config {

// PN532 hardware SPI pins.
constexpr uint8_t PIN_PN532_SCK = 12;
constexpr uint8_t PIN_PN532_MISO = 13;
constexpr uint8_t PIN_PN532_MOSI = 11;
constexpr uint8_t PIN_PN532_SS = 10;

// User interface and simulated TX output pins.
constexpr uint8_t PIN_PTT_BUTTON = 4;
constexpr uint8_t PIN_TX_GATE = 5;
constexpr uint8_t PIN_LED_ALLOW = 6;
constexpr uint8_t PIN_LED_DENY = 7;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t AUTH_GRACE_PERIOD_MS = 1500;
constexpr uint32_t PTT_DEBOUNCE_MS = 30;
constexpr uint16_t NFC_POLL_TIMEOUT_MS = 50;
constexpr uint32_t MAIN_LOOP_DELAY_MS = 5;
constexpr uint8_t NFC_UID_MAX_LENGTH = 10;

}  // namespace config
}  // namespace hermes

