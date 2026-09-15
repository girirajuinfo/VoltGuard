use serde::Deserialize;
use serde_json::json;
use std::io::{self, Read};

#[derive(Debug, Deserialize)]
struct PhysicsResult {
    status: String,
}

fn make_decision(status: &str) -> serde_json::Value {
    match status {
        "SAFE" => json!({
            "decision": "ALLOW",
            "reason": "Physics constraints satisfied",
            "severity": "NORMAL"
        }),

        "UNSAFE" => json!({
            "decision": "DROP",
            "reason": "Physics constraint violation",
            "severity": "HIGH"
        }),

        _ => json!({
            "decision": "DROP",
            "reason": "Invalid or unknown physics status",
            "severity": "HIGH"
        }),
    }
}

fn main() {
    let mut input = String::new();

    if io::stdin().read_to_string(&mut input).is_err() {
        println!("{}", json!({
            "decision": "DROP",
            "reason": "Unable to read input",
            "severity": "HIGH"
        }));
        return;
    }

    let physics_result: Result<PhysicsResult, _> =
        serde_json::from_str(&input);

    match physics_result {
        Ok(result) => {
            println!("{}", make_decision(&result.status));
        }

        Err(_) => {
            println!("{}", json!({
                "decision": "DROP",
                "reason": "Invalid physics result",
                "severity": "HIGH"
            }));
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn safe_allows_command() {
        let result = make_decision("SAFE");

        assert_eq!(result["decision"], "ALLOW");
        assert_eq!(result["severity"], "NORMAL");
    }

    #[test]
    fn unsafe_drops_command() {
        let result = make_decision("UNSAFE");

        assert_eq!(result["decision"], "DROP");
        assert_eq!(result["severity"], "HIGH");
    }

    #[test]
    fn unknown_status_drops_command() {
        let result = make_decision("UNKNOWN");

        assert_eq!(result["decision"], "DROP");
    }

    #[test]
    fn empty_status_drops_command() {
        let result = make_decision("");

        assert_eq!(result["decision"], "DROP");
    }
}