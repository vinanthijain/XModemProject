#include "UARTSerial.hpp"
#include "XModem.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0] << " <send|receive> <file_path> <serial_port>" << std::endl;
            return 1;
        }

        std::string mode = argv[1];
        std::string filePath = argv[2];
        std::string serialPort = argv[3];

        UARTSerial uart(serialPort, B9600);
        XModem xmodem(uart);

        if (mode == "send") {
            xmodem.sendFile(filePath);
        } else if (mode == "receive") {
            xmodem.receiveFile(filePath);
        } else {
            std::cerr << "Invalid mode: " << mode << std::endl;
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
