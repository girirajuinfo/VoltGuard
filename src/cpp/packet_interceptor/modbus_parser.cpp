#include "modbus_parser.h"

#include <cstddef>

namespace voltguard {

namespace {

constexpr std::size_t MIN_REQUEST_SIZE = 12;

constexpr std::uint16_t MODBUS_PROTOCOL_ID = 0;

} // namespace

std::uint16_t ModbusParser::read_uint16(
    const std::vector<std::uint8_t>& data,
    std::size_t offset
) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(data[offset]) << 8) |
        static_cast<std::uint16_t>(data[offset + 1])
    );
}

bool ModbusParser::is_supported_function(
    std::uint8_t function_code
) {
    switch (static_cast<ModbusFunction>(function_code)) {
        case ModbusFunction::ReadHoldingRegisters:
        case ModbusFunction::ReadInputRegisters:
        case ModbusFunction::WriteSingleRegister:
        case ModbusFunction::WriteMultipleRegisters:
            return true;

        default:
            return false;
    }
}

bool ModbusParser::validate_common_fields(
    const std::vector<std::uint8_t>& data
) {
    if (data.size() < MIN_REQUEST_SIZE) {
        return false;
    }

    const std::uint16_t protocol_id = read_uint16(data, 2);

    if (protocol_id != MODBUS_PROTOCOL_ID) {
        return false;
    }

    const std::uint16_t length = read_uint16(data, 4);

    const std::size_t expected_size =
        6 + static_cast<std::size_t>(length);

    if (expected_size != data.size()) {
        return false;
    }

    return true;
}

bool ModbusParser::parse(
    const std::vector<std::uint8_t>& data,
    ModbusRequest& request,
    const std::string& source_ip,
    const std::string& destination_ip,
    std::uint16_t source_port,
    std::uint16_t destination_port
) {
    request.reset();

    request.source_ip = source_ip;
    request.destination_ip = destination_ip;
    request.source_port = source_port;
    request.destination_port = destination_port;

    if (!validate_common_fields(data)) {
        return false;
    }

    request.transaction_id = read_uint16(data, 0);
    request.protocol_id = read_uint16(data, 2);
    request.length = read_uint16(data, 4);

    request.unit_id = data[6];
    request.function_code = data[7];

    if (!is_supported_function(request.function_code)) {
        return false;
    }

    switch (static_cast<ModbusFunction>(request.function_code)) {

        case ModbusFunction::ReadHoldingRegisters:
        case ModbusFunction::ReadInputRegisters: {
            request.address = read_uint16(data, 8);
            request.quantity = read_uint16(data, 10);

            if (request.quantity == 0) {
                return false;
            }

            break;
        }

        case ModbusFunction::WriteSingleRegister: {
            request.address = read_uint16(data, 8);
            request.value = read_uint16(data, 10);

            break;
        }

        case ModbusFunction::WriteMultipleRegisters: {
            request.address = read_uint16(data, 8);
            request.quantity = read_uint16(data, 10);

            const std::uint8_t byte_count = data[12];

            if (request.quantity == 0) {
                return false;
            }

            const std::size_t expected_byte_count =
                static_cast<std::size_t>(request.quantity) * 2;

            if (byte_count != expected_byte_count) {
                return false;
            }

            const std::size_t expected_size =
                13 + static_cast<std::size_t>(byte_count);

            if (data.size() != expected_size) {
                return false;
            }

            break;
        }
    }

    request.valid = true;
    return true;
}

} // namespace voltguard