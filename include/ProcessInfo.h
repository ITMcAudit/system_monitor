#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace sysmon {

/**
 * @brief Encapsulates information about a single running process
 * 
 * Memory management: Uses unique_ptr for child ownership, raw pointer for parent reference
 * Thread-safety: Immutable after construction; tree modifications are single-threaded
 */
struct ProcessInfo {
    uint32_t pid{0};                        // Process identifier
    uint32_t parentPid{0};                  // Parent process identifier
    std::string name;                       // Executable name
    double cpuPercent{0.0};                 // Current CPU usage percentage
    uint64_t memoryBytes{0};                // Memory consumption in bytes
    uint64_t creationTime{0};               // Process creation timestamp
    
    // Tree structure
    ProcessInfo* parent{nullptr};           // Non-owning pointer to parent
    std::vector<std::unique_ptr<ProcessInfo>> children;  // Owned child processes
    
    ProcessInfo() = default;
    
    // Disable copy (use move semantics)
    ProcessInfo(const ProcessInfo&) = delete;
    ProcessInfo& operator=(const ProcessInfo&) = delete;
    
    // Enable move
    ProcessInfo(ProcessInfo&&) = default;
    ProcessInfo& operator=(ProcessInfo&&) = default;
    
    /**
     * @brief Calculate total CPU usage including all children
     */
    double getTotalCpuWithChildren() const {
        double total = cpuPercent;
        for (const auto& child : children) {
            total += child->getTotalCpuWithChildren();
        }
        return total;
    }
    
    /**
     * @brief Calculate total memory usage including all children
     */
    uint64_t getTotalMemoryWithChildren() const {
        uint64_t total = memoryBytes;
        for (const auto& child : children) {
            total += child->getTotalMemoryWithChildren();
        }
        return total;
    }
};

} // namespace sysmon
