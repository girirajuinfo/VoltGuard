#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include "modbus_parser.h"

int main() {
    // Local test Modbus/TCP request:
    // Transaction ID = 1
    // Protocol ID    = 0
    // Length         = 6
    // Unit ID        = 1
    // Function Code  = 03
    // Address        = 0
    // Quantity       = 2

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

    std::cout << "========================================\n";
    std::cout << " VoltGuard Modbus/TCP Parser\n";
    std::cout << "========================================\n";

    if (!parsed) {
        std::cout << "Status: INVALID PACKET\n";
        return 1;
    }

    std::cout << "Source IP       : " << request.source_ip << '\n';
    std::cout << "Source Port     : " << request.source_port << '\n';
    std::cout << "Destination IP  : " << request.destination_ip << '\n';
    std::cout << "Destination Port: " << request.destination_port << '\n';

    std::cout << "Transaction ID  : " << request.transaction_id << '\n';
    std::cout << "Protocol ID     : " << request.protocol_id << '\n';
    std::cout << "Length          : " << request.length << '\n';
    std::cout << "Unit ID         : " << static_cast<int>(request.unit_id) << '\n';

    std::cout << "Function Code   : 0x"
              << std::hex
              << std::setw(2)
              << std::setfill('0')
              << static_cast<int>(request.function_code)
              << std::dec << '\n';

    std::cout << "Address         : " << request.address << '\n';
    std::cout << "Quantity        : " << request.quantity << '\n';
    std::cout << "Value           : " << request.value << '\n';

    std::cout << "Status          : "
              << (request.valid ? "VALID" : "INVALID")
              << '\n';

    std::cout << "========================================\n";

    return 0;
}