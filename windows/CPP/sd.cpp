// Mini Stream Deck - Native Windows C++ controller
//
// Arduino serial protocol (one command per line):
//   BTN:1 ... BTN:9
//   VOL_UP
//   VOL_DOWN
//
// Default serial settings: COM3, 9600 baud, 8 data bits, no parity, 1 stop bit.
// You can choose another port when launching the program, e.g. MiniStreamDeck.exe COM5.
//
// Build with MinGW-w64:
//   g++ -std=c++17 -O2 -static -static-libgcc -static-libstdc++ -Wall -Wextra sd.cpp -o MiniStreamDeck.exe -lshell32 -luser32
//
// Build with Visual Studio (x64 Native Tools Command Prompt):
//   cl /std:c++17 /O2 /MT /EHsc sd.cpp /link shell32.lib user32.lib

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr unsigned long kBaudRate = CBR_9600;
constexpr std::size_t kMaxCommandLength = 256;
volatile LONG g_running = 1;

BOOL WINAPI consoleControlHandler(DWORD signal) {
    switch (signal) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
            InterlockedExchange(&g_running, 0);
            return TRUE;
        default:
            return FALSE;
    }
}

std::wstring widenAscii(const std::string& value) {
    return std::wstring(value.begin(), value.end());
}

std::wstring serialDevicePath(const std::string& port) {
    const std::wstring widePort = widenAscii(port);
    if (widePort.rfind(L"\\\\.\\", 0) == 0) {
        return widePort;
    }
    // The \\.\ prefix also works for COM ports numbered COM10 and above.
    return L"\\\\.\\" + widePort;
}

bool launchApp(const wchar_t* command, const char* displayName) {
    const auto result = reinterpret_cast<INT_PTR>(
        ShellExecuteW(nullptr, L"open", command, nullptr, nullptr, SW_SHOWNORMAL));

    if (result > 32) {
        std::cout << "Opened: " << displayName << '\n';
        return true;
    }

    std::cerr << "Could not open " << displayName
              << " (ShellExecute error " << result << ").\n";
    if (result == SE_ERR_FNF || result == SE_ERR_PNF) {
        std::cerr << "Check that the application is installed and available to Windows.\n";
    }
    return false;
}

void sendShortcut(const std::vector<WORD>& keys) {
    std::vector<INPUT> inputs;
    inputs.reserve(keys.size() * 2);

    // Press keys in order.
    for (const WORD key : keys) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = key;
        inputs.push_back(input);
    }

    // Release keys in reverse order, matching a normal keyboard shortcut.
    for (auto it = keys.rbegin(); it != keys.rend(); ++it) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = *it;
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(input);
    }

    const UINT sent = SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
    if (sent != inputs.size()) {
        std::cerr << "Windows could not send the complete keyboard shortcut.\n";
    }
}

void openVSCode() {
    // The VS Code launcher is commonly installed as code.cmd, so invoke it
    // through cmd.exe to make it work with the standard PATH installation.
    const auto result = reinterpret_cast<INT_PTR>(
        ShellExecuteW(nullptr, L"open", L"cmd.exe", L"/c code", nullptr, SW_HIDE));
    if (result > 32) {
        std::cout << "Opened: Visual Studio Code\n";
    } else {
        std::cerr << "Could not launch Visual Studio Code (ShellExecute error "
                  << result << "). Check that VS Code is installed and its 'code' command is in PATH.\n";
    }
}

void openBrowser() {
    const auto result = reinterpret_cast<INT_PTR>(
        ShellExecuteW(nullptr, L"open", L"https://www.google.com", nullptr, nullptr,
                      SW_SHOWNORMAL));
    if (result > 32) {
        std::cout << "Opened Browser\n";
    } else {
        std::cerr << "Could not open the default browser (ShellExecute error "
                  << result << ").\n";
    }
}

void openCapture() {
    sendShortcut({VK_LWIN, VK_SHIFT, 'S'});
    std::cout << "Opened Snipping Tool\n";
}

void openTaskManager() {
    sendShortcut({VK_CONTROL, VK_SHIFT, VK_ESCAPE});
    std::cout << "Opened Task Manager\n";
}

void lockPC() {
    if (LockWorkStation()) {
        std::cout << "Locking PC\n";
    } else {
        std::cerr << "Windows could not lock the workstation (error "
                  << GetLastError() << ").\n";
    }
}

void volumeUp() {
    sendShortcut({VK_VOLUME_UP});
    std::cout << "Volume UP\n";
}

void volumeDown() {
    sendShortcut({VK_VOLUME_DOWN});
    std::cout << "Volume DOWN\n";
}

