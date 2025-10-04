#include "IProcessCollector.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <vector>
#include <unordered_map>

#pragma comment(lib, "psapi.lib")

namespace sysmon {

class WindowsProcessCollector : public IProcessCollector {
public:
    WindowsProcessCollector() = default;
    ~WindowsProcessCollector() override {
        shutdown();
    }
    
    bool initialize() override {
        // Get system page size for memory calculations
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        pageSize_ = sysInfo.dwPageSize;
        return true;
    }
    
    void shutdown() override {
        // No cleanup needed
    }
    
    std::vector<std::unique_ptr<ProcessInfo>> enumerateProcesses() override {
        std::vector<std::unique_ptr<ProcessInfo>> processes;
        
        // Create snapshot of all processes
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return processes;
        }
        
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        
        if (Process32FirstW(snapshot, &pe32)) {
            do {
                auto procInfo = std::make_unique<ProcessInfo>();
                procInfo->pid = pe32.th32ProcessID;
                procInfo->parentPid = pe32.th32ParentProcessID;
                
                // Convert wide string to narrow string
                int size = WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, 
                                              nullptr, 0, nullptr, nullptr);
                if (size > 0) {
                    std::vector<char> buffer(size);
                    WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, 
                                       buffer.data(), size, nullptr, nullptr);
                    procInfo->name = buffer.data();
                }
                
                // Get additional process information
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
                                             FALSE, pe32.th32ProcessID);
                if (hProcess) {
                    // Get memory info
                    PROCESS_MEMORY_COUNTERS_EX pmc;
                    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
                        procInfo->memoryBytes = pmc.WorkingSetSize;
                    }
                    
                    // Get CPU usage (simplified)
                    FILETIME createTime, exitTime, kernelTime, userTime;
                    if (GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
                        procInfo->creationTime = fileTimeToUInt64(createTime);
                        
                        // Calculate CPU percentage (requires delta sampling in real implementation)
                        uint64_t totalTime = fileTimeToUInt64(kernelTime) + fileTimeToUInt64(userTime);
                        
                        // Store for delta calculation
                        auto it = lastCpuTimes_.find(pe32.th32ProcessID);
                        if (it != lastCpuTimes_.end()) {
                            uint64_t delta = totalTime - it->second.first;
                            uint64_t timeDelta = GetTickCount64() - it->second.second;
                            if (timeDelta > 0) {
                                procInfo->cpuPercent = (delta * 100.0) / (timeDelta * 10000.0);
                            }
                        }
                        lastCpuTimes_[pe32.th32ProcessID] = {totalTime, GetTickCount64()};
                    }
                    
                    CloseHandle(hProcess);
                }
                
                processes.push_back(std::move(procInfo));
                
            } while (Process32NextW(snapshot, &pe32));
        }
        
        CloseHandle(snapshot);
        return processes;
    }
    
    bool terminateProcess(uint32_t pid) override {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (!hProcess) {
            return false;
        }
        
        BOOL result = TerminateProcess(hProcess, 1);
        CloseHandle(hProcess);
        
        return result != 0;
    }
    
private:
    uint64_t fileTimeToUInt64(const FILETIME& ft) {
        ULARGE_INTEGER uli;
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        return uli.QuadPart;
    }
    
    DWORD pageSize_{0};
    std::unordered_map<uint32_t, std::pair<uint64_t, uint64_t>> lastCpuTimes_;
};

std::unique_ptr<IProcessCollector> createProcessCollector() {
    return std::make_unique<WindowsProcessCollector>();
}

} // namespace sysmon
