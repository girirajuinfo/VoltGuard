# VoltGuard — Day 7 Backend Security Test Matrix

## Purpose

This test matrix defines the controlled local tests used to validate the VoltGuard backend.

All tests use simulated or mock data.

No real PLC, SCADA device, industrial controller, or physical equipment is involved.

The objective is to verify that the backend:

- Accepts valid safe commands
- Rejects unsafe commands
- Rejects malformed input
- Rejects invalid values
- Handles boundary conditions correctly
- Fails closed when invalid data is received
- Produces deterministic results

---

## Test Matrix

| Test ID | Test Case | Input | Expected Result |
|---|---|---|---|
| TEST-01 | Valid SAFE Modbus request | Function code 0x03, normal read | Parser valid → Physics SAFE → ALLOW |
| TEST-02 | Valid UNSAFE Modbus request | Function code 0x06, value 3500 | Parser valid → Physics UNSAFE → DROP |
| TEST-03 | Malformed Modbus request | Invalid/truncated packet | Parser rejects → no ALLOW |
| TEST-04 | Invalid function code | Unsupported function code | Reject or safely handle → no ALLOW |
| TEST-05 | Invalid address/value | Invalid physical/register input | Validation failure → DROP/REJECT |
| TEST-06 | Physics boundary condition | Value exactly at configured safe limit | Result follows configured boundary rule |
| TEST-07 | Physics value beyond limit | Value above configured safe limit | Physics UNSAFE → DROP |
| TEST-08 | Invalid JSON | Malformed JSON input | DROP/REJECT |
| TEST-09 | Missing required field | Required field removed | DROP/REJECT |
| TEST-10 | Unknown physics status | UNKNOWN status | Rust decision → DROP |

---

## Expected Security Behavior

### SAFE

```text
SAFE
 ↓
ALLOW