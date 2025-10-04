# Complete Build Instructions
## System Monitor - Step-by-Step Build Guide

**Date**: October 4, 2025  
**Platform**: Windows (with Linux/macOS instructions included)

---

## Prerequisites

### Windows

#### 1. Install Visual Studio 2022
- Download: https://visualstudio.microsoft.com/downloads/
- Install "Desktop development with C++" workload
- Includes MSVC compiler and Windows SDK

#### 2. Install CMake
**Option A - Chocolatey** (recommended):
```powershell
# Install Chocolatey if not already installed
Set-ExecutionPolicy Bypass -Scope Process -Force
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# Install CMake
choco install cmake -y
```

**Option B - Direct Download**:
- Download: https://cmake.org/download/
- Install to PATH

#### 3. Install Git (if not already installed)
```powershell
choco install git -y
```

#### 4. Verify Installation
```powershell
# Check CMake
cmake --version
# Should show: cmake version 3.20 or higher

# Check MSVC
where cl
# Should show: C:\Program Files\Microsoft Visual Studio\...\cl.exe

# Check Git
git --version
```

---

## Build Process

### Quick Build (Automated)

```powershell
# Navigate to project directory
cd C:\SystemMonitor

# Run the build script
.\build.ps1 -BuildType Release

# Expected output:
# - CMake configuration
# - FTXUI download and build
# - SystemMonitor compilation
# - Executable: build\Release\SystemMonitor.exe
```

### Manual Build (Step by Step)

```powershell
# 1. Create build directory
mkdir build
cd build

# 2. Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# 3. Build Release version
cmake --build . --config Release

# 4. Build Debug version (optional)
cmake --build . --config Debug

# 5. Executable locations:
# Release: build\Release\SystemMonitor.exe
# Debug: build\Debug\SystemMonitor.exe
```

---

## Running the Application

### Basic Usage

```powershell
# Run with default settings
.\build\Release\SystemMonitor.exe

# Show help
.\build\Release\SystemMonitor.exe --help

# Custom CPU sampling interval (1 second)
.\build\Release\SystemMonitor.exe --cpu-interval 1000

# Custom memory interval (5 seconds)
.\build\Release\SystemMonitor.exe --memory-interval 5000

# Set alert thresholds
.\build\Release\SystemMonitor.exe --cpu-threshold 85 --memory-threshold 90

# Disable colors
.\build\Release\SystemMonitor.exe --no-colors

# Expand process tree by default
.\build\Release\SystemMonitor.exe --expand-tree

# Set custom FPS
.\build\Release\SystemMonitor.exe --fps 60
```

### With Configuration File

```powershell
# Copy example config
copy examples\.sysmonrc ~\.sysmonrc

# Edit configuration
notepad ~\.sysmonrc

# Run (will auto-load config)
.\build\Release\SystemMonitor.exe
```

---

## Expected Build Output

### Successful Build

```
System Monitor Build Script
===========================

Creating build directory...
Configuring CMake...
-- The CXX compiler identification is MSVC 19.39.33519.0
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.39.33519/bin/Hostx64/x64/cl.exe - works
-- Configuring done
-- Generating done
-- Build files have been written to: C:/SystemMonitor/build

Building project (Release)...
[  5%] Fetching FTXUI v5.0.0...
[ 10%] Building FTXUI components...
[ 25%] Building Configuration.cpp
[ 30%] Building SystemDataCollector.cpp
[ 35%] Building ProcessTreeBuilder.cpp
[ 40%] Building WindowsSystemCollector.cpp
[ 45%] Building WindowsProcessCollector.cpp
[ 50%] Building MonitorUI.cpp
[ 55%] Building main.cpp
[ 60%] Linking CXX executable SystemMonitor.exe
[100%] Built target SystemMonitor

Build successful!
Executable location: build\Release\SystemMonitor.exe

To run the application:
  cd build\Release
  .\SystemMonitor.exe
```

### File Sizes (Approximate)
- SystemMonitor.exe: ~2-3 MB (Release build)
- SystemMonitor.exe: ~5-8 MB (Debug build with symbols)

---

## Linux Build Instructions

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake git

# Fedora/RHEL
sudo dnf install -y gcc-c++ cmake git

# Arch Linux
sudo pacman -S base-devel cmake git
```

### Build

```bash
cd ~/system_monitor
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./SystemMonitor
```

---

## macOS Build Instructions

### Prerequisites

```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake git

