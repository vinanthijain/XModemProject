#include "XModem.hpp"
#include "XModemProtocol.hpp"
#include "UARTSerial.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <thread>
#include <chrono>

XModem::XModem(UARTSerial& uart) : uart(uart) {}

uint16_t XModem::calculateCRC(const uint8_t* data, size_t length) {
	uint16_t crc = 0;
	for (size_t i = 0; i < length; ++i) {
		crc ^= static_cast<uint16_t>(data[i]) << 8;
		for (int j = 0; j < 8; ++j) {
			crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
		}
	}
	return crc;
}

void XModem::sendPacket(uint8_t packetNumber, const uint8_t* data) {
	std::cout << "Sending packet: " << static_cast<int>(packetNumber) << std::endl;

	uint8_t response;

	uint8_t header[3] = {
		static_cast<uint8_t>(XModemProtocol::SOH),
		packetNumber,
		static_cast<uint8_t>(~packetNumber)
	};

	std::cout << "Header: " << std::hex
	          << static_cast<int>(header[0]) << " "
	          << static_cast<int>(header[1]) << " "
	          << static_cast<int>(header[2]) << std::dec << std::endl;

	uint16_t crc = calculateCRC(data, PACKET_SIZE);
	uint8_t crcBytes[2] = {static_cast<uint8_t>(crc >> 8), static_cast<uint8_t>(crc & 0xFF)};

	uart.write(header, sizeof(header));
	uart.write(data, PACKET_SIZE);
	uart.write(crcBytes, sizeof(crcBytes));

	uart.read(&response, 1);
	std::cout << "Received response: " << std::hex << static_cast<int>(response) << std::dec << std::endl;
	if (response == static_cast<uint8_t>(XModemProtocol::NAK)) {
		std::cerr << "Error sending packet: Failed to send packet: NAK received" << std::endl;
		throw std::runtime_error("NAK received");
	}
	else if (response != static_cast<uint8_t>(XModemProtocol::ACK)) {
		throw std::runtime_error("Unexpected response received");
	}
}

int XModem::receivePacket(uint8_t expectedPacketNumber, uint8_t* data, const std::string& filePath,std::ofstream& file) {
	int retryCount = 0;
	const int maxRetries = 3;
	while (retryCount < maxRetries) {
		uint8_t header[3];
		uart.read(header, sizeof(header));
		std::cout << "Received header: "
		          << std::hex << static_cast<int>(header[0]) << " "
		          << static_cast<int>(header[1]) << " "
		          << static_cast<int>(header[2]) << std::dec << std::endl;

		if (header[0] == static_cast<uint8_t>(XModemProtocol::CAN) ||
		        (header[0] != static_cast<uint8_t>(XModemProtocol::SOH) &&
		         header[0] != static_cast<uint8_t>(XModemProtocol::ETB) &&
		         header[0] != static_cast<uint8_t>(XModemProtocol::EOT))) {
			std::cerr << "Invalid or CAN header byte received, canceling transmission." << std::endl;
			cancelTransmission(filePath,file);
			return -1;
		}

		if (header[0] == static_cast<uint8_t>(XModemProtocol::EOT)) {
			uint8_t ack = static_cast<uint8_t>(XModemProtocol::ACK);
			uart.write(&ack, 1);
			std::cout << "EOT received, acknowledged, waiting for ETB" << std::endl;
			continue;
		}

		if (header[0] == static_cast<uint8_t>(XModemProtocol::ETB)) {
			uint8_t ack = static_cast<uint8_t>(XModemProtocol::ACK);
			uart.write(&ack, 1);
			std::cout << "ETB received, transmission complete" << std::endl;
			return 0;
		}

		if (header[1] != expectedPacketNumber || header[2] != static_cast<uint8_t>(~expectedPacketNumber)) {
			std::cerr << "Packet header mismatch: expected " << static_cast<int>(expectedPacketNumber)
			          << " but got " << static_cast<int>(header[1]) << std::endl;
			uint8_t nak = static_cast<uint8_t>(XModemProtocol::NAK);
			uart.write(&nak, 1);
			retryCount++;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			if (retryCount == maxRetries) {
                //cancelTransmission(filePath,file);
				throw std::runtime_error("Packet header mismatch after maximum retries");
			}
			continue;
		}

		uart.read(data, PACKET_SIZE);

		uint8_t crcBytes[2];
		uart.read(crcBytes, 2);
		uint16_t receivedCRC = (crcBytes[0] << 8) | crcBytes[1];
		uint16_t calculatedCRC = calculateCRC(data, PACKET_SIZE);

		if (receivedCRC != calculatedCRC) {
			std::cerr << "CRC Mismatch: Expected CRC=" << calculatedCRC << ", Received CRC= " << receivedCRC << std::endl;
			uint8_t nak = static_cast<uint8_t>(XModemProtocol::NAK);
			uart.write(&nak, 1);
			retryCount++;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			if (retryCount == maxRetries) {
                //cancelTransmission(filePath,file);
				throw std::runtime_error("CRC mismatch after maximum retries");
			}
			continue;
		}

		uint8_t ack = static_cast<uint8_t>(XModemProtocol::ACK);
		std::cout << "Sending ACK from Receiver " << static_cast<int>(header[1]) << std::endl;
		uart.write(&ack, 1);
		return PACKET_SIZE;
	}

	throw std::runtime_error("Failed to receive packet after maximum retries");
}

