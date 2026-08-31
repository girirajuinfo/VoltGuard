import json
import sys
from dataclasses import asdict

from src.python.physics_engine import PhysicsEngine, PipelineCommand


class ModbusPhysicsBridge:
    """Converts parsed Modbus commands into physics-engine inputs."""

    def __init__(self, config_path):
        self.engine = PhysicsEngine(config_path)

    def convert_command(self, modbus_command):
        function_code = modbus_command["function_code"]
        value = modbus_command.get("value", 0)

        if function_code in (0x03, 0x04):
            pump_speed = 1500.0
        elif function_code in (0x06, 0x10):
            pump_speed = float(value)
        else:
            raise ValueError(
                f"Unsupported Modbus function code: 0x{function_code:02X}"
            )

        return PipelineCommand(
            valve_position_percent=50.0,
            pump_enabled=pump_speed > 0,
            pump_speed_rpm=pump_speed,
        )

    def evaluate(self, modbus_command):
        physical_command = self.convert_command(modbus_command)
        state = self.engine.simulate(physical_command)

        return {
            "command": modbus_command,
            "physical_input": asdict(physical_command),
            "physical_state": asdict(state),
            "status": state.status,
            "violations": state.violations,
        }


def main():
    """Read one JSON command from stdin and return one JSON result."""

    try:
        command = json.load(sys.stdin)

        bridge = ModbusPhysicsBridge(
            "configs/physics_limits.json"
        )

        result = bridge.evaluate(command)

        print(json.dumps(result))

    except (KeyError, TypeError, ValueError, json.JSONDecodeError) as error:
        print(
            json.dumps({
                "status": "ERROR",
                "error": str(error),
            })
        )
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())