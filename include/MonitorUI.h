#pragma once

#include "SystemDataCollector.h"
#include "ProcessTreeBuilder.h"
#include "Configuration.h"
#include <ftxui/component/component.hpp>
#include <memory>

namespace sysmon {

/**
 * @brief Main UI controller orchestrating all interface components
 * 
 * Manages FTXUI components, navigation, and rendering
 */
class MonitorUI {
public:
    MonitorUI(SystemDataCollector& dataCollector,
              ProcessTreeBuilder& processBuilder,
              const Configuration& config);
    
    /**
     * @brief Run the UI event loop (blocking)
     */
    void run();
    
    /**
     * @brief Request UI shutdown
     */
    void shutdown();
    
private:
    ftxui::Component createMainLayout();
    ftxui::Component createCPUWidget();
    ftxui::Component createMemoryWidget();
    ftxui::Component createDiskWidget();
    ftxui::Component createNetworkWidget();
    ftxui::Component createProcessTreeWidget();
    ftxui::Component createStatusBar();
    
    SystemDataCollector& dataCollector_;
    ProcessTreeBuilder& processBuilder_;
    Configuration config_;
    
    bool shouldQuit_{false};
    int selectedProcessIndex_{0};
    bool showKillConfirmation_{false};
};

} // namespace sysmon
