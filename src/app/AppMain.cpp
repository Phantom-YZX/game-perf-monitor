#include "app/AppMain.h"
#include "app/SettingsCenter.h"
#include "util/Logger.h"
#include "util/PlatformUtils.h"
#include "util/PathProvider.h"
#include "sampler/AdapterRegistry.h"
#include "sampler/DataBus.h"

#include "ui/main/MainWindow.h"
#include "ui/hud/HudWindow.h"

#include <QApplication>
#include <QFile>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QStyle>

#if GAME_PERF_MOCK_ENABLED
#include "sampler/mock/MockCpuAdapter.h"
#include "sampler/mock/MockGpuAdapter.h"
#include "sampler/mock/MockFpsAdapter.h"
#include "sampler/mock/MockFrameTimeAdapter.h"
#include "sampler/mock/MockRamAdapter.h"
#include "sampler/mock/MockVramAdapter.h"
#include "sampler/mock/MockThermalAdapter.h"
#include "sampler/mock/MockVoltageAdapter.h"
#include "sampler/mock/MockPowerAdapter.h"
#include "sampler/mock/MockFanAdapter.h"
#include "sampler/mock/MockNetAdapter.h"
#endif

namespace gpm {

AppMain::AppMain(QObject* parent) : QObject(parent) {}
AppMain::~AppMain() = default;

int AppMain::run(int /*argc*/, char** /*argv*/) {
    PlatformUtils::enableDpiAwareness();
    PathProvider::ensureDirectories();

    Logger::get()->info("AppMain: starting up");

    registerAdapters();

    m_sampler = std::make_unique<SamplerService>();

    m_mainWindow = new MainWindow();
    m_mainWindow->show();

    m_hudWindow  = new HudWindow();
    m_mainWindow->setHudWindow(m_hudWindow);
    // 1A 阶段 HUD 默认不显示，用户在设置里勾上后才显示
    // （保留位置 / 锁定 / 字号设置在 UI 层）

    // UI 节拍：DataBus::sigUiTick → mainWindow 拉取数据
    connect(&DataBus::instance(), &DataBus::sigUiTick,
            m_mainWindow, &MainWindow::onUiTick);
    connect(&DataBus::instance(), &DataBus::sigUiTick,
            m_hudWindow,  &HudWindow::onUiTick);

    // 启动采样（默认 1s 采样 + 500ms UI 节拍）
    startSampling();
    return 0; // QApplication::exec 由 main.cpp 调用
}

void AppMain::shutdown() {
    if (m_sampler) m_sampler->stop();
    if (m_hudWindow)  { m_hudWindow->hide();  m_hudWindow->deleteLater();  }
    if (m_mainWindow) { m_mainWindow->hide(); m_mainWindow->deleteLater(); }
    Logger::get()->info("AppMain: shutdown");
}

void AppMain::registerAdapters() {
    auto& reg = AdapterRegistry::instance();
    reg.clear();

#if GAME_PERF_MOCK_ENABLED
    reg.registerAdapter(std::make_shared<MockCpuAdapter>());
    reg.registerAdapter(std::make_shared<MockGpuAdapter>());
    reg.registerAdapter(std::make_shared<MockFpsAdapter>());
    reg.registerAdapter(std::make_shared<MockFrameTimeAdapter>());
    reg.registerAdapter(std::make_shared<MockRamAdapter>());
    reg.registerAdapter(std::make_shared<MockVramAdapter>());
    reg.registerAdapter(std::make_shared<MockThermalAdapter>());
    reg.registerAdapter(std::make_shared<MockVoltageAdapter>());
    reg.registerAdapter(std::make_shared<MockPowerAdapter>());
    reg.registerAdapter(std::make_shared<MockFanAdapter>());
    reg.registerAdapter(std::make_shared<MockNetAdapter>());
    Logger::get()->info("AppMain: 11 mock adapters registered");
#else
    Logger::get()->info("AppMain: mock adapters DISABLED (real adapters will be added in Phase 2)");
#endif
}

void AppMain::startSampling() {
    const auto& s = SettingsCenter::instance().settings();
    // Phase 1A 启用所有可用适配器（暂时都是 mock）；GameProfile 还没起作用
    QStringList enabled;
#if GAME_PERF_MOCK_ENABLED
    for (const auto& a : AdapterRegistry::instance().all()) {
        enabled << a->id();
    }
#endif
    m_sampler->start(enabled, s.defaultIntervalMs, s.defaultUiRefreshMs);
}

void AppMain::onSamplingTickUi() {
    // reserved for future use
}

} // namespace gpm
