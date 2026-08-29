from __future__ import annotations

import json
import math
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class PipelineCommand:
    """
    Command values supplied to the mock industrial pipeline.

    valve_position_percent:
        Valve opening from 0% to 100%.

    pump_enabled:
        True when the pump is running, False when stopped.

    pump_speed_rpm:
        Pump rotational speed in revolutions per minute (RPM).
    """

    valve_position_percent: float
    pump_enabled: bool
    pump_speed_rpm: float


@dataclass
class PipelineState:
    """
    Predicted physical state of the mock pipeline.

    flow_lpm:
        Predicted flow rate in litres per minute.

    pressure_bar:
        Predicted pressure in bar.

    status:
        Either SAFE or UNSAFE.

    violations:
        List of safety-limit violations.
    """

    valve_position_percent: float
    pump_enabled: bool
    pump_speed_rpm: float

    flow_lpm: float
    pressure_bar: float

    status: str
    violations: list[str] = field(default_factory=list)


class PhysicsEngine:
    """
    Simplified educational physics engine for VoltGuard.

    IMPORTANT:
        This is a mock/educational model.
        It is NOT a real industrial process model.

    Simplified relationships:

        Flow =
            pump speed factor
            × valve opening factor
            × reference flow

        Pressure =
            base pressure
            + pump speed factor × pressure rise

    Model parameters and safety limits are loaded
    from configs/physics_limits.json.
    """

    def __init__(self, config_path: str | Path):
        """
        Create a physics engine using the supplied JSON configuration.
        """

        self.config_path = Path(config_path)

        if not self.config_path.is_file():
            raise FileNotFoundError(
                f"Physics configuration not found: {self.config_path}"
            )

        self.model, self.limits = self._load_configuration()

    def _load_configuration(self) -> tuple[dict, dict]:
        """
        Load model parameters and safety limits from JSON.
        """

        with self.config_path.open("r", encoding="utf-8") as file:
            config = json.load(file)

        if "model" not in config:
            raise ValueError(
                "Physics configuration is missing the 'model' section"
            )

        if "limits" not in config:
            raise ValueError(
                "Physics configuration is missing the 'limits' section"
            )

        return config["model"], config["limits"]

    def simulate(self, command: PipelineCommand) -> PipelineState:
        """
        Calculate the predicted physical state for a command.

        The command is validated first.
        The resulting flow and pressure are then calculated.
        Finally, the calculated state is checked against
        the configured safety limits.
        """

        self._validate_command(command)

        if not command.pump_enabled:
            flow_lpm = 0.0
            pressure_bar = self.model["base_pressure_bar"]

        else:
            valve_factor = (
                command.valve_position_percent / 100.0
            )

            pump_speed_factor = (
                command.pump_speed_rpm
                / self.model["reference_pump_speed_rpm"]
            )

            flow_lpm = (
                pump_speed_factor
                * valve_factor
                * self.model["reference_flow_lpm"]
            )

            pressure_bar = (
                self.model["base_pressure_bar"]
                + (
                    pump_speed_factor
                    * self.model["pressure_rise_bar"]
                )
            )

        violations = self._evaluate_safety(
            command=command,
            flow_lpm=flow_lpm,
            pressure_bar=pressure_bar,
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

    def _validate_command(
        self,
        command: PipelineCommand,
    ) -> None:
        """
        Validate command values before running the simulation.

        Invalid or physically nonsensical input is rejected
        with ValueError.
        """

        if not isinstance(command.pump_enabled, bool):
            raise ValueError(
                "Pump enabled state must be True or False"
            )

        if not math.isfinite(command.valve_position_percent):
            raise ValueError(
                "Valve position must be a finite number"
            )

        if not 0.0 <= command.valve_position_percent <= 100.0:
            raise ValueError(
                "Valve position must be between 0 and 100 percent"
            )

        if not math.isfinite(command.pump_speed_rpm):
            raise ValueError(
                "Pump speed must be a finite number"
            )

        if command.pump_speed_rpm < 0.0:
            raise ValueError(
                "Pump speed cannot be negative"
            )

        if (
            not command.pump_enabled
            and command.pump_speed_rpm != 0.0
        ):
            raise ValueError(
                "Disabled pump must have zero RPM"
            )

    def _evaluate_safety(
        self,
        command: PipelineCommand,
        flow_lpm: float,
        pressure_bar: float,
    ) -> list[str]:
        """
        Compare the predicted state with configured safety limits.

        Returns a list of all detected safety violations.
        An empty list means the state is considered SAFE.
        """

        violations: list[str] = []

        if (
            command.pump_speed_rpm
            > self.limits["max_pump_speed_rpm"]
        ):
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