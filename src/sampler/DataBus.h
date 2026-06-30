#pragma once

#include "util/SampleBlock.h"

#include <QObject>
#include <QMutex>

#include <deque>

namespace gpm {

/**
 * @brief 全局采样数据总线（Qt Signal/Slot 单例）。
 *
 * 设计：
 *   - SamplerService 在工作线程上 tick() 所有 adapter → DataBus::push()
 *   - UI（RealtimePanel / HudWindow）按 500ms 节拍从 DataBus::latest() 拉取
 *   - SessionRecorder（阶段 1B）订阅 sigBlock 信号持久化到 SQLite
 *
 * DataBus 内部维护"最近 N 个 SampleBlock"环形缓冲，给图表 / 历史面板使用。
 *
 * 线程模型：
 *   - push() 任何线程可调
 *   - latest() / snapshot() 任何线程可调
 *   - 内部有 mutex 保护，但 push 是热路径，要尽量轻
 */
class DataBus : public QObject {
    Q_OBJECT
public:
    static DataBus& instance();

    /// 默认缓冲深度 = 600 点（5 分钟 @1s/点；UI 500ms 节拍时 = 5 分钟）
    void setBufferSize(int n);
    int  bufferSize() const { return m_bufSize; }

    /// 推入一个采样点（任何线程）
    void push(const SampleBlock& b);

    /// 拉取最近一个采样点（任何线程；空时返回 std::nullopt）
    SampleBlock latest() const;

    /// 拉取最近 N 个采样点（线程安全；按时间升序）
    std::deque<SampleBlock> snapshot(int n = -1) const;

    /// 启动时清空（切换会话前调用）
    void clear();

signals:
    /// 每帧推送（新采样进入时立即触发，仅给 Recorder / Debug 用）
    void sigBlock(const gpm::SampleBlock& block);

    /// UI 节拍信号（500ms 一次），UI 在槽里调用 latest() / snapshot() 拉取
    void sigUiTick();

private:
    explicit DataBus(QObject* parent = nullptr);

    mutable QMutex      m_mtx;
    std::deque<SampleBlock> m_buf;
    int m_bufSize = 600;
};

} // namespace gpm

Q_DECLARE_METATYPE(gpm::SampleBlock)
