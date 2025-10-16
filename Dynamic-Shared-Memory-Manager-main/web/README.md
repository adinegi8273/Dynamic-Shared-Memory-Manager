# Web Frontend for Dynamic Shared Memory Manager

This folder provides a minimal web UI (Flask + vanilla HTML/JS) to interact with the allocator via the interactive CLI.

## Prerequisites
- Python 3.x
- C++17 compiler
  - Windows: MSVC (Developer Command Prompt) or MSYS2/MinGW g++
  - Linux/WSL/macOS: g++/clang++

## Build the CLI
From the project root:

```bash
# g++ (Linux/WSL/MSYS2)
g++ -std=c++17 main.cpp SharedMemory.cpp FirstFitAllocator.cpp -o shm_cli

# Windows (MSVC - from Developer Command Prompt)
cl /std:c++17 main.cpp SharedMemory.cpp FirstFitAllocator.cpp
```

Notes:
- On Windows, no -pthread is required. The code uses a Windows fallback.
- The server will attempt auto-build with g++ if shm_cli is missing, but manual build is recommended on Windows.

## Install Python dependencies
```bash
pip install flask
```

## Start the web server
From the project root:
```bash
python web/app.py
```
Open `http://localhost:5000` in your browser.

## Using the UI
- Click “Start Backend” to launch the CLI subprocess.
- Use the Command box to run:
  - alloc <pid> <size>
  - free <pid> <offset>
  - show
  - help
- Use the Free form to free by pid and offset.
- Output appears in the Output panel.

## Troubleshooting
- Build errors like sys/mman.h: No such file or directory on Windows:
  - Use the latest code which includes Windows fallbacks.
  - Build with MSVC or MSYS2/MinGW.
- If the auto-build fails, build manually (see Build the CLI) and restart web/app.py.
- Port conflicts on 5000: set PORT=5050 before running web/app.py.

## Known Limitations
- Windows fallback uses a process-local heap and CRITICAL_SECTION; it does not provide true cross-process shared memory. If you need real shared memory on Windows, replace the fallback with CreateFileMapping/MapViewOfFile and a named mutex.
