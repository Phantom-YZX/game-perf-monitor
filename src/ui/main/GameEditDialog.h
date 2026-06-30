#pragma once

#include "storage/GameProfile.h"

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QSpinBox;
class QComboBox;
class QListWidget;
class QPushButton;

namespace gpm {

/**
 * @brief 新增 / 编辑单游戏配置的对话框。
 *
 * 字段：displayName、match.exePath、match.exeName、match.windowTitleRegex、
 *      enabledAdapters（多选）、HUD 字段（增/删/上移/下移）
 */
class GameEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit GameEditDialog(QWidget* parent = nullptr);

    GameProfile profile() const { return m_p; }
    void setProfile(const GameProfile& p);
    void presetFromForeground(const QString& path, const QString& name, const QString& title);

private slots:
    void onAddField();
    void onRemoveField();
    void onMoveUp();
    void onMoveDown();
    void accept() override;

private:
    void rebuildFieldList();
    void loadIntoUi();
    void saveFromUi();

    GameProfile m_p;

    QLineEdit*    m_edDisplay     = nullptr;
    QLineEdit*    m_edExePath     = nullptr;
    QLineEdit*    m_edExeName     = nullptr;
    QLineEdit*    m_edTitleRegex  = nullptr;
    QListWidget*  m_lstAdapters   = nullptr;
    QSpinBox*     m_spInterval    = nullptr;
    QSpinBox*     m_spHistory     = nullptr;
    QSpinBox*     m_spUiRefresh   = nullptr;
    QListWidget*  m_lstFields     = nullptr;
    QPushButton*  m_btnAddField   = nullptr;
    QPushButton*  m_btnRemoveField= nullptr;
    QPushButton*  m_btnMoveUp     = nullptr;
    QPushButton*  m_btnMoveDown   = nullptr;
};

} // namespace gpm
