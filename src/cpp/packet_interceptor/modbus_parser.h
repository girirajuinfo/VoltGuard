#ifndef VOLTGUARD_MODBUS_PARSER_H
#define VOLTGUARD_MODBUS_PARSER_H

#include <cstdint>
#include <string>
#include <vector>

#include "modbus_request.h"

namespace voltguard {

enum class ModbusFunction : std::uint8_t {
    ReadHoldingRegisters = 0x03,
    ReadInputRegisters = 0x04,
    WriteSingleRegister = 0x06,
    WriteMultipleRegisters = 0x10
};

class ModbusParser {
public:
    static bool parse(
        const std::vector<std::uint8_t>& data,
        ModbusRequest& request,
        const std::string& source_ip = "",
        const std::string& destination_ip = "",
        std::uint16_t source_port = 0,
        std::uint16_t destination_port = 0
    );

    static bool is_supported_function(
        std::uint8_t function_code
    );

private:
    static std::uint16_t read_uint16(
        const std::vector<std::uint8_t>& data,
        std::size_t offset
    );

    static bool validate_common_fields(
        const std::vector<std::uint8_t>& data
    );
};

} // namespace voltguard

#endif // VOLTGUARD_MODBUS_PARSER_H