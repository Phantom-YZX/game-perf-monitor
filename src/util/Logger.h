#pragma once

#include <QString>

#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace gpm {

/**
 * @brief spdlog 封装。输出到 stderr + 滚动文件（10MB × 5 个）。
 *
 * 使用：
 *   gpm::Logger::init();
 *   gpm::Logger::get()->info("started, version={}", VERSION);
 */
class Logger {
public:
    /// 启动时调用一次，文件落盘到 PathProvider::logFilePath()
    static void init();

    static std::shared_ptr<spdlog::logger> get();

    /// 关闭时调用一次
    static void shutdown();
};

} // namespace gpm
