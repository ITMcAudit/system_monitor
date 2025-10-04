#include "ISystemCollector.h"
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>
#include <net/if.h>
#include <net/route.h>
#include <ifaddrs.h>
#include <vector>

namespace sysmon {

class MacOSSystemCollector : public ISystemCollector {
public:
    MacOSSystemCollector() = default;
    ~MacOSSystemCollector() override {
        shutdown();
    }
    
    bool initialize() override {
        // Get number of CPUs
        size_t len = sizeof(numCores_);
        sysctlbyname("hw.ncpu", &numCores_, &len, nullptr, 0);
        
        // Initialize last CPU times
        collectCPUTimes(lastCpuTimes_);
        
        // Initialize network stats
        getNetworkStats(lastNetworkRecv_, lastNetworkSent_);
        
        return true;
    }
    
    void shutdown() override {
        // No cleanup needed
    }
    
    void collectCPUMetrics(SystemMetrics& metrics) override {
        std::vector<host_cpu_load_info_data_t> currentTimes;
        collectCPUTimes(currentTimes);
        
        if (lastCpuTimes_.empty()) {
            lastCpuTimes_ = currentTimes;
            return;
        }
        
        // Calculate overall CPU usage
        uint64_t totalUser = 0, totalSystem = 0, totalIdle = 0, totalNice = 0;
        
        for (size_t i = 0; i < currentTimes.size() && i < lastCpuTimes_.size(); ++i) {
            totalUser += currentTimes[i].cpu_ticks[CPU_STATE_USER] - 
                        lastCpuTimes_[i].cpu_ticks[CPU_STATE_USER];
            totalSystem += currentTimes[i].cpu_ticks[CPU_STATE_SYSTEM] - 
                          lastCpuTimes_[i].cpu_ticks[CPU_STATE_SYSTEM];
            totalIdle += currentTimes[i].cpu_ticks[CPU_STATE_IDLE] - 
                        lastCpuTimes_[i].cpu_ticks[CPU_STATE_IDLE];
            totalNice += currentTimes[i].cpu_ticks[CPU_STATE_NICE] - 
                        lastCpuTimes_[i].cpu_ticks[CPU_STATE_NICE];
        }
        
        uint64_t totalTicks = totalUser + totalSystem + totalIdle + totalNice;
        if (totalTicks > 0) {
            metrics.cpuUsagePercent = 100.0 * (totalUser + totalSystem + totalNice) / totalTicks;
        }
        
        // Calculate per-core usage
        metrics.perCoreCpuUsage.resize(currentTimes.size());
        for (size_t i = 0; i < currentTimes.size() && i < lastCpuTimes_.size(); ++i) {
            uint64_t user = currentTimes[i].cpu_ticks[CPU_STATE_USER] - 
                           lastCpuTimes_[i].cpu_ticks[CPU_STATE_USER];
            uint64_t system = currentTimes[i].cpu_ticks[CPU_STATE_SYSTEM] - 
                             lastCpuTimes_[i].cpu_ticks[CPU_STATE_SYSTEM];
            uint64_t idle = currentTimes[i].cpu_ticks[CPU_STATE_IDLE] - 
                           lastCpuTimes_[i].cpu_ticks[CPU_STATE_IDLE];
            uint64_t nice = currentTimes[i].cpu_ticks[CPU_STATE_NICE] - 
                           lastCpuTimes_[i].cpu_ticks[CPU_STATE_NICE];
            
            uint64_t total = user + system + idle + nice;
            if (total > 0) {
                metrics.perCoreCpuUsage[i] = 100.0 * (user + system + nice) / total;
            }
        }
        
        lastCpuTimes_ = currentTimes;
    }
    
