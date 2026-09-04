# VoltGuard — Day 6
## Complete C++ → Python → Rust Security Pipeline

### Objective

Day 6 connects the existing VoltGuard components into a complete local security pipeline.

The final flow is:

C++ Modbus Parser
        ↓
Parsed Modbus Command
        ↓
Python Physics Engine
        ↓
Physical Safety Evaluation
        ↓
Rust Decision Engine
        ↓
ALLOW / DROP


## Architecture

The Day 6 pipeline consists of three main processing stages.

### 1. C++ Modbus Parser

The C++ parser receives test Modbus traffic and validates the packet.

It extracts fields including:

- Transaction ID
- Protocol ID
- Unit ID
- Function Code
- Address
- Quantity
- Register Value

The parser produces the validated command as JSON.


### 2. Python Physics Engine

The Python integration layer receives the parsed Modbus command.

The bridge converts the Modbus command into physical variables such as:

- Valve position
- Pump enabled state
- Pump speed

The existing physics engine then predicts:

- Flow
- Pressure
- Safety status
- Safety violations

The physics model is a simplified educational/mock model and does not represent a real industrial process.


### 3. Rust Decision Engine

The Rust decision engine receives the physics result.

Decision logic:

SAFE     → ALLOW
UNSAFE   → DROP
INVALID  → DROP

The decision engine fails closed for invalid or unknown physics results.


## Data Flow

The complete local data flow is:

Modbus Test Traffic
        ↓
C++ Modbus Parser
        ↓
JSON Parsed Command
        ↓
Python Modbus Physics Bridge
        ↓
Physics Simulation
        ↓
SAFE / UNSAFE
        ↓
Rust Decision Engine
        ↓
ALLOW / DROP


## Modbus Command Example

A normal read command:

{
    "transaction_id": 1,
    "protocol_id": 0,
    "unit_id": 1,
    "function_code": 3,
    "address": 0,
    "quantity": 2,
    "value": 0
}


## SAFE Example

The normal Modbus command is converted into:

{
    "valve_position_percent": 50.0,
    "pump_enabled": true,
    "pump_speed_rpm": 1500.0
}

Predicted physical state:

- Flow: 25.0 L/min
- Pressure: 5.5 bar
- Status: SAFE
- Violations: None

The Rust decision engine produces:

{
    "decision": "ALLOW",
    "reason": "Physics constraints satisfied",
    "severity": "NORMAL"
}


## UNSAFE Example

A Modbus write command with pump speed 3500 RPM:

{
    "transaction_id": 3,
    "protocol_id": 0,
    "unit_id": 1,
    "function_code": 6,
    "address": 32,
    "quantity": 0,
    "value": 3500
}

The Python physics engine produces:

- Pump speed: 3500 RPM
- Flow: 58.33 L/min
- Pressure: 11.5 bar
- Status: UNSAFE

Violations:

- Pump speed exceeds maximum safe limit
- Predicted pressure exceeds maximum safe limit

The Rust decision engine produces:

{
    "decision": "DROP",
    "reason": "Physics constraint violation",
    "severity": "HIGH"
}


## Invalid Input Handling

The Rust decision engine was also tested with invalid input.

Unknown status:

{
    "status": "UNKNOWN"
}

Result:

{
    "decision": "DROP",
    "reason": "Invalid or unknown physics status",
    "severity": "HIGH"
}

Invalid JSON also results in a DROP decision.

This ensures that invalid or untrusted input is not treated as safe.


## Integration Tests

The following Python tests were executed:

- Physics engine tests
- Modbus physics bridge tests
- Backend pipeline tests

Total:

24 tests passed
0 tests failed


## C++ Compilation Test

The C++ parser was compiled using:

g++ -std=c++17 -Wall -Wextra -pedantic \
src/cpp/packet_interceptor/modbus_parser.cpp \
src/cpp/packet_interceptor/main.cpp \
-o /tmp/voltguard_parser

Compilation completed successfully with no errors.


## End-to-End SAFE Test

The complete pipeline was tested:

C++ Parser
    ↓
Python Physics Engine
    ↓
Rust Decision Engine

Result:

{
    "decision": "ALLOW",
    "reason": "Physics constraints satisfied",
    "severity": "NORMAL"
}


## End-to-End UNSAFE Test

An unsafe Modbus command was passed through the complete pipeline.

Result:

{
    "decision": "DROP",
    "reason": "Physics constraint violation",
    "severity": "HIGH"
}


## Files Added

Day 6 added the following integration components:

src/python/integration/backend_pipeline.py

tests/python/test_backend_pipeline.py


## Security Properties

The Day 6 implementation provides:

- Local-only processing
- Structured JSON communication
- Physics-aware command evaluation
- Fail-closed handling of invalid input
- Separation between parser, physics engine, and decision engine
- Automated integration testing
- No communication with real industrial equipment


## Known Limitations

The current implementation is intended for educational and research purposes.

The physical model is simplified and does not represent a real industrial control system.

The current pipeline:

- Uses simulated physics
- Uses test Modbus traffic
- Does not connect to real PLCs
- Does not control real pumps or valves
- Does not perform packet dropping on a live network
- Does not include a production-grade deployment architecture


## Day 6 Completion Status

Day 6 successfully connects:

C++ Modbus Parser
        ↓
Python Physics Engine
        ↓
Rust Decision Engine

Verified results:

SAFE command   → ALLOW
UNSAFE command → DROP
INVALID input  → DROP

All 24 Python tests passed successfully.

The complete backend security pipeline is operational locally.