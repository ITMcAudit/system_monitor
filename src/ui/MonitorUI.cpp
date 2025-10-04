#include "MonitorUI.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace ftxui;

namespace sysmon {

namespace {
    std::string formatBytes(uint64_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit = 0;
        double value = static_cast<double>(bytes);
        
        while (value >= 1024.0 && unit < 4) {
            value /= 1024.0;
            ++unit;
        }
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << value << " " << units[unit];
        return oss.str();
    }
    
    std::string formatPercentage(double percent) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << percent << "%";
        return oss.str();
    }
    
    Color getUsageColor(double percent) {
        if (percent < 60.0) return Color::Green;
        if (percent < 80.0) return Color::Yellow;
        return Color::Red;
    }
}

MonitorUI::MonitorUI(SystemDataCollector& dataCollector,
                     ProcessTreeBuilder& processBuilder,
                     const Configuration& config)
    : dataCollector_(dataCollector),
      processBuilder_(processBuilder),
      config_(config) {
}

void MonitorUI::run() {
    auto screen = ScreenInteractive::Fullscreen();
    auto mainLayout = createMainLayout();
    
    screen.Loop(mainLayout);
}

void MonitorUI::shutdown() {
    shouldQuit_ = true;
}

Component MonitorUI::createMainLayout() {
    auto cpuWidget = createCPUWidget();
    auto memoryWidget = createMemoryWidget();
    auto diskWidget = createDiskWidget();
    auto networkWidget = createNetworkWidget();
    auto processWidget = createProcessTreeWidget();
    auto statusBar = createStatusBar();
    
    auto layout = Container::Vertical({
        cpuWidget,
        memoryWidget,
        Container::Horizontal({
            diskWidget,
            networkWidget,
        }),
        processWidget,
        statusBar,
    });
    
    return Renderer(layout, [=] {
        return vbox({
            cpuWidget->Render() | border | size(HEIGHT, EQUAL, 8),
            memoryWidget->Render() | border | size(HEIGHT, EQUAL, 5),
            hbox({
                diskWidget->Render() | border | flex,
                separator(),
                networkWidget->Render() | border | flex,
            }) | size(HEIGHT, EQUAL, 5),
            processWidget->Render() | border | flex,
            separator(),
            statusBar->Render() | size(HEIGHT, EQUAL, 1),
        });
    });
}

Component MonitorUI::createCPUWidget() {
    return Renderer([&] {
        auto metrics = dataCollector_.getMetrics();
        
        Elements cores;
        for (size_t i = 0; i < metrics.perCoreCpuUsage.size() && i < 16; ++i) {
            double usage = metrics.perCoreCpuUsage[i];
            auto color = getUsageColor(usage);
            
            cores.push_back(vbox({
                text("Core " + std::to_string(i)),
                gauge(usage / 100.0) | color,
                text(formatPercentage(usage)) | align_right,
            }));
        }
        
        return vbox({
            text("CPU Usage") | bold,
            separator(),
            hbox({
                vbox({
                    text("Overall:"),
                    gauge(metrics.cpuUsagePercent / 100.0) | 
                        getUsageColor(metrics.cpuUsagePercent),
                    text(formatPercentage(metrics.cpuUsagePercent)) | bold,
                }) | size(WIDTH, EQUAL, 20),
                separator(),
                hbox(cores) | flex,
            }),
        });
    });
}

Component MonitorUI::createMemoryWidget() {
    return Renderer([&] {
        auto metrics = dataCollector_.getMetrics();
        
        return vbox({
            text("Memory") | bold,
            separator(),
            hbox({
                text("Used: " + formatBytes(metrics.usedMemoryBytes)),
                separator(),
                text("Total: " + formatBytes(metrics.totalMemoryBytes)),
                separator(),
                gauge(metrics.memoryUsagePercent / 100.0) | 
                    getUsageColor(metrics.memoryUsagePercent) | flex,
                separator(),
                text(formatPercentage(metrics.memoryUsagePercent)) | bold,
            }),
        });
    });
}

Component MonitorUI::createDiskWidget() {
    return Renderer([&] {
        auto metrics = dataCollector_.getMetrics();
        
        return vbox({
            text("Disk I/O") | bold,
            separator(),
            text("Read:  " + formatBytes(metrics.diskReadBytesPerSec) + "/s"),
            text("Write: " + formatBytes(metrics.diskWriteBytesPerSec) + "/s"),
        });
    });
}

Component MonitorUI::createNetworkWidget() {
    return Renderer([&] {
        auto metrics = dataCollector_.getMetrics();
        
        return vbox({
            text("Network I/O") | bold,
            separator(),
            text("Recv: " + formatBytes(metrics.networkRecvBytesPerSec) + "/s"),
            text("Send: " + formatBytes(metrics.networkSendBytesPerSec) + "/s"),
        });
    });
}

Component MonitorUI::createProcessTreeWidget() {
    return Renderer([&] {
        auto processes = processBuilder_.getProcessTree();
        
        Elements processLines;
        processLines.push_back(hbox({
            text("PID") | size(WIDTH, EQUAL, 8),
            separator(),
            text("CPU%") | size(WIDTH, EQUAL, 8),
            separator(),
            text("Memory") | size(WIDTH, EQUAL, 12),
            separator(),
            text("Name") | flex,
        }) | bold);
        
        processLines.push_back(separator());
        
        // Flatten tree for display (limit to avoid overflow)
        std::function<void(const ProcessInfo&, int, int&)> addProcess;
        addProcess = [&](const ProcessInfo& proc, int depth, int& count) {
            if (count >= static_cast<int>(config_.maxProcessDisplay)) return;
            
            std::string indent(depth * 2, ' ');
            
            processLines.push_back(hbox({
                text(std::to_string(proc.pid)) | size(WIDTH, EQUAL, 8),
                separator(),
                text(formatPercentage(proc.cpuPercent)) | size(WIDTH, EQUAL, 8),
                separator(),
                text(formatBytes(proc.memoryBytes)) | size(WIDTH, EQUAL, 12),
                separator(),
                text(indent + proc.name) | flex,
            }));
            
            ++count;
            
            if (config_.expandTreeByDefault || depth == 0) {
                for (const auto& child : proc.children) {
                    addProcess(*child, depth + 1, count);
                }
            }
        };
        
        int count = 0;
        for (const auto& root : processes) {
            addProcess(*root, 0, count);
        }
        
        return vbox({
            text("Processes (" + std::to_string(processes.size()) + " roots)") | bold,
            separator(),
            vbox(processLines),
        });
    });
}

Component MonitorUI::createStatusBar() {
    return Renderer([&] {
        auto metrics = dataCollector_.getMetrics();
        
        // Format current time
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
        localtime_s(&tm, &time_t);
        
        std::ostringstream timeStr;
        timeStr << std::put_time(&tm, "%H:%M:%S");
        
        // Check for alerts
        std::string alerts;
        if (metrics.cpuUsagePercent > config_.cpuAlertThreshold) {
            alerts += " [CPU ALERT] ";
        }
        if (metrics.memoryUsagePercent > config_.memoryAlertThreshold) {
            alerts += " [MEMORY ALERT] ";
        }
        
        return hbox({
            text(timeStr.str()),
            separator(),
            text(alerts) | color(Color::Red) | bold,
            filler(),
            text("q:Quit r:Refresh Tab:Navigate") | dim,
        });
    });
}

} // namespace sysmon
