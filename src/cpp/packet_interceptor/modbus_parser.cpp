#include "modbus_parser.h"

#include <cstddef>

namespace voltguard {

namespace {

constexpr std::size_t MIN_REQUEST_SIZE = 8;

constexpr std::uint16_t MODBUS_PROTOCOL_ID = 0;

ModbusParseError last_error = ModbusParseError::None;

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

void ModbusParser::set_error(ModbusParseError error) {
    last_error = error;
}

ModbusParseError ModbusParser::get_last_error() {
    return last_error;
}

const char* ModbusParser::error_message(
    ModbusParseError error
) {
    switch (error) {
        case ModbusParseError::None:
            return "No error";

        case ModbusParseError::PacketTooShort:
            return "Packet is too short";

        case ModbusParseError::InvalidProtocolId:
            return "Invalid Modbus protocol ID";

        case ModbusParseError::InvalidLength:
            return "Invalid Modbus length field";

        case ModbusParseError::MissingFunctionCode:
            return "Function code is missing";

        case ModbusParseError::UnsupportedFunctionCode:
            return "Unsupported Modbus function code";

        case ModbusParseError::InvalidPayload:
            return "Invalid function payload";

        case ModbusParseError::InvalidQuantity:
            return "Invalid register quantity";

        case ModbusParseError::InvalidByteCount:
            return "Invalid byte count";

        default:
            return "Unknown parser error";
    }
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

ModbusParseError ModbusParser::validate_packet(
    const std::vector<std::uint8_t>& data
) {
    // MBAP header requires at least 7 bytes:
    //
    // Transaction ID = 2
    // Protocol ID    = 2
    // Length         = 2
    // Unit ID        = 1
    //
    // The function code is byte 7, so an additional byte
    // is required before we can inspect it.
    if (data.size() < MIN_REQUEST_SIZE) {
        return ModbusParseError::PacketTooShort;
    }

    const std::uint16_t protocol_id =
        read_uint16(data, 2);

    if (protocol_id != MODBUS_PROTOCOL_ID) {
        return ModbusParseError::InvalidProtocolId;
    }

    const std::uint16_t length =
        read_uint16(data, 4);

    const std::size_t expected_size =
        6 + static_cast<std::size_t>(length);

    if (expected_size != data.size()) {
        return ModbusParseError::InvalidLength;
    }

    if (data.size() < 8) {
        return ModbusParseError::MissingFunctionCode;
    }

    const std::uint8_t function_code = data[7];

    if (!is_supported_function(function_code)) {
        return ModbusParseError::UnsupportedFunctionCode;
    }

    switch (static_cast<ModbusFunction>(function_code)) {

        case ModbusFunction::ReadHoldingRegisters:
        case ModbusFunction::ReadInputRegisters: {
            if (data.size() < 12) {
                return ModbusParseError::InvalidPayload;
            }

            const std::uint16_t quantity =
                read_uint16(data, 10);

            if (quantity == 0) {
                return ModbusParseError::InvalidQuantity;
            }

            if (length != 6) {
                return ModbusParseError::InvalidPayload;
            }

            break;
        }

        case ModbusFunction::WriteSingleRegister: {
            if (data.size() < 12) {
                return ModbusParseError::InvalidPayload;
            }

            if (length != 6) {
                return ModbusParseError::InvalidPayload;
            }

            break;
        }

        case ModbusFunction::WriteMultipleRegisters: {
            if (data.size() < 13) {
                return ModbusParseError::InvalidPayload;
            }

            const std::uint16_t quantity =
                read_uint16(data, 10);

            if (quantity == 0) {
                return ModbusParseError::InvalidQuantity;
            }

            const std::uint8_t byte_count = data[12];

            const std::size_t expected_byte_count =
                static_cast<std::size_t>(quantity) * 2;

            if (byte_count != expected_byte_count) {
                return ModbusParseError::InvalidByteCount;
            }

            const std::size_t expected_size =
                13 + static_cast<std::size_t>(byte_count);

            if (data.size() != expected_size) {
                return ModbusParseError::InvalidPayload;
            }

            break;
        }
    }

    return ModbusParseError::None;
}

bool ModbusParser::validate_common_fields(
    const std::vector<std::uint8_t>& data
) {
    return validate_packet(data) == ModbusParseError::None;
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
    set_error(ModbusParseError::None);

    request.source_ip = source_ip;
    request.destination_ip = destination_ip;
    request.source_port = source_port;
    request.destination_port = destination_port;

    const ModbusParseError validation_error =
        validate_packet(data);

    if (validation_error != ModbusParseError::None) {
        set_error(validation_error);
        return false;
    }

    request.transaction_id = read_uint16(data, 0);
    request.protocol_id = read_uint16(data, 2);
    request.length = read_uint16(data, 4);

    request.unit_id = data[6];
    request.function_code = data[7];

    switch (static_cast<ModbusFunction>(request.function_code)) {

        case ModbusFunction::ReadHoldingRegisters:
        case ModbusFunction::ReadInputRegisters:
            request.address = read_uint16(data, 8);
            request.quantity = read_uint16(data, 10);
            break;

        case ModbusFunction::WriteSingleRegister:
            request.address = read_uint16(data, 8);
            request.value = read_uint16(data, 10);
            break;

        case ModbusFunction::WriteMultipleRegisters:
            request.address = read_uint16(data, 8);
            request.quantity = read_uint16(data, 10);
            break;
    }

    request.valid = true;
    return true;
}

} // namespace voltguard