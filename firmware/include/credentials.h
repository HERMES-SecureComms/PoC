#pragma once

#include <Arduino.h>

namespace hermes {
namespace credentials {

// EXAMPLE ONLY: replace this UID with the value printed for your test tag.
// A UID is a cloneable identifier, not a secret or cryptographic credential.
constexpr uint8_t AUTHORIZED_UID[] = {0x04, 0x12, 0xAB, 0xCD};
constexpr size_t AUTHORIZED_UID_LENGTH = sizeof(AUTHORIZED_UID);

}  // namespace credentials
}  // namespace hermes

