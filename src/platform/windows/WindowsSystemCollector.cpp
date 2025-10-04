#include "ISystemCollector.h"
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <iphlpapi.h>
#include <vector>
#include <string>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "iphlpapi.lib")

namespace sysmon {

class WindowsSystemCollector : public ISystemCollector {
public:
    WindowsSystemCollector() = default;
    ~WindowsSystemCollector() override {
        shutdown();
    }
    
    bool initialize() override {
        // Initialize PDH
        PDH_STATUS status = PdhOpenQuery(nullptr, 0, &cpuQuery_);
        if (status != ERROR_SUCCESS) {
            return false;
        }
        
        // Add CPU counter
        status = PdhAddEnglishCounter(cpuQuery_, L"\\Processor(_Total)\\% Processor Time", 
                                      0, &cpuTotal_);
        if (status != ERROR_SUCCESS) {
            return false;
        }
        
        // Get number of processors
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        numCores_ = sysInfo.dwNumberOfProcessors;
        
        // Add per-core counters
        coreCounters_.resize(numCores_);
        for (DWORD i = 0; i < numCores_; ++i) {
            std::wstring counterPath = L"\\Processor(" + std::to_wstring(i) + 
                                      L")\\% Processor Time";
            status = PdhAddEnglishCounter(cpuQuery_, counterPath.c_str(), 0, &coreCounters_[i]);
            if (status != ERROR_SUCCESS) {
                return false;
            }
        }
        
        // Collect initial sample
        PdhCollectQueryData(cpuQuery_);
        
        // Initialize network tracking
        lastNetworkSample_ = GetTickCount64();
        getNetworkStats(lastNetworkRecv_, lastNetworkSent_);
        
        // Initialize disk tracking
        lastDiskSample_ = GetTickCount64();
        getDiskStats(lastDiskRead_, lastDiskWrite_);
        
        return true;
    }
    
    void shutdown() override {
        if (cpuQuery_) {
            PdhCloseQuery(cpuQuery_);
            cpuQuery_ = nullptr;
        }
    }
    
    void collectCPUMetrics(SystemMetrics& metrics) override {
        PDH_STATUS status = PdhCollectQueryData(cpuQuery_);
        if (status != ERROR_SUCCESS) {
            return;
        }
        
        // Get total CPU
        PDH_FMT_COUNTERVALUE counterVal;
        status = PdhGetFormattedCounterValue(cpuTotal_, PDH_FMT_DOUBLE, nullptr, &counterVal);
        if (status == ERROR_SUCCESS) {
            metrics.cpuUsagePercent = counterVal.doubleValue;
        }
        
        // Get per-core CPU
        metrics.perCoreCpuUsage.resize(numCores_);
        for (DWORD i = 0; i < numCores_; ++i) {
            status = PdhGetFormattedCounterValue(coreCounters_[i], PDH_FMT_DOUBLE, 
                                                nullptr, &counterVal);
            if (status == ERROR_SUCCESS) {
                metrics.perCoreCpuUsage[i] = counterVal.doubleValue;
            }
        }
    }
    
    void collectMemoryMetrics(SystemMetrics& metrics) override {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        
        if (GlobalMemoryStatusEx(&memInfo)) {
            metrics.totalMemoryBytes = memInfo.ullTotalPhys;
            metrics.usedMemoryBytes = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
            metrics.memoryUsagePercent = static_cast<double>(metrics.usedMemoryBytes) / 
                                        static_cast<double>(metrics.totalMemoryBytes) * 100.0;
        }
    }
    
    void collectDiskMetrics(SystemMetrics& metrics) override {
        uint64_t currentRead = 0, currentWrite = 0;
        getDiskStats(currentRead, currentWrite);
        
        ULONGLONG currentTime = GetTickCount64();
        double elapsedSeconds = (currentTime - lastDiskSample_) / 1000.0;
        
        if (elapsedSeconds > 0 && lastDiskSample_ > 0) {
            metrics.diskReadBytesPerSec = static_cast<uint64_t>(
                (currentRead - lastDiskRead_) / elapsedSeconds);
            metrics.diskWriteBytesPerSec = static_cast<uint64_t>(
                (currentWrite - lastDiskWrite_) / elapsedSeconds);
        }
        
        lastDiskRead_ = currentRead;
        lastDiskWrite_ = currentWrite;
        lastDiskSample_ = currentTime;
    }
    
    void collectNetworkMetrics(SystemMetrics& metrics) override {
        uint64_t currentRecv = 0, currentSent = 0;
        getNetworkStats(currentRecv, currentSent);
        
        ULONGLONG currentTime = GetTickCount64();
        double elapsedSeconds = (currentTime - lastNetworkSample_) / 1000.0;
        
        if (elapsedSeconds > 0 && lastNetworkSample_ > 0) {
            metrics.networkRecvBytesPerSec = static_cast<uint64_t>(
                (currentRecv - lastNetworkRecv_) / elapsedSeconds);
            metrics.networkSendBytesPerSec = static_cast<uint64_t>(
                (currentSent - lastNetworkSent_) / elapsedSeconds);
        }
        
        lastNetworkRecv_ = currentRecv;
        lastNetworkSent_ = currentSent;
        lastNetworkSample_ = currentTime;
    }
    
private:
    void getNetworkStats(uint64_t& recvBytes, uint64_t& sentBytes) {
        recvBytes = 0;
        sentBytes = 0;
        
        PMIB_IF_TABLE2 ifTable = nullptr;
        if (GetIfTable2(&ifTable) == NO_ERROR) {
            for (ULONG i = 0; i < ifTable->NumEntries; ++i) {
                MIB_IF_ROW2& row = ifTable->Table[i];
                if (row.OperStatus == IfOperStatusUp) {
                    recvBytes += row.InOctets;
                    sentBytes += row.OutOctets;
                }
            }
            FreeMibTable(ifTable);
        }
    }
    
    void getDiskStats(uint64_t& readBytes, uint64_t& writeBytes) {
        // Simplified: Windows disk I/O requires more complex tracking
        // This is a placeholder implementation
        readBytes = 0;
        writeBytes = 0;
    }
    
    PDH_HQUERY cpuQuery_{nullptr};
    PDH_HCOUNTER cpuTotal_{nullptr};
    std::vector<PDH_HCOUNTER> coreCounters_;
    DWORD numCores_{0};
    
    uint64_t lastNetworkRecv_{0};
    uint64_t lastNetworkSent_{0};
    ULONGLONG lastNetworkSample_{0};
    
    uint64_t lastDiskRead_{0};
    uint64_t lastDiskWrite_{0};
    ULONGLONG lastDiskSample_{0};
};

std::unique_ptr<ISystemCollector> createSystemCollector() {
    return std::make_unique<WindowsSystemCollector>();
}

} // namespace sysmon
