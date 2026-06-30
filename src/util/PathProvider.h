#pragma once

#include <QString>
#include <QDir>

namespace gpm {

/**
 * @brief 标准化路径：所有写入都通过 PathProvider，避免散落字符串。
 *
 *   AppConfigLocation/
 *     global.json                       全局设置
 *     games/<gameId>.json               单游戏配置
 *     hud/                              HUD 字段模板（未来）
 *
 *   AppDataLocation/
 *     logs/                             spdlog 滚动文件
 *     reports.db                        SQLite 会话库
 *     cache/                            临时缓存
 *
 *   AppDataLocation/reports/            HTML 报告
 */
class PathProvider {
public:
    static QString appConfigDir();   // <AppConfig>/GamePerfMonitor
    static QString appDataDir();     // <AppData>/GamePerfMonitor
    static QString globalConfigPath();
    static QString gamesDir();
    static QString gameConfigPath(const QString& gameId);
    static QString logsDir();
    static QString logFilePath();
    static QString reportsDir();
    static QString sessionDbPath();
    static QString cacheDir();

    /// 启动时确保所有目录存在
    static void ensureDirectories();
};

} // namespace gpm
