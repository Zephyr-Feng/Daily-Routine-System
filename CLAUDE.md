# Daily-Routine-System

A C++17 schedule management application built with CMake and OpenSSL.

## Architecture

```
src/
├── main.cpp       # Entry point — currently prints a test task name
├── task.h/.cpp    # Task class: id, name, start/reminder time, priority (high/medium/low), classify (study/play/life)
├── auth.h/.cpp    # User auth: SHA256 password hashing via OpenSSL, register/login stored in users.txt
```

## Development Environment

- **VM**: `ssh code@192.168.153.128` (Ubuntu 22.04 VM in VMware)
  - Project also cloned at `~/Daily-Routine-System/` on VM
- **Local**: `D:\claude\Daily-Routine-System`
- **Current branch**: `picture_ui`

## Build (on VM)

```bash
mkdir -p build && cd build
cmake ..
make
./myschedule
```

## Key Dependencies

- **CMake >= 3.10**
- **OpenSSL** (`libssl-dev`) — `sudo apt install libssl-dev`
- **C++17** required

## Conventions

- `using namespace std;` used throughout
- Auth stores hashed passwords in `users.txt` (local file)
- Static `next_id` counter for auto-incrementing Task IDs

## Git

- **Repo**: git@github.com:Zephyr-Feng/Daily-Routine-System.git
- **GitHub**: https://github.com/Zephyr-Feng/Daily-Routine-System