    void collectMemoryMetrics(SystemMetrics& metrics) override {
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
        vm_statistics64_data_t vmStats;
        
        if (host_statistics64(mach_host_self(), HOST_VM_INFO64,
                             (host_info64_t)&vmStats, &count) == KERN_SUCCESS) {
            
            // Get page size
            vm_size_t pageSize;
            host_page_size(mach_host_self(), &pageSize);
            
            // Get total physical memory
            int mib[2] = {CTL_HW, HW_MEMSIZE};
            uint64_t memSize;
            size_t len = sizeof(memSize);
            sysctl(mib, 2, &memSize, &len, nullptr, 0);
            
            metrics.totalMemoryBytes = memSize;
            
            // Calculate used memory
            uint64_t usedPages = vmStats.active_count + vmStats.inactive_count + 
                                vmStats.wire_count;
            metrics.usedMemoryBytes = usedPages * pageSize;
            
            if (memSize > 0) {
                metrics.memoryUsagePercent = 100.0 * metrics.usedMemoryBytes / memSize;
            }
        }
    }
    
    void collectDiskMetrics(SystemMetrics& metrics) override {
        // macOS disk I/O statistics are more complex
        // Simplified implementation - would need IOKit for full functionality
        metrics.diskReadBytesPerSec = 0;
        metrics.diskWriteBytesPerSec = 0;
    }
    
    void collectNetworkMetrics(SystemMetrics& metrics) override {
        uint64_t recvBytes, sentBytes;
        getNetworkStats(recvBytes, sentBytes);
        
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastNetworkTime_);
        double seconds = elapsed.count() / 1000.0;
        
        if (seconds > 0 && lastNetworkTime_.time_since_epoch().count() > 0) {
            metrics.networkRecvBytesPerSec = static_cast<uint64_t>(
                (recvBytes - lastNetworkRecv_) / seconds);
            metrics.networkSendBytesPerSec = static_cast<uint64_t>(
                (sentBytes - lastNetworkSent_) / seconds);
        }
        
        lastNetworkRecv_ = recvBytes;
        lastNetworkSent_ = sentBytes;
        lastNetworkTime_ = now;
    }
    
private:
    void collectCPUTimes(std::vector<host_cpu_load_info_data_t>& times) {
        processor_cpu_load_info_t cpuLoad;
        mach_msg_type_number_t processorCount;
        natural_t processorCount_nat;
        
        if (host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO,
                               &processorCount_nat,
                               (processor_info_array_t*)&cpuLoad,
                               &processorCount) == KERN_SUCCESS) {
            
            times.resize(processorCount_nat);
            for (natural_t i = 0; i < processorCount_nat; ++i) {
                for (int j = 0; j < CPU_STATE_MAX; ++j) {
                    times[i].cpu_ticks[j] = cpuLoad[i].cpu_ticks[j];
                }
            }
            
            vm_deallocate(mach_task_self(), (vm_address_t)cpuLoad,
                         processorCount * sizeof(*cpuLoad));
        }
    }
    
    void getNetworkStats(uint64_t& recvBytes, uint64_t& sentBytes) {
        recvBytes = 0;
        sentBytes = 0;
        
        struct ifaddrs* ifap = nullptr;
        if (getifaddrs(&ifap) == 0) {
            for (struct ifaddrs* ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next) {
                if (ifa->ifa_addr == nullptr || 
                    ifa->ifa_addr->sa_family != AF_LINK) {
                    continue;
                }
                
                // Skip loopback
                if (ifa->ifa_flags & IFF_LOOPBACK) {
                    continue;
                }
                
                struct if_data* ifData = (struct if_data*)ifa->ifa_data;
                if (ifData != nullptr) {
                    recvBytes += ifData->ifi_ibytes;
                    sentBytes += ifData->ifi_obytes;
                }
            }
            freeifaddrs(ifap);
        }
    }
    
    int numCores_{0};
    std::vector<host_cpu_load_info_data_t> lastCpuTimes_;
    
    uint64_t lastNetworkRecv_{0};
    uint64_t lastNetworkSent_{0};
    std::chrono::steady_clock::time_point lastNetworkTime_;
};

std::unique_ptr<ISystemCollector> createSystemCollector() {
    return std::make_unique<MacOSSystemCollector>();
}

} // namespace sysmon
