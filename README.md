# VoltGuard

## Physics-Aware ICS/SCADA Security System

VoltGuard is a cybersecurity research and demonstration project designed to protect Industrial Control System (ICS) and SCADA environments by combining **Modbus/TCP traffic analysis**, **physics-aware validation**, and **security decision-making**.

Traditional network security mechanisms can determine whether a Modbus request is valid at the protocol level, but a valid command can still cause an unsafe physical condition.

VoltGuard addresses this problem by evaluating both:

1. The security and validity of the Modbus/TCP request
2. The expected physical impact of the command

The system then makes a security decision:

```text
Modbus/TCP Command
        ↓
C++ Modbus Parser
        ↓
Physics Validation
        ↓
Security Decision Engine
        ↓
   ┌────┴────┐
   ↓         ↓
 ALLOW      DROP
```

VoltGuard is designed as a **local, controlled prototype** for cybersecurity research and demonstration. It does not connect to or control real industrial equipment.

---

# Project Objective

The primary objective of VoltGuard is to demonstrate a **physics-aware security architecture for ICS/SCADA environments**.

A Modbus command may be perfectly valid according to the protocol but still produce an unsafe physical state.

For example:

```text
Valid Modbus Command
        ↓
Protocol Check: VALID
        ↓
Physics Check: SAFE
        ↓
ALLOW
```

Whereas:

```text
Valid Modbus Command
        ↓
Protocol Check: VALID
        ↓
Physics Check: UNSAFE
        ↓
DROP
```

This allows VoltGuard to detect potentially dangerous commands that cannot be identified through protocol validation alone.

---

# Key Features

- Modbus/TCP traffic parsing
- C++ Modbus packet parser
- Structured Modbus request representation
- Modbus request validation
- Support for common Modbus function codes
- Local Modbus test traffic
- Physics-based process validation
- Simulated industrial process variables
- Configurable safety limits
- Physics state evaluation
- Rust-based security decision engine
- Fail-closed decision logic
- SAFE → ALLOW decision
- UNSAFE → DROP decision
- INVALID → DROP / REJECT
- Unknown conditions handled safely
- C++ → Python → Rust backend pipeline
- Security event handling
- Test and validation framework
- Qt-based security dashboard
- Real-time security and physics information display
- Security result visualization
- Report generation
- End-to-end security demonstration
- Local-only testing environment

---

# System Architecture

VoltGuard follows a multi-layer architecture consisting of protocol analysis, physics validation, security decision-making, and visualization.

```text
                    ┌─────────────────────────┐
                    │      Qt Dashboard       │
                    │   Monitoring / Control   │
                    └────────────┬────────────┘
                                 │
                                 ▼
                    ┌─────────────────────────┐
                    │   Modbus/TCP Test       │
                    │        Traffic          │
                    └────────────┬────────────┘
                                 │
                                 ▼
                    ┌─────────────────────────┐
                    │      C++ Parser         │
                    │   Modbus/TCP Analysis   │
                    └────────────┬────────────┘
                                 │
                                 ▼
                    ┌─────────────────────────┐
                    │   Python Physics        │
                    │        Engine           │
                    └────────────┬────────────┘
                                 │
                                 ▼
                    ┌─────────────────────────┐
                    │    Rust Decision        │
                    │        Engine            │
                    └────────────┬────────────┘
                                 │
                         ┌───────┴───────┐
                         ▼               ▼
                      ALLOW             DROP
                         │               │
                         └───────┬───────┘
                                 ▼
                    ┌─────────────────────────┐
                    │   Security Result       │
                    │   Events / Reports      │
                    └────────────┬────────────┘
                                 │
                                 ▼
                    ┌─────────────────────────┐
                    │      Qt Dashboard       │
                    └─────────────────────────┘
```

---

# Core Components

## 1. C++ Modbus/TCP Parser

The C++ component is responsible for processing Modbus/TCP test traffic.

It extracts and validates relevant protocol information from Modbus requests.

The parser handles information such as:

- Transaction ID
- Protocol ID
- Unit ID
- Function code
- Register address
- Register quantity
- Register values
- Request structure

