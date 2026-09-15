import json
import subprocess
import sys


RUST_ENGINE = "src/rust/decision_engine/target/debug/decision_engine"


def run_physics(modbus_command):
    """Send a Modbus command to the existing Python physics bridge."""

    process = subprocess.run(
        [
            sys.executable,
            "-m",
            "src.python.integration.modbus_physics_bridge",
        ],
        input=json.dumps(modbus_command),
        text=True,
        capture_output=True,
    )

    if process.returncode != 0:
        raise RuntimeError("Physics engine failed")

    return json.loads(process.stdout)


def run_decision(physics_result):
    """Send the physics result to the existing Rust decision engine."""

    process = subprocess.run(
        [RUST_ENGINE],
        input=json.dumps(physics_result),
        text=True,
        capture_output=True,
    )

    if process.returncode != 0:
        raise RuntimeError("Rust decision engine failed")

    return json.loads(process.stdout)


def evaluate(modbus_command):
    """Run the complete Python → Rust backend pipeline."""

    physics_result = run_physics(modbus_command)
    decision = run_decision(physics_result)

    return {
        "parsed_command": physics_result["command"],
        "physics": {
            "status": physics_result["status"],
            "predicted_flow": physics_result["physical_state"]["flow_lpm"],
            "predicted_pressure": physics_result["physical_state"]["pressure_bar"],
            "violations": physics_result["violations"],
        },
        "decision": {
            "action": decision["decision"],
            "reason": decision["reason"],
            "severity": decision["severity"],
        },
    }


def main():
    try:
        command = json.load(sys.stdin)
        result = evaluate(command)
        print(json.dumps(result))

    except Exception as error:
        print(
            json.dumps(
                {
                    "status": "REJECTED",
                    "decision": {
                        "action": "DROP",
                        "reason": str(error),
                        "severity": "HIGH",
                    },
                }
            )
        )


if __name__ == "__main__":
    main()