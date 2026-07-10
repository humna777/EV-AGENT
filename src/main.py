import time
import RPi.GPIO as GPIO

from agents.verification_agent import run_verification
from agents.energy_manager import run_energy_manager
from agents.battery_manager import run_battery_manager
from agents.wpt_control_agent import read_sensor_line, start_transfer, stop_transfer
from agents.fault_detector import run_fault_detector
from agents.shared_config import RELAY_GPIO_PIN

GPIO.setmode(GPIO.BCM)
GPIO.setup(RELAY_GPIO_PIN, GPIO.OUT)
GPIO.output(RELAY_GPIO_PIN, GPIO.LOW)  # relay OFF at startup


def relay_on():
    GPIO.output(RELAY_GPIO_PIN, GPIO.HIGH)
    print("[Orchestrator] Relay ON - power path enabled")


def relay_off():
    GPIO.output(RELAY_GPIO_PIN, GPIO.LOW)
    print("[Orchestrator] Relay OFF - power path disabled")


def wait_for_readings(max_tries=10):
    """Keep reading serial lines from STM32 until we get valid JSON."""
    for _ in range(max_tries):
        readings = read_sensor_line()
        if readings is not None:
            return readings
        time.sleep(0.2)
    return None


def charging_session():
    print("[Orchestrator] Authorized - starting charging session")
    relay_on()
    start_transfer()
    try:
        while True:
            readings = wait_for_readings()
            if readings is None:
                print("[Orchestrator] No data from STM32 - check serial link")
                time.sleep(1)
                continue

            # 1. Fault Detector - highest priority
            fault, reason = run_fault_detector(readings)
            if fault:
                print(f"[Orchestrator] Stopping session due to fault: {reason}")
                break

            # 2. Energy Manager - which source is feeding the system
            run_energy_manager(readings)

            # 3. Battery Manager - charge / throttle / stop
            decision, _ = run_battery_manager(readings)
            if decision == "stop":
                print("[Orchestrator] Battery full / unsafe - ending session")
                break

            time.sleep(2)  # poll every 2 seconds
    finally:
        stop_transfer()
        relay_off()
        print("[Orchestrator] Charging session ended")


def main():
    print("=== EV Charger Agentic AI - Orchestrator started ===")
    try:
        while True:
            authorized, name, uid = run_verification()
            if authorized:
                charging_session()
            else:
                print(f"[Orchestrator] Card {uid} not authorized. Try again.")
                time.sleep(2)
    except KeyboardInterrupt:
        pass
    finally:
        GPIO.cleanup()
        print("=== Orchestrator stopped, GPIO cleaned up ===")


if __name__ == "__main__":
    main()
