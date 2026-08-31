# VoltGuard Day 5 — Rust Decision Engine

## Objective

Connect the C++ Modbus parser and Python physics engine to a Rust decision engine.

## Data Flow

Modbus Parser
→ Python Physics Engine
→ SAFE / UNSAFE
→ Rust Decision Engine
→ ALLOW / DROP

## Decision Policy

- SAFE → ALLOW
- UNSAFE → DROP
- Invalid input → DROP
- Unknown status → DROP

The engine uses a fail-closed policy.

## Testing

Rust unit tests:

- 4 passed
- 0 failed

End-to-end verification:

- SAFE command → ALLOW
- UNSAFE command → DROP
- Invalid JSON → DROP
- Unknown status → DROP

## SAFE Example

Physics result:

- Status: SAFE

Rust decision:

- Decision: ALLOW
- Severity: NORMAL

## UNSAFE Example

Physics result:

- Status: UNSAFE
- Pressure: 11.5 bar
- Pump speed: 3500 RPM

Rust decision:

- Decision: DROP
- Severity: HIGH

## Architecture

C++ Modbus Parser
→ JSON
→ Python Physics Engine
→ JSON
→ Rust Decision Engine
→ Security Decision

## Limitations

- The physics model is a simplified educational/mock model.
- No real industrial equipment is connected.
- No packet dropping is implemented yet.
- No Qt dashboard is implemented.
- No Raspberry Pi deployment is implemented.
- The Rust engine currently evaluates the physics status rather than directly controlling network traffic.

## Safety Notice

VoltGuard is an educational/mock ICS/SCADA security project.

The physics model and safety limits must not be treated as real industrial safety limits.