# Install Xcode Command Line Tools
xcode-select --install
```

### Build

```bash
cd ~/system_monitor
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
./SystemMonitor
```

---

## Troubleshooting

### Issue: "CMake not found"
**Solution**:
```powershell
# Verify CMake installation
where cmake

# If not found, reinstall:
choco install cmake -y

# Restart PowerShell to refresh PATH
```

### Issue: "MSVC compiler not found"
**Solution**:
1. Open Visual Studio Installer
2. Modify Visual Studio 2022
3. Ensure "Desktop development with C++" is checked
4. Install and restart

### Issue: "Cannot find cl.exe"
**Solution**:
```powershell
# Use Developer Command Prompt for VS 2022 instead:
# Start Menu -> Visual Studio 2022 -> Developer Command Prompt
# Then run build.ps1
```

### Issue: "FTXUI download fails"
**Solution**:
```powershell
# Check internet connection
# Check firewall settings
# Or manually clone FTXUI:
cd build
git clone https://github.com/ArthurSonzogni/ftxui
cd ftxui
git checkout v5.0.0
cd ../..
cmake .. -DFETCHCONTENT_FULLY_DISCONNECTED=ON
```

### Issue: "Build fails with errors"
**Solution**:
```powershell
# Clean build
rm -r build
mkdir build

# Try Debug build for better error messages
.\build.ps1 -BuildType Debug

# Check error output carefully
```

### Issue: "Application crashes on startup"
**Solution**:
```powershell
# Run Debug build to see error messages
.\build\Debug\SystemMonitor.exe

# Check permissions (may need admin for some metrics)
# Right-click SystemMonitor.exe -> Run as Administrator
```

---

## Verification

### Test the Build

```powershell
# 1. Help text (should display and exit)
.\build\Release\SystemMonitor.exe --help

# 2. Invalid config (should error gracefully)
.\build\Release\SystemMonitor.exe --cpu-interval 50000

# 3. Short run (Ctrl+C to exit after viewing)
.\build\Release\SystemMonitor.exe

# 4. Check CPU/Memory usage
# Task Manager -> Details -> SystemMonitor.exe
# Should use < 1% CPU, ~20-50 MB RAM
```

### Expected UI

When you run the application, you should see:

```
System Monitor v1.0.0
Initializing...

Starting system data collector...
Starting process tree builder...
Launching UI...
```

Then the terminal UI should display:

```
┌─────────────────────────────────────────────────────────────────┐
│ CPU Usage                                                       │
├─────────────────────────────────────────────────────────────────┤
│ Overall: 23.5%  ████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░      │
│ Core 0: 45.2%   Core 1: 12.3%   Core 2: 56.7%   Core 3: 8.9%  │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ Memory                                                          │
├─────────────────────────────────────────────────────────────────┤
│ Used: 12.5 GB / Total: 16.0 GB  ████████████████████░░░  78.1% │
└─────────────────────────────────────────────────────────────────┘

┌────────────────────────────┬────────────────────────────────────┐
│ Disk I/O                   │ Network I/O                        │
├────────────────────────────┼────────────────────────────────────┤
│ Read:  125.3 MB/s          │ Recv: 2.5 MB/s                     │
│ Write: 45.7 MB/s           │ Send: 1.2 MB/s                     │
└────────────────────────────┴────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ Processes (45 roots)                                            │
├─────────────────────────────────────────────────────────────────┤
│ PID    │ CPU%   │ Memory     │ Name                            │
├────────┼────────┼────────────┼─────────────────────────────────┤
│ 1234   │ 15.2%  │ 256.0 MB   │ chrome.exe                      │
│ 5678   │ 8.5%   │ 128.0 MB   │   chrome.exe (renderer)         │
│ 9012   │ 2.1%   │ 64.0 MB    │   chrome.exe (renderer)         │
│ 4321   │ 5.6%   │ 512.0 MB   │ code.exe                        │
└─────────────────────────────────────────────────────────────────┘

12:34:56 │ q:Quit r:Refresh Tab:Navigate
```

### Keyboard Controls

- **q**: Quit application
- **r**: Force refresh
- **Tab**: Navigate between widgets
- **Ctrl+C**: Emergency exit

---

## Performance Verification

### Expected Metrics

| Metric | Expected | How to Verify |
|--------|----------|---------------|
| CPU Usage | < 1% | Task Manager → SystemMonitor.exe |
| Memory | 20-50 MB | Task Manager → SystemMonitor.exe |
| Startup Time | < 1 second | Time from launch to UI |
| UI Refresh | Smooth 30-60 FPS | Visual observation |

### Benchmark

```powershell
# Monitor SystemMonitor's own resource usage
# Run for 5 minutes and observe:
1. CPU should stay below 1%
2. Memory should be stable (no leaks)
3. UI should remain responsive
4. No crashes or freezes
```

---

## Installation (Optional)

### System-wide Installation

```powershell
# Copy to Program Files
mkdir "C:\Program Files\SystemMonitor"
copy build\Release\SystemMonitor.exe "C:\Program Files\SystemMonitor\"

