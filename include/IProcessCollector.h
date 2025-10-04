#pragma once

#include "ProcessInfo.h"
#include <vector>
#include <memory>

namespace sysmon {

/**
 * @brief Platform abstraction interface for process enumeration
 */
class IProcessCollector {
public:
    virtual ~IProcessCollector() = default;
    
    /**
     * @brief Enumerate all accessible processes
     * @return Vector of process information structures
     */
    virtual std::vector<std::unique_ptr<ProcessInfo>> enumerateProcesses() = 0;
    
    /**
     * @brief Terminate a specific process
     * @param pid Process identifier to terminate
     * @return true if successful, false otherwise
     */
    virtual bool terminateProcess(uint32_t pid) = 0;
    
    /**
     * @brief Initialize collector
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Cleanup collector resources
     */
    virtual void shutdown() = 0;
};

/**
 * @brief Factory function to create platform-specific process collector
 */
std::unique_ptr<IProcessCollector> createProcessCollector();

} // namespace sysmon
