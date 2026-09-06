#include <cstdint>
#include <iostream>
#include <vector>

#include "../../src/cpp/packet_interceptor/modbus_parser.h"

using voltguard::ModbusParser;
using voltguard::ModbusRequest;

struct TestCase {
    const char* name;
    std::vector<std::uint8_t> packet;
    bool expected_valid;
};

int main() {
    const std::vector<std::uint8_t> valid_read = {
        0x00, 0x01,
        0x00, 0x00,
        0x00, 0x06,
        0x01,
        0x03,
        0x00, 0x00,
        0x00, 0x02
    };

    const std::vector<std::uint8_t> invalid_protocol = {
        0x00, 0x01,
        0x00, 0x01,
        0x00, 0x06,
        0x01,
        0x03,
        0x00, 0x00,
        0x00, 0x02
    };

    const std::vector<std::uint8_t> invalid_function = {
        0x00, 0x01,
        0x00, 0x00,
        0x00, 0x06,
        0x01,
        0x05,
        0x00, 0x00,
        0x00, 0x02
    };

    const std::vector<std::uint8_t> zero_quantity = {
        0x00, 0x01,
        0x00, 0x00,
        0x00, 0x06,
        0x01,
        0x03,
        0x00, 0x00,
        0x00, 0x00
    };

    const std::vector<std::uint8_t> truncated_packet = {
        0x00, 0x01,
        0x00, 0x00,
        0x00, 0x06,
        0x01
    };

    const std::vector<std::uint8_t> invalid_length = {
        0x00, 0x01,
        0x00, 0x00,
        0x00, 0x07,
        0x01,
        0x03,
        0x00, 0x00,
        0x00, 0x02
    };

    const std::vector<std::uint8_t> empty_packet = {};

    const std::vector<std::uint8_t> valid_write_multiple = {
        0x00, 0x02,
        0x00, 0x00,
        0x00, 0x0B,
        0x01,
        0x10,
        0x00, 0x20,
        0x00, 0x02,
        0x04,
        0x00, 0x01,
        0x00, 0x02
    };

    const std::vector<std::uint8_t> zero_write_quantity = {
        0x00, 0x02,
        0x00, 0x00,
        0x00, 0x07,
        0x01,
        0x10,
        0x00, 0x20,
        0x00, 0x00,
        0x00
    };

    const std::vector<std::uint8_t> invalid_byte_count = {
        0x00, 0x02,
        0x00, 0x00,
        0x00, 0x0B,
        0x01,
        0x10,
        0x00, 0x20,
        0x00, 0x02,
        0x03,
        0x00, 0x01,
        0x00, 0x02
    };

    const std::vector<TestCase> tests = {
        {"valid read request", valid_read, true},
        {"invalid protocol ID", invalid_protocol, false},
        {"unsupported function code", invalid_function, false},
        {"zero register quantity", zero_quantity, false},
        {"truncated packet", truncated_packet, false},
        {"invalid length", invalid_length, false},
        {"empty packet", empty_packet, false},
        {"valid write multiple request", valid_write_multiple, true},
        {"zero write quantity", zero_write_quantity, false},
        {"invalid byte count", invalid_byte_count, false}
    };

    int failures = 0;

    for (const auto& test : tests) {
        ModbusRequest request;

        const bool result = ModbusParser::parse(
            test.packet,
            request,
            "127.0.0.1",
            "127.0.0.1",
            54321,
            502
        );

        const bool passed = (result == test.expected_valid);

        std::cout
            << (passed ? "PASS" : "FAIL")
            << " | "
            << test.name;

        if (!result) {
            std::cout
                << " | error: "
                << ModbusParser::error_message(
                    ModbusParser::get_last_error()
                );
        }

        std::cout << '\n';

        if (!passed) {
            ++failures;
        }
    }

    std::cout << "\nC++ parser robustness tests: "
              << (failures == 0 ? "PASS" : "FAIL")
              << '\n';

    return failures == 0 ? 0 : 1;
}