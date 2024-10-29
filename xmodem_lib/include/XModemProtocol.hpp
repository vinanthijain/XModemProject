#pragma once
#include <cstdint>

enum class XModemProtocol : uint8_t {
    SOH = 0x01,   // Start of Header
    EOT = 0x04,   // End of Transmission
    ACK = 0x06,   // Acknowledge 
    NAK = 0x15,   // Negative Acknowledge
    ETB = 0x17,   // End of Transmission Block
    CAN = 0x18    // Cancel
};

constexpr int PACKET_SIZE = 128;
constexpr int TIMEOUT_SECONDS = 3;