# VoltGuard Modbus/TCP Test Traffic

This directory contains local static Modbus/TCP test data for VoltGuard.

No packets in this directory are transmitted to real industrial systems.

## Files

### requests.hex

Contains normal Modbus/TCP requests.

Supported examples include:

- Function Code 03 — Read Holding Registers
- Function Code 04 — Read Input Registers
- Function Code 06 — Write Single Register
- Function Code 10 — Write Multiple Registers

These represent normal read and write operations.

### malformed.hex

Contains incomplete or malformed Modbus/TCP packets.

These packets are used to verify that the parser rejects invalid input safely.

### unsupported.hex

Contains a Modbus/TCP request using Function Code 05.

Function Code 05 is intentionally outside the currently supported VoltGuard function set.

It is used to verify unsupported-command handling.

## Safety

These files are static hexadecimal test data only.

VoltGuard Day 2 testing does not send Modbus/TCP traffic to real PLCs, RTUs, HMIs, or other industrial control equipment.