The parser provides a structured representation of the request for the next stage of the pipeline.

### Supported Operations

The parser is designed to handle common Modbus operations including:

- Read Holding Registers
- Read Input Registers
- Write Single Register
- Write Multiple Registers

Invalid or malformed requests are rejected rather than being passed blindly to later stages.

---

# 2. Python Physics Engine

The Python physics engine evaluates the expected physical state resulting from a Modbus command.

Instead of considering only whether a packet is syntactically valid, VoltGuard determines whether the resulting process state remains within configured safety limits.

The prototype models variables such as:

- Valve position
- Flow
- Pressure
- Pump state
- Pump speed

Example:

```text
Valve Position : 60%
Flow           : 42 L/min
Pressure       : 5.2 bar
Pump State     : ON
Pump Speed     : 70%

Physics Status : SAFE
```

An unsafe state can be detected when a simulated physical variable exceeds its configured limit.

Example:

```text
Pressure       : 12.5 bar
Maximum Limit  : 10.0 bar

Physics Status : UNSAFE
Reason         : Pressure limit exceeded
```

Safety limits are designed to be configurable rather than hard-coded into the overall security workflow.

---

# 3. Rust Decision Engine

The Rust component acts as the security decision layer.

It receives the structured result from the physics/security analysis and determines whether the request should be allowed or dropped.

The decision policy follows a fail-closed approach:

```text
SAFE
 ↓
ALLOW
```

```text
UNSAFE
 ↓
DROP
```

```text
INVALID
 ↓
DROP
```

```text
UNKNOWN
 ↓
DROP
```

The purpose of this layer is to ensure that ambiguous or invalid results are not accidentally treated as safe.

---

# 4. End-to-End Backend Pipeline

The core VoltGuard backend connects the individual components into a single security pipeline.

```text
C++ Modbus Parser
        ↓
Structured Modbus Request
        ↓
Python Physics Engine
        ↓
Physical Safety Evaluation
        ↓
Rust Decision Engine
        ↓
Final Security Decision
```

The pipeline allows a Modbus command to be evaluated from both a **network/protocol perspective** and a **physical-process perspective**.

---

# 5. Qt Dashboard

The Qt dashboard provides the graphical interface for the VoltGuard prototype.

The dashboard is intended to present the results of the security pipeline in an understandable form.

The interface provides information such as:

- System status
- Scan/control actions
- Modbus request information
- Security decision
- Physics state
- Safety status
- Security events
- Reason for ALLOW/DROP
- Report access

The dashboard acts primarily as the **presentation and control layer** while the backend performs the actual security analysis.

---

# Security Decision Flow

## Safe Scenario

A normal command is received through the local test environment.

```text
Modbus Request
      ↓
C++ Parser
      ↓
Valid Request
      ↓
Physics Engine
      ↓
SAFE
      ↓
Rust Decision Engine
      ↓
ALLOW
```

The dashboard can then display the successful security decision.

---

## Unsafe Scenario

A command produces a simulated physical state that violates configured safety limits.

```text
Modbus Request
      ↓
C++ Parser
      ↓
Valid Request
      ↓
Physics Engine
      ↓
UNSAFE
      ↓
Rust Decision Engine
      ↓
DROP
```

The system records the security result and provides the reason for the decision.

---

## Invalid Scenario

Malformed or invalid requests are handled safely.

```text
Invalid Modbus Request
          ↓
      Validation
          ↓
       REJECT
          ↓
        DROP
```

This prevents invalid input from being treated as a valid command.

---

# Fail-Closed Security Model

VoltGuard follows a fail-closed security principle.

The system should not allow a request simply because no explicit attack was detected.

Instead:

```text
SAFE      → ALLOW
UNSAFE    → DROP
INVALID   → DROP
UNKNOWN   → DROP
ERROR     → DROP / REJECT
```

This design reduces the risk of unsafe commands being allowed because of malformed input, unexpected states, or processing errors.

---

# Project Development Phases

VoltGuard was developed according to a four-week project plan.

