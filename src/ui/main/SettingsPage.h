#pragma once

#include "storage/GameProfile.h"

#include <QWidget>

class QListWidget;
class QPushButton;
class QSpinBox;
class QCheckBox;
class QComboBox;

namespace gpm {

/**
 * @brief 设置页（1A）：
 *   - 通用：随 Windows 启动 / 最小化到托盘 / 采样参数
 *   - 游戏库：列表 + 增/删/编辑
 *   - HUD：可见性、字号、透明度、横/竖布局、字段勾选
 *   - dev：Debug 构建下显示 mockMode 开关
 */
class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(QWidget* parent = nullptr);

    const AppSettings& settings() const { return m_settings; }
    const QList<GameProfile>& games() const { return m_games; }

signals:
    void sigHudSettingsChanged(const HudConfig& h);
    void sigHudVisibilityChanged(bool visible);
    void sigHudPositionChanged();

private slots:
    void onAddGame();
    void onEditGame();
    void onRemoveGame();
    void onAddFromForeground();
    void onBrowseExe();
    void onSave();
    void onSelectionChanged();
    void onHudEnableChanged(bool v);
    void onHudOpacityChanged(int v);
    void onHudFontChanged(int v);
    void onHudDirectionChanged(int idx);

private:
    void buildGeneralGroup();
    void buildGameListGroup();
    void buildHudGroup();
    void buildDevGroup();
    void loadGames();
    void storeGames();

    AppSettings       m_settings;
    QList<GameProfile> m_games;
    int               m_currentGameIdx = -1;

    // 通用
    QCheckBox*  m_chkStartWithWin = nullptr;
    QCheckBox*  m_chkMinTray      = nullptr;
    QComboBox*  m_cboLanguage     = nullptr;
    QSpinBox*   m_spInterval      = nullptr;
    QSpinBox*   m_spHistory       = nullptr;
    QSpinBox*   m_spUiRefresh     = nullptr;

    // 游戏库
    QListWidget*  m_lstGames      = nullptr;
    QPushButton*  m_btnAdd        = nullptr;
    QPushButton*  m_btnAddFg      = nullptr;
    QPushButton*  m_btnBrowse     = nullptr;
    QPushButton*  m_btnEdit       = nullptr;
    QPushButton*  m_btnRemove     = nullptr;
    QPushButton*  m_btnSave       = nullptr;

    // HUD
    QCheckBox*  m_chkHudEnable   = nullptr;
    QSpinBox*   m_spHudOpacity   = nullptr;
    QSpinBox*   m_spHudFont      = nullptr;
    QComboBox*  m_cboHudDir      = nullptr;
    QListWidget* m_lstHudFields  = nullptr;
};

} // namespace gpm
