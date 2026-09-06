# VoltGuard — Day 8 Integration & Security Evidence

## Objective

Day 8 focuses on demonstrating the complete VoltGuard backend security pipeline.

The system processes a simulated Modbus command through three security layers:

C++ Modbus Parser
        ↓
Python Physics Engine
        ↓
Rust Decision Engine
        ↓
Final Security Decision

All testing is performed locally using simulated data.

No real PLC, SCADA device, industrial controller, or physical equipment is involved.

---

## Integration Flow

### 1. C++ Modbus Parser

The parser receives a simulated Modbus request and validates:

- Transaction ID
- Protocol ID
- Unit ID
- Function code
- Register address
- Register quantity
- Command value

A valid request is converted into structured JSON.

### 2. Python Physics Engine

The Python layer converts the parsed Modbus command into a physical command.

The physics engine calculates:

- Pump state
- Pump speed
- Valve position
- Predicted flow
- Predicted pressure

The calculated state is compared against the configured physics limits.

### 3. Rust Decision Engine

The Rust decision engine receives the physics status.

Decision policy:

SAFE → ALLOW

UNSAFE → DROP

Unknown or invalid status → DROP

Invalid input → DROP

This provides fail-closed security behavior.

---

## Successful SAFE Flow

Example:

```text
Modbus Request
      ↓
C++ Parser
      ↓
Valid Command
      ↓
Python Physics Engine
      ↓
Physics Status: SAFE
      ↓
Rust Decision Engine
      ↓
ALLOW