void XModem::sendFile(const std::string& filePath) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file) {
		std::cout<<"Failed to open file for reading"<<std::endl;
        throw std::runtime_error("Failed to open file for reading");
	}

	uint8_t response;
	uart.read(&response, 1);
	if (response != 'C') {
		std::cerr << "Error: Expected 'C' from receiver but received: " << (int)response << std::endl;
		std::cout << "Receiver not ready. Expected 'C'" << std::endl;
	}

	uart.flush();

	uint8_t buffer[PACKET_SIZE] = {};
	uint8_t packetNumber = 1;

	while (file.read(reinterpret_cast<char*>(buffer), PACKET_SIZE) || file.gcount() > 0) {
		size_t bytesRead = file.gcount();
		if (bytesRead < PACKET_SIZE) {
			std::memset(buffer + bytesRead, 0, PACKET_SIZE - bytesRead);
		}
		while(true) {
			try {
				sendPacket(packetNumber, buffer);
				break;
			} catch (const std::runtime_error& e) {
				std::cerr << "Error sending packet: " << e.what() << std::endl;
				std::cout << "Resending packet number: " << (int)packetNumber << std::endl;
				uart.flush();
			}
		}
		packetNumber++;
	}

	uint8_t eot = static_cast<uint8_t>(XModemProtocol::EOT);
	std::cout << "Sending EOT " << std::endl;
	uart.write(&eot, 1);

	uart.read(&response, 1);

	std::cout << "Received response: " << std::hex << static_cast<int>(response) << std::dec << std::endl;
	if (response != static_cast<uint8_t>(XModemProtocol::ACK)) {
		throw std::runtime_error("Failed to complete file transmission");
	}

	uint8_t etb = static_cast<uint8_t>(XModemProtocol::ETB);
	std::cout << "Sending ETB " << std::endl;
	uart.write(&etb, 1);

	uart.read(&response, 1);

	std::cout << "Received response: " << std::hex << static_cast<int>(response) << std::dec << std::endl;
	if (response != static_cast<uint8_t>(XModemProtocol::ACK)) {
		throw std::runtime_error("Failed to complete file transmission");
	}

}

void XModem::cancelTransmission(const std::string& filePath,std::ofstream& file) {
    if (file.is_open()) {
        file.close();
        std::ofstream resetFile(filePath, std::ios::out | std::ios::trunc);
        resetFile.close();
        std::cout << "Partial file reset due to canceled transmission." << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    uart.flush();
    receiveFile(filePath);
}

bool XModem::waitForResponse() {
	struct timeval tv;
	fd_set readfds;

	tv.tv_sec = TIMEOUT_SECONDS;
	tv.tv_usec = 0;

	FD_ZERO(&readfds);
	FD_SET(uart.getFd(), &readfds);

	int retval = select(uart.getFd() + 1, &readfds, NULL, NULL, &tv);
	if (retval == -1) {
		perror("select()");
		return false;
	}
	return retval > 0;
}

void XModem::sendInitialC() {
	char c = 'C';
	uart.write(reinterpret_cast<uint8_t*>(&c), 1);
}

void XModem::receiveFile(const std::string& filePath) {
	std::ofstream file(filePath, std::ios::binary);
	if (!file) {
		throw std::runtime_error("Failed to open file for writing");
	}

	uint8_t buffer[PACKET_SIZE];
	uint8_t packetNumber = 1;
	uint8_t ack = static_cast<uint8_t>(XModemProtocol::ACK);

	uart.flush();

	while (true) {
		sendInitialC();
		std::cout<<"Sending C"<<std::endl;

		if (waitForResponse()) {
			if (receivePacket(packetNumber, buffer, filePath, file) == PACKET_SIZE) {
				file.write(reinterpret_cast<char*>(buffer), PACKET_SIZE);
				++packetNumber;
				break;
			} else {
				std::cerr << "Failed to receive the first packet, resending 'C'..." << std::endl;
			}
		} else {
			std::cout << "Timeout. Resending 'C'..." << std::endl;
		}
	}

	//uart.flush();
	while (receivePacket(packetNumber, buffer, filePath, file) == PACKET_SIZE) {
		file.write(reinterpret_cast<char*>(buffer), PACKET_SIZE);
		++packetNumber;
	}
}
