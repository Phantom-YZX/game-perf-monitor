#include "ui/main/GameEditDialog.h"
#include "sampler/AdapterRegistry.h"
#include "storage/GameConfigStore.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace gpm {

GameEditDialog::GameEditDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QStringLiteral("游戏配置"));
    resize(720, 640);
    auto* root = new QVBoxLayout(this);

    // ---- 基本信息 ----
    auto* baseBox = new QGroupBox(QStringLiteral("基本信息"), this);
    auto* baseForm = new QFormLayout(baseBox);
    m_edDisplay = new QLineEdit(baseBox);
    baseForm->addRow(QStringLiteral("显示名称："), m_edDisplay);

    // ---- 匹配规则 ----
    auto* matchBox = new QGroupBox(QStringLiteral("匹配规则（任一命中）"), this);
    auto* matchForm = new QFormLayout(matchBox);
    m_edExePath = new QLineEdit(matchBox);
    m_edExePath->setPlaceholderText(QStringLiteral("如 C:\\Games\\*\\Cyberpunk2077.exe（通配符）"));
    m_edExeName = new QLineEdit(matchBox);
    m_edExeName->setPlaceholderText(QStringLiteral("如 Cyberpunk2077.exe（通配符）"));
    m_edTitleRegex = new QLineEdit(matchBox);
    m_edTitleRegex->setPlaceholderText(QStringLiteral("如 .*Cyberpunk.*（ECMAScript 正则）"));
    matchForm->addRow(QStringLiteral("exe 路径："), m_edExePath);
    matchForm->addRow(QStringLiteral("exe 名称："), m_edExeName);
    matchForm->addRow(QStringLiteral("窗口标题："), m_edTitleRegex);

    // ---- 启用的适配器 ----
    auto* adBox = new QGroupBox(QStringLiteral("启用适配器（不勾选 = 用全局默认）"), this);
    auto* adLay = new QVBoxLayout(adBox);
    m_lstAdapters = new QListWidget(adBox);
    m_lstAdapters->setSelectionMode(QAbstractItemView::NoSelection);
    for (const auto& a : AdapterRegistry::instance().all()) {
        auto* it = new QListWidgetItem(QStringLiteral("%1 — %2").arg(a->id(), a->displayName()),
                                       m_lstAdapters);
        it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
        it->setCheckState(Qt::Unchecked);
        it->setData(Qt::UserRole, a->id());
    }
    adLay->addWidget(m_lstAdapters);

    // ---- 采样参数 ----
    auto* spBox = new QGroupBox(QStringLiteral("采样"), this);
    auto* spForm = new QFormLayout(spBox);
    m_spInterval  = new QSpinBox(spBox); m_spInterval->setRange(100, 5000); m_spInterval->setSuffix(" ms");
    m_spHistory   = new QSpinBox(spBox); m_spHistory->setRange(60, 5000);
    m_spUiRefresh = new QSpinBox(spBox); m_spUiRefresh->setRange(50, 2000); m_spUiRefresh->setSuffix(" ms");
    spForm->addRow(QStringLiteral("采样间隔："), m_spInterval);
    spForm->addRow(QStringLiteral("历史点数："), m_spHistory);
    spForm->addRow(QStringLiteral("UI 刷新："),  m_spUiRefresh);

    // ---- HUD 字段 ----
    auto* hudBox = new QGroupBox(QStringLiteral("HUD 字段"), this);
    auto* hudLay = new QVBoxLayout(hudBox);
    m_lstFields = new QListWidget(hudBox);
    m_lstFields->setDragDropMode(QAbstractItemView::InternalMove);
    auto* fieldBtns = new QHBoxLayout;
    m_btnAddField = new QPushButton(QStringLiteral("新增字段"), hudBox);
    m_btnRemoveField = new QPushButton(QStringLiteral("删除"), hudBox);
    m_btnMoveUp = new QPushButton(QStringLiteral("上移"), hudBox);
    m_btnMoveDown = new QPushButton(QStringLiteral("下移"), hudBox);
    for (auto* b : { m_btnAddField, m_btnRemoveField, m_btnMoveUp, m_btnMoveDown }) {
        fieldBtns->addWidget(b);
    }
    fieldBtns->addStretch(1);
    hudLay->addWidget(m_lstFields);
    hudLay->addLayout(fieldBtns);

    connect(m_btnAddField,    &QPushButton::clicked, this, &GameEditDialog::onAddField);
    connect(m_btnRemoveField, &QPushButton::clicked, this, &GameEditDialog::onRemoveField);
    connect(m_btnMoveUp,      &QPushButton::clicked, this, &GameEditDialog::onMoveUp);
    connect(m_btnMoveDown,    &QPushButton::clicked, this, &GameEditDialog::onMoveDown);

    // ---- OK / Cancel ----
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);

    root->addWidget(baseBox);
    root->addWidget(matchBox);
    root->addWidget(spBox);
    root->addWidget(adBox, 1);
    root->addWidget(hudBox, 1);
    root->addWidget(bb);
}

