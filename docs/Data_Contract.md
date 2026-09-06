# VoltGuard Backend Data Contract

## Purpose

This document defines the structured data exchanged between the VoltGuard
backend components.

The current backend uses JSON-compatible structured data between the
Python and Rust components.

All data is generated from local simulated/test traffic.

---

## 1. C++ Parser → Python

The C++ Modbus parser produces a normalized command containing:

- transaction_id
- protocol_id
- unit_id
- function_code
- address
- quantity
- value

Example:

{
  "transaction_id": 1,
  "protocol_id": 0,
  "unit_id": 1,
  "function_code": 3,
  "address": 0,
  "quantity": 2,
  "value": 0
}

The parser is responsible for Modbus packet validation and parsing.

---

## 2. Python Physics Layer

The Python integration layer converts the normalized Modbus command into
physics-engine input.

The physics result contains:

- command
- physical_input
- physical_state
- status
- violations

Example status:

SAFE

or:

UNSAFE

Invalid physical input must not be classified as SAFE.

---

## 3. Python → Rust

The Rust decision engine receives the security-relevant physics status.

Current input:

{
  "status": "SAFE"
}

Valid status values are:

- SAFE
- UNSAFE

Unexpected or missing status values are treated as invalid.

---

## 4. Rust Decision Result

The Rust decision engine returns:

- decision
- reason
- severity

Example:

{
  "decision": "ALLOW",
  "reason": "Physics constraints satisfied",
  "severity": "NORMAL"
}

For unsafe or invalid conditions:

{
  "decision": "DROP",
  "reason": "...",
  "severity": "HIGH"
}

---

## 5. Security Policy

The decision policy is fail-closed.

SAFE:

SAFE → ALLOW

UNSAFE:

UNSAFE → DROP

INVALID:

INVALID → DROP

UNKNOWN:

UNKNOWN → DROP

Missing or malformed decision input must never produce ALLOW.

---

## 6. Component Responsibilities

C++ Parser
- Parse Modbus test traffic
- Validate packet structure
- Produce normalized command data

Python Physics Engine
- Evaluate physical conditions
- Calculate simulated flow and pressure
- Report SAFE or UNSAFE
- Report physical violations

Rust Decision Engine
- Apply the security decision policy
- ALLOW safe conditions
- DROP unsafe or invalid conditions

Reports
- Store structured security results when reporting is implemented

---

## 7. Scope and Limitations

VoltGuard uses a simplified educational/mock physics model.

The configured physical limits are demonstration values and must not be
interpreted as real industrial safety limits.

The current project does not implement:

- Real PLC communication
- Real packet interception
- Real packet dropping
- Production ICS safety controls
