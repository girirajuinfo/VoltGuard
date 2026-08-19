#ifndef VOLTGUARD_MODBUS_PARSER_H
#define VOLTGUARD_MODBUS_PARSER_H

#include <cstdint>
#include <string>
#include <vector>

#include "modbus_request.h"

namespace voltguard {

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