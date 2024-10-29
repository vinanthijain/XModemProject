#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MockUARTSerial.hpp"

class XModemTest : public ::testing::Test {
protected:
    MockUARTSerial mockUART;
    XModem xmodem;

    XModemTest() : xmodem(mockUART) {}
};

// Test for calculateCRC
TEST_F(XModemTest, CalculateCRC_ValidData_ReturnsCorrectCRC) {
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    uint16_t expectedCRC = 0x0D03; 
    EXPECT_EQ(xmodem.calculateCRC(data, sizeof(data)), expectedCRC);
}

// Test for sendPacket with a successful ACK response
TEST_F(XModemTest, SendPacket_ACKReceived_NoExceptionThrown) {
    uint8_t packetData[PACKET_SIZE] = {0xAA}; 
    uint8_t ackResponse = static_cast<uint8_t>(XModemProtocol::ACK);
    uint8_t header[3] = {static_cast<uint8_t>(XModemProtocol::SOH), 1, static_cast<uint8_t>(~1)};

    EXPECT_CALL(mockUART, write(_, _)).Times(3);
    
    EXPECT_CALL(mockUART, read(_, 1))
        .WillOnce([&ackResponse](unsigned char* buffer, size_t size) {
            if (size >= 1) {
                buffer[0] = ackResponse;
            }
            return 1;
        });

    EXPECT_NO_THROW(xmodem.sendPacket(1, packetData));
}

// Test for sendPacket with a NAK response
TEST_F(XModemTest, SendPacket_NAKReceived_ThrowsException) {
    uint8_t packetData[PACKET_SIZE] = {0xBB};  
    uint8_t nakResponse = static_cast<uint8_t>(XModemProtocol::NAK);
    uint8_t header[3] = {static_cast<uint8_t>(XModemProtocol::SOH), 1, static_cast<uint8_t>(~1)};

    EXPECT_CALL(mockUART, write(_, _)).Times(3);

    EXPECT_CALL(mockUART, read(_, 1))
        .WillOnce([&nakResponse](unsigned char* buffer, size_t size) {
            if (size >= 1) {
                buffer[0] = nakResponse;
            }
            return 1; 
        });

    EXPECT_THROW(xmodem.sendPacket(1, packetData), std::runtime_error);
}

// Test for receivePacket with matching packet numbers
TEST_F(XModemTest, ReceivePacket_ValidPacket_ReturnsPacketSize) {
    uint8_t header[3] = {static_cast<uint8_t>(XModemProtocol::SOH), 1, static_cast<uint8_t>(~1)};
    uint8_t data[PACKET_SIZE] = {0xAB};
    uint8_t crc[2] = {0x80, 0x0C}; 
    std::string testFilePath = "test_file.txt";
    std::ofstream testFile(testFilePath, std::ios::out | std::ios::trunc);

    EXPECT_CALL(mockUART, read(_, 3))
        .WillOnce([&header](unsigned char* buffer, long unsigned int) {
            std::copy(header, header + 3, buffer);
            return 3; 
        });

    EXPECT_CALL(mockUART, read(_, PACKET_SIZE))
        .WillOnce([&data](unsigned char* buffer, long unsigned int) {
            std::copy(data, data + PACKET_SIZE, buffer);
            return PACKET_SIZE; 
        });

    EXPECT_CALL(mockUART, read(_, 2))
        .WillOnce([&crc](unsigned char* buffer, long unsigned int) {
            std::copy(crc, crc + 2, buffer);
            return 2; 
        });

    EXPECT_CALL(mockUART, write(_, 1)).Times(1);

    uint8_t receivedData[PACKET_SIZE];
    EXPECT_EQ(xmodem.receivePacket(1, receivedData,testFilePath, testFile), PACKET_SIZE);
    testFile.close();
}

// Test for receivePacket with CRC mismatch
TEST_F(XModemTest, ReceivePacket_CRCErrors_ThrowsException) {
    uint8_t header[3] = {static_cast<uint8_t>(XModemProtocol::SOH), 1, static_cast<uint8_t>(~1)};
    uint8_t data[PACKET_SIZE] = {0xAB}; 
    //uint8_t crc[2] = {0x0D, 0x03};
    uint8_t crc[2] = {0x80, 0x0D};
    std::string testFilePath = "test_file.txt";
    std::ofstream testFile(testFilePath, std::ios::out | std::ios::trunc);

    EXPECT_CALL(mockUART, read(_, 3))
        .WillRepeatedly([&header](unsigned char* buffer, long unsigned int) {
            std::copy(header, header + 3, buffer);
            return 3; 
        });

    EXPECT_CALL(mockUART, read(_, PACKET_SIZE))
        .WillRepeatedly([&data](unsigned char* buffer, long unsigned int) {
            std::copy(data, data + PACKET_SIZE, buffer);
            return PACKET_SIZE; 
        });

    EXPECT_CALL(mockUART, read(_, 2))
        .WillRepeatedly([&crc](unsigned char* buffer, long unsigned int) {
            std::copy(crc, crc + 2, buffer);
            return 2; 
        });

    EXPECT_CALL(mockUART, write(_, 1)).Times(testing::AtLeast(1)); 

    uint8_t receivedData[PACKET_SIZE];
    EXPECT_THROW(xmodem.receivePacket(1, receivedData,testFilePath, testFile), std::runtime_error);
    testFile.close();
}