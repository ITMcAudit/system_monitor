# System Monitor Architecture

## Table of Contents
1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Component Design](#component-design)
4. [Threading Model](#threading-model)
5. [Data Flow](#data-flow)
6. [Platform Abstraction](#platform-abstraction)
7. [Security Architecture](#security-architecture)
8. [Performance Considerations](#performance-considerations)

## Overview

The System Monitor is a cross-platform terminal-based application designed to provide real-time system resource monitoring with minimal overhead. The architecture emphasizes:

- **Separation of Concerns**: Clear boundaries between collection, processing, and presentation
- **Thread Safety**: Concurrent data collection without UI blocking
- **Platform Independence**: Unified interface across Windows, Linux, and macOS
- **Security by Design**: Minimal privileges, no data persistence, privacy-first approach
- **Performance**: <1% CPU overhead, ~20MB memory footprint

## System Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     User Interface Layer                     │
│                        (FTXUI)                               │
│  ┌─────────────┬──────────────┬─────────────┬──────────┐   │
│  │ CPU Widget  │ Memory Widget│ Disk Widget │ Network  │   │
│  │             │              │             │ Widget   │   │
│  └─────────────┴──────────────┴─────────────┴──────────┘   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │            Process Tree Widget                       │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                            ▲
                            │ Thread-safe read
                            │
┌─────────────────────────────────────────────────────────────┐
│                   Business Logic Layer                       │
│  ┌───────────────────────┬──────────────────────────────┐   │
│  │ SystemDataCollector   │  ProcessTreeBuilder          │   │
│  │                       │                              │   │
│  │ - Metric aggregation  │  - Tree construction         │   │
│  │ - Threshold detection │  - Parent-child linking      │   │
│  │ - Rate calculations   │  - Smart pointer management  │   │
│  └───────────────────────┴──────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                            ▲
                            │ Platform-independent interface
                            │
┌─────────────────────────────────────────────────────────────┐
│                Platform Abstraction Layer                    │
│  ┌──────────────────────┬──────────────────────────────┐    │
│  │ ISystemCollector     │  IProcessCollector           │    │
│  │ (pure virtual)       │  (pure virtual)              │    │
│  └──────────────────────┴──────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                            ▲
                            │ Platform-specific implementation
                            │
┌──────────────┬─────────────────────┬────────────────────────┐
│   Windows    │       Linux         │        macOS           │
│              │                     │                        │
│ - PDH API    │ - /proc filesystem  │ - sysctl               │
│ - ToolHelp32 │ - /proc/[pid]/stat  │ - Mach APIs            │
│ - IP Helper  │ - /proc/net/dev     │ - libproc              │
└──────────────┴─────────────────────┴────────────────────────┘
```

### Layer Responsibilities

#### 1. Presentation Layer (UI)
- **Technology**: FTXUI (Modern C++ terminal UI library)
- **Responsibilities**:
  - Render metrics using gauges, tables, and trees
  - Handle keyboard input and navigation
  - Manage screen layout and responsiveness
  - Provide visual feedback (colors, alerts)
- **Thread Context**: Main thread only
- **Update Frequency**: 30-60 FPS (configurable)

#### 2. Business Logic Layer
- **Components**: SystemDataCollector, ProcessTreeBuilder
- **Responsibilities**:
  - Coordinate platform-specific collectors
  - Aggregate and normalize metrics
  - Calculate derived values (percentages, rates)
  - Detect threshold violations
  - Build process hierarchies
- **Thread Context**: Dedicated collection threads
- **Sampling Frequency**: 1-5 seconds (configurable)

#### 3. Platform Abstraction Layer
- **Interfaces**: ISystemCollector, IProcessCollector
- **Responsibilities**:
  - Define platform-independent contracts
  - Provide factory functions
  - Isolate platform differences
- **Implementation**: Header-only interfaces

#### 4. Platform-Specific Layer
- **Implementations**: Windows, Linux, macOS collectors
- **Responsibilities**:
  - Query OS-specific APIs
  - Handle platform quirks
  - Manage platform resources
- **Compilation**: Conditional compilation per platform

## Component Design

### SystemDataCollector

```cpp
class SystemDataCollector {
    // Thread-safe singleton pattern for metrics
    mutable std::mutex metricsMutex_;
    SystemMetrics currentMetrics_;
    
    // Collection thread
    std::thread collectionThread_;
    std::atomic<bool> running_;
    
    // Platform-specific collector
    std::unique_ptr<ISystemCollector> collector_;
};
```

**Design Patterns**:
- **Dependency Injection**: Platform collector injected via factory
- **Producer-Consumer**: Collection thread produces, UI consumes
- **RAII**: Thread lifecycle managed by object lifetime

**Thread Safety**:
- Mutex-protected shared state
- Atomic running flag
- Complete metric snapshot swaps

### ProcessTreeBuilder

```cpp
class ProcessTreeBuilder {
    // Thread-safe process tree
    mutable std::mutex treeMutex_;
    std::vector<std::unique_ptr<ProcessInfo>> processRoots_;
    
    // Enumeration thread
    std::thread enumerationThread_;
    std::atomic<bool> running_;
};
```

**Design Patterns**:
- **Composite**: Tree structure with uniform interface
- **Smart Pointers**: unique_ptr for ownership, raw pointers for parent refs
- **Deep Copy**: Thread-safe tree snapshots via cloning

**Memory Management**:
- Automatic cleanup via smart pointers
- No circular references (parent uses raw pointer)
- Exception-safe construction

### MonitorUI

```cpp
class MonitorUI {
    // FTXUI component composition
    Component createMainLayout();
    Component createCPUWidget();
    Component createMemoryWidget();
    // ... more widgets
    
    // References to data sources
    SystemDataCollector& dataCollector_;
    ProcessTreeBuilder& processBuilder_;
};
```

**Design Patterns**:
- **Composite**: Nested UI components
- **Observer**: Reactive rendering on data changes
- **Strategy**: Different widgets for different metrics

## Threading Model

### Thread Architecture

```
Main Thread (UI)
├─ Event Loop (FTXUI)
├─ Render Loop (30-60 FPS)
└─ Input Handling

Collection Thread (SystemDataCollector)
├─ CPU Sampling (1s interval)
├─ Memory Sampling (5s interval)
├─ Disk I/O Sampling (1s interval)
└─ Network Sampling (1s interval)

Enumeration Thread (ProcessTreeBuilder)
└─ Process Enumeration (2s interval)
```

### Synchronization Mechanisms

**Mutexes**:
```cpp
std::mutex metricsMutex_;    // Protects SystemMetrics
std::mutex treeMutex_;       // Protects process tree
```

**Atomic Flags**:
```cpp
std::atomic<bool> running_;  // Thread termination signal
```

**Lock Ordering**:
- Single lock per critical section (no lock ordering issues)
- Locks held for minimal duration (copy-out pattern)

### Deadlock Prevention

1. **No Nested Locks**: Each critical section uses single lock
2. **No Lock Inversion**: Consistent lock acquisition order
3. **Timeout-Free**: No timed waits that could fail
4. **RAII Guards**: std::lock_guard prevents forgotten unlocks

## Data Flow

### Metric Collection Flow

```
OS API Call (Platform Layer)
    ↓
Raw Data (uint64_t, counts)
    ↓
Normalization (Business Logic)
    ↓
SystemMetrics struct
    ↓
Mutex-Protected Swap
    ↓
UI Read (lock_guard)
    ↓
FTXUI Rendering
```

### Process Tree Flow

```
Platform Process Enumeration
    ↓
std::vector<ProcessInfo> (flat list)
    ↓
Tree Construction Algorithm
    ↓
Parent-Child Linking (PID map)
    ↓
Root Extraction
    ↓
Mutex-Protected Swap
    ↓
Deep Copy for UI
    ↓
Tree Rendering
```

## Platform Abstraction

### Abstraction Strategy

**Interface Definition**:
```cpp
class ISystemCollector {
public:
    virtual void collectCPUMetrics(SystemMetrics& metrics) = 0;
    virtual void collectMemoryMetrics(SystemMetrics& metrics) = 0;
    // ... more methods
};
```

**Platform Implementation**:
```cpp
#ifdef _WIN32
    class WindowsSystemCollector : public ISystemCollector { ... };
#elif __linux__
    class LinuxSystemCollector : public ISystemCollector { ... };
#elif __APPLE__
    class MacOSSystemCollector : public ISystemCollector { ... };
#endif
```

**Factory Pattern**:
```cpp
std::unique_ptr<ISystemCollector> createSystemCollector() {
    #ifdef _WIN32
        return std::make_unique<WindowsSystemCollector>();
    #elif __linux__
        return std::make_unique<LinuxSystemCollector>();
    #elif __APPLE__
        return std::make_unique<MacOSSystemCollector>();
    #endif
}
```

### Platform-Specific APIs

| Metric | Windows | Linux | macOS |
|--------|---------|-------|-------|
| CPU | PDH API | /proc/stat | host_processor_info |
| Memory | GlobalMemoryStatusEx | /proc/meminfo | vm_statistics64 |
| Disk I/O | (Performance Counters) | /proc/diskstats | IOKit |
| Network | IP Helper API | /proc/net/dev | getifaddrs |
| Processes | ToolHelp32 | /proc/[pid]/* | sysctl, proc_pidinfo |

## Security Architecture

### Security Principles

1. **Least Privilege**:
   - Runs with standard user permissions
   - No administrator/root required for core functionality
   - Read-only system access (except process termination)

2. **Defense in Depth**:
   - Input validation at multiple layers
   - Bounds checking on all array access
   - Safe integer arithmetic (overflow prevention)

3. **Fail Securely**:
   - Errors result in graceful degradation
   - No sensitive data exposed in error messages
   - Secure defaults on configuration errors

4. **Privacy by Default**:
   - No data collection or telemetry
   - No network connections
   - No persistent storage
   - All processing in volatile memory

### Attack Surface

**Minimized Attack Vectors**:
- ✅ No network endpoints
- ✅ No file parsing (except simple config)
- ✅ No user-supplied code execution
- ✅ No privilege escalation attempts
- ✅ No external dependencies (except FTXUI)

**Potential Risks** (Mitigated):
- **Process Name Injection**: Sanitized before display
- **Integer Overflow**: Bounds checking, safe arithmetic
- **Race Conditions**: Mutex protection, atomic operations
- **Memory Leaks**: Smart pointers, RAII patterns

### Security Features

**Memory Safety**:
```cpp
// Smart pointers eliminate manual memory management
std::unique_ptr<ProcessInfo> proc = std::make_unique<ProcessInfo>();

// RAII ensures cleanup
std::lock_guard<std::mutex> lock(mutex_);

// Bounds-checked containers
std::vector<double> cpuUsage; // automatic bounds checking
```

**Thread Safety**:
```cpp
// Atomic flags for clean shutdown
std::atomic<bool> running_{false};

// Mutex-protected shared state
std::lock_guard<std::mutex> lock(metricsMutex_);
SystemMetrics copy = currentMetrics_; // safe copy
```

## Performance Considerations

### Resource Overhead

**CPU Usage**:
- Target: <1% on modern systems
- Collection threads sleep between samples
- UI rendering throttled to 30-60 FPS
- Efficient algorithms (O(n) for most operations)

**Memory Usage**:
- Base: ~20 MB
- Per-process: ~500 bytes
- Circular buffers prevent unbounded growth
- Smart pointers eliminate leaks

**I/O Operations**:
- Minimal disk access (config file only)
- No network I/O
- Efficient /proc reads on Linux (cached file descriptors)

### Optimization Strategies

1. **Lazy Evaluation**: Metrics calculated only when displayed
2. **Batch Updates**: Multiple metrics collected per sample
3. **Delta Calculation**: Only compute changes, not absolutes
4. **Selective Rendering**: Redraw only changed UI regions

### Scalability

**Small Systems** (embedded, single-core):
- Reduced sampling frequency
- Limited process tree depth
- Minimal UI refresh rate

**Large Systems** (servers, many cores):
- Efficient tree construction (hash maps)
- Process filtering options
- Adaptive sampling intervals

## Extension Points

### Adding New Metrics

1. Add field to `SystemMetrics` struct
2. Implement collection in platform collectors
3. Create UI widget for display
4. Update documentation

### Adding New Platforms

1. Implement `ISystemCollector` interface
2. Implement `IProcessCollector` interface
3. Add platform detection to CMake
4. Update factory functions

### Custom UI Layouts

1. Create new `Component` composition
2. Implement custom rendering logic
3. Integrate with existing event handling

## Conclusion

The System Monitor architecture balances simplicity with extensibility, security with functionality, and performance with features. The clear separation of concerns, robust thread safety, and platform abstraction enable maintainable, portable, and secure system monitoring.

---

**Document Version**: 1.0  
**Last Updated**: October 3, 2025  
**Maintained By**: System Monitor Development Team
