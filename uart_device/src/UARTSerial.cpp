#include "UARTSerial.hpp"
#include <iostream>

UARTSerial::UARTSerial(const std::string& portName, int baudRate) {
    serialPort = open(portName.c_str(), O_RDWR);
    if (serialPort < 0) {
        throw std::runtime_error("Failed to open UART port");
    }

    termios tty{};
    if (tcgetattr(serialPort, &tty) != 0) {
        close(serialPort);
        throw std::runtime_error("Failed to get UART port attributes");
    }

    cfsetospeed(&tty, baudRate);
    cfsetispeed(&tty, baudRate);
    tty.c_cflag |= CREAD | CLOCAL;

    if (tcsetattr(serialPort, TCSANOW, &tty) != 0) {
        close(serialPort);
        throw std::runtime_error("Failed to set UART port attributes");
    }
}

UARTSerial::~UARTSerial() {
    closePort();
}

int UARTSerial::read(uint8_t* buffer, size_t size) {
    return ::read(serialPort, buffer, size);
}

int UARTSerial::write(const uint8_t* buffer, size_t size) {
    return ::write(serialPort, buffer, size);
}

void UARTSerial::flush() {
    tcflush(serialPort, TCIOFLUSH);
}

void UARTSerial::closePort() {
    if (serialPort >= 0) {
        close(serialPort);
        serialPort = -1;
    }
}

int UARTSerial::getFd() const {
    return serialPort;
}