## Week 1 — Modbus/TCP Parsing and Physics Baseline

The first phase focuses on establishing the foundation of the system.

### Modbus/TCP

- Create controlled Modbus/TCP test traffic
- Implement C++ packet parsing
- Extract Modbus request information
- Validate Modbus requests
- Build structured request representations

### Physics Baseline

- Establish the simulated physical process
- Define process variables
- Define safety limits
- Implement initial physics evaluation

---

## Week 2 — Backend Integration and Dashboard Foundation

The second phase focuses on connecting the protocol and physics layers.

### Backend Integration

```text
C++ Parser
    ↓
Python Physics Engine
```

The Modbus request is converted into appropriate physical inputs and evaluated by the physics engine.

### Dashboard Foundation

The Qt dashboard is introduced as the graphical interface for:

- System status
- Packet information
- Physics information
- Security results

---

## Week 3 — Security Enforcement and Real-Time Visualization

The third phase focuses on security decisions and visualization.

### Security Enforcement

The Rust decision engine evaluates the physics/security result:

```text
SAFE   → ALLOW
UNSAFE → DROP
```

Invalid and unknown conditions are handled using fail-closed logic.

### Visualization

The dashboard presents:

- Modbus request information
- Physics state
- Security status
- ALLOW/DROP decision
- Security events

---

## Week 4 — Deployment Preparation, UI Refinement and Finalization

The final phase focuses on completing the prototype and preparing it for demonstration.

Activities include:

- Backend stabilization
- End-to-end integration
- Dashboard refinement
- Security testing
- Report generation
- Final UI improvements
- Documentation
- Final project verification
- Demonstration preparation

The project remains a controlled local prototype and does not target deployment against real industrial systems.

---

# Technology Stack

## Programming Languages

- C++
- Python
- Rust

## GUI

- Qt

## Security / Networking

- Modbus/TCP
- Network protocol parsing
- Input validation
- Security decision logic
- Fail-closed security model

## Development Environment

- Linux / Kali Linux
- Visual Studio Code
- Git
- GitHub

---

# Project Structure

```text
VoltGuard/
│
├── assets/
│   └── .gitkeep
│
├── configs/
│   └── .gitkeep
│
├── docs/
│   └── .gitkeep
│
├── reports/
│   └── .gitkeep
│
├── scripts/
│   └── .gitkeep
│
├── src/
│   ├── cpp/
│   │   └── packet_interceptor/
│   │
│   ├── python/
│   │   └── physics_engine/
│   │
│   ├── qt/
│   │   └── dashboard/
│   │
│   └── rust/
│       └── decision_engine/
│
├── tests/
│   ├── cpp/
│   │   └── .gitkeep
│   │
│   ├── python/
│   │   └── .gitkeep
│   │
│   └── rust/
│       └── .gitkeep
│
├── traffic/
│   └── .gitkeep
│
├── README.md
├── requirements.txt
└── .gitignore
```

---

# Testing

VoltGuard includes testing of individual components and the complete backend pipeline.

Testing focuses on:

- Modbus parser functionality
- Valid Modbus requests
- Invalid Modbus requests
- Malformed input
- Physics safety evaluation
- Safety boundary conditions
- Rust decision logic
- Fail-closed behavior
- SAFE → ALLOW
- UNSAFE → DROP
- INVALID → DROP
- Unknown status handling
- End-to-end pipeline execution
- Repeated local execution
- Regression testing
- Dashboard integration

All testing is performed using controlled local or simulated traffic.

---

# Security Test Matrix

| Test Case | Expected Result |
|---|---|
| Valid + Safe Modbus request | ALLOW |
| Valid + Unsafe Modbus request | DROP |
| Malformed Modbus request | REJECT / DROP |
| Invalid function code | REJECT / DROP |
| Invalid register information | REJECT / DROP |
| Unsafe pressure condition | DROP |
| Unsafe flow condition | DROP |
| Unsafe valve condition | DROP |
| Invalid physics result | DROP |
| Unknown security status | DROP |
| Invalid Rust input | DROP |
| Boundary safety condition | Validated |
| Backend processing error | Fail Closed |

