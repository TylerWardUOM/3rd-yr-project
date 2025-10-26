#pragma once
#include <string>
#include <vector>
#include <cstdint>

class SerialLink {
public:
    SerialLink();
    ~SerialLink();

    bool connect(const std::string& portName, int baud = 115200);
    void disconnect();
    bool isConnected() const;

    bool sendBytes(const std::vector<uint8_t>& data);
    bool readBytes(std::vector<uint8_t>& buffer, size_t nBytes);
    bool sendRaw(const void* data, size_t nBytes);

private:
    void* hSerial;  ///< Windows HANDLE stored as void* for portability
};
