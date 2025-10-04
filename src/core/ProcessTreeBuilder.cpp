#include "ProcessTreeBuilder.h"
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <thread>

namespace sysmon {

ProcessTreeBuilder::ProcessTreeBuilder(const Configuration& config)
    : config_(config), collector_(createProcessCollector()) {
}

ProcessTreeBuilder::~ProcessTreeBuilder() {
    stop();
}

bool ProcessTreeBuilder::start() {
    if (!collector_->initialize()) {
        return false;
    }
    
    running_ = true;
    enumerationThread_ = std::thread(&ProcessTreeBuilder::enumerationLoop, this);
    
    return true;
}

void ProcessTreeBuilder::stop() {
    running_ = false;
    if (enumerationThread_.joinable()) {
        enumerationThread_.join();
    }
    collector_->shutdown();
}

std::vector<std::unique_ptr<ProcessInfo>> ProcessTreeBuilder::getProcessTree() const {
    std::lock_guard<std::mutex> lock(treeMutex_);
    
    // Deep copy the tree for thread safety
    std::vector<std::unique_ptr<ProcessInfo>> copy;
    copy.reserve(processRoots_.size());
    
    for (const auto& root : processRoots_) {
        copy.push_back(deepCopy(*root));
    }
    
    return copy;
}

bool ProcessTreeBuilder::terminateProcess(uint32_t pid) {
    return collector_->terminateProcess(pid);
}

void ProcessTreeBuilder::refresh() {
    // Trigger immediate enumeration by waking the thread
    // (simplified - could use condition variable)
}

void ProcessTreeBuilder::enumerationLoop() {
    using namespace std::chrono;
    
    while (running_) {
        auto processes = collector_->enumerateProcesses();
        buildTree(processes);
        
        {
            std::lock_guard<std::mutex> lock(treeMutex_);
            processRoots_ = std::move(processes);
        }
        
        std::this_thread::sleep_for(milliseconds(config_.processSampleIntervalMs));
    }
}

void ProcessTreeBuilder::buildTree(std::vector<std::unique_ptr<ProcessInfo>>& processes) {
    // Create lookup map: PID -> ProcessInfo*
    std::unordered_map<uint32_t, ProcessInfo*> pidMap;
    for (auto& proc : processes) {
        pidMap[proc->pid] = proc.get();
    }
    
    // Build parent-child relationships
    std::vector<std::unique_ptr<ProcessInfo>> roots;
    
    for (auto& proc : processes) {
        auto parentIt = pidMap.find(proc->parentPid);
        
        if (parentIt != pidMap.end() && parentIt->second->pid != proc->pid) {
            // Found parent - verify creation time to avoid PID reuse issues
            ProcessInfo* parent = parentIt->second;
            
            // If parent was created before child, establish relationship
            if (parent->creationTime < proc->creationTime) {
                proc->parent = parent;
                // We'll move children in a second pass
            } else {
                // Parent PID reused - treat as root
                proc->parent = nullptr;
            }
        } else {
            // No parent found - root process
            proc->parent = nullptr;
        }
    }
    
    // Second pass: move processes to their parents' children vectors
    auto it = processes.begin();
    while (it != processes.end()) {
        if ((*it)->parent != nullptr) {
            ProcessInfo* parent = (*it)->parent;
            parent->children.push_back(std::move(*it));
            it = processes.erase(it);
        } else {
            ++it;
        }
    }
    
    // Remaining processes in the vector are roots
}

std::unique_ptr<ProcessInfo> ProcessTreeBuilder::deepCopy(const ProcessInfo& source) const {
    auto copy = std::make_unique<ProcessInfo>();
    copy->pid = source.pid;
    copy->parentPid = source.parentPid;
    copy->name = source.name;
    copy->cpuPercent = source.cpuPercent;
    copy->memoryBytes = source.memoryBytes;
    copy->creationTime = source.creationTime;
    copy->parent = nullptr; // Don't copy parent pointer
    
    // Deep copy children
    copy->children.reserve(source.children.size());
    for (const auto& child : source.children) {
        auto childCopy = deepCopy(*child);
        childCopy->parent = copy.get();
        copy->children.push_back(std::move(childCopy));
    }
    
    return copy;
}

} // namespace sysmon
