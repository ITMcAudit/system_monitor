#pragma once

#include "SystemMetrics.h"
#include "ISystemCollector.h"
#include "Configuration.h"
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>

namespace sysmon {

/**
 * @brief Main system data collection coordinator
 * 
 * Manages platform-specific collectors and provides thread-safe access to metrics
 * Thread-safety: All public methods are thread-safe
 */
class SystemDataCollector {
public:
    explicit SystemDataCollector(const Configuration& config);
    ~SystemDataCollector();
    
    /**
     * @brief Start collection threads
     */
    bool start();
    
    /**
     * @brief Stop collection threads
     */
    void stop();
    
    /**
     * @brief Get current system metrics (thread-safe)
     */
    SystemMetrics getMetrics() const;
    
    /**
     * @brief Force immediate refresh of all metrics
     */
    void refresh();
    
private:
    void collectionLoop();
    void collectAllMetrics();
    
    Configuration config_;
    std::unique_ptr<ISystemCollector> collector_;
    
    mutable std::mutex metricsMutex_;
    SystemMetrics currentMetrics_;
    
    std::atomic<bool> running_{false};
    std::thread collectionThread_;
};

} // namespace sysmon
