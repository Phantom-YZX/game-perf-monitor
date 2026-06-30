#include "util/Logger.h"
#include "util/PathProvider.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace gpm {

namespace {
std::shared_ptr<spdlog::logger> g_logger;
}

void Logger::init() {
    if (g_logger) return;

    PathProvider::ensureDirectories();

    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console->set_level(spdlog::level::debug);
    console->set_pattern("[%H:%M:%S.%e] [%^%l%$] [%n] %v");

    auto file = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        PathProvider::logFilePath().toStdString(),
        10 * 1024 * 1024,  // 10MB
        5                  // 5 个文件
    );
    file->set_level(spdlog::level::info);
    file->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");

    g_logger = std::make_shared<spdlog::logger>("gpm", spdlog::sinks_init_list{console, file});
    g_logger->set_level(spdlog::level::debug);
    g_logger->flush_on(spdlog::level::warn);
    spdlog::set_default_logger(g_logger);

    g_logger->info("Logger initialized. log file: {}", PathProvider::logFilePath().toStdString());
}

std::shared_ptr<spdlog::logger> Logger::get() {
    if (!g_logger) init();
    return g_logger;
}

void Logger::shutdown() {
    spdlog::shutdown();
    g_logger.reset();
}

} // namespace gpm