void processCommand(std::string command) {
    // Remove whitespace, including the carriage return sent by many Arduino sketches.
    const auto first = command.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return;
    const auto last = command.find_last_not_of(" \t\r\n");
    command = command.substr(first, last - first + 1);

    if (command.rfind("BTN:", 0) == 0) {
        const std::string numberText = command.substr(4);
        try {
            std::size_t parsed = 0;
            const int button = std::stoi(numberText, &parsed);
            if (parsed != numberText.size()) {
                throw std::invalid_argument("unexpected characters");
            }

            std::cout << "\nButton " << button << " pressed\n";
            switch (button) {
                case 1: launchApp(L"wt.exe", "Windows Terminal"); break;
                case 2: openVSCode(); break;
                case 3: openBrowser(); break;
                case 4: launchApp(L"explorer.exe", "File Explorer"); break;
                case 5: launchApp(L"calc.exe", "Calculator"); break;
                case 6: openCapture(); break;
                case 7: openTaskManager(); break;
                case 8: lockPC(); break;
                case 9: launchApp(L"notepad.exe", "Notepad"); break;
                default: std::cout << "Unknown button: " << button << '\n'; break;
            }
        } catch (...) {
            std::cout << "Invalid button command: " << command << '\n';
        }
    } else if (command == "VOL_UP") {
        volumeUp();
    } else if (command == "VOL_DOWN") {
        volumeDown();
    }
}

HANDLE openSerialPort(const std::string& port) {
    const std::wstring devicePath = serialDevicePath(port);
    HANDLE serial = CreateFileW(devicePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (serial == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }

    DCB settings{};
    settings.DCBlength = sizeof(settings);
    if (!GetCommState(serial, &settings)) {
        const DWORD error = GetLastError();
        CloseHandle(serial);
        SetLastError(error);
        return INVALID_HANDLE_VALUE;
    }

    settings.BaudRate = kBaudRate;
    settings.ByteSize = 8;
    settings.Parity = NOPARITY;
    settings.StopBits = ONESTOPBIT;
    settings.fBinary = TRUE;
    settings.fParity = FALSE;
    settings.fOutxCtsFlow = FALSE;
    settings.fOutxDsrFlow = FALSE;
    settings.fDtrControl = DTR_CONTROL_ENABLE;
    settings.fDsrSensitivity = FALSE;
    settings.fOutX = FALSE;
    settings.fInX = FALSE;
    settings.fRtsControl = RTS_CONTROL_DISABLE;
    settings.fAbortOnError = FALSE;

    if (!SetCommState(serial, &settings)) {
        const DWORD error = GetLastError();
        CloseHandle(serial);
        SetLastError(error);
        return INVALID_HANDLE_VALUE;
    }

    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout = 20;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 5;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 5;

    if (!SetCommTimeouts(serial, &timeouts)) {
        const DWORD error = GetLastError();
        CloseHandle(serial);
        SetLastError(error);
        return INVALID_HANDLE_VALUE;
    }

    // Discard any boot messages already waiting in the receive buffer.
    PurgeComm(serial, PURGE_RXCLEAR | PURGE_TXCLEAR);
    return serial;
}
} // namespace

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCtrlHandler(consoleControlHandler, TRUE);

    const std::string port = (argc >= 2 && argv[1][0] != '\0') ? argv[1] : "COM3";

    std::cout << "================================\n"
              << "       MINI STREAM DECK\n"
              << "================================\n"
              << "Connecting to " << port << " at 9600 baud...\n";

    HANDLE serial = openSerialPort(port);
    if (serial == INVALID_HANDLE_VALUE) {
        const DWORD error = GetLastError();
        std::cerr << "\nSerial connection error (Windows error " << error << ").\n\n"
                  << "Check the following:\n"
                  << "1. The Arduino is connected.\n"
                  << "2. The correct COM port is selected.\n"
                  << "3. Arduino Serial Monitor is closed.\n"
                  << "4. No other program is using the COM port.\n\n"
                  << "You can select a port when starting the program, e.g. "
                  << "MiniStreamDeck.exe COM5\n";
        std::cout << "\nPress Enter to exit...";
        std::cin.get();
        return 1;
    }

    // Opening the serial connection may reset an Arduino. Allow it to restart.
    Sleep(2000);
    PurgeComm(serial, PURGE_RXCLEAR);

    std::cout << "Arduino connected!\nWaiting for commands...\n\n";

    std::string line;
    char buffer[128];
    DWORD bytesRead = 0;

    while (InterlockedCompareExchange(&g_running, 1, 1) != 0) {
        if (!ReadFile(serial, buffer, sizeof(buffer), &bytesRead, nullptr)) {
            std::cerr << "\nSerial read failed (Windows error " << GetLastError() << ").\n";
            break;
        }

        for (DWORD i = 0; i < bytesRead; ++i) {
            const char ch = buffer[i];
            if (ch == '\n') {
                processCommand(line);
                line.clear();
            } else if (line.size() < kMaxCommandLength) {
                line.push_back(ch);
            } else {
                // Discard malformed/overlong input until the next newline.
                line.clear();
            }
        }
    }

    CloseHandle(serial);
    std::cout << "\nStream Deck stopped.\nSerial connection closed.\n";
    return 0;
}
