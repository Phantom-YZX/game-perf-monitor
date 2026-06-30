#include "ui/main/SettingsPage.h"
#include "ui/main/GameEditDialog.h"
#include "app/SettingsCenter.h"
#include "storage/GameConfigStore.h"
#include "util/Logger.h"
#include "util/PlatformUtils.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>

namespace gpm {

SettingsPage::SettingsPage(QWidget* parent) : QWidget(parent) {
    m_settings = SettingsCenter::instance().settings();
    loadGames();

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    buildGeneralGroup();
    buildGameListGroup();
    buildHudGroup();
#ifndef NDEBUG
    buildDevGroup();
#endif

    auto* bottom = new QHBoxLayout;
    m_btnSave = new QPushButton(QStringLiteral("保存"), this);
    connect(m_btnSave, &QPushButton::clicked, this, &SettingsPage::onSave);
    bottom->addStretch(1);
    bottom->addWidget(m_btnSave);
    root->addLayout(bottom);

    root->addStretch(1);
    onSelectionChanged();
}

void SettingsPage::buildGeneralGroup() {
    auto* box = new QGroupBox(QStringLiteral("通用"), this);
    auto* lay = new QGridLayout(box);

    m_chkStartWithWin = new QCheckBox(QStringLiteral("随 Windows 启动"), box);
    m_chkStartWithWin->setChecked(m_settings.startWithWindows);
    m_chkMinTray = new QCheckBox(QStringLiteral("关闭主窗口最小化到托盘"), box);
    m_chkMinTray->setChecked(m_settings.minimizeToTray);
    m_cboLanguage = new QComboBox(box);
    m_cboLanguage->addItems({ "zh-CN", "en-US" });
    m_cboLanguage->setCurrentText(m_settings.language);

    m_spInterval  = new QSpinBox(box); m_spInterval->setRange(100, 5000); m_spInterval->setSuffix(" ms");
    m_spInterval->setValue(m_settings.defaultIntervalMs);
    m_spHistory   = new QSpinBox(box); m_spHistory->setRange(60, 5000);
    m_spHistory->setValue(m_settings.defaultHistoryPoints);
    m_spUiRefresh = new QSpinBox(box); m_spUiRefresh->setRange(50, 2000); m_spUiRefresh->setSuffix(" ms");
    m_spUiRefresh->setValue(m_settings.defaultUiRefreshMs);

    int r = 0;
    lay->addWidget(m_chkStartWithWin, r, 0, 1, 2); ++r;
    lay->addWidget(m_chkMinTray,      r, 0, 1, 2); ++r;
    lay->addWidget(new QLabel(QStringLiteral("语言：")), r, 0);
    lay->addWidget(m_cboLanguage,         r, 1); ++r;
    lay->addWidget(new QLabel(QStringLiteral("采样间隔：")), r, 0);
    lay->addWidget(m_spInterval,          r, 1); ++r;
    lay->addWidget(new QLabel(QStringLiteral("历史点数：")), r, 0);
    lay->addWidget(m_spHistory,           r, 1); ++r;
    lay->addWidget(new QLabel(QStringLiteral("UI 刷新：")), r, 0);
    lay->addWidget(m_spUiRefresh,         r, 1); ++r;

    static_cast<QVBoxLayout*>(this->layout())->addWidget(box);
}

void SettingsPage::buildGameListGroup() {
    auto* box = new QGroupBox(QStringLiteral("游戏库"), this);
    auto* h = new QHBoxLayout(box);

    m_lstGames = new QListWidget(box);
    m_lstGames->setMinimumWidth(280);
    for (const auto& g : m_games) {
        m_lstGames->addItem(g.displayName.isEmpty() ? g.gameId.left(8) : g.displayName);
    }
    connect(m_lstGames, &QListWidget::itemSelectionChanged,
            this, &SettingsPage::onSelectionChanged);

    auto* right = new QVBoxLayout;
    m_btnAdd     = new QPushButton(QStringLiteral("新建…"),  box);
    m_btnAddFg   = new QPushButton(QStringLiteral("从前台进程加入"), box);
    m_btnBrowse  = new QPushButton(QStringLiteral("从 EXE 文件…"), box);
    m_btnEdit    = new QPushButton(QStringLiteral("编辑…"),  box);
    m_btnRemove  = new QPushButton(QStringLiteral("删除"),   box);

    connect(m_btnAdd,    &QPushButton::clicked, this, &SettingsPage::onAddGame);
    connect(m_btnAddFg,  &QPushButton::clicked, this, &SettingsPage::onAddFromForeground);
    connect(m_btnBrowse, &QPushButton::clicked, this, &SettingsPage::onBrowseExe);
    connect(m_btnEdit,   &QPushButton::clicked, this, &SettingsPage::onEditGame);
    connect(m_btnRemove, &QPushButton::clicked, this, &SettingsPage::onRemoveGame);

    for (auto* b : { m_btnAdd, m_btnAddFg, m_btnBrowse, m_btnEdit, m_btnRemove }) {
        right->addWidget(b);
    }
    right->addStretch(1);

    h->addWidget(m_lstGames, 1);
    h->addLayout(right);
    static_cast<QVBoxLayout*>(this->layout())->addWidget(box);
}

void SettingsPage::buildHudGroup() {
    auto* box = new QGroupBox(QStringLiteral("HUD 悬浮窗"), this);
    auto* lay = new QGridLayout(box);

    m_chkHudEnable = new QCheckBox(QStringLiteral("启用 HUD"), box);
    connect(m_chkHudEnable, &QCheckBox::toggled,
            this, &SettingsPage::onHudEnableChanged);

    m_spHudOpacity = new QSpinBox(box); m_spHudOpacity->setRange(10, 100); m_spHudOpacity->setSuffix(" %");
    connect(m_spHudOpacity, qOverload<int>(&QSpinBox::valueChanged),
            this, &SettingsPage::onHudOpacityChanged);

    m_spHudFont = new QSpinBox(box); m_spHudFont->setRange(8, 64); m_spHudFont->setSuffix(" px");
    connect(m_spHudFont, qOverload<int>(&QSpinBox::valueChanged),
            this, &SettingsPage::onHudFontChanged);

    m_cboHudDir = new QComboBox(box);
    m_cboHudDir->addItems({ "vertical", "horizontal" });
    connect(m_cboHudDir, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &SettingsPage::onHudDirectionChanged);

    m_lstHudFields = new QListWidget(box);
    m_lstHudFields->setSelectionMode(QAbstractItemView::NoSelection);

    int r = 0;
    lay->addWidget(m_chkHudEnable, r, 0, 1, 2); ++r;
    lay->addWidget(new QLabel(QStringLiteral("透明度：")), r, 0);
    lay->addWidget(m_spHudOpacity,                    r, 1); ++r;
    lay->addWidget(new QLabel(QStringLiteral("字号：")),   r, 0);
    lay->addWidget(m_spHudFont,                       r, 1); ++r;
    lay->addWidget(new QLabel(QStringLiteral("布局：")),   r, 0);
    lay->addWidget(m_cboHudDir,                       r, 1); ++r;
    lay->addWidget(new QLabel(QStringLiteral("字段（Ctrl+点击切换显示）：")), r, 0);
    lay->addWidget(m_lstHudFields, r, 1, 3, 1); ++r;

    static_cast<QVBoxLayout*>(this->layout())->addWidget(box);
}

#ifndef NDEBUG
void SettingsPage::buildDevGroup() {
    auto* box = new QGroupBox(QStringLiteral("开发 (仅 Debug 构建可见)"), this);
    auto* lay = new QVBoxLayout(box);
    auto* chk = new QCheckBox(QStringLiteral("dev.mockMode（强制启用 Mock 适配器）"), box);
    chk->setChecked(m_settings.devMockMode);
    connect(chk, &QCheckBox::toggled, this, [this](bool v) {
        m_settings.devMockMode = v;
    });
    lay->addWidget(chk);
    lay->addWidget(new QLabel(QStringLiteral("注意：Release 构建 / 第二阶段后此开关被忽略（#ifdef NDEBUG 包裹）。"), box));
    static_cast<QVBoxLayout*>(this->layout())->addWidget(box);
}
#endif

void SettingsPage::loadGames() {
    m_games.clear();
    for (const auto& id : GameConfigStore::listAll()) {
        m_games.append(GameConfigStore::load(id));
    }
}

void SettingsPage::storeGames() {
    for (auto& g : m_games) {
        GameConfigStore::save(g);
    }
}

void SettingsPage::onSelectionChanged() {
    m_currentGameIdx = m_lstGames ? m_lstGames->currentRow() : -1;
    if (m_currentGameIdx < 0 || m_currentGameIdx >= m_games.size()) {
        if (m_lstHudFields) m_lstHudFields->clear();
        return;
    }
    const auto& g = m_games[m_currentGameIdx];

    m_chkHudEnable->setChecked(g.hud.enabled);
    m_spHudOpacity->setValue(int(g.hud.opacity * 100));
    m_spHudFont->setValue(g.hud.baseFontSize);
    m_cboHudDir->setCurrentText(g.hud.direction);

    m_lstHudFields->clear();
    for (int i = 0; i < g.hud.fields.size(); ++i) {
        const auto& f = g.hud.fields[i];
        auto* item = new QListWidgetItem(
            QStringLiteral("[%1] %2 — %3%4")
                .arg(f.enabled ? "✓" : " ")
                .arg(f.label)
                .arg(f.format)
                .arg(f.color),
            m_lstHudFields);
        item->setData(Qt::UserRole, i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(f.enabled ? Qt::Checked : Qt::Unchecked);
    }
    connect(m_lstHudFields, &QListWidget::itemChanged, this, [this](QListWidgetItem* it) {
        int i = it->data(Qt::UserRole).toInt();
        if (m_currentGameIdx < 0 || m_currentGameIdx >= m_games.size()) return;
        if (i < 0 || i >= m_games[m_currentGameIdx].hud.fields.size()) return;
        m_games[m_currentGameIdx].hud.fields[i].enabled = (it->checkState() == Qt::Checked);
        emit sigHudSettingsChanged(m_games[m_currentGameIdx].hud);
    });
}

void SettingsPage::onAddGame() {
    auto* dlg = new GameEditDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    if (dlg->exec() == QDialog::Accepted) {
        auto p = dlg->profile();
        m_games.append(p);
        GameConfigStore::save(p);
        m_lstGames->addItem(p.displayName);
        m_lstGames->setCurrentRow(m_lstGames->count() - 1);
    }
}

void SettingsPage::onAddFromForeground() {
    const QString path = PlatformUtils::foregroundProcessPath();
    const QString name = PlatformUtils::foregroundProcessName();
    const QString title = PlatformUtils::foregroundWindowTitle();
    if (path.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("无前台进程"),
            QStringLiteral("未能获取前台进程，请先切到目标游戏窗口再点此按钮。"));
        return;
    }
    auto* dlg = new GameEditDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->presetFromForeground(path, name, title);
    if (dlg->exec() == QDialog::Accepted) {
        auto p = dlg->profile();
        m_games.append(p);
        GameConfigStore::save(p);
        m_lstGames->addItem(p.displayName);
        m_lstGames->setCurrentRow(m_lstGames->count() - 1);
    }
}

void SettingsPage::onBrowseExe() {
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择游戏 EXE"), QString(), "Executable (*.exe)");
    if (path.isEmpty()) return;
    QFileInfo fi(path);
    auto* dlg = new GameEditDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->presetFromForeground(path, fi.fileName(), QString());
    if (dlg->exec() == QDialog::Accepted) {
        auto p = dlg->profile();
        m_games.append(p);
        GameConfigStore::save(p);
        m_lstGames->addItem(p.displayName);
        m_lstGames->setCurrentRow(m_lstGames->count() - 1);
    }
}

void SettingsPage::onEditGame() {
    if (m_currentGameIdx < 0 || m_currentGameIdx >= m_games.size()) return;
    auto* dlg = new GameEditDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setProfile(m_games[m_currentGameIdx]);
    if (dlg->exec() == QDialog::Accepted) {
        m_games[m_currentGameIdx] = dlg->profile();
        GameConfigStore::save(m_games[m_currentGameIdx]);
        m_lstGames->currentItem()->setText(m_games[m_currentGameIdx].displayName);
    }
}

void SettingsPage::onRemoveGame() {
    if (m_currentGameIdx < 0 || m_currentGameIdx >= m_games.size()) return;
    const auto& g = m_games[m_currentGameIdx];
    if (QMessageBox::question(this, QStringLiteral("删除"),
        QStringLiteral("确定删除 \"%1\"？").arg(g.displayName))
        != QMessageBox::Yes) return;
    GameConfigStore::remove(g.gameId);
    delete m_lstGames->takeItem(m_currentGameIdx);
    m_games.removeAt(m_currentGameIdx);
}

void SettingsPage::onSave() {
    m_settings.startWithWindows    = m_chkStartWithWin->isChecked();
    m_settings.minimizeToTray      = m_chkMinTray->isChecked();
    m_settings.language            = m_cboLanguage->currentText();
    m_settings.defaultIntervalMs   = m_spInterval->value();
    m_settings.defaultHistoryPoints= m_spHistory->value();
    m_settings.defaultUiRefreshMs  = m_spUiRefresh->value();

    auto& sc = SettingsCenter::instance();
    sc.setSettings(m_settings);
    Logger::get()->info("SettingsPage: saved global settings");
}

void SettingsPage::onHudEnableChanged(bool v) {
    if (m_currentGameIdx < 0) return;
    m_games[m_currentGameIdx].hud.enabled = v;
    emit sigHudVisibilityChanged(v);
    emit sigHudSettingsChanged(m_games[m_currentGameIdx].hud);
}

void SettingsPage::onHudOpacityChanged(int v) {
    if (m_currentGameIdx < 0) return;
    m_games[m_currentGameIdx].hud.opacity = v / 100.0;
    emit sigHudSettingsChanged(m_games[m_currentGameIdx].hud);
}

void SettingsPage::onHudFontChanged(int v) {
    if (m_currentGameIdx < 0) return;
    m_games[m_currentGameIdx].hud.baseFontSize = v;
    emit sigHudSettingsChanged(m_games[m_currentGameIdx].hud);
}

void SettingsPage::onHudDirectionChanged(int idx) {
    if (m_currentGameIdx < 0) return;
    m_games[m_currentGameIdx].hud.direction = (idx == 0 ? "vertical" : "horizontal");
    emit sigHudSettingsChanged(m_games[m_currentGameIdx].hud);
}

} // namespace gpm
