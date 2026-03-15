#include "hardware/SerialLink.h"
#include <windows.h>
#include <iostream>

SerialLink::SerialLink()
    : hSerial(INVALID_HANDLE_VALUE) {}

SerialLink::~SerialLink() {
    disconnect();
}

bool SerialLink::connect(const std::string& portName, int baud) {

    HANDLE handle = CreateFileA(portName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    
    if (handle == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening serial port " << portName << std::endl;
        return false;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(handle, &dcbSerialParams)) {
        std::cerr << "Error getting serial port state" << std::endl;
        CloseHandle(handle);
        return false;
    }

    // Set serial port parameters
    dcbSerialParams.BaudRate = baud;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(handle, &dcbSerialParams)) {
        std::cerr << "Error setting serial port state" << std::endl;
        CloseHandle(handle);
        return false;
    }

    // Set timeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutConstant = 5;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    SetCommTimeouts(handle, &timeouts);

    hSerial = handle;
    std::cout << "Connected to serial port " << portName << std::endl;
    return true;
}

void SerialLink::disconnect() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        std::cout << "Serial port disconnected" << std::endl;
    }
}

bool SerialLink::isConnected() const {
    return hSerial != INVALID_HANDLE_VALUE;
}

// Reads bytes from the serial port into the provided buffer
bool SerialLink::readBytes(std::vector<uint8_t>& buffer, size_t nBytes) {
    if (hSerial == INVALID_HANDLE_VALUE) {
        return false;
    }

    buffer.resize(nBytes);
    DWORD bytesRead;
    if (!ReadFile(hSerial, buffer.data(), nBytes, &bytesRead, NULL) || bytesRead != nBytes) {
        std::cerr << "Error reading from serial port" << std::endl;
        return false;
    }

    return true;
}

bool SerialLink::sendBytes(const std::vector<uint8_t>& data) {
    if (hSerial == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(hSerial, data.data(), data.size(), &bytesWritten, NULL) || bytesWritten != data.size()) {
        std::cerr << "Error writing to serial port" << std::endl;
        return false;
    }

    return true;
}

bool SerialLink::sendRaw(const void* data, size_t nBytes) {
    if (!isConnected()) return false;

    DWORD bytesWritten = 0;
    BOOL result = WriteFile(static_cast<HANDLE>(hSerial),
                            data,
                            static_cast<DWORD>(nBytes),
                            &bytesWritten,
                            nullptr);

    if (!result || bytesWritten != nBytes) {
        std::cerr << "[SerialLink] Error writing to port (" << bytesWritten << " / "
                  << nBytes << " bytes written)" << std::endl;
        return false;
    }
    return true;
}

size_t SerialLink::readAvailable(std::vector<uint8_t>& buffer, size_t maxBytes) {
    if (!isConnected()) return 0;

    COMSTAT status;
    DWORD errors;
    ClearCommError(static_cast<HANDLE>(hSerial), &errors, &status);

    DWORD waiting = status.cbInQue;
    if (waiting == 0) {
        buffer.clear();
        return 0; // nothing to read
    }

    DWORD toRead = (waiting > maxBytes) ? (DWORD)maxBytes : waiting;
    buffer.resize(toRead);

    DWORD bytesRead = 0;
    if (!ReadFile(static_cast<HANDLE>(hSerial),
                  buffer.data(),
                  toRead,
                  &bytesRead,
                  nullptr))
    {
        std::cerr << "[SerialLink] ReadFile error\n";
        buffer.clear();
        return 0;
    }

    // Trim buffer to actual bytes read
    buffer.resize(bytesRead);
    return bytesRead;
}