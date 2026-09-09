#pragma once

#include <Arduino.h>

namespace hermes {

class PttController {
 public:
  PttController();

  void begin();
  void setSystemReady();
  void latchPermanentFault();
  void update(bool authenticated, uint32_t nowMs);

  bool isTxEnabled() const;

 private:
  bool readButtonPressed() const;
  void setTxEnabled(bool enabled, uint32_t nowMs);
  void logTimedEvent(const __FlashStringHelper* event, uint32_t nowMs) const;

  bool systemReady_;
  bool permanentFault_;
  bool rawPressed_;
  bool stablePressed_;
  bool txEnabled_;
  bool denialLogged_;
  uint32_t rawStateChangedAtMs_;
};

}  // namespace hermes

