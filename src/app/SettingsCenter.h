#pragma once

#include "storage/GameProfile.h"

#include <QObject>
#include <QStringList>

namespace gpm {

/**
 * @brief 全局设置的内存镜像 + 通知中心。
 *
 *   - 启动时 load
 *   - 任意 UI 改动 → set() → 自动 save
 *   - 业务代码（采样器 / HUD）订阅 sigChanged
 */
class SettingsCenter : public QObject {
    Q_OBJECT
public:
    static SettingsCenter& instance();

    const AppSettings& settings() const { return m_s; }
    AppSettings settings() { return m_s; }

    /// 整体替换（自动 save）
    void setSettings(const AppSettings& s);

    /// 修改单个字段（自动 save）
    void setStartWithWindows(bool v);
    void setMinimizeToTray(bool v);
    void setLanguage(const QString& lang);
    void setDefaultIntervalMs(int v);
    void setDefaultHistoryPoints(int v);
    void setDefaultUiRefreshMs(int v);

    /// dev 开关：仅 Debug 构建读 devMockMode；Release 永远 false
    bool devMockMode() const;

    /// 重新从磁盘加载
    void reload();

signals:
    void sigChanged();

private:
    explicit SettingsCenter(QObject* parent = nullptr);
    void saveDebounced();

    AppSettings m_s;
};

} // namespace gpm
