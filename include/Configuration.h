#pragma once

#include <string>
#include <cstdint>

namespace sysmon {

/**
 * @brief Application configuration parameters
 * 
 * Supports loading from command-line args, environment variables, and config files
 */
struct Configuration {
    // Sampling intervals (milliseconds)
    uint32_t cpuSampleIntervalMs{1000};         // Default: 1 second
    uint32_t memorySampleIntervalMs{5000};      // Default: 5 seconds
    uint32_t diskSampleIntervalMs{1000};        // Default: 1 second
    uint32_t networkSampleIntervalMs{1000};     // Default: 1 second
    uint32_t processSampleIntervalMs{2000};     // Default: 2 seconds
    
    // Alert thresholds (percentages)
    double cpuAlertThreshold{90.0};             // Default: 90%
    double memoryAlertThreshold{90.0};          // Default: 90%
    
    // Display preferences
    uint32_t targetFrameRateHz{30};             // Default: 30 FPS
    bool showPerCoreStats{true};                // Show individual core stats
    bool useColors{true};                       // Enable color output
    std::string colorScheme{"default"};         // Color scheme name
    
    // Process tree settings
    bool expandTreeByDefault{false};            // Expand all tree nodes
    uint32_t maxProcessDisplay{1000};           // Max processes to display
    
    /**
     * @brief Load configuration from command-line arguments
     */
    void loadFromArgs(int argc, char* argv[]);
    
    /**
     * @brief Load configuration from environment variables
     */
    void loadFromEnvironment();
    
    /**
     * @brief Load configuration from file
     */
    bool loadFromFile(const std::string& filepath);
    
    /**
     * @brief Validate configuration values
     */
    bool validate() const;
    
    /**
     * @brief Print current configuration
     */
    void print() const;
};

} // namespace sysmon
