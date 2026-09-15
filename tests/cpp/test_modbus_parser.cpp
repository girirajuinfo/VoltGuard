#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "../../src/cpp/packet_interceptor/modbus_parser.h"

using voltguard::ModbusParser;
using voltguard::ModbusRequest;

void test_read_holding_registers() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x01, 0x00, 0x00, 0x00, 0x06,
        0x01, 0x03, 0x00, 0x00, 0x00, 0x02
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(
        packet,
        request,
        "192.168.1.10",
        "192.168.1.20",
        54321,
        502
    );

    assert(result);
    assert(request.valid);
    assert(request.transaction_id == 1);
    assert(request.protocol_id == 0);
    assert(request.unit_id == 1);
    assert(request.function_code == 0x03);
    assert(request.address == 0);
    assert(request.quantity == 2);
    assert(request.source_ip == "192.168.1.10");
    assert(request.destination_ip == "192.168.1.20");
    assert(request.source_port == 54321);
    assert(request.destination_port == 502);

    std::cout << "[PASS] Read Holding Registers\n";
}

void test_read_input_registers() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x02, 0x00, 0x00, 0x00, 0x06,
        0x01, 0x04, 0x00, 0x10, 0x00, 0x03
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(result);
    assert(request.valid);
    assert(request.function_code == 0x04);
    assert(request.address == 0x10);
    assert(request.quantity == 3);

    std::cout << "[PASS] Read Input Registers\n";
}

void test_write_single_register() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x03, 0x00, 0x00, 0x00, 0x06,
        0x01, 0x06, 0x00, 0x20, 0x00, 0x64
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(result);
    assert(request.valid);
    assert(request.function_code == 0x06);
    assert(request.address == 0x20);
    assert(request.value == 100);

    std::cout << "[PASS] Write Single Register\n";
}

void test_write_multiple_registers() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x04, 0x00, 0x00, 0x00, 0x0B,
        0x01, 0x10, 0x00, 0x30, 0x00, 0x02,
        0x04, 0x00, 0x64, 0x00, 0xC8
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(result);
    assert(request.valid);
    assert(request.function_code == 0x10);
    assert(request.address == 0x30);
    assert(request.quantity == 2);

    std::cout << "[PASS] Write Multiple Registers\n";
}

void test_incomplete_packet() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x05, 0x00, 0x00, 0x00
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(!result);
    assert(!request.valid);

    std::cout << "[PASS] Incomplete packet rejected\n";
}

void test_malformed_packet() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x06, 0x00, 0x00, 0x00, 0x06,
        0x01, 0x03
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(!result);
    assert(!request.valid);

    std::cout << "[PASS] Malformed packet rejected\n";
}

void test_invalid_protocol_id() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x07, 0x00, 0x01, 0x00, 0x06,
        0x01, 0x03, 0x00, 0x00, 0x00, 0x02
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(!result);
    assert(!request.valid);

    std::cout << "[PASS] Invalid protocol ID rejected\n";
}

void test_supported_function_codes() {
    assert(ModbusParser::is_supported_function(0x03));
    assert(ModbusParser::is_supported_function(0x04));
    assert(ModbusParser::is_supported_function(0x06));
    assert(ModbusParser::is_supported_function(0x10));

    std::cout << "[PASS] Supported function codes recognized\n";
}
void test_unsupported_function_code() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x08, 0x00, 0x00, 0x00, 0x06,
        0x01, 0x05, 0x00, 0x00, 0x00, 0x01
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(!result);
    assert(!request.valid);
    assert(
        ModbusParser::get_last_error() ==
        voltguard::ModbusParseError::UnsupportedFunctionCode
    );

    std::cout << "[PASS] Unsupported function code rejected\n";
}

void test_invalid_length() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x09, 0x00, 0x00, 0x00, 0x07,
        0x01, 0x03, 0x00, 0x00, 0x00, 0x02
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(!result);
    assert(!request.valid);
    assert(
        ModbusParser::get_last_error() ==
        voltguard::ModbusParseError::InvalidLength
    );

    std::cout << "[PASS] Invalid length rejected\n";
}

