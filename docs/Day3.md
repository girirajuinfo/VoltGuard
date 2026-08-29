# VoltGuard Day 3 — Physics Engine

## 1. Purpose

Day 3 introduces the initial Python Physics Engine for VoltGuard.

The engine provides a simplified educational model of an industrial
pipeline. It predicts basic physical conditions from an industrial
command and determines whether the predicted state is SAFE or UNSAFE.

The conceptual flow is:

Modbus Command
        ↓
Parsed Command
        ↓
Physics Engine
        ↓
Predicted Physical State
        ↓
Safety Evaluation
        ↓
SAFE / UNSAFE

This implementation is a mock educational model and is NOT intended
for controlling or protecting a real industrial plant.

---

## 2. Implementation Location

The main physics engine is located at:

src/python/physics_engine/physics_engine.py

Configuration is stored at:

configs/physics_limits.json

Python tests are located at:

tests/python/test_physics_engine.py

---

## 3. Pipeline Variables

The model uses the following variables.

### Valve Position

Unit: percent (%)

Valid range:

0% to 100%

0% represents a closed valve.

100% represents a fully open valve.

### Pump Speed

Unit: revolutions per minute (RPM)

The pump speed must not be negative.

### Pump State

The pump can be:

- Enabled
- Disabled

A disabled pump must have 0 RPM.

### Flow Rate

Unit: litres per minute (L/min)

The engine predicts flow from pump speed and valve position.

### Pressure

Unit: bar

The engine predicts pressure from pump speed.

---

## 4. Mock Model

The model uses simple relationships.

### Valve Factor

Valve factor is calculated as:

Valve Position / 100

Example:

50% valve = 0.5

### Pump Speed Factor

Pump speed factor is calculated as:

Pump Speed / Reference Pump Speed

The configured reference pump speed is:

3000 RPM

### Flow

The simplified flow equation is:

Flow =
Pump Speed Factor
× Valve Factor
× Reference Flow

The configured reference flow is:

100 L/min.

Example:

Pump speed = 1500 RPM
Valve position = 50%

Pump speed factor = 1500 / 3000 = 0.5

Valve factor = 50 / 100 = 0.5

Flow = 0.5 × 0.5 × 100

Flow = 25 L/min

### Pressure

The simplified pressure equation is:

Pressure =
Base Pressure
+
Pump Speed Factor
× Pressure Rise

Configured values:

Base pressure = 1 bar

Pressure rise = 9 bar

Example:

Pump speed = 1500 RPM

Pressure = 1 + (1500 / 3000 × 9)

Pressure = 5.5 bar

---

## 5. Mock Safety Limits

The current demonstration limits are:

| Parameter | Mock Limit |
|---|---:|
| Minimum pressure | 1 bar |
| Maximum pressure | 10 bar |
| Maximum flow | 100 L/min |
| Maximum pump speed | 3000 RPM |

These values are MOCK/DEMO values only.

They are not real industrial safety limits and must not be
used for real plant operation.

---

## 6. Safety Evaluation

The physics engine evaluates the predicted state against the
configured safety limits.

The engine can detect:

- Pump speed above the maximum limit
- Predicted flow above the maximum limit
- Predicted pressure above the maximum limit
- Predicted pressure below the minimum limit

If one or more violations exist:

Status = UNSAFE

If no violations exist:

Status = SAFE

Multiple violations can be reported at the same time.

---

## 7. Input Validation

The engine rejects invalid command values before simulation.

Validation includes:

- Valve position must be between 0% and 100%
- Valve position must be finite
- Pump speed must be finite
- Pump speed cannot be negative
- Pump enabled state must be True or False
- A disabled pump must have 0 RPM

This prevents invalid values such as NaN, infinity, negative
pump speed, and contradictory pump states from entering the
simulation.

---

## 8. Structured Result

The engine returns a PipelineState object containing:

- Valve position
- Pump state
- Pump speed
- Predicted flow
- Predicted pressure
- Safety status
- Safety violations

Example:

{
    "status": "UNSAFE",
    "violations": [
        "Pump speed exceeds maximum safe limit",
        "Predicted pressure exceeds maximum safe limit"
    ]
}

---

## 9. Testing

The Python test suite contains 15 tests.

The tests cover:

1. Normal operating condition
2. High pump speed
3. Excessive pressure
4. Excessive flow
5. Negative pump speed
6. Invalid valve position
7. Safe pump-speed boundary
8. Pump-disabled condition
9. NaN pump speed
10. Infinite pump speed
11. Disabled pump with non-zero speed
12. Zero valve position
13. Full valve position
14. Zero pump speed
15. NaN valve position

All tests passed during Day 3 development.

Test command:

python3 -m unittest tests.python.test_physics_engine -v

Result:

Ran 15 tests

OK

---

## 10. Demonstration

The normal demonstration uses:

Valve position: 50%

Pump enabled: True

Pump speed: 1500 RPM

Predicted flow: 25 L/min

Predicted pressure: 5.5 bar

Result:

SAFE

---

## 11. Known Limitations

This is a simplified educational model.

It does not represent:

- Real fluid dynamics
- Real pump curves
- Real valve characteristics
- Pipe friction
- Temperature effects
- Fluid viscosity
- Transient pressure behavior
- Real SCADA process dynamics
- Real industrial safety requirements

The equations are intentionally simple so that the system can
be understood, tested, and extended during later project stages.

---

## 12. Day 4 Direction

The next stage will build on the physics engine.

Potential future work includes connecting parsed Modbus commands
to the physics evaluation layer and preparing the result for the
VoltGuard decision-making pipeline.

The Rust decision engine, packet dropping, inline IPS,
Qt dashboard, and real industrial hardware are NOT implemented
in Day 3.