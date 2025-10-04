#include "IProcessCollector.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <cstring>

namespace sysmon {

class LinuxProcessCollector : public IProcessCollector {
public:
    LinuxProcessCollector() {
        pageSize_ = sysconf(_SC_PAGESIZE);
        clockTicks_ = sysconf(_SC_CLK_TCK);
    }
    
    ~LinuxProcessCollector() override {
        shutdown();
    }
    
    bool initialize() override {
        // Initialize last CPU measurement time
        lastSampleTime_ = std::chrono::steady_clock::now();
        return true;
    }
    
    void shutdown() override {
        // No cleanup needed
    }
    
    std::vector<std::unique_ptr<ProcessInfo>> enumerateProcesses() override {
        std::vector<std::unique_ptr<ProcessInfo>> processes;
        
        DIR* dir = opendir("/proc");
        if (!dir) {
            return processes;
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            // Check if directory name is a number (PID)
            if (entry->d_type == DT_DIR) {
                const char* name = entry->d_name;
                if (std::isdigit(name[0])) {
                    uint32_t pid = std::stoi(name);
                    auto procInfo = readProcessInfo(pid);
                    if (procInfo) {
                        processes.push_back(std::move(procInfo));
                    }
                }
            }
        }
        
        closedir(dir);
        
        // Calculate CPU percentages
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastSampleTime_);
        double seconds = elapsed.count() / 1000.0;
        
        for (auto& proc : processes) {
            auto it = lastCpuTimes_.find(proc->pid);
            if (it != lastCpuTimes_.end()) {
                uint64_t cpuDelta = proc->creationTime - it->second; // Reusing creationTime field temporarily
                if (seconds > 0) {
                    proc->cpuPercent = (cpuDelta / clockTicks_) / seconds * 100.0;
                }
            }
            lastCpuTimes_[proc->pid] = proc->creationTime; // Store for next iteration
        }
        
        lastSampleTime_ = now;
        
        return processes;
    }
    
    bool terminateProcess(uint32_t pid) override {
        // Send SIGTERM for graceful termination
        return kill(pid, SIGTERM) == 0;
    }
    
private:
    std::unique_ptr<ProcessInfo> readProcessInfo(uint32_t pid) {
        // Read /proc/[pid]/stat
        std::string statPath = "/proc/" + std::to_string(pid) + "/stat";
        std::ifstream statFile(statPath);
        
        if (!statFile.is_open()) {
            return nullptr;
        }
        
        auto procInfo = std::make_unique<ProcessInfo>();
        procInfo->pid = pid;
        
        std::string line;
        std::getline(statFile, line);
        
        // Parse stat file (format is complex due to process name containing spaces/parens)
        size_t commStart = line.find('(');
        size_t commEnd = line.rfind(')');
        
        if (commStart == std::string::npos || commEnd == std::string::npos) {
            return nullptr;
        }
        
        // Extract process name
        procInfo->name = line.substr(commStart + 1, commEnd - commStart - 1);
        
        // Parse remaining fields
        std::istringstream iss(line.substr(commEnd + 2));
        char state;
        uint64_t utime, stime;
        long cutime, cstime, priority, nice, numThreads, itrealvalue;
        unsigned long long starttime;
        unsigned long vsize;
        long rss;
        
        iss >> state >> procInfo->parentPid >> /* pgrp */ >> /* session */ >> /* tty_nr */
            >> /* tpgid */ >> /* flags */ >> /* minflt */ >> /* cminflt */
            >> /* majflt */ >> /* cmajflt */ >> utime >> stime >> cutime >> cstime
            >> priority >> nice >> numThreads >> itrealvalue >> starttime;
        
        iss >> vsize >> rss;
        
        // Calculate memory usage (RSS in pages)
        procInfo->memoryBytes = rss * pageSize_;
        
        // Store total CPU time for delta calculation
        procInfo->creationTime = utime + stime; // Temporary storage
        
        // Read actual creation time from /proc/[pid]/stat starttime field
        // This is in clock ticks since boot
        std::ifstream uptimeFile("/proc/uptime");
        if (uptimeFile.is_open()) {
            double uptime;
            uptimeFile >> uptime;
            // Convert starttime to absolute time
            procInfo->creationTime = static_cast<uint64_t>(
                (uptime - starttime / clockTicks_) * 1000);
        }
        
        return procInfo;
    }
    
    long pageSize_{0};
    long clockTicks_{0};
    std::chrono::steady_clock::time_point lastSampleTime_;
    std::unordered_map<uint32_t, uint64_t> lastCpuTimes_;
};

std::unique_ptr<IProcessCollector> createProcessCollector() {
    return std::make_unique<LinuxProcessCollector>();
}

} // namespace sysmon