---

# Example Security Analysis

Consider a simulated pump and valve system.

A Modbus command changes the valve position.

The system processes it as follows:

```text
Step 1
Modbus command received
        ↓

Step 2
C++ parser validates the request
        ↓

Step 3
Command is converted into simulated
physical process values
        ↓

Step 4
Python physics engine evaluates
the physical state
        ↓

Step 5
Rust decision engine evaluates
the security status
        ↓

Step 6
Final decision:
ALLOW or DROP
```

This demonstrates why protocol validation and physical validation can complement each other in ICS security.

---

# Security Events and Reports

VoltGuard can record the outcome of security analysis and provide information useful for investigation and demonstration.

A security result can contain information such as:

```text
Request
Function Code
Register Information
Physical State
Safety Status
Security Decision
Reason
```

Example:

```text
Function Code : Write Single Register

Physics Status : UNSAFE

Violation      : Pressure exceeds safety limit

Decision       : DROP
```

The project also includes report-generation functionality for documenting security analysis results.

---

# Safe Development and Testing

VoltGuard is intentionally developed using controlled test environments.

The project does not require interaction with:

- Real PLCs
- Real SCADA networks
- Industrial controllers
- Production ICS infrastructure
- Critical infrastructure

All demonstrations should use:

- Local test traffic
- Simulated process values
- Controlled development environments
- Authorized systems only

---

# Use Cases

VoltGuard can be used as a research and educational prototype for exploring:

- ICS cybersecurity
- SCADA security
- Modbus/TCP security
- Protocol-aware security systems
- Physics-aware intrusion detection
- Security decision engines
- Fail-closed security architecture
- Industrial cybersecurity concepts
- Cyber-physical system security

---

# Project Outcome

VoltGuard demonstrates a security architecture where a command is evaluated beyond basic protocol validity.

The project combines:

```text
Network Security
       +
Protocol Analysis
       +
Physics-Based Validation
       +
Security Decision Making
       +
Security Visualization
```

This creates a prototype approach for detecting commands that may be technically valid at the protocol level but potentially unsafe from a physical-process perspective.

---

# Current Project Status

**Project Status: Completed**

The completed prototype includes:

- Modbus/TCP parsing
- C++ packet processing
- Python physics engine
- Physics-aware validation
- Rust decision engine
- Fail-closed security decisions
- Integrated backend pipeline
- Security testing
- Qt dashboard
- Security visualization
- Report generation
- End-to-end demonstration
- Documentation

---

# Limitations

VoltGuard is a prototype and has several limitations.

It is not intended to replace a production-grade ICS security solution.

The current system uses controlled and simulated process behavior rather than a complete physical industrial process.

It does not provide:

- Real PLC control
- Production SCADA integration
- Real industrial packet interception
- Production firewall functionality
- Real-time control of industrial equipment
- Safety-certified industrial protection

Further development would be required before considering any deployment in an actual industrial environment.

---

# Future Improvements

Possible future improvements include:

- Expanded Modbus function-code coverage
- More detailed physics models
- Additional industrial process simulations
- Advanced anomaly detection
- More ICS protocol support
- Improved real-time visualization
- Hardware/edge deployment research
- Advanced security analytics
- Expanded automated testing
- Additional reporting capabilities
- Integration with authorized laboratory ICS environments

---

# Disclaimer

VoltGuard is developed strictly for **educational, research, and cybersecurity demonstration purposes**.

The project should only be used on systems, networks, and traffic for which proper authorization has been obtained.

Do not use VoltGuard to interfere with real industrial control systems, critical infrastructure, or production SCADA/ICS environments.

---

# Author

**Giriraju C M**

Cybersecurity | VAPT | Network Security | Ethical Hacking | ICS Security

GitHub:

https://github.com/girirajuinfo

---

# License

This project is licensed under the terms specified in the `LICENSE` file.
```

**This version does not mention the 10-day implementation at all.** It presents VoltGuard correctly as the **4-week Infotact project**, with the four official development phases and the actual architecture.