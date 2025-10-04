#include "ISystemCollector.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/sysinfo.h>

namespace sysmon {

class LinuxSystemCollector : public ISystemCollector {
public:
    LinuxSystemCollector() = default;
    ~LinuxSystemCollector() override {
        shutdown();
    }
    
    bool initialize() override {
        // Get number of CPU cores
        numCores_ = sysconf(_SC_NPROCESSORS_ONLN);
        
        // Read initial CPU stats
        readCpuStats(lastTotalTime_, lastIdleTime_, lastCoreStats_);
        
        // Read initial network stats
        readNetworkStats(lastNetworkRecv_, lastNetworkSent_);
        
        // Read initial disk stats
        readDiskStats(lastDiskRead_, lastDiskWrite_);
        
        return true;
    }
    
    void shutdown() override {
        // No cleanup needed
    }
    
    void collectCPUMetrics(SystemMetrics& metrics) override {
        uint64_t totalTime, idleTime;
        std::vector<std::pair<uint64_t, uint64_t>> coreStats;
        
        readCpuStats(totalTime, idleTime, coreStats);
        
        // Calculate overall CPU usage
        uint64_t totalDelta = totalTime - lastTotalTime_;
        uint64_t idleDelta = idleTime - lastIdleTime_;
        
        if (totalDelta > 0) {
            metrics.cpuUsagePercent = 100.0 * (1.0 - static_cast<double>(idleDelta) / 
                                               static_cast<double>(totalDelta));
        }
        
        // Calculate per-core CPU usage
        metrics.perCoreCpuUsage.resize(numCores_);
        for (size_t i = 0; i < coreStats.size() && i < numCores_; ++i) {
            uint64_t coreTotalDelta = coreStats[i].first - lastCoreStats_[i].first;
            uint64_t coreIdleDelta = coreStats[i].second - lastCoreStats_[i].second;
            
            if (coreTotalDelta > 0) {
                metrics.perCoreCpuUsage[i] = 100.0 * (1.0 - static_cast<double>(coreIdleDelta) / 
                                                       static_cast<double>(coreTotalDelta));
            }
        }
        
        lastTotalTime_ = totalTime;
        lastIdleTime_ = idleTime;
        lastCoreStats_ = coreStats;
    }
    
    void collectMemoryMetrics(SystemMetrics& metrics) override {
        std::ifstream meminfo("/proc/meminfo");
        if (!meminfo.is_open()) {
            return;
        }
        
        std::string line;
        uint64_t memTotal = 0, memFree = 0, memAvailable = 0, buffers = 0, cached = 0;
        
        while (std::getline(meminfo, line)) {
            std::istringstream iss(line);
            std::string key;
            uint64_t value;
            std::string unit;
            
            if (iss >> key >> value >> unit) {
                // Convert from kB to bytes
                value *= 1024;
                
                if (key == "MemTotal:") memTotal = value;
                else if (key == "MemFree:") memFree = value;
                else if (key == "MemAvailable:") memAvailable = value;
                else if (key == "Buffers:") buffers = value;
                else if (key == "Cached:") cached = value;
            }
        }
        
        metrics.totalMemoryBytes = memTotal;
        // Use MemAvailable if available, otherwise calculate
        uint64_t availableMemory = memAvailable > 0 ? memAvailable : (memFree + buffers + cached);
        metrics.usedMemoryBytes = memTotal - availableMemory;
        
        if (memTotal > 0) {
            metrics.memoryUsagePercent = 100.0 * static_cast<double>(metrics.usedMemoryBytes) / 
                                        static_cast<double>(memTotal);
        }
    }
    
    void collectDiskMetrics(SystemMetrics& metrics) override {
        uint64_t diskRead, diskWrite;
        readDiskStats(diskRead, diskWrite);
        
        // Calculate rates (bytes per second)
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastDiskTime_);
        double seconds = elapsed.count() / 1000.0;
        
        if (seconds > 0 && lastDiskTime_.time_since_epoch().count() > 0) {
            metrics.diskReadBytesPerSec = static_cast<uint64_t>(
                (diskRead - lastDiskRead_) / seconds);
            metrics.diskWriteBytesPerSec = static_cast<uint64_t>(
                (diskWrite - lastDiskWrite_) / seconds);
        }
        
        lastDiskRead_ = diskRead;
        lastDiskWrite_ = diskWrite;
        lastDiskTime_ = now;
    }
    
