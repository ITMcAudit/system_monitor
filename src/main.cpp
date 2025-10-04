#include "Configuration.h"
#include "SystemDataCollector.h"
#include "ProcessTreeBuilder.h"
#include "MonitorUI.h"
#include <iostream>
#include <csignal>
#include <memory>

using namespace sysmon;

// Global pointer for signal handling
static MonitorUI* g_ui = nullptr;

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        if (g_ui) {
            g_ui->shutdown();
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "System Monitor v1.0.0\n";
    std::cout << "Initializing...\n\n";
    
    // Load configuration
    Configuration config;
    config.loadFromEnvironment();
    config.loadFromArgs(argc, argv);
    
    if (!config.validate()) {
        std::cerr << "Invalid configuration\n";
        return 1;
    }
    
    // Set up signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    try {
        // Create core components
        SystemDataCollector dataCollector(config);
        ProcessTreeBuilder processBuilder(config);
        MonitorUI ui(dataCollector, processBuilder, config);
        
        g_ui = &ui;
        
        // Start data collection
        std::cout << "Starting system data collector...\n";
        if (!dataCollector.start()) {
            std::cerr << "Failed to start system data collector\n";
            return 1;
        }
        
        std::cout << "Starting process tree builder...\n";
        if (!processBuilder.start()) {
            std::cerr << "Failed to start process tree builder\n";
            return 1;
        }
        
        // Give collectors time to gather initial data
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "Launching UI...\n";
        
        // Run UI (blocking)
        ui.run();
        
        // Cleanup
        std::cout << "\nShutting down...\n";
        dataCollector.stop();
        processBuilder.stop();
        
        g_ui = nullptr;
        
        std::cout << "Goodbye!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
