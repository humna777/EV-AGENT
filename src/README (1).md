# EV Charger – Agentic AI Layer

Everything from the *Agentic AI Layer Implementation Guide* PDF, pulled out into
real files in the exact folder structure the guide describes (Section 15).
Copy this whole folder onto the Pi instead of retyping anything in `nano`.

## Folder map

```
evcharger/
├── main.py                 # Orchestrator - run this on the Pi
├── setup_db.py              # (Re)builds users.db with RFID UIDs
├── test_gemini.py           # One-off check that your Gemini key works
├── dashboard.py              # Optional Streamlit live dashboard
├── requirements.txt          # All Python packages from Section 7
├── .env.example              # Copy to .env and paste your real Gemini key
├── .gitignore                # Keeps venv/.env/users.db out of git
├── users.db                  # Pre-built with the two sample UIDs from the guide
├── systemd/
│   └── evcharger.service     # Auto-start unit file (Section 13)
└── agents/
    ├── __init__.py
    ├── shared_config.py      # Pins, thresholds, serial port/baud
    ├── verification_agent.py # RFID -> SQLite lookup
    ├── energy_manager.py     # Solar/wind/grid choice + Gemini reasoning
    ├── battery_manager.py    # Charge/throttle/stop + Gemini reasoning
    ├── wpt_control_agent.py  # Serial START/STOP + sensor line reads
    └── fault_detector.py     # Safety threshold checks
```

## Getting it onto the Pi

Once the Pi is flashed and on Wi-Fi with SSH enabled (Part A/B of the guide),
copy the folder over from your laptop in one shot:

```
scp -r evcharger pi@evcharger.local:~/
```

(If `evcharger.local` doesn't resolve, use the Pi's IP address instead — same
as in Section 4.2.)

## One-time setup on the Pi

```
cd ~/evcharger
python3 -m venv venv
source venv/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
```

Add your real Gemini key (get one at aistudio.google.com/app/apikey, Part F):

```
cp .env.example .env
nano .env        # paste your key after GEMINI_API_KEY=
```

Confirm the key works:

```
python test_gemini.py
```

`users.db` already ships with two placeholder cards (`12345678`, `87654321`).
To register your real cards, scan one and read its UID:

```
python3 -c "from agents.verification_agent import scan_card; print(scan_card())"
```

Then edit the `sample_users` list in `setup_db.py` with the real UID(s) and
re-run `python setup_db.py`.

## Running it

```
python main.py
```

Optional live dashboard — open a **second** SSH session for this one, since
it also touches the serial port (Section 12):

```
streamlit run dashboard.py --server.address=0.0.0.0
```

## Auto-start on boot (Section 13)

```
sudo cp systemd/evcharger.service /etc/systemd/system/evcharger.service
sudo systemctl daemon-reload
sudo systemctl enable evcharger.service
sudo systemctl start evcharger.service
journalctl -u evcharger.service -f   # watch live logs
```

## Hardware reminders

- RC522 RFID reader must be powered from the Pi's **3.3V** pin, never 5V.
- Relay signal pin -> GPIO 17; it closes (HIGH) to allow charging.
- STM32 must print one JSON line per second over USB-serial with exactly
  these keys: `solar_v, solar_i, wind_v, grid_ok, batt_v, batt_temp, rx_v,
  rx_i, fault`, and listen for `'S'` (start) / `'X'` (stop) on the same UART.
- `main.py` and `verification_agent.py` import `RPi.GPIO` and `mfrc522`,
  which only work on actual Raspberry Pi hardware — don't try running them
  on a laptop. `setup_db.py` and `test_gemini.py` are hardware-free and run
  anywhere with the right packages installed.

## Common errors

See Section 14.3 of the original guide for the troubleshooting table
(permission errors on the serial port, missing `agents` module, GPIO already
in use, Gemini quota errors, RC522 hangs).
