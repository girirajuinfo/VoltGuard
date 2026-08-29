from __future__ import annotations

import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import List


@dataclass
class PipelineCommand:
    """Command variables supplied to the mock pipeline."""

    valve_position_percent: float
    pump_enabled: bool
    pump_speed_rpm: float


@dataclass
class PipelineState:
    """Predicted physical state of the mock pipeline."""

    valve_position_percent: float
    pump_enabled: bool
    pump_speed_rpm: float

    flow_lpm: float
    pressure_bar: float

    status: str
    violations: List[str] = field(default_factory=list)


class PhysicsEngine:
    """
    Simplified educational physics engine for VoltGuard.

    This is NOT a real industrial process model.

    The model uses simple relationships:

        Flow = pump contribution × valve opening

        Pressure = base pressure + pump contribution

    The model parameters and safety limits are loaded
    from a JSON configuration file.
    """

    def __init__(self, config_path: str | Path):
        self.config_path = Path(config_path)
        self.limits = self._load_limits()

    def _load_limits(self) -> dict:
        """
        Load mock model parameters and safety limits
        from the JSON configuration.
        """

        with self.config_path.open("r", encoding="utf-8") as file:
            config = json.load(file)

        self.model = config["model"]

        return config["limits"]

    def simulate(self, command: PipelineCommand) -> PipelineState:
        """
        Predict the physical state of the mock pipeline
        for a supplied command.
        """

        self._validate_command(command)

        if not command.pump_enabled:
            flow_lpm = 0.0
            pressure_bar = self.model["base_pressure_bar"]

        else:
            valve_factor = command.valve_position_percent / 100.0

            flow_lpm = (
                command.pump_speed_rpm
                / self.model["reference_pump_speed_rpm"]
            ) * valve_factor * self.model["reference_flow_lpm"]

            pressure_bar = (
                self.model["base_pressure_bar"]
                + (
                    command.pump_speed_rpm
                    / self.model["reference_pump_speed_rpm"]
                ) * self.model["pressure_rise_bar"]
            )

        violations = self._evaluate_safety(
            command,
            flow_lpm,
            pressure_bar,
        )

        status = "UNSAFE" if violations else "SAFE"

        return PipelineState(
            valve_position_percent=command.valve_position_percent,
            pump_enabled=command.pump_enabled,
            pump_speed_rpm=command.pump_speed_rpm,
            flow_lpm=flow_lpm,
            pressure_bar=pressure_bar,
            status=status,
            violations=violations,
        )

    def _validate_command(self, command: PipelineCommand) -> None:
        """
        Reject physically invalid command values.
        """

        if not 0.0 <= command.valve_position_percent <= 100.0:
            raise ValueError(
                "Valve position must be between 0 and 100 percent"
            )

        if command.pump_speed_rpm < 0.0:
            raise ValueError(
                "Pump speed cannot be negative"
            )

        if not isinstance(command.pump_enabled, bool):
            raise ValueError(
                "Pump enabled state must be True or False"
            )

    def _evaluate_safety(
        self,
        command: PipelineCommand,
        flow_lpm: float,
        pressure_bar: float,
    ) -> List[str]:
        """
        Return all safety-limit violations.
        """

        violations: List[str] = []

        if command.pump_speed_rpm > self.limits["max_pump_speed_rpm"]:
            violations.append(
                "Pump speed exceeds maximum safe limit"
            )

        if flow_lpm > self.limits["max_flow_lpm"]:
            violations.append(
                "Predicted flow exceeds maximum safe limit"
            )

        if pressure_bar < self.limits["min_pressure_bar"]:
            violations.append(
                "Predicted pressure is below minimum safe limit"
            )

        if pressure_bar > self.limits["max_pressure_bar"]:
            violations.append(
                "Predicted pressure exceeds maximum safe limit"
            )

        return violations