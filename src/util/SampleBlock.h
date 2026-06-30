#pragma once

#include <QString>
#include <QDateTime>

#include <cstdint>
#include <optional>

namespace gpm {

/**
 * @brief 单次采样的"一个适配器"产出。指标值用 std::optional<double>
 *        表示：缺数据 = std::nullopt，UI 层映射为 "N/A"。
 *
 * - 数值字段：CPU/GPU/RAM/VRAM/温度/电压/功耗/频率/风扇/网络等
 * - 文本字段：fps（FPS 整数）、frameTime（ms）、oneLow / pointOneLow（百分位 FPS）
 * - 文本指标独立保留是为了不与浮点指标混淆（数据缺失语义明确）
 */
struct SampleBlock {
    quint64 seq        = 0;     ///< 会话内单调递增序列号
    qint64  tsMs       = 0;     ///< Unix epoch 毫秒
    qint64  deltaMs    = 0;     ///< 与上一帧间隔（用于帧时间聚合）

    // ---- 百分比（0-100）----
    std::optional<double> cpuUsagePct;
    std::optional<double> gpuUsagePct;
    std::optional<double> ramUsagePct;
    std::optional<double> vramUsagePct;

    // ---- 频率（MHz）----
    std::optional<double> cpuFreqMhz;
    std::optional<double> gpuFreqMhz;
    std::optional<double> ramFreqMhz;

    // ---- 温度（℃）----
    std::optional<double> cpuTempC;
    std::optional<double> gpuTempC;

    // ---- 电压（V）----
    std::optional<double> cpuVoltageV;
    std::optional<double> gpuVoltageV;

    // ---- 功耗（W）----
    std::optional<double> cpuPowerW;
    std::optional<double> gpuPowerW;

    // ---- 风扇（RPM / %）----
    std::optional<double> cpuFanRpm;
    std::optional<double> gpuFanRpm;
    std::optional<double> cpuFanPct;
    std::optional<double> gpuFanPct;

    // ---- 显存 / 内存（MB）----
    std::optional<double> ramUsedMb;
    std::optional<double> ramTotalMb;
    std::optional<double> vramUsedMb;
    std::optional<double> vramTotalMb;

    // ---- 网络（KB/s + 延迟 ms）----
    std::optional<double> netRxKbps;
    std::optional<double> netTxKbps;
    std::optional<double> netLatencyMs;

    // ---- FPS / 帧时间 / 百分位 ----
    std::optional<double> fps;
    std::optional<double> frameTimeMs;
    std::optional<double> oneLowPct;       ///< 1% low FPS
    std::optional<double> pointOneLowPct;  ///< 0.1% low FPS
    std::optional<QString> presentMode;    ///< "窗口化" / "独占全屏" / "无边框全屏" 等
};

} // namespace gpm
