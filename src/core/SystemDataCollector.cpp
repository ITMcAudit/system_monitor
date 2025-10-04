#include "SystemDataCollector.h"
#include <chrono>
#include <thread>

namespace sysmon {

SystemDataCollector::SystemDataCollector(const Configuration& config)
    : config_(config), collector_(createSystemCollector()) {
}

SystemDataCollector::~SystemDataCollector() {
    stop();
}

bool SystemDataCollector::start() {
    if (!collector_->initialize()) {
        return false;
    }
    
    running_ = true;
    collectionThread_ = std::thread(&SystemDataCollector::collectionLoop, this);
    
    return true;
}

void SystemDataCollector::stop() {
    running_ = false;
    if (collectionThread_.joinable()) {
        collectionThread_.join();
    }
    collector_->shutdown();
}

SystemMetrics SystemDataCollector::getMetrics() const {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    return currentMetrics_;
}

void SystemDataCollector::refresh() {
    collectAllMetrics();
}

void SystemDataCollector::collectionLoop() {
    using namespace std::chrono;
    
    auto lastCpuSample = steady_clock::now();
    auto lastMemorySample = steady_clock::now();
    auto lastDiskSample = steady_clock::now();
    auto lastNetworkSample = steady_clock::now();
    
    while (running_) {
        auto now = steady_clock::now();
        bool updated = false;
        
        SystemMetrics newMetrics;
        
        // CPU sampling
        if (duration_cast<milliseconds>(now - lastCpuSample).count() >= 
            static_cast<int64_t>(config_.cpuSampleIntervalMs)) {
            collector_->collectCPUMetrics(newMetrics);
            lastCpuSample = now;
            updated = true;
        }
        
        // Memory sampling
        if (duration_cast<milliseconds>(now - lastMemorySample).count() >= 
            static_cast<int64_t>(config_.memorySampleIntervalMs)) {
            collector_->collectMemoryMetrics(newMetrics);
            lastMemorySample = now;
            updated = true;
        }
        
        // Disk sampling
        if (duration_cast<milliseconds>(now - lastDiskSample).count() >= 
            static_cast<int64_t>(config_.diskSampleIntervalMs)) {
            collector_->collectDiskMetrics(newMetrics);
            lastDiskSample = now;
            updated = true;
        }
        
        // Network sampling
        if (duration_cast<milliseconds>(now - lastNetworkSample).count() >= 
            static_cast<int64_t>(config_.networkSampleIntervalMs)) {
            collector_->collectNetworkMetrics(newMetrics);
            lastNetworkSample = now;
            updated = true;
        }
        
        if (updated) {
            newMetrics.timestampMs = duration_cast<milliseconds>(
                system_clock::now().time_since_epoch()).count();
            
            std::lock_guard<std::mutex> lock(metricsMutex_);
            // Merge new metrics with current (preserve values not updated this cycle)
            if (newMetrics.cpuUsagePercent > 0) {
                currentMetrics_.cpuUsagePercent = newMetrics.cpuUsagePercent;
                currentMetrics_.perCoreCpuUsage = newMetrics.perCoreCpuUsage;
            }
            if (newMetrics.totalMemoryBytes > 0) {
                currentMetrics_.totalMemoryBytes = newMetrics.totalMemoryBytes;
                currentMetrics_.usedMemoryBytes = newMetrics.usedMemoryBytes;
                currentMetrics_.memoryUsagePercent = newMetrics.memoryUsagePercent;
            }
            currentMetrics_.diskReadBytesPerSec = newMetrics.diskReadBytesPerSec;
            currentMetrics_.diskWriteBytesPerSec = newMetrics.diskWriteBytesPerSec;
            currentMetrics_.networkRecvBytesPerSec = newMetrics.networkRecvBytesPerSec;
            currentMetrics_.networkSendBytesPerSec = newMetrics.networkSendBytesPerSec;
            currentMetrics_.timestampMs = newMetrics.timestampMs;
        }
        
        // Sleep for a short interval
        std::this_thread::sleep_for(milliseconds(100));
    }
}

void SystemDataCollector::collectAllMetrics() {
    SystemMetrics newMetrics;
    
    collector_->collectCPUMetrics(newMetrics);
    collector_->collectMemoryMetrics(newMetrics);
    collector_->collectDiskMetrics(newMetrics);
    collector_->collectNetworkMetrics(newMetrics);
    
    newMetrics.timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    std::lock_guard<std::mutex> lock(metricsMutex_);
    currentMetrics_ = newMetrics;
}

} // namespace sysmon
