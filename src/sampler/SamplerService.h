#pragma once

#include "sampler/IDataAdapter.h"
#include "util/SampleBlock.h"

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <QThreadPool>
#include <QRunnable>

#include <atomic>
#include <memory>
#include <vector>

namespace gpm {

/**
 * @brief 采样服务调度器。
 *
 *   start(GameProfile::enabledAdapters, intervalMs)
 *     - 注册到 AdapterRegistry
 *     - 启动 QTimer 每 intervalMs 触发一次 tick
 *   tick()：
 *     - 顺序调用所有 active adapter 的 tick(block) 合并到一个 SampleBlock
 *     - 推入 DataBus::push()
 *   stop()：停止 QTimer、关闭线程池
 *
 * 线程模型：
 *   - 适配器 tick() 默认在主线程（QTimer 在主线程上启动）
 *   - 高开销适配器（PresentMon）可重写 tick() 内部用 QThreadPool
 *   - DataBus 自身线程安全
 */
class SamplerService : public QObject {
    Q_OBJECT
public:
    explicit SamplerService(QObject* parent = nullptr);
    ~SamplerService() override;

    bool isRunning() const { return m_running; }

    /// intervalMs：采样间隔（默认 1000ms）；uiRefreshMs：UI 节拍（默认 500ms）
    void start(const QStringList& enabledAdapters, int intervalMs, int uiRefreshMs);
    void stop();

    /// 内部计数器：方便测试 / 状态栏
    quint64 sampleCount() const { return m_seq; }

signals:
    void started();
    void stopped();
    void errorOccurred(const QString& message);

private slots:
    void onSamplingTick();
    void onUiTick();

private:
    void buildActive(const QStringList& enabledAdapters);

    QTimer* m_sampleTimer = nullptr;
    QTimer* m_uiTimer      = nullptr;

    std::vector<IDataAdapterPtr> m_active;
    quint64 m_seq = 0;
    std::atomic_bool m_running { false };
};

} // namespace gpm
