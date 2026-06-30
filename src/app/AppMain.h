#pragma once

#include "sampler/SamplerService.h"

#include <QObject>
#include <memory>

class QSystemTrayIcon;

namespace gpm {

class MainWindow;
class HudWindow;
class SamplerService;

/**
 * @brief 应用编排：单实例 + 启动初始化 + 关闭。
 */
class AppMain : public QObject {
    Q_OBJECT
public:
    explicit AppMain(QObject* parent = nullptr);
    ~AppMain() override;

    int run(int argc, char** argv);
    void shutdown();

    MainWindow*  mainWindow()  const { return m_mainWindow; }
    HudWindow*   hudWindow()   const { return m_hudWindow; }
    SamplerService* sampler()   const { return m_sampler.get(); }

private slots:
    void onSamplingTickUi();

private:
    void registerAdapters();
    void startSampling();

    MainWindow*  m_mainWindow  = nullptr;
    HudWindow*   m_hudWindow   = nullptr;
    std::unique_ptr<SamplerService> m_sampler;
    QSystemTrayIcon* m_tray     = nullptr;
};

} // namespace gpm
