MINI STREAM DECK - WINDOWS BUILD AND USE
========================================

FILES
-----
sd.cpp       C++ source code for the Windows controller.
build.bat    Builds MiniStreamDeck.exe on a Windows PC with a C++ compiler.

BUILD THE EXECUTABLE
--------------------
1. Install either MinGW-w64 (with g++ added to PATH) or Visual Studio Build
   Tools with the C++ desktop development tools.
2. Put sd.cpp and build.bat in the same folder.
3. Double-click build.bat, or run it from Command Prompt.
4. A successful build creates MiniStreamDeck.exe in that folder.

RUN
---
- Connect the Arduino and close Arduino IDE Serial Monitor / other serial tools.
- Double-click MiniStreamDeck.exe to use the default COM3 port.
- For a different port, open Command Prompt in the folder and run:
      MiniStreamDeck.exe COM5
  Replace COM5 with the Arduino's actual port.

ARDUINO / SERIAL REQUIREMENTS
-----------------------------
The program expects the Arduino to send one command per line at 9600 baud:
    BTN:1 through BTN:9
    VOL_UP
    VOL_DOWN

The Arduino firmware and wiring do not need to change as long as they already
send those exact commands at 9600 baud.

BUTTON MAPPING
--------------
1  Windows Terminal
2  Visual Studio Code
3  Default browser (opens Google)
4  File Explorer
5  Calculator
6  Snipping Tool
7  Task Manager
8  Lock PC
9  Notepad

Volume commands use the Windows media-volume keys.

PORTABILITY NOTES
-----------------
The compiled EXE does not require Python, pyserial, or pyautogui. The supplied
build commands statically link the C++ runtime, so recipients should not need
separate MinGW or Visual C++ runtime DLLs.
It is intended for Windows. Each computer needs a suitable Arduino USB-serial
driver and the Arduino's COM port may differ. Visual Studio Code and Windows
Terminal must be installed for buttons 1 and 2 to work; the VS Code 'code'
launcher must be available to Windows, as in the original Python version.

Build the EXE once on a Windows build machine, then distribute MiniStreamDeck.exe.
Recipients do not need a C++ compiler.
