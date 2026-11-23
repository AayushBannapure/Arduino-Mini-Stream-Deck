# Arduino Mini Stream Deck

In this project, I built an **Arduino-based Mini Stream Deck**.

## About the Project

A Stream Deck is a hardware device with several programmable macros.

Although the macros in this project are not directly programmable, it still offers **9 macros**, along with **Volume and Brightness Control**.

This is a beginner project and is not very polished (hardware-wise). Also, it is currently compatible with **Windows 11 only**.

## Hardware & Implementation

I have used:
- **2.4" Touch TFT Display**
- **Joystick Module**
- **Arduino**

The hardware is connected to the computer through the **COM Port**, which is also used for serial communication.

I have used **PySerial** to receive instructions from the Arduino and execute the respective macros using various methods on Windows 11.

### Joystick Controls

The joystick uses only **one axis (X-axis)** due to no analogue GPIO pins being left on the Arduino.

The controls are as follows:

| Input | Function |
|:---|:---|
| +X | VOL_UP |
| -X | VOL_DWN |
| CTRL + X | Brightness UP / DOWN |

Doing the same while holding `CTRL` controls the screen brightness.

## Project Preview

Below is a screenshot of the project:

<img width="1280" height="960" alt="Arduino Mini Stream Deck" src="https://github.com/user-attachments/assets/db09d10b-4d95-4bdd-87dd-dff80e144e2a" />

## How to Run

1. **Download the repository** and extract it into a folder.

2. **Connect the hardware.** Your display might have a different pin layout. Use the `MCUFriend - TouchScreen_Calibr_native.ino` example to identify your pins and modify the Arduino firmware accordingly.

3. **Connect the joystick**, modifying the Arduino firmware accordingly.

4. **Verify the COM Port** being used by the Arduino. Modify both the Arduino firmware and Python script accordingly.

5. **Connect the Arduino** and then run the Python script. Make sure you close the serial port from the Arduino IDE / PlatformIO window before running the script.

6. **Enjoy!**
