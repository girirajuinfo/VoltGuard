import unittest

from src.python.integration.backend_pipeline import evaluate


class TestBackendPipeline(unittest.TestCase):

    def test_safe_command_allows(self):
        command = {
            "transaction_id": 1,
            "protocol_id": 0,
            "unit_id": 1,
            "function_code": 3,
            "address": 0,
            "quantity": 2,
            "value": 0,
        }

        result = evaluate(command)

        self.assertEqual(result["physics"]["status"], "SAFE")
        self.assertEqual(result["decision"]["action"], "ALLOW")

    def test_unsafe_command_drops(self):
        command = {
            "transaction_id": 3,
            "protocol_id": 0,
            "unit_id": 1,
            "function_code": 6,
            "address": 32,
            "quantity": 0,
            "value": 3500,
        }

        result = evaluate(command)

        self.assertEqual(result["physics"]["status"], "UNSAFE")
        self.assertEqual(result["decision"]["action"], "DROP")

    def test_command_values_preserved(self):
        command = {
            "transaction_id": 5,
            "protocol_id": 0,
            "unit_id": 1,
            "function_code": 6,
            "address": 32,
            "quantity": 0,
            "value": 1500,
        }

        result = evaluate(command)

        self.assertEqual(
            result["parsed_command"]["transaction_id"],
            5,
        )

        self.assertEqual(
            result["parsed_command"]["function_code"],
            6,
        )

        self.assertEqual(
            result["parsed_command"]["value"],
            1500,
        )

    def test_invalid_command_is_rejected(self):
        command = {
            "status": "UNKNOWN"
        }

        with self.assertRaises(Exception):
            evaluate(command)


if __name__ == "__main__":
    unittest.main()