# VoltGuard

**Physics-Aware ICS/SCADA Security System**

VoltGuard is an educational cybersecurity project designed to
analyze industrial control commands and evaluate their possible
physical consequences before a security decision is made.

The project combines:

- Modbus/TCP packet parsing
- Physics-based process simulation
- Safety-limit evaluation
- Future decision-engine integration
- Future security monitoring and response

> **Important:** VoltGuard is a research/educational prototype.
> It is not a real industrial safety system and must not be used
> to control or protect real industrial equipment.

---

## Project Architecture

The planned VoltGuard pipeline is:

```text
Modbus/TCP Traffic
        |
        v
C++ Packet Parser
        |
        v
Parsed Modbus Command
        |
        v
Python Physics Engine
        |
        v
Predicted Physical State
        |
        v
Safety Evaluation
        |
        v
SAFE / UNSAFE
        |
        v
Future Decision Engine