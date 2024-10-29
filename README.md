## Introduction:

The XModem protocol is a byte-oriented file transfer protocol that utilizes a simple packet-based approach to send data over serial communication. 
It is widely used in scenarios where robustness and simplicity are prioritized over speed, such as firmware updates and communication with 
embedded systems.

This project provides:

A library for sending and receiving files using the XModem protocol.
A command-line utility to demonstrate file transfer functionality.
Unit tests to validate protocol behavior and correctness.

## Folder Structure:

├── CMakeLists.txt           # Main CMake configuration file
├── xmodem_utility           # Command-line utility for file transfer
│   ├── main.cpp             # Main source file for the command-line utility
├── xmodem_lib               # Library implementation for the XModem protocol
│   ├── include              # Public headers for the XModem library
│   │   ├── XModem.hpp       # Header for XModem protocol implementation
│   │   ├── XModemProtocol.hpp# Header for XModem protocol constants
│   ├── src                  # Implementation files for the library
│   │   ├── XModem.cpp       # XModem protocol implementation
├── tests                    # Unit tests and test resources
│   ├── gtest                # Google Test files
│   │   ├── XModemTests.cpp  # Unit tests for the XModem library
│   └── gmock                # Google Mock files
│       └── MockUARTSerial.hpp # Mock for UARTSerial class
├── uart_device              # UART device communication implementation
│   ├── include              # Public headers for UART communication
│   │   ├── UARTSerial.hpp    # Header for UART serial communication
│   └── src                  # Implementation files for UART communication
│       └── UARTSerial.cpp    # UART serial communication implementation


## XModem Protocol Overview:

The XModem protocol transfers data in 128-byte packets, each consisting of:

Header (3 bytes): Contains the Start-of-Header byte (0x01), packet number, and the complement of the packet number.
Data (128 bytes): The actual data payload. If the data size is smaller than 128 bytes, the remaining space is padded with zeros.
CRC (2 bytes): A 16-bit CRC for error detection.

## XModem Protocol Control Characters:

Symbol   Description                                             Value
-------  --------------------------------------------------      -----
SOH      Start of Header                                          0x01
EOT      End of Transmission                                      0x04
ACK      Acknowledge                                              0x06
NAK      Not Acknowledge                                          0x15
ETB      End of Transmission Block (Return to Amulet OS mode)     0x17
CAN      Cancel (Force receiver to start sending C's)             0x18
C        ASCII "C"                                                0x43


## Workflow:

Sender Initialization: The sender waits for a 'C' from the receiver, indicating readiness.
Packet Transmission: Each packet is sent with a header, data, and CRC.
Error Handling: If a NAK is received, the packet is retransmitted.
End of Transmission: An EOT and ETB character is sent to indicate completion.
Assumptions and Design Decisions
UART as Communication Medium: This implementation assumes UART is used for serial communication.
File Transfer Initiation: The receiver initiates the transfer by sending a 'C' character to the sender.
Blocking Communication: The library employs blocking reads and writes, suitable for low-speed and reliable communication links.
Timeouts and Retries: The implementation includes basic retry mechanisms and timeout handling for improved robustness.
Dependencies
CMake (minimum version 3.10): For building the project.
Google Test (gtest): For running unit tests.
C++11 compatible compiler: Required for modern C++ features.

## Build Instructions:

Clone the repository:
git clone <repository-url>
cd XModemProject

## Build the project:

mkdir build
cd build
cmake ..
make

./bin/unit_tests
The tests will validate the functionality of the library using the Google Test framework.

./xmodem_utility send /path/to/source_file /dev/ttyUSB0
Receiving a File
To receive a file:

./xmodem_utility receive /path/to/destination_file /dev/ttyUSB0
Replace /dev/ttyUSB0 with your system's serial port.

## Troubleshooting:

Cannot open UART port: Ensure the specified serial port exists and the program has permission to access it.
NAK or CRC errors: Verify the connection quality and UART configuration (baud rate, parity, etc.).
Timeout issues: Confirm the receiver is running and communicating correctly.
