# Eraclea-1

A satellite mission simulator with separated On-Board Computer (OBC) and Ground Control Station (GCS) communicating via TCP/IP. The OBC manages satellite state transitions and data acquisition using multithreaded tasks; the Ground Station provides a console interface to send commands and monitor telemetry.

## Overview

Eraclea-1 simulates a satellite system split into two independent processes:

- **OBC (On-Board Computer)**: Runs onboard subsystems (sensor acquisition, data processing, health monitoring) and accepts TC (Telecommand) packets from the Ground Station over TCP. Responds with TM (Telemetry) packets.
- **Ground Control Station (GCS)**: Interactive console that connects to the OBC, sends commands, and displays incoming telemetry in real time.

The communication protocol is PUS-based (simplified), using TCP sockets on `localhost:5000`.

## Build

```bash
make clean
make
```

This produces two executables:
- `eraclea_obc` — the satellite onboard software
- `eraclea_ground` — the ground control station

## Run

**Terminal 1 — Start the OBC:**
```bash
./eraclea_obc
```

The OBC will start listening on port 5000.

**Terminal 2 — Start the Ground Station:**
```bash
./eraclea_ground
```

The Ground Station will connect to the OBC and display the command menu.

## System States

The satellite transitions through these states under ground control:

- **SYS_IDLE**: System powered but inactive. Ground can send ACTIVATE to move to ACTIVE.
- **SYS_ACTIVE**: System ready for operation. Ground can send START DOWNLINK (→ DOWNLINK) or DEACTIVATE (→ IDLE).
- **SYS_DOWNLINK**: Data acquisition and transmission active. Ground can send STOP DOWNLINK (→ ACTIVE).

## Ground Control Commands

Once connected to the OBC, the Ground Station menu offers:

1. **ACTIVATE** — Transition from IDLE → ACTIVE (sensor tasks start)
2. **START DOWNLINK** — Transition from ACTIVE → DOWNLINK (telemetry stream begins)
3. **STOP DOWNLINK** — Transition from DOWNLINK → ACTIVE (halt telemetry stream)
4. **DEACTIVATE** — Transition from ACTIVE → IDLE (shut down sensor tasks)
0. **EXIT** — Disconnect and close the ground station

The ground station tracks OBC state locally and enforces valid state transitions.

## OBC Subsystems

The OBC runs four background tasks once activated:

- **Sensor Task** — Simulates sensor readings (temperature, pressure, etc.) and deposits them in a data buffer
- **Processing Task** — Reads from the buffer, processes sensor data, and queues telemetry packets
- **Health Task** — Monitors system health and sends status telemetry
- **TM Sender Task** — Sends queued telemetry packets to the ground station when connected

## Communication Protocol

All packets (TC and TM) follow a simplified PUS packet structure:
- **Header**: version, type (1=TC, 0=TM), service, subtype, length
- **Data**: variable-length payload (up to 256 bytes)

Telecommand types (service 128 / Mode Control, subtypes):
- `1` — Activate
- `2` — Start Downlink
- `3` — Stop Downlink
- `4` — Deactivate

Telemetry is sent with service 3 (Housekeeping) for health reports and service 129 (Sensor Data) for measurements.

## Project Structure

```
eraclea-1/
├── obc_main.c              # OBC entry point and TCP server loop
├── ground_main.c           # GCS entry point and command UI
├── Makefile                # Build system (produces two binaries)
├── README.md               # This file
│
├── core/
│   ├── system.c            # State machine and task management
│   └── system.h
│
├── comm/
│   ├── pus.h               # PUS packet definitions
│   ├── pus.c               # PUS utilities
│   ├── tc_handler.c        # Process incoming TC packets
│   ├── tc_handler.h
│   ├── tm_manager.c        # Queue and send TM packets
│   ├── tm_manager.h
│   ├── link.c              # TCP socket abstraction
│   └── link.h
│
├── data/
│   ├── buffer.c            # Circular buffer for sensor data
│   └── buffer.h
│
├── sensor/
│   ├── sensor.c            # Sensor simulation tasks
│   └── sensor.h
│
├── platform/
│   ├── platform.c          # Platform abstraction (delay, time)
│   └── platform.h
│
└── ground/
    ├── ground_output.c     # Telemetry display formatting
    └── ground_output.h
```

