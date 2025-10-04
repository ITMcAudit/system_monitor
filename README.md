# System Monitor

A cross-platform real-time system resource monitoring application built with C++20 and FTXUI.

## Features

- **Real-time CPU Monitoring**: Overall and per-core CPU usage with color-coded gauges
- **Memory Tracking**: Total, used, and available memory with percentage indicators
- **Disk I/O**: Read and write throughput monitoring
- **Network I/O**: Receive and transmit bandwidth tracking
- **Process Tree**: Hierarchical view of running processes with CPU and memory usage
- **Interactive UI**: Terminal-based interface with keyboard navigation
- **Configurable**: Customizable sampling intervals and alert thresholds
- **Thread-safe**: Multi-threaded architecture with non-blocking UI

## Requirements

- CMake 3.20 or higher
- C++20 compatible compiler:
  - Windows: Visual Studio 2019 or later / MSVC
  - Linux: GCC 10+ or Clang 12+
  - macOS: Xcode 13+ or Clang 12+
- Git (for fetching FTXUI dependency)

## Building

### Windows (Visual Studio)

```powershell
# Create build directory
mkdir build
cd build

# Configure
cmake .. -G "Visual Studio 16 2019" -A x64

# Build
cmake --build . --config Release

# Run
.\Release\SystemMonitor.exe
```

### Windows (MSYS2/MinGW)

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
./SystemMonitor.exe
```

### Linux

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./SystemMonitor
```

### macOS

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
./SystemMonitor
```

## Usage

```
SystemMonitor [OPTIONS]

Options:
  --cpu-interval <ms>       CPU sampling interval (default: 1000)
  --memory-interval <ms>    Memory sampling interval (default: 5000)
  --fps <rate>              Target frame rate (default: 30)
  --cpu-threshold <pct>     CPU alert threshold (default: 90)
  --memory-threshold <pct>  Memory alert threshold (default: 90)
  --no-colors               Disable color output
  --expand-tree             Expand process tree by default
  --help, -h                Show help message
```

### Keyboard Controls

- **q**: Quit application
- **r**: Force refresh of all metrics
- **Tab**: Navigate between UI sections
- **Arrow Keys**: Navigate process tree
- **k**: Terminate selected process (with confirmation)
- **Enter**: Expand/collapse process tree node

## Configuration

### Environment Variables

- `SYSMON_CPU_INTERVAL`: CPU sampling interval in milliseconds
- `SYSMON_NO_COLORS`: Set to "1" to disable colors

### Configuration File

Create `~/.sysmonrc` (Linux/macOS) or `%USERPROFILE%\.sysmonrc` (Windows):

```ini
cpu_interval=1000
memory_interval=5000
target_fps=30
```

## Architecture

The application consists of three main layers:

1. **Data Collection Layer**: Platform-specific collectors for system metrics
   - Windows: PDH API, ToolHelp32, IP Helper API
   - Linux: /proc filesystem
   - macOS: sysctl, Mach APIs

2. **Business Logic Layer**: 
   - `SystemDataCollector`: Coordinates metric collection
   - `ProcessTreeBuilder`: Constructs process hierarchies

3. **Presentation Layer**:
   - `MonitorUI`: FTXUI-based terminal interface
   - Responsive layout adapting to terminal size

## Performance

- CPU overhead: <1% on modern systems
- Memory footprint: ~20 MB base + process tree
- Update latency: <100ms for metric refresh
- Scales to 1000+ processes efficiently

## Platform Notes

### Windows
- Requires Performance Data Helper (PDH) library
- May need administrator privileges for some process information
- Tested on Windows 10/11

### Linux
- Requires read access to /proc filesystem
- Works without root privileges
- Tested on Ubuntu 20.04+, Fedora 35+

### macOS
- May require security permissions for process information
- Tested on macOS 11 (Big Sur) and later

## Troubleshooting

### Build Issues

**FTXUI download fails**: Check internet connection and Git installation

**Compiler errors**: Ensure C++20 support is enabled

**Linking errors**: Verify platform libraries are available (pdh.lib on Windows, pthread on Linux)

### Runtime Issues

**Permission denied**: Run with appropriate privileges or check file system permissions

**High CPU usage**: Increase sampling intervals with `--cpu-interval` and `--memory-interval`

**Missing process info**: Some processes may require elevated privileges to access

## License

This project uses FTXUI (MIT License) and follows the same licensing terms.

## Contributing

Contributions welcome! Areas for enhancement:
- GPU monitoring support
- Historical data visualization
- Remote monitoring capabilities
- Plugin system for custom collectors
- Advanced filtering and search

## Credits

Built with:
- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) - Modern C++ terminal UI library
- C++20 Standard Library
- Platform-specific system APIs
