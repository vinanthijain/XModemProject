#pragma once
#include <stdexcept>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstdint>

class UARTSerial {
public:
    UARTSerial(const std::string& portName, int baudRate);
    ~UARTSerial();
    virtual int read(uint8_t* buffer, size_t size);
    virtual int write(const uint8_t* buffer, size_t size);
    void closePort();
    virtual int getFd() const; 
    virtual void flush();

private:
    std::string device;
    int serialPort;
};
