#pragma once

#include <vector>
#include <cstdint>

namespace sysmon {

/**
 * @brief Aggregates current readings for all monitored system resources
 * 
 * Thread-safety: Designed for atomic updates via complete instance swaps
 */
struct SystemMetrics {
    // CPU metrics
    double cpuUsagePercent{0.0};           // Overall CPU usage (0-100)
    std::vector<double> perCoreCpuUsage;   // Per-core CPU usage (0-100)
    
    // Memory metrics
    uint64_t totalMemoryBytes{0};          // Total physical memory
    uint64_t usedMemoryBytes{0};           // Currently used memory
    double memoryUsagePercent{0.0};        // Memory usage percentage
    
    // Disk metrics
    uint64_t diskReadBytesPerSec{0};       // Disk read throughput
    uint64_t diskWriteBytesPerSec{0};      // Disk write throughput
    
    // Network metrics
    uint64_t networkRecvBytesPerSec{0};    // Network receive throughput
    uint64_t networkSendBytesPerSec{0};    // Network transmit throughput
    
    // Timestamp
    uint64_t timestampMs{0};               // Sample timestamp in milliseconds
    
    SystemMetrics() = default;
    SystemMetrics(const SystemMetrics&) = default;
    SystemMetrics& operator=(const SystemMetrics&) = default;
    SystemMetrics(SystemMetrics&&) = default;
    SystemMetrics& operator=(SystemMetrics&&) = default;
};

} // namespace sysmon
