# VoltGuard Qt Dashboard

The VoltGuard dashboard is a Qt 6 Widgets and C++17 monitoring console for the existing physics-aware ICS/SCADA security pipeline. It is a visualization and control layer only: command safety is decided by the Python physics bridge and Rust decision engine.

## Architecture

The dashboard starts the trusted backend with `QProcess`:

```text
Qt Dashboard -> JSON stdin -> python3 -m src.python.integration.backend_pipeline
             <- JSON stdout <- Python physics bridge + Rust decision engine
```

The executable walks upward from its own directory until it finds `src/`, `configs/`, and `traffic/`. That directory is used as the process working directory, so Python can resolve the `src.python...` module without a hardcoded machine path.

## Requirements

- Qt 6 Widgets, including `qmake6`
- A C++17 compiler and `make`
- Python 3 available as `python3`
- The existing VoltGuard Rust debug decision engine built at its normal backend path

## Build

From this directory:

```bash
qmake6 VoltGuardDashboard.pro
make clean
make -j$(nproc)
```

## Run

```bash
./VoltGuardDashboard
```

The application does not invoke a shell. It launches `python3` with the argument list `-m src.python.integration.backend_pipeline`, sends one JSON command through stdin, closes the write channel, and parses stdout as JSON asynchronously. Backend stderr is kept separate and written to the event log.

## Demonstration commands

Use **LOAD SAFE TEST**, then **ANALYZE COMMAND**:

- Transaction ID `1`, protocol `0`, unit `1`
- Function `03`, address `0`, quantity `2`, value `0`
- Expected backend result: `SAFE`, `ALLOW`, `NORMAL`, flow `25.0 L/min`, pressure `5.5 bar`

Use **LOAD UNSAFE TEST**, then **ANALYZE COMMAND**:

- Transaction ID `3`, protocol `0`, unit `1`
- Function `06`, address `32`, quantity `0`, value `3500`
- Expected backend result: `UNSAFE`, `DROP`, `HIGH`, flow approximately `58.33 L/min`, pressure `11.5 bar`

The history table retains the last 100 completed analyses in memory. Session statistics are also memory-only and count actual backend responses.

## Troubleshooting

- **Project root not found:** run the executable from the dashboard build output after confirming the executable remains under the VoltGuard project tree, or rebuild in `src/qt/dashboard/`.
- **Python cannot import `src`:** confirm the root-detection markers exist and that the backend is started with the root as its working directory. The dashboard does this automatically.
- **Backend timeout:** inspect the event log and run the backend directly from the project root to verify the Python and Rust components.
- **No stylesheet:** keep `ui/styles.qss` beside the executable under the dashboard directory.

## Known limitations

The current backend response does not expose the bridge's `physical_input` object. The dashboard therefore displays pump speed and valve position as `Not reported` rather than deriving or inventing values. The backend remains the source of truth for all safety decisions and physics metrics.
