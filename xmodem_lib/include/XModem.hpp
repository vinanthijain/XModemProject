#pragma once
#include "XModemProtocol.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstring> 

class UARTSerial; 

class XModem {
public:
    explicit XModem(UARTSerial& uart);
    void sendPacket(uint8_t packetNumber, const uint8_t* data);
    int receivePacket(uint8_t expectedPacketNumber, uint8_t* data, const std::string& filePath, std::ofstream& file);
    void sendFile(const std::string& filePath);
    void receiveFile(const std::string& filePath);
    uint16_t calculateCRC(const uint8_t* data, size_t length); 

private:
    UARTSerial& uart;
    bool waitForResponse();
    void sendInitialC();
    void cancelTransmission(const std::string& filePath, std::ofstream& file);
};