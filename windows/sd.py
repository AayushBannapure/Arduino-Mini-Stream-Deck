
import serial
import time
import subprocess
import webbrowser
import pyautogui

# =====================================================
# 1. SERIAL CONFIGURATION
# =====================================================

PORT = "COM3"
BAUD_RATE = 9600

pyautogui.PAUSE = 0.02

# =====================================================
# 2. APPLICATION LAUNCHER
# =====================================================

def launch_app(command, name):

    try:
        subprocess.Popen(command)
        print(f"Opened: {name}")

    except FileNotFoundError:
        print(f"Could not find: {name}")

    except Exception as e:
        print(f"Error opening {name}: {e}")


# =====================================================
# 3. BUTTON ACTIONS
# =====================================================

def open_terminal():
    launch_app(["wt.exe"], "Windows Terminal")


def open_vscode():
    launch_app(["code.cmd"], "Visual Studio Code")


def open_browser():
    webbrowser.open(url="https://www.google.com")
    print("Opened Browser")


def open_files():
    launch_app(["explorer.exe"], "File Explorer")


def open_calculator():
    launch_app(["calc.exe"], "Calculator")


def open_capture():
    pyautogui.hotkey("win", "shift", "s")
   #print("Opened Snipping Tool")


def open_task_manager():
    pyautogui.hotkey("ctrl", "shift", "esc")
    print("Opened Task Manager")


def lock_pc():
    pyautogui.hotkey("win", "d")
    print("Minimise Windows")


def open_notepad():
    launch_app(["notepad.exe"], "Notepad")


# =====================================================
# 4. BUTTON MAPPING
# =====================================================

BUTTON_ACTIONS = {

    1: open_terminal,
    2: open_vscode,
    3: open_browser,

    4: open_files,
    5: open_calculator,
    6: open_capture,

    7: open_task_manager,
    8: lock_pc,
    9: open_notepad

}


# =====================================================
# 5. VOLUME CONTROL
# =====================================================

def volume_up():

    pyautogui.press("volumeup")
   #print("Volume UP")


def volume_down():

    pyautogui.press("volumedown")
   #print("Volume DOWN")


# =====================================================
# 6. COMMAND PROCESSOR
# =====================================================

def process_command(command):

    command = command.strip()

    # Touchscreen button commands
    if command.startswith("BTN:"):

        try:
            button_number = int(command.split(":")[1])

            action = BUTTON_ACTIONS.get(button_number)

            if action:
                print(f"\nButton {button_number} pressed")
                action()

            else:
               print(f"Unknown button: {button_number}")

        except ValueError:
            print(f"Invalid button command: {command}")

    # Joystick volume commands
    elif command == "VOL_UP":

        volume_up()

    elif command == "VOL_DOWN":

        volume_down()


# =====================================================
# 7. MAIN SERIAL LOOP
# =====================================================

def main():

    print("================================")
    print("       MINI STREAM DECK")
    print("================================")

    print(f"Connecting to {PORT}...")

    try:

        ser = serial.Serial(
            PORT,
            BAUD_RATE,
            timeout=0.1
        )

        # Arduino resets when serial connection opens
        time.sleep(2)

        ser.reset_input_buffer()

        print("Arduino connected!")
        print("Waiting for commands...\n")

        while True:

            if ser.in_waiting:

                raw_data = ser.readline()

                command = raw_data.decode(
                    "utf-8",
                    errors="ignore"
                ).strip()

                if command:
                    process_command(command)

    except serial.SerialException as e:

        print("\nSerial connection error:")
        print(e)

        print("\nCheck the following:")
        print("1. Arduino is connected.")
        print("2. Correct COM port is selected.")
        print("3. Serial Monitor is closed.")
        print("4. No other program is using the COM port.")

    except KeyboardInterrupt:

        print("\nStream Deck stopped.")

    finally:

        if "ser" in locals() and ser.is_open:
            ser.close()

        print("Serial connection closed.")


# =====================================================
# 8. START APPLICATION
# =====================================================

if __name__ == "__main__":
    main()
