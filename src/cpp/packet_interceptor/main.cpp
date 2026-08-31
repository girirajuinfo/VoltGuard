#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include "modbus_parser.h"

int main() {
    const std::vector<std::uint8_t> packet = {
        0x00, 0x01, 0x00, 0x00, 0x00, 0x06,
        0x01, 0x03, 0x00, 0x00, 0x00, 0x02
    };

    voltguard::ModbusRequest request;

    const bool parsed = voltguard::ModbusParser::parse(
        packet,
        request,
        "192.168.1.10",
        "192.168.1.20",
        54321,
        502
    );

    if (!parsed) {
        std::cerr << "ERROR: Modbus packet rejected\n";
        return 1;
    }

    // JSON output for the local Python integration bridge.
    std::cout << "{"
              << "\"transaction_id\":" << request.transaction_id << ","
              << "\"protocol_id\":" << request.protocol_id << ","
              << "\"unit_id\":" << static_cast<int>(request.unit_id) << ","
              << "\"function_code\":" << static_cast<int>(request.function_code) << ","
              << "\"address\":" << request.address << ","
              << "\"quantity\":" << request.quantity << ","
              << "\"value\":" << request.value
              << "}\n";

    return 0;
}