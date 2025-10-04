cmake_minimum_required(VERSION 3.20)
project(SystemMonitor VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable comprehensive warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Fetch FTXUI library
include(FetchContent)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
    GIT_TAG v5.0.0
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
    FetchContent_Populate(ftxui)
    add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Platform-specific source files
set(PLATFORM_SOURCES "")
if(WIN32)
    set(PLATFORM_SOURCES 
        src/platform/windows/WindowsSystemCollector.cpp
        src/platform/windows/WindowsProcessCollector.cpp
    )
elseif(APPLE)
    set(PLATFORM_SOURCES 
        src/platform/macos/MacOSSystemCollector.cpp
        src/platform/macos/MacOSProcessCollector.cpp
    )
else()
    set(PLATFORM_SOURCES 
        src/platform/linux/LinuxSystemCollector.cpp
        src/platform/linux/LinuxProcessCollector.cpp
    )
endif()

# Main executable
add_executable(SystemMonitor
    src/main.cpp
    src/core/SystemDataCollector.cpp
    src/core/ProcessTreeBuilder.cpp
    src/ui/MonitorUI.cpp
    src/ui/CPUWidget.cpp
    src/ui/MemoryWidget.cpp
    src/ui/DiskWidget.cpp
    src/ui/NetworkWidget.cpp
    src/ui/ProcessTreeWidget.cpp
    src/config/Configuration.cpp
    ${PLATFORM_SOURCES}
)

# Include directories
target_include_directories(SystemMonitor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(SystemMonitor PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(SystemMonitor PRIVATE 
        pdh.lib
        psapi.lib
        iphlpapi.lib
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(SystemMonitor PRIVATE 
        pthread
    )
elseif(APPLE)
    target_link_libraries(SystemMonitor PRIVATE
        "-framework CoreFoundation"
        "-framework IOKit"
    )
endif()

# Installation
install(TARGETS SystemMonitor DESTINATION bin)

# Build type specific settings
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(SystemMonitor PRIVATE DEBUG_BUILD)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_definitions(SystemMonitor PRIVATE RELEASE_BUILD)
    if(MSVC)
        target_compile_options(SystemMonitor PRIVATE /O2)
    else()
        target_compile_options(SystemMonitor PRIVATE -O3)
    endif()
endif()
