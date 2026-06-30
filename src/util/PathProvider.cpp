#include "util/PathProvider.h"

#include <QStandardPaths>
#include <QDir>
#include <QUuid>

namespace gpm {

namespace {
constexpr auto kAppName = "GamePerfMonitor";
}

QString PathProvider::appConfigDir() {
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation))
        .filePath(kAppName);
}

QString PathProvider::appDataDir() {
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
        .filePath(kAppName);
}

QString PathProvider::globalConfigPath() {
    return QDir(appConfigDir()).filePath("global.json");
}

QString PathProvider::gamesDir() {
    return QDir(appConfigDir()).filePath("games");
}

QString PathProvider::gameConfigPath(const QString& gameId) {
    return QDir(gamesDir()).filePath(gameId + ".json");
}

QString PathProvider::logsDir() {
    return QDir(appDataDir()).filePath("logs");
}

QString PathProvider::logFilePath() {
    return QDir(logsDir()).filePath("game-perf-monitor.log");
}

QString PathProvider::reportsDir() {
    return QDir(appDataDir()).filePath("reports");
}

QString PathProvider::sessionDbPath() {
    return QDir(appDataDir()).filePath("reports.db");
}

QString PathProvider::cacheDir() {
    return QDir(appDataDir()).filePath("cache");
}

void PathProvider::ensureDirectories() {
    const QStringList dirs = {
        appConfigDir(),
        gamesDir(),
        appDataDir(),
        logsDir(),
        reportsDir(),
        cacheDir()
    };
    for (const QString& d : dirs) {
        QDir().mkpath(d);
    }
}

} // namespace gpm
