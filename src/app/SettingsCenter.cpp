#include "app/SettingsCenter.h"
#include "storage/ConfigStore.h"
#include "util/Logger.h"

#include <QTimer>

namespace gpm {

namespace {
constexpr int kDebounceMs = 200;
}

SettingsCenter::SettingsCenter(QObject* parent) : QObject(parent) {
    m_s = ConfigStore::load();
    Logger::get()->info("SettingsCenter: loaded (devMockMode={})", devMockMode());
}

SettingsCenter& SettingsCenter::instance() {
    static SettingsCenter c;
    return c;
}

void SettingsCenter::setSettings(const AppSettings& s) {
    m_s = s;
    saveDebounced();
    emit sigChanged();
}

void SettingsCenter::setStartWithWindows(bool v) { m_s.startWithWindows = v; saveDebounced(); emit sigChanged(); }
void SettingsCenter::setMinimizeToTray(bool v)   { m_s.minimizeToTray   = v; saveDebounced(); emit sigChanged(); }
void SettingsCenter::setLanguage(const QString& lang) { m_s.language = lang; saveDebounced(); emit sigChanged(); }
void SettingsCenter::setDefaultIntervalMs(int v) { m_s.defaultIntervalMs    = v; saveDebounced(); emit sigChanged(); }
void SettingsCenter::setDefaultHistoryPoints(int v) { m_s.defaultHistoryPoints = v; saveDebounced(); emit sigChanged(); }
void SettingsCenter::setDefaultUiRefreshMs(int v) { m_s.defaultUiRefreshMs   = v; saveDebounced(); emit sigChanged(); }

bool SettingsCenter::devMockMode() const {
#ifndef NDEBUG
    return m_s.devMockMode;
#else
    return false;  // Release / Phase 2：硬编码 false
#endif
}

void SettingsCenter::reload() {
    m_s = ConfigStore::load();
    emit sigChanged();
}

void SettingsCenter::saveDebounced() {
    // 简单 debounce：200ms 内多次 set 只写一次盘
    QTimer::singleShot(kDebounceMs, this, [this]() {
        ConfigStore::save(m_s);
    });
}

} // namespace gpm
