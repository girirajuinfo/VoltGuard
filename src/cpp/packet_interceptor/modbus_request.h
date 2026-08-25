#ifndef VOLTGUARD_MODBUS_REQUEST_H
#define VOLTGUARD_MODBUS_REQUEST_H

#include <cstdint>
#include <string>

namespace voltguard {

struct ModbusRequest {
    // Network information
    std::string source_ip;
    std::string destination_ip;

    std::uint16_t source_port = 0;
    std::uint16_t destination_port = 0;

    // Modbus/TCP MBAP header
    std::uint16_t transaction_id = 0;
    std::uint16_t protocol_id = 0;
    std::uint16_t length = 0;

    // Modbus protocol fields
    std::uint8_t unit_id = 0;
    std::uint8_t function_code = 0;

    // Request data
    std::uint16_t address = 0;
    std::uint16_t quantity = 0;
    std::uint16_t value = 0;

    // Parser state
    bool valid = false;

    // Reset the request to a known initial state.
    void reset() {
        *this = ModbusRequest{};
    }
};

} // namespace voltguard

#endif // VOLTGUARD_MODBUS_REQUEST_H