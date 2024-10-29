#pragma once
#include <cstring>
#include <stdexcept>
#include "XModem.hpp"
#include "XModemProtocol.hpp"
#include "UARTSerial.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::SetArrayArgument;
using ::testing::Throw;
using ::testing::Invoke;

class MockUARTSerial : public UARTSerial {
public:
    MockUARTSerial() : UARTSerial("/dev/pts/4", 9600) {}
    MOCK_METHOD(int, write, (const uint8_t* data, size_t length), (override));
    MOCK_METHOD(int, read, (uint8_t* data, size_t length), (override));
    MOCK_METHOD(int, getFd, (), (const, override));
    MOCK_METHOD(void, flush, (), (override));
};