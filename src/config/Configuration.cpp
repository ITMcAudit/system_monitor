#include "Configuration.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

namespace sysmon {

void Configuration::loadFromArgs(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--cpu-interval" && i + 1 < argc) {
            cpuSampleIntervalMs = std::stoi(argv[++i]);
        } else if (arg == "--memory-interval" && i + 1 < argc) {
            memorySampleIntervalMs = std::stoi(argv[++i]);
        } else if (arg == "--fps" && i + 1 < argc) {
            targetFrameRateHz = std::stoi(argv[++i]);
        } else if (arg == "--no-colors") {
            useColors = false;
        } else if (arg == "--expand-tree") {
            expandTreeByDefault = true;
        } else if (arg == "--cpu-threshold" && i + 1 < argc) {
            cpuAlertThreshold = std::stod(argv[++i]);
        } else if (arg == "--memory-threshold" && i + 1 < argc) {
            memoryAlertThreshold = std::stod(argv[++i]);
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "System Monitor - Usage:\n"
                      << "  --cpu-interval <ms>       CPU sampling interval (default: 1000)\n"
                      << "  --memory-interval <ms>    Memory sampling interval (default: 5000)\n"
                      << "  --fps <rate>              Target frame rate (default: 30)\n"
                      << "  --cpu-threshold <pct>     CPU alert threshold (default: 90)\n"
                      << "  --memory-threshold <pct>  Memory alert threshold (default: 90)\n"
                      << "  --no-colors               Disable color output\n"
                      << "  --expand-tree             Expand process tree by default\n"
                      << "  --help, -h                Show this help\n";
            std::exit(0);
        }
    }
}

void Configuration::loadFromEnvironment() {
    const char* interval = std::getenv("SYSMON_CPU_INTERVAL");
    if (interval) {
        cpuSampleIntervalMs = std::stoi(interval);
    }
    
    const char* colors = std::getenv("SYSMON_NO_COLORS");
    if (colors && std::string(colors) == "1") {
        useColors = false;
    }
}

bool Configuration::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    // Simple key=value parser
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            if (key == "cpu_interval") {
                cpuSampleIntervalMs = std::stoi(value);
            } else if (key == "memory_interval") {
                memorySampleIntervalMs = std::stoi(value);
            } else if (key == "target_fps") {
                targetFrameRateHz = std::stoi(value);
            }
        }
    }
    
    return true;
}

bool Configuration::validate() const {
    if (cpuSampleIntervalMs < 100 || cpuSampleIntervalMs > 10000) {
        std::cerr << "Invalid CPU interval: " << cpuSampleIntervalMs << "\n";
        return false;
    }
    
    if (memorySampleIntervalMs < 1000 || memorySampleIntervalMs > 3600000) {
        std::cerr << "Invalid memory interval: " << memorySampleIntervalMs << "\n";
        return false;
    }
    
    if (targetFrameRateHz < 1 || targetFrameRateHz > 120) {
        std::cerr << "Invalid frame rate: " << targetFrameRateHz << "\n";
        return false;
    }
    
    return true;
}

void Configuration::print() const {
    std::cout << "Configuration:\n"
              << "  CPU Interval: " << cpuSampleIntervalMs << " ms\n"
              << "  Memory Interval: " << memorySampleIntervalMs << " ms\n"
              << "  Target FPS: " << targetFrameRateHz << "\n"
              << "  CPU Alert: " << cpuAlertThreshold << "%\n"
              << "  Memory Alert: " << memoryAlertThreshold << "%\n"
              << "  Colors: " << (useColors ? "enabled" : "disabled") << "\n";
}

} // namespace sysmon
