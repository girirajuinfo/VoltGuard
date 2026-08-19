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

    std::cout << "========================================\n";
    std::cout << " All parser tests passed.\n";
    std::cout << "========================================\n";

    return 0;
}
