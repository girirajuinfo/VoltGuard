from pathlib import Path
import sys

# Find the VoltGuard project root.
PROJECT_ROOT = Path(__file__).resolve().parent.parent

# Allow Python to import the project's src package.
sys.path.insert(0, str(PROJECT_ROOT))

from src.python.physics_engine import PhysicsEngine, PipelineCommand


CONFIG_PATH = PROJECT_ROOT / "configs" / "physics_limits.json"


def main():
    engine = PhysicsEngine(CONFIG_PATH)

    command = PipelineCommand(
        valve_position_percent=50.0,
        pump_enabled=True,
        pump_speed_rpm=1500.0,
    )

    state = engine.simulate(command)

    print("========================================")
    print(" VoltGuard Physics Engine Demo")
    print("========================================")
    print(f"Valve Position : {state.valve_position_percent:.1f}%")
    print(f"Pump Enabled   : {state.pump_enabled}")
    print(f"Pump Speed     : {state.pump_speed_rpm:.0f} RPM")
    print(f"Flow Rate      : {state.flow_lpm:.2f} L/min")
    print(f"Pressure       : {state.pressure_bar:.2f} bar")
    print(f"Status         : {state.status}")

    if state.violations:
        print("Violations:")
        for violation in state.violations:
            print(f"  - {violation}")

    print("========================================")


if __name__ == "__main__":
    main()