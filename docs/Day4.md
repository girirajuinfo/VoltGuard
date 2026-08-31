# VoltGuard Day 4 — C++ to Python Integration

## Objective

Connect the C++ Modbus/TCP parser with the Python physics engine.

## Data Flow

Modbus/TCP test traffic
→ C++ Modbus parser
→ JSON output
→ Python integration bridge
→ Physics Engine
→ SAFE / UNSAFE

## Integration Method

The integration uses local JSON over standard input/output.

The C++ parser produces a JSON representation of the parsed Modbus command. The Python bridge reads this JSON and converts the command into the existing `PipelineCommand` structure.

No network traffic is generated.

## Supported Mapping

| Modbus Function | Physical Interpretation |
|---|---|
| 0x03 Read Holding Registers | Normal operating command |
| 0x04 Read Input Registers | Normal operating command |
| 0x06 Write Single Register | Register value used as pump speed |
| 0x10 Write Multiple Registers | Register value used as pump speed |

The bridge currently uses a fixed 50% valve position for the simplified demonstration model.

## SAFE Example

A Modbus function code 0x03 command produces:

- Pump speed: 1500 RPM
- Flow: 25 L/min
- Pressure: 5.5 bar
- Status: SAFE

## UNSAFE Example

A function code 0x06 command with value 3500 produces:

- Pump speed: 3500 RPM
- Flow: approximately 58.33 L/min
- Pressure: 11.5 bar
- Status: UNSAFE

Violations include excessive pump speed and excessive pressure.

## Testing

Python physics engine tests:

- 15 tests passed

Integration bridge tests:

- 5 tests passed

Total:

- 20 tests passed

The complete local C++ → Python pipeline was also tested successfully for SAFE and UNSAFE cases.

## Known Limitations

- The physics model is simplified and educational.
- The valve position is currently fixed at 50% by the bridge.
- JSON output is generated directly by the C++ demonstration program.
- No real industrial equipment is connected.
- No packet dropping or blocking is implemented.
- No Rust decision engine is implemented yet.
- No Qt dashboard is implemented yet.

## Safety Notice

This is a mock educational ICS/SCADA security project.

The physics values and safety limits are demonstration values only and must not be treated as real industrial safety limits.