void GameEditDialog::setProfile(const GameProfile& p) {
    m_p = p;
    loadIntoUi();
    rebuildFieldList();
}

void GameEditDialog::presetFromForeground(const QString& path, const QString& name, const QString& title) {
    m_p = GameConfigStore::createNew(name);
    m_p.match.exePath          = path;
    m_p.match.exeName          = name;
    m_p.match.windowTitleRegex = title;
    m_p.displayName            = name;
    loadIntoUi();
    rebuildFieldList();
}

void GameEditDialog::loadIntoUi() {
    m_edDisplay->setText(m_p.displayName);
    m_edExePath->setText(m_p.match.exePath);
    m_edExeName->setText(m_p.match.exeName);
    m_edTitleRegex->setText(m_p.match.windowTitleRegex);
    m_spInterval->setValue(m_p.intervalMs);
    m_spHistory->setValue(m_p.historyPoints);
    m_spUiRefresh->setValue(m_p.uiRefreshMs);

    for (int i = 0; i < m_lstAdapters->count(); ++i) {
        auto* it = m_lstAdapters->item(i);
        const QString id = it->data(Qt::UserRole).toString();
        it->setCheckState(m_p.enabledAdapters.contains(id) ? Qt::Checked : Qt::Unchecked);
    }
}

void GameEditDialog::saveFromUi() {
    m_p.displayName = m_edDisplay->text();
    m_p.match.exePath = m_edExePath->text();
    m_p.match.exeName = m_edExeName->text();
    m_p.match.windowTitleRegex = m_edTitleRegex->text();
    m_p.intervalMs     = m_spInterval->value();
    m_p.historyPoints  = m_spHistory->value();
    m_p.uiRefreshMs    = m_spUiRefresh->value();

    m_p.enabledAdapters.clear();
    for (int i = 0; i < m_lstAdapters->count(); ++i) {
        auto* it = m_lstAdapters->item(i);
        if (it->checkState() == Qt::Checked)
            m_p.enabledAdapters << it->data(Qt::UserRole).toString();
    }
}

void GameEditDialog::rebuildFieldList() {
    m_lstFields->clear();
    for (int i = 0; i < m_p.hud.fields.size(); ++i) {
        const auto& f = m_p.hud.fields[i];
        m_lstFields->addItem(QStringLiteral("%1 — %2%3 (%4 px, %5)")
            .arg(f.id, f.label, f.format).arg(f.fontSize).arg(f.color));
    }
}

void GameEditDialog::onAddField() {
    HudFieldConfig f;
    f.id = "new"; f.source = "fps"; f.label = "label";
    f.format = "%.0f"; f.color = "#FFFFFF"; f.fontSize = 14; f.enabled = true;
    m_p.hud.fields.append(f);
    rebuildFieldList();
}

void GameEditDialog::onRemoveField() {
    int i = m_lstFields->currentRow();
    if (i < 0 || i >= m_p.hud.fields.size()) return;
    m_p.hud.fields.removeAt(i);
    rebuildFieldList();
}

void GameEditDialog::onMoveUp() {
    int i = m_lstFields->currentRow();
    if (i <= 0) return;
    m_p.hud.fields.move(i, i - 1);
    rebuildFieldList();
    m_lstFields->setCurrentRow(i - 1);
}

void GameEditDialog::onMoveDown() {
    int i = m_lstFields->currentRow();
    if (i < 0 || i >= m_p.hud.fields.size() - 1) return;
    m_p.hud.fields.move(i, i + 1);
    rebuildFieldList();
    m_lstFields->setCurrentRow(i + 1);
}

void GameEditDialog::accept() {
    saveFromUi();
    QDialog::accept();
}

} // namespace gpm
