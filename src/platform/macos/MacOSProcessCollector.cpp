#include "IProcessCollector.h"
#include <libproc.h>
#include <sys/proc_info.h>
#include <sys/sysctl.h>
#include <signal.h>
#include <vector>
#include <unordered_map>

namespace sysmon {

class MacOSProcessCollector : public IProcessCollector {
public:
    MacOSProcessCollector() = default;
    ~MacOSProcessCollector() override {
        shutdown();
    }
    
    bool initialize() override {
        lastSampleTime_ = std::chrono::steady_clock::now();
        return true;
    }
    
    void shutdown() override {
        // No cleanup needed
    }
    
    std::vector<std::unique_ptr<ProcessInfo>> enumerateProcesses() override {
        std::vector<std::unique_ptr<ProcessInfo>> processes;
        
        // Get list of all process IDs
        int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
        size_t size = 0;
        
        // Get size needed
        if (sysctl(mib, 4, nullptr, &size, nullptr, 0) < 0) {
            return processes;
        }
        
        // Allocate buffer
        std::vector<struct kinfo_proc> procList(size / sizeof(struct kinfo_proc));
        
        // Get actual process list
        if (sysctl(mib, 4, procList.data(), &size, nullptr, 0) < 0) {
            return processes;
        }
        
        size_t procCount = size / sizeof(struct kinfo_proc);
        
        // Get current time for CPU calculation
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastSampleTime_);
        double seconds = elapsed.count() / 1000.0;
        
        for (size_t i = 0; i < procCount; ++i) {
            auto& kp = procList[i];
            auto procInfo = std::make_unique<ProcessInfo>();
            
            procInfo->pid = kp.kp_proc.p_pid;
            procInfo->parentPid = kp.kp_eproc.e_ppid;
            procInfo->name = kp.kp_proc.p_comm;
            
            // Get detailed process info
            struct proc_taskinfo taskInfo;
            int ret = proc_pidinfo(procInfo->pid, PROC_PIDTASKINFO, 0,
                                  &taskInfo, sizeof(taskInfo));
            
            if (ret == sizeof(taskInfo)) {
                // Memory usage (resident size)
                procInfo->memoryBytes = taskInfo.pti_resident_size;
                
                // CPU usage calculation
                uint64_t totalTime = taskInfo.pti_total_user + taskInfo.pti_total_system;
                
                auto it = lastCpuTimes_.find(procInfo->pid);
                if (it != lastCpuTimes_.end() && seconds > 0) {
                    uint64_t cpuDelta = totalTime - it->second;
                    // Convert nanoseconds to percentage
                    procInfo->cpuPercent = (cpuDelta / 1000000000.0) / seconds * 100.0;
                }
                
                lastCpuTimes_[procInfo->pid] = totalTime;
            }
            
            // Get process creation time
            struct proc_bsdinfo bsdInfo;
            ret = proc_pidinfo(procInfo->pid, PROC_PIDTBSDINFO, 0,
                              &bsdInfo, sizeof(bsdInfo));
            
            if (ret == sizeof(bsdInfo)) {
                procInfo->creationTime = bsdInfo.pbi_start_tvsec * 1000 +
                                        bsdInfo.pbi_start_tvusec / 1000;
            }
            
            processes.push_back(std::move(procInfo));
        }
        
        lastSampleTime_ = now;
        
        return processes;
    }
    
    bool terminateProcess(uint32_t pid) override {
        // Send SIGTERM for graceful termination
        return kill(pid, SIGTERM) == 0;
    }
    
private:
    std::chrono::steady_clock::time_point lastSampleTime_;
    std::unordered_map<uint32_t, uint64_t> lastCpuTimes_;
};

std::unique_ptr<IProcessCollector> createProcessCollector() {
    return std::make_unique<MacOSProcessCollector>();
}

} // namespace sysmon
