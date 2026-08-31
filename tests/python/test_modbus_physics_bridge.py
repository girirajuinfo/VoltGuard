from pathlib import Path
import unittest

from src.python.integration.modbus_physics_bridge import ModbusPhysicsBridge


PROJECT_ROOT = Path(__file__).resolve().parents[2]
CONFIG_PATH = PROJECT_ROOT / "configs" / "physics_limits.json"


class TestModbusPhysicsBridge(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.bridge = ModbusPhysicsBridge(CONFIG_PATH)

    def test_safe_read_command(self):
        command = {
            "transaction_id": 1,
            "unit_id": 1,
            "function_code": 3,
            "address": 0,
            "quantity": 2,
            "value": 0,
        }

        result = self.bridge.evaluate(command)

        self.assertEqual(result["status"], "SAFE")

    def test_unsafe_write_command(self):
        command = {
            "transaction_id": 3,
            "unit_id": 1,
            "function_code": 6,
            "address": 32,
            "quantity": 0,
            "value": 3500,
        }

        result = self.bridge.evaluate(command)

        self.assertEqual(result["status"], "UNSAFE")
        self.assertTrue(result["violations"])

    def test_command_values_are_preserved(self):
        command = {
            "transaction_id": 5,
            "unit_id": 2,
            "function_code": 6,
            "address": 32,
            "quantity": 0,
            "value": 1500,
        }

        result = self.bridge.evaluate(command)

        self.assertEqual(result["command"], command)
        self.assertEqual(
            result["physical_input"]["pump_speed_rpm"],
            1500.0,
        )

    def test_unsupported_function_code_rejected(self):
        command = {
            "transaction_id": 5,
            "unit_id": 1,
            "function_code": 5,
            "address": 0,
            "quantity": 0,
            "value": 1,
        }

        with self.assertRaises(ValueError):
            self.bridge.evaluate(command)

    def test_invalid_physics_input_rejected(self):
        command = {
            "transaction_id": 6,
            "unit_id": 1,
            "function_code": 6,
            "address": 32,
            "quantity": 0,
            "value": -100,
        }

        with self.assertRaises(ValueError):
            self.bridge.evaluate(command)


if __name__ == "__main__":
    unittest.main(verbosity=2)