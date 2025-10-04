#pragma once

#include "ProcessInfo.h"
#include "IProcessCollector.h"
#include "Configuration.h"
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>

namespace sysmon {

/**
 * @brief Constructs and maintains process hierarchy tree
 * 
 * Thread-safety: All public methods are thread-safe
 */
class ProcessTreeBuilder {
public:
    explicit ProcessTreeBuilder(const Configuration& config);
    ~ProcessTreeBuilder();
    
    /**
     * @brief Start process enumeration thread
     */
    bool start();
    
    /**
     * @brief Stop enumeration thread
     */
    void stop();
    
    /**
     * @brief Get current process tree (thread-safe copy)
     */
    std::vector<std::unique_ptr<ProcessInfo>> getProcessTree() const;
    
    /**
     * @brief Terminate a specific process
     */
    bool terminateProcess(uint32_t pid);
    
    /**
     * @brief Force immediate refresh of process tree
     */
    void refresh();
    
private:
    void enumerationLoop();
    void buildTree(std::vector<std::unique_ptr<ProcessInfo>>& processes);
    std::unique_ptr<ProcessInfo> deepCopy(const ProcessInfo& source) const;
    
    Configuration config_;
    std::unique_ptr<IProcessCollector> collector_;
    
    mutable std::mutex treeMutex_;
    std::vector<std::unique_ptr<ProcessInfo>> processRoots_;
    
    std::atomic<bool> running_{false};
    std::thread enumerationThread_;
};

} // namespace sysmon
