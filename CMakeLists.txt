cmake_minimum_required(VERSION 3.21)

project(game_perf_monitor
    VERSION 1.0.0
    DESCRIPTION "Windows game performance monitor"
    LANGUAGES CXX)

# ------------------------------------------------------------------
# 全局编译选项
# ------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Debug 构建启用 mock 模块注册（Mock 适配器编译进二进制）
# Release 构建不启用，必须使用真实适配器
set(_gpmMockDefault OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_gpmMockDefault ON)
endif()
option(GAME_PERF_ENABLE_MOCK "Build with mock data adapters (Phase 1A/1B only)" ${_gpmMockDefault})

# 让顶层目录能直接 include 各子目录头文件（约定 #include "util/Logger.h"）
set(GPM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src)

# ------------------------------------------------------------------
# Qt 6 (QWidget + Qt Charts；不使用 QML / Qt Quick)
# ------------------------------------------------------------------
find_package(Qt6 6.6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
    Network
    Concurrent
    Charts
)

qt_standard_project_setup(REQUIRES 6.6)

# ------------------------------------------------------------------
# 第三方库 (vcpkg)
# ------------------------------------------------------------------
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt    REQUIRED)
find_package(inja   REQUIRED)
find_package(SQLiteCpp REQUIRED)

# ------------------------------------------------------------------
# 主可执行文件
# ------------------------------------------------------------------
qt_add_executable(GamePerfMonitor WIN32
    src/app/main.cpp
    src/app/AppMain.h
    src/app/AppMain.cpp
    src/app/SettingsCenter.h
    src/app/SettingsCenter.cpp
    src/app/PrivilegeGuard.h
    src/app/PrivilegeGuard.cpp
    src/util/Logger.h
    src/util/Logger.cpp
    src/util/PathProvider.h
    src/util/PathProvider.cpp
    src/util/PlatformUtils.h
    src/util/PlatformUtils.cpp
    src/util/SampleBlock.h
    src/sampler/IDataAdapter.h
    src/sampler/DataBus.h
    src/sampler/DataBus.cpp
    src/sampler/AdapterRegistry.h
    src/sampler/AdapterRegistry.cpp
    src/sampler/SamplerService.h
    src/sampler/SamplerService.cpp
    src/storage/JsonStore.h
    src/storage/JsonStore.cpp
    src/storage/ConfigStore.h
    src/storage/ConfigStore.cpp
    src/storage/GameConfigStore.h
    src/storage/GameConfigStore.cpp
    src/storage/GameProfile.h
    src/storage/GameProfile.cpp
    src/game/GameMatcher.h
    src/game/GameMatcher.cpp
    src/ui/main/MainWindow.h
    src/ui/main/MainWindow.cpp
    src/ui/main/RealtimePanel.h
    src/ui/main/RealtimePanel.cpp
    src/ui/main/SettingsPage.h
    src/ui/main/SettingsPage.cpp
    src/ui/main/GameEditDialog.h
    src/ui/main/GameEditDialog.cpp
    src/ui/hud/HudWindow.h
    src/ui/hud/HudWindow.cpp
    src/ui/hud/HudFieldModel.h
    src/ui/hud/HudFieldModel.cpp
    src/ui/hud/HudFieldDelegate.h
    src/ui/hud/HudFieldDelegate.cpp
    src/ui/hud/HudLayoutEngine.h
    src/ui/hud/HudLayoutEngine.cpp
)

# 资源
qt_add_resources(GamePerfMonitor "qss"
    PREFIX "/qss"
    FILES
        resources/qss/rtss.qss
        resources/qss/hud.qss
)
# 图标（可选）：缺文件时跳过，使用 Qt 内置默认图标
if(EXISTS "${CMAKE_SOURCE_DIR}/resources/icons/app.ico")
    qt_add_resources(GamePerfMonitor "icons"
        PREFIX "/icons"
        FILES
            resources/icons/app.ico
            resources/icons/tray.ico
    )
else()
    message(STATUS "GamePerfMonitor: no icons/*.ico found, using Qt default icons")
endif()

# ------------------------------------------------------------------
# 链接
# ------------------------------------------------------------------
target_link_libraries(GamePerfMonitor PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
    Qt6::Concurrent
    Qt6::Charts
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    fmt::fmt
    pantor::inja
    SQLiteCpp
)

if(WIN32)
    target_link_libraries(GamePerfMonitor PRIVATE
        advapi32
        iphlpapi
        pdh
        user32
        dwmapi
        shell32
    )
    set_target_properties(GamePerfMonitor PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

target_include_directories(GamePerfMonitor PRIVATE
    ${GPM_INCLUDE_DIR}
)

# ------------------------------------------------------------------
# Mock 适配器 (Phase 1A/1B 演示用)
# ------------------------------------------------------------------
if(GAME_PERF_ENABLE_MOCK)
    target_compile_definitions(GamePerfMonitor PRIVATE GAME_PERF_MOCK_ENABLED=1)
    target_sources(GamePerfMonitor PRIVATE
        src/sampler/mock/MockAdapterBase.h
        src/sampler/mock/MockCpuAdapter.h
        src/sampler/mock/MockGpuAdapter.h
        src/sampler/mock/MockFpsAdapter.h
        src/sampler/mock/MockRamAdapter.h
        src/sampler/mock/MockVramAdapter.h
        src/sampler/mock/MockThermalAdapter.h
        src/sampler/mock/MockVoltageAdapter.h
        src/sampler/mock/MockPowerAdapter.h
        src/sampler/mock/MockFanAdapter.h
        src/sampler/mock/MockNetAdapter.h
        src/sampler/mock/MockFrameTimeAdapter.h
    )
    message(STATUS "GamePerfMonitor: Mock adapters ENABLED (Phase 1A/1B)")
else()
    target_compile_definitions(GamePerfMonitor PRIVATE GAME_PERF_MOCK_ENABLED=0)
    message(STATUS "GamePerfMonitor: Mock adapters DISABLED (real adapters only)")
endif()
