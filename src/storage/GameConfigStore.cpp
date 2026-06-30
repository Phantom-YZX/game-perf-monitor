#include "storage/GameConfigStore.h"

#include "storage/JsonStore.h"
#include "util/Logger.h"
#include "util/PathProvider.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QUuid>

namespace gpm {

GameProfile GameConfigStore::load(const QString& gameId) {
    auto opt = JsonStore::read(PathProvider::gameConfigPath(gameId));
    if (!opt) return GameProfile{};
    return GameProfile::fromJson(*opt);
}

bool GameConfigStore::save(GameProfile& profile) {
    profile.updatedAtMs = QDateTime::currentMSecsSinceEpoch();
    if (profile.createdAtMs == 0) profile.createdAtMs = profile.updatedAtMs;

    const bool ok = JsonStore::writeAtomic(PathProvider::gameConfigPath(profile.gameId), profile.toJson());
    if (ok) {
        Logger::get()->info("GameConfigStore: saved {}", profile.gameId.toStdString());
    } else {
        Logger::get()->error("GameConfigStore: failed to save {}", profile.gameId.toStdString());
    }
    return ok;
}

QStringList GameConfigStore::listAll() {
    QDir dir(PathProvider::gamesDir());
    if (!dir.exists()) return {};

    QStringList out;
    const auto files = dir.entryInfoList({ "*.json" }, QDir::Files, QDir::Name);
    for (const auto& file : files) {
        out << file.completeBaseName();
    }
    return out;
}

bool GameConfigStore::remove(const QString& gameId) {
    QFile file(PathProvider::gameConfigPath(gameId));
    if (!file.exists()) return true;

    const bool ok = file.remove();
    if (ok) Logger::get()->info("GameConfigStore: removed {}", gameId.toStdString());
    return ok;
}

GameProfile GameConfigStore::createNew(const QString& displayName) {
    GameProfile profile;
    profile.gameId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    profile.displayName = displayName.isEmpty() ? QStringLiteral("New Game") : displayName;
    profile.createdAtMs = profile.updatedAtMs = QDateTime::currentMSecsSinceEpoch();

    auto add = [&](const QString& id, const QString& source, const QString& label,
                   const QString& format, const QString& color, int fontSize) {
        HudFieldConfig field;
        field.id = id;
        field.source = source;
        field.label = label;
        field.format = format;
        field.color = color;
        field.fontSize = fontSize;
        field.enabled = true;
        profile.hud.fields.append(field);
    };

    add("fps", "fps", "FPS", "%.0f", "#22d3ee", 18);
    add("onelow", "frameTime", "1% LOW", "%.0f", "#4ade80", 12);
    add("pointonelow", "frameTime", "0.1% LOW", "%.0f", "#fbbf24", 12);
    add("frametime", "frameTime", "FRAME", "%.1f ms", "#e4e4e7", 12);
    add("cpu", "cpu", "CPU", "%.0f%%", "#60a5fa", 14);
    add("gpu", "gpu", "GPU", "%.0f%%", "#4ade80", 14);
    add("gputemp", "thermal", "GPU C", "%.0f C", "#f87171", 12);
    add("vramused", "vram", "VRAM", "%.0f MB", "#22d3ee", 12);

    return profile;
}

} // namespace gpm
