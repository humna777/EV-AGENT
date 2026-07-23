# Agentic AI Architecture — EV-AGENT

This document describes the multi-agent system that powers the **Agentic AI-Enabled Wireless Power Transfer (WPT) System for Sustainable EV Charging**. The system uses a set of specialized autonomous agents, coordinated by a Central Orchestrator, to manage battery charging, source selection, user authentication, fault safety, and wireless power activation — with no manual intervention required during normal operation.

---

## System Overview

```
                     ┌────────────────────────┐
                     │   CENTRAL ORCHESTRATOR │
                     └────────────┬───────────┘
        ┌───────────┬─────────────┼─────────────┬───────────┐
        ▼           ▼             ▼             ▼           ▼
   Battery      Energy       Verification    Fault       WPT Control
  Management   Management       Agent       Detection      Agent
    Agent        Agent                        Agent
        └───────────┴─────────────┴─────────────┴───────────┘
                     SHARED MEMORY & SYSTEM STATE LAYER
```

Each agent operates independently, reads real-time sensor/hardware data relevant to its role, and reports its state to the Central Orchestrator through the shared memory layer. The Orchestrator uses this combined state to make coordinated system-level decisions (e.g., only activating WPT charging once verification succeeds AND no fault is present AND a source is available).

---

## 1. Central Orchestrator

**Role:** Acts as the decision-making core of the system. Aggregates the state reported by all other agents and issues coordinated commands.

**Reads:**
- Verification status (authenticated / not authenticated)
- Active source availability (grid / solar)
- Battery state (charging / full / low)
- Fault status (safe / fault detected)

**Decision Logic:**
- Charging is only enabled when: user is verified **AND** a valid power source is available **AND** no fault is active
- If a fault is reported at any point, immediately halts WPT Control Agent regardless of other states
- Logs and updates shared system state after every decision cycle

**Output:** Enable/disable signal to the WPT Control Agent; system status updates to the dashboard

---

## 2. Battery Management Agent

**Role:** Monitors and controls the EV-side battery during charging.

**Reads:**
- Battery voltage
- State of Charge (SoC)
- Charging current

**Decision Logic:**
- Tracks SoC in real time during a charging session
- Signals the Orchestrator when the battery reaches full charge (to stop the session)
- Flags abnormal battery behavior (e.g., unexpected voltage drop) as a potential concern for the Fault Detection Agent

**Output:** Battery status (SoC %, charging/full/idle) sent to shared memory and displayed on the dashboard

---

## 3. Energy Management Agent

**Role:** Controls and selects the source side of the system — choosing between the **WAPDA grid** and **solar generation**.

**Reads:**
- Solar panel output availability
- Grid (WAPDA) availability
- Current load/demand requirement

**Decision Logic:**
- Prioritizes solar when sufficient generation is available (renewable-first strategy)
- Falls back to grid supply when solar output is insufficient or unavailable
- Continuously re-evaluates source availability during a charging session and can switch sources without interrupting charging

**Output:** Active source selection (Solar / Grid) sent to the Central Orchestrator and WPT Control Agent

---

## 4. Verification Agent

**Role:** Handles user authentication at the physical User Authentication Panel (UAP) before allowing any charging session to begin.

**Reads:**
- RFID tag input from the authentication panel

**Decision Logic:**
- Compares scanned RFID tag against authorized user records
- Approves or denies access based on match result
- Sends a clear authenticated/denied signal to the Orchestrator — no charging session can proceed without approval

**Output:** Authentication result (Verified / Denied) sent to the Central Orchestrator

---

## 5. Fault Detection Agent

**Role:** Continuously monitors the electrical safety of the system during operation.

**Reads:**
- Voltage sensor values (transmitter/receiver side)
- Current sensor values

**Decision Logic:**
- Compares real-time voltage/current readings against defined safe operating thresholds
- Flags a fault the moment any reading falls outside the safe range (e.g., overcurrent, undervoltage, abnormal fluctuation)
- Fault signal is treated as highest priority — overrides all other agent states and forces an immediate system shutdown via the Orchestrator

**Output:** Fault status (Safe / Fault Detected) sent to the Central Orchestrator in real time

---

## 6. WPT Control Agent

**Role:** Activates and manages the actual wireless charging process between the transmitter and receiver coils.

**Reads:**
- Enable/disable command from the Central Orchestrator
- Active source signal from the Energy Management Agent

**Decision Logic:**
- Only activates the WPT kit (resonant coupling transmitter/receiver) once the Orchestrator confirms: user verified + source available + no fault
- Immediately deactivates charging if the Orchestrator sends a stop/fault signal
- Manages the handoff between grid and solar source without interrupting an active charging session

**Output:** Charging activation state (Active / Inactive) to the EV battery via the WPT coils

---

## Shared Memory & System State Layer

All five agents and the Central Orchestrator read from and write to a shared system state, which stores the live values of:

| State Variable | Updated By |
|---|---|
| `battery_soc` | Battery Management Agent |
| `active_source` | Energy Management Agent |
| `auth_status` | Verification Agent |
| `fault_status` | Fault Detection Agent |
| `charging_active` | WPT Control Agent |

This shared state is what allows agents to operate independently while still acting as one coordinated system — the Central Orchestrator simply reads this shared state each cycle to make its next decision.

---

## Notes

- This document reflects the actual agent architecture designed and implemented in this Final Year Project (BS Electrical Engineering, IIUI Islamabad).
- Individual agent implementation files are located in `/src`.
- Circuit schematics, testing photos, and the architecture diagram are located in `/reports`.
