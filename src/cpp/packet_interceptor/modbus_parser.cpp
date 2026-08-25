#include "modbus_parser.h"

#include <cstddef>

namespace voltguard {

namespace {

// Minimum Modbus/TCP request size:
//
// MBAP Header = 7 bytes
// PDU minimum = 5 bytes
//
// Total = 12 bytes
constexpr std::size_t MIN_REQUEST_SIZE = 12;

constexpr std::uint16_t MODBUS_PROTOCOL_ID = 0;

constexpr std::uint8_t READ_HOLDING_REGISTERS = 0x03;
constexpr std::uint8_t READ_INPUT_REGISTERS = 0x04;
constexpr std::uint8_t WRITE_SINGLE_REGISTER = 0x06;
constexpr std::uint8_t WRITE_MULTIPLE_REGISTERS = 0x10;

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

    // The length field describes the bytes after the MBAP
    // transaction ID, protocol ID, and length fields.
    //
    // A complete request must contain:
    // 6 bytes of MBAP information after the transaction ID
    // plus the PDU bytes represented by the length field.
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

    switch (request.function_code) {

        case READ_HOLDING_REGISTERS:
        case READ_INPUT_REGISTERS: {
            request.address = read_uint16(data, 8);
            request.quantity = read_uint16(data, 10);

            if (request.quantity == 0) {
                return false;
            }

            break;
        }

        case WRITE_SINGLE_REGISTER: {
            request.address = read_uint16(data, 8);
            request.value = read_uint16(data, 10);

            break;
        }

        case WRITE_MULTIPLE_REGISTERS: {
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

        default:
            return false;
    }

    request.valid = true;
    return true;
}

} // namespace voltguard
