@echo off
setlocal
cd /d "%~dp0"

echo Building Mini Stream Deck...

where g++ >nul 2>nul
if %errorlevel%==0 (
    g++ -std=c++17 -O2 -static -static-libgcc -static-libstdc++ -Wall -Wextra sd.cpp -o MiniStreamDeck.exe -lshell32 -luser32
    if errorlevel 1 goto build_failed
    goto build_success
)

where cl >nul 2>nul
if %errorlevel%==0 (
    cl /std:c++17 /O2 /MT /EHsc sd.cpp /link shell32.lib user32.lib /OUT:MiniStreamDeck.exe
    if errorlevel 1 goto build_failed
    goto build_success
)

echo No supported C++ compiler was found.
echo Install MinGW-w64 and add g++.exe to PATH, or run this script from
 echo the Visual Studio x64 Native Tools Command Prompt.
pause
exit /b 1

:build_failed
echo Build failed. Check the compiler messages above.
pause
exit /b 1

:build_success
echo.
echo Build successful: MiniStreamDeck.exe
echo Run it with the default port using: MiniStreamDeck.exe
 echo Or choose another port using: MiniStreamDeck.exe COM5
pause
exit /b 0
