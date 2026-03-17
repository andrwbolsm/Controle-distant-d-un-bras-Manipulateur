# Contrôle Distant d'un Bras Manipulateur

Remote velocity control of a 6-DOF Robotis arm over a simulated UDP/IP network.  
MEA4 — Polytech'Montpellier

---

## How it works

Three programs run in parallel:

```
[ command.c ] ──UDP:2000──► [ retard.c ] ──UDP:2001──► [ main.cpp / V-REP ]
      ◄──────────────────────────────────────────────────────── (feedback)
```

| File | Role |
|---|---|
| `command.c` | Sends joint velocity setpoints every 10 ms, measures round-trip latency, logs to `latence.csv` |
| `retard.c` | Buffers packets in a circular queue to simulate a fixed delay (`max_msg × dt = 250 ms` by default) |
| `main.cpp` | Receives commands, applies them to V-REP via remote API, echoes back joint feedback |

All three share the same packet type:
```c
struct mesg { float q[6]; long long time; };
```

---

## Build

```bash
g++ main.cpp remoteApi/extApi.c remoteApi/extApiPlatform.c -I remoteApi/ -lpthread -o robot
gcc retard.c -o retard
gcc command.c -lm -o command
```

## Run

```bash
# 1. Open robotis_crayon.ttt in V-REP and start the simulation
./retard   # starts the delay simulator
./control    # connects to V-REP at 172.23.96.1:5555
./command  # starts sending commands, writes latence.csv
```

---

## Key parameters

| Parameter | File | Default |
|---|---|---|
| Control period | `command.c` / `retard.c` | 10 ms |
| Simulated delay | `retard.c` (`max_msg × dt`) | 250 ms |
| V-REP address | `main.cpp` | `172.23.96.1:5555` |

---

## Notes

- Packet loss and sinusoidal/keyboard inputs are implemented but commented out.
- Latency is logged to `latence.csv` (`t_in, t_rcv, latence_us, q0_in, q0_rcv`).
- Requires Linux and the V-REP remote API headers.