    void collectNetworkMetrics(SystemMetrics& metrics) override {
        uint64_t netRecv, netSent;
        readNetworkStats(netRecv, netSent);
        
        // Calculate rates (bytes per second)
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastNetworkTime_);
        double seconds = elapsed.count() / 1000.0;
        
        if (seconds > 0 && lastNetworkTime_.time_since_epoch().count() > 0) {
            metrics.networkRecvBytesPerSec = static_cast<uint64_t>(
                (netRecv - lastNetworkRecv_) / seconds);
            metrics.networkSendBytesPerSec = static_cast<uint64_t>(
                (netSent - lastNetworkSent_) / seconds);
        }
        
        lastNetworkRecv_ = netRecv;
        lastNetworkSent_ = netSent;
        lastNetworkTime_ = now;
    }
    
private:
    void readCpuStats(uint64_t& totalTime, uint64_t& idleTime, 
                      std::vector<std::pair<uint64_t, uint64_t>>& coreStats) {
        std::ifstream stat("/proc/stat");
        if (!stat.is_open()) {
            return;
        }
        
        std::string line;
        coreStats.clear();
        
        while (std::getline(stat, line)) {
            if (line.substr(0, 3) == "cpu") {
                std::istringstream iss(line);
                std::string cpu;
                uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
                
                iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
                
                uint64_t total = user + nice + system + idle + iowait + irq + softirq + steal;
                
                if (cpu == "cpu") {
                    // Overall CPU
                    totalTime = total;
                    idleTime = idle + iowait;
                } else {
                    // Per-core CPU
                    coreStats.push_back({total, idle + iowait});
                }
            }
        }
    }
    
    void readNetworkStats(uint64_t& recvBytes, uint64_t& sentBytes) {
        std::ifstream netdev("/proc/net/dev");
        if (!netdev.is_open()) {
            return;
        }
        
        recvBytes = 0;
        sentBytes = 0;
        
        std::string line;
        // Skip header lines
        std::getline(netdev, line);
        std::getline(netdev, line);
        
        while (std::getline(netdev, line)) {
            // Skip loopback interface
            if (line.find("lo:") != std::string::npos) {
                continue;
            }
            
            std::istringstream iss(line);
            std::string iface;
            uint64_t recv, sent;
            uint64_t dummy;
            
            iss >> iface >> recv;
            // Skip 7 more fields to get to transmit bytes
            for (int i = 0; i < 7; ++i) iss >> dummy;
            iss >> sent;
            
            recvBytes += recv;
            sentBytes += sent;
        }
    }
    
    void readDiskStats(uint64_t& readBytes, uint64_t& writeBytes) {
        std::ifstream diskstats("/proc/diskstats");
        if (!diskstats.is_open()) {
            return;
        }
        
        readBytes = 0;
        writeBytes = 0;
        
        std::string line;
        while (std::getline(diskstats, line)) {
            std::istringstream iss(line);
            uint64_t major, minor;
            std::string device;
            uint64_t reads, readsMerged, sectorsRead, readTime;
            uint64_t writes, writesMerged, sectorsWritten, writeTime;
            
            iss >> major >> minor >> device >> reads >> readsMerged >> sectorsRead >> readTime
                >> writes >> writesMerged >> sectorsWritten >> writeTime;
            
            // Only count physical disks (sd*, nvme*, vd*)
            if (device.find("loop") == std::string::npos &&
                device.find("ram") == std::string::npos) {
                // Sector size is typically 512 bytes
                readBytes += sectorsRead * 512;
                writeBytes += sectorsWritten * 512;
            }
        }
    }
    
    size_t numCores_{0};
    
    uint64_t lastTotalTime_{0};
    uint64_t lastIdleTime_{0};
    std::vector<std::pair<uint64_t, uint64_t>> lastCoreStats_;
    
    uint64_t lastNetworkRecv_{0};
    uint64_t lastNetworkSent_{0};
    std::chrono::steady_clock::time_point lastNetworkTime_;
    
    uint64_t lastDiskRead_{0};
    uint64_t lastDiskWrite_{0};
    std::chrono::steady_clock::time_point lastDiskTime_;
};

std::unique_ptr<ISystemCollector> createSystemCollector() {
    return std::make_unique<LinuxSystemCollector>();
}

} // namespace sysmon
