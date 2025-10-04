#pragma once

#include "SystemMetrics.h"
#include <memory>

namespace sysmon {

/**
 * @brief Platform abstraction interface for system metric collection
 * 
 * Platform-specific implementations provide concrete behavior
 */
class ISystemCollector {
public:
    virtual ~ISystemCollector() = default;
    
    /**
     * @brief Collect current CPU metrics
     */
    virtual void collectCPUMetrics(SystemMetrics& metrics) = 0;
    
    /**
     * @brief Collect current memory metrics
     */
    virtual void collectMemoryMetrics(SystemMetrics& metrics) = 0;
    
    /**
     * @brief Collect current disk I/O metrics
     */
    virtual void collectDiskMetrics(SystemMetrics& metrics) = 0;
    
    /**
     * @brief Collect current network I/O metrics
     */
    virtual void collectNetworkMetrics(SystemMetrics& metrics) = 0;
    
    /**
     * @brief Initialize collector (called once at startup)
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Cleanup collector resources
     */
    virtual void shutdown() = 0;
};

/**
 * @brief Factory function to create platform-specific collector
 */
std::unique_ptr<ISystemCollector> createSystemCollector();

} // namespace sysmon