void test_invalid_quantity() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x0A, 0x00, 0x00, 0x00, 0x06,
        0x01, 0x03, 0x00, 0x00, 0x00, 0x00
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(!result);
    assert(!request.valid);
    assert(
        ModbusParser::get_last_error() ==
        voltguard::ModbusParseError::InvalidQuantity
    );

    std::cout << "[PASS] Invalid quantity rejected\n";
}

void test_invalid_byte_count() {
    const std::vector<std::uint8_t> packet = {
    0x00, 0x0B, 0x00, 0x00, 0x00, 0x09,
    0x01, 0x10, 0x00, 0x30, 0x00, 0x02,
    0x02, 0x00, 0x64
};

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(!result);
    assert(!request.valid);
    assert(
        ModbusParser::get_last_error() ==
        voltguard::ModbusParseError::InvalidByteCount
    );

    std::cout << "[PASS] Invalid byte count rejected\n";
}

void test_error_message() {
    assert(
        std::string(
            ModbusParser::error_message(
                voltguard::ModbusParseError::PacketTooShort
            )
        ) == "Packet is too short"
    );

    assert(
        std::string(
            ModbusParser::error_message(
                voltguard::ModbusParseError::UnsupportedFunctionCode
            )
        ) == "Unsupported Modbus function code"
    );

    std::cout << "[PASS] Parser error messages verified\n";
}

void test_transaction_id() {
    const std::vector<std::uint8_t> packet = {
        0x12, 0x34, 0x00, 0x00, 0x00, 0x06,
        0x01, 0x03, 0x00, 0x00, 0x00, 0x02
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(result);
    assert(request.transaction_id == 0x1234);

    std::cout << "[PASS] Transaction ID extracted correctly\n";
}

void test_unit_id() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x20, 0x00, 0x00, 0x00, 0x06,
        0x05, 0x03, 0x00, 0x10, 0x00, 0x02
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(result);
    assert(request.unit_id == 0x05);

    std::cout << "[PASS] Unit ID extracted correctly\n";
}

void test_address_extraction() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x21, 0x00, 0x00, 0x00, 0x06,
        0x01, 0x03, 0x12, 0x34, 0x00, 0x02
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(result);
    assert(request.address == 0x1234);
    assert(request.quantity == 2);

    std::cout << "[PASS] Address and quantity extracted correctly\n";
}

void test_write_value_extraction() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x22, 0x00, 0x00, 0x00, 0x06,
        0x01, 0x06, 0x00, 0x20, 0x00, 0x64
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(result);
    assert(request.address == 0x0020);
    assert(request.value == 0x0064);

    std::cout << "[PASS] Write register address/value extracted correctly\n";
}
void test_incomplete_payload() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x23, 0x00, 0x00, 0x00, 0x06,
        0x01, 0x03, 0x00
    };

    ModbusRequest request;

    const bool result = ModbusParser::parse(packet, request);

    assert(!result);
    assert(!request.valid);

    std::cout << "[PASS] Incomplete function payload rejected\n";
}
int main() {
    std::cout << "========================================\n";
    std::cout << " VoltGuard Modbus Parser Tests\n";
    std::cout << "========================================\n";

    test_read_holding_registers();
    test_read_input_registers();
    test_write_single_register();
    test_write_multiple_registers();
    test_incomplete_packet();
    test_malformed_packet();
    test_invalid_protocol_id();
    test_supported_function_codes();
    test_unsupported_function_code();
    test_invalid_length();
    test_invalid_quantity();
    test_invalid_byte_count();
    test_error_message();
    test_transaction_id();
    test_unit_id();
    test_address_extraction();
    test_write_value_extraction();
    test_incomplete_payload();

    std::cout << "========================================\n";
    std::cout << " All parser tests passed.\n";
    std::cout << "========================================\n";

    return 0;
}