# Add to PATH
$env:Path += ";C:\Program Files\SystemMonitor"
[Environment]::SetEnvironmentVariable("Path", $env:Path, [System.EnvironmentVariableTarget]::Machine)

# Now run from anywhere:
SystemMonitor.exe
```

### Create Desktop Shortcut

```powershell
$WshShell = New-Object -comObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("$env:USERPROFILE\Desktop\System Monitor.lnk")
$Shortcut.TargetPath = "C:\SystemMonitor\build\Release\SystemMonitor.exe"
$Shortcut.WorkingDirectory = "C:\SystemMonitor"
$Shortcut.Description = "System Monitor v1.0.0"
$Shortcut.Save()
```

---

## Development Build

### For Development/Debugging

```powershell
# Build Debug version
.\build.ps1 -BuildType Debug

# Run with debugger attached (Visual Studio)
devenv build\SystemMonitor.sln

# Or run standalone
.\build\Debug\SystemMonitor.exe
```

### Sanitizer Builds (Advanced)

```powershell
# Address Sanitizer (detect memory errors)
cmake .. -DCMAKE_BUILD_TYPE=Debug -DSANITIZE_ADDRESS=ON
cmake --build . --config Debug

# Thread Sanitizer (detect race conditions)
cmake .. -DCMAKE_BUILD_TYPE=Debug -DSANITIZE_THREAD=ON
cmake --build . --config Debug
```

---

## Continuous Integration

### GitHub Actions (Already Configured)

The project includes `.github/workflows/ci.yml` which will:
1. Build on Windows, Linux, macOS
2. Run security scans
3. Upload artifacts

### Manual CI Test

```powershell
# Simulate CI build
.\build.ps1 -Clean -BuildType Release
.\build\Release\SystemMonitor.exe --help
```

---

## Next Steps After Building

1. ✅ **Test on your system** - Run and verify all metrics display correctly
2. ✅ **Performance check** - Verify < 1% CPU usage
3. ✅ **Create GitHub release** - Tag v1.0.0 and upload binary
4. ✅ **Share with others** - Distribute the executable
5. ✅ **Collect feedback** - Identify any platform-specific issues

---

## Support

If you encounter issues during build:

1. **Check prerequisites** - Ensure all tools installed correctly
2. **Clean build** - `.\build.ps1 -Clean`
3. **Check logs** - Review CMake output for specific errors
4. **GitHub Issues** - Report build problems with full error output
5. **Documentation** - Review `docs/ARCHITECTURE.md` for technical details

---

## Build Flags Reference

### CMake Options

```powershell
# Standard Release build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Debug build with symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Custom install prefix
cmake .. -DCMAKE_INSTALL_PREFIX="C:\Custom\Path"

# Verbose build output
cmake --build . --verbose

# Parallel build (use all CPU cores)
cmake --build . --parallel
```

### Compiler Flags (Automatic)

**Release Build**:
- Optimization: `/O2` (MSVC) or `-O3` (GCC/Clang)
- Warnings: `/W4 /WX` (MSVC) or `-Wall -Wextra -Werror` (GCC/Clang)

**Debug Build**:
- Optimization: None
- Debug symbols: Full
- Sanitizers: Available

---

**Build Instructions Version**: 1.0  
**Last Updated**: October 4, 2025  
**Status**: Production Ready

---

## Quick Reference Card

```
┌─────────────────────────────────────────────────────────────┐
│                 QUICK BUILD REFERENCE                       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Prerequisites:                                             │
│    • Visual Studio 2022 (C++ workload)                      │
│    • CMake 3.20+                                            │
│    • Git                                                    │
│                                                             │
│  Build Command:                                             │
│    .\build.ps1 -BuildType Release                           │
│                                                             │
│  Run Command:                                               │
│    .\build\Release\SystemMonitor.exe                        │
│                                                             │
│  Help:                                                      │
│    .\build\Release\SystemMonitor.exe --help                 │
│                                                             │
│  Clean Build:                                               │
│    .\build.ps1 -Clean -BuildType Release                    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

**Ready to build? Run:** `.\build.ps1 -BuildType Release`
