from pathlib import Path
import sys
import unittest


# Add the VoltGuard project root to Python's import path.
PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT))


from src.python.physics_engine import PhysicsEngine, PipelineCommand


CONFIG_PATH = PROJECT_ROOT / "configs" / "physics_limits.json"


class TestPhysicsEngine(unittest.TestCase):
    """Tests for the VoltGuard mock physics engine."""

    @classmethod
    def setUpClass(cls):
        cls.engine = PhysicsEngine(CONFIG_PATH)

    def test_normal_operating_condition_is_safe(self):
        command = PipelineCommand(
            valve_position_percent=50.0,
            pump_enabled=True,
            pump_speed_rpm=1500.0,
        )

        state = self.engine.simulate(command)

        self.assertEqual(state.status, "SAFE")
        self.assertAlmostEqual(state.flow_lpm, 25.0)
        self.assertAlmostEqual(state.pressure_bar, 5.5)
        self.assertEqual(state.violations, [])

    def test_high_pump_speed_is_unsafe(self):
        command = PipelineCommand(
            valve_position_percent=50.0,
            pump_enabled=True,
            pump_speed_rpm=3500.0,
        )

        state = self.engine.simulate(command)

        self.assertEqual(state.status, "UNSAFE")
        self.assertIn(
            "Pump speed exceeds maximum safe limit",
            state.violations,
        )

    def test_excessive_pressure_is_unsafe(self):
        command = PipelineCommand(
            valve_position_percent=100.0,
            pump_enabled=True,
            pump_speed_rpm=3500.0,
        )

        state = self.engine.simulate(command)

        self.assertEqual(state.status, "UNSAFE")
        self.assertIn(
            "Predicted pressure exceeds maximum safe limit",
            state.violations,
        )

    def test_excessive_flow_is_unsafe(self):
        command = PipelineCommand(
            valve_position_percent=100.0,
            pump_enabled=True,
            pump_speed_rpm=3001.0,
        )

        state = self.engine.simulate(command)

        self.assertEqual(state.status, "UNSAFE")
        self.assertIn(
            "Predicted flow exceeds maximum safe limit",
            state.violations,
        )

    def test_negative_pump_speed_is_rejected(self):
        command = PipelineCommand(
            valve_position_percent=50.0,
            pump_enabled=True,
            pump_speed_rpm=-100.0,
        )

        with self.assertRaises(ValueError):
            self.engine.simulate(command)

    def test_invalid_valve_position_is_rejected(self):
        command = PipelineCommand(
            valve_position_percent=120.0,
            pump_enabled=True,
            pump_speed_rpm=1500.0,
        )

        with self.assertRaises(ValueError):
            self.engine.simulate(command)

    def test_safe_pump_speed_boundary(self):
        command = PipelineCommand(
            valve_position_percent=50.0,
            pump_enabled=True,
            pump_speed_rpm=3000.0,
        )

        state = self.engine.simulate(command)

        self.assertEqual(state.status, "SAFE")
        self.assertEqual(state.pressure_bar, 10.0)

    def test_pump_disabled_is_safe(self):
        command = PipelineCommand(
            valve_position_percent=50.0,
            pump_enabled=False,
            pump_speed_rpm=0.0,
        )

        state = self.engine.simulate(command)

        self.assertEqual(state.status, "SAFE")
        self.assertEqual(state.flow_lpm, 0.0)
        self.assertEqual(state.pressure_bar, 1.0)


if __name__ == "__main__":
    unittest.main(verbosity=2)