#include "ui/main/MainWindow.h"

#include "sampler/DataBus.h"
#include "ui/hud/HudWindow.h"
#include "ui/main/RealtimePanel.h"
#include "ui/main/SettingsPage.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QStackedWidget>
#include <QStatusBar>
#include <QVBoxLayout>

namespace gpm {
namespace {

constexpr int DashboardPage = 0;
constexpr int OverlayPage = 1;
constexpr int ProfilesPage = 2;
constexpr int SettingsPageIndex = 3;
constexpr int ReportsPage = 4;

QLabel* textLabel(const QString& text, const QString& objectName = {}) {
    auto* label = new QLabel(text);
    if (!objectName.isEmpty()) label->setObjectName(objectName);
    label->setWordWrap(true);
    return label;
}

QFrame* card(const QString& title, const QString& body = {}) {
    auto* frame = new QFrame;
    frame->setObjectName("metricCard");
    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(8);
    layout->addWidget(textLabel(title, "cardTitle"));
    if (!body.isEmpty()) layout->addWidget(textLabel(body, "mutedText"));
    return frame;
}

HudConfig defaultHudConfig() {
    HudConfig cfg;
    cfg.enabled = true;
    cfg.locked = false;
    cfg.opacity = 0.86;
    cfg.direction = "vertical";
    cfg.x = 80;
    cfg.y = 80;
    cfg.baseFontSize = 13;
    cfg.backgroundColor = "#000000";
    cfg.backgroundAlpha = 0.52;
    cfg.backgroundRadius = 6;

    auto add = [&](const QString& id, const QString& source, const QString& label,
                   const QString& format, const QString& color, int size = 13) {
        HudFieldConfig field;
        field.id = id;
        field.source = source;
        field.label = label;
        field.format = format;
        field.color = color;
        field.fontSize = size;
        field.enabled = true;
        cfg.fields.append(field);
    };

    add("fps", "fps", "FPS", "%.0f", "#22d3ee", 18);
    add("onelow", "frameTime", "1% LOW", "%.0f", "#4ade80", 12);
    add("pointonelow", "frameTime", "0.1% LOW", "%.0f", "#fbbf24", 12);
    add("frametime", "frameTime", "FRAME", "%.1f ms", "#e4e4e7", 12);
    add("cpu", "cpu", "CPU", "%.0f%%", "#60a5fa", 12);
    add("cputemp", "thermal", "CPU C", "%.0f C", "#f87171", 12);
    add("cpupower", "power", "CPU W", "%.0f W", "#fbbf24", 12);
    add("gpu", "gpu", "GPU", "%.0f%%", "#4ade80", 12);
    add("gputemp", "thermal", "GPU C", "%.0f C", "#f87171", 12);
    add("gpupower", "power", "GPU W", "%.0f W", "#fbbf24", 12);
    add("ramused", "ram", "RAM", "%.0f MB", "#fbbf24", 12);
    add("vramused", "vram", "VRAM", "%.0f MB", "#22d3ee", 12);
    add("netlatency", "net", "PING", "%.0f ms", "#e4e4e7", 12);
    return cfg;
}

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("GamePerfMonitor 1A"));
    resize(1240, 760);
    loadQss();
    buildUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::setHudWindow(HudWindow* hud) {
    m_hud = hud;
    if (m_hud) m_hud->applyConfig(defaultHudConfig());
}

void MainWindow::loadQss() {
    QFile file(QStringLiteral(":/qss/rtss.qss"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
}

void MainWindow::buildUi() {
    auto* root = new QWidget(this);
    auto* outer = new QHBoxLayout(root);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    m_sidebar = buildSidebar();
    m_topBar = buildTopBar();
    m_stack = new QStackedWidget(root);

    m_realtime = new RealtimePanel(this);
    m_overlay = buildOverlayPage();
    m_profiles = buildProfilesPage();
    m_settings = new SettingsPage(this);
    m_reports = buildReportsPage();

    m_stack->addWidget(m_realtime);
    m_stack->addWidget(m_overlay);
    m_stack->addWidget(m_profiles);
    m_stack->addWidget(m_settings);
    m_stack->addWidget(m_reports);

    auto* right = new QVBoxLayout;
    right->setContentsMargins(0, 0, 0, 0);
    right->setSpacing(0);
    right->addWidget(m_topBar);
    right->addWidget(m_stack, 1);

    outer->addWidget(m_sidebar);
    outer->addLayout(right, 1);
    setCentralWidget(root);

    connect(m_settings, &SettingsPage::sigHudSettingsChanged, this, [this](const HudConfig& cfg) {
        if (m_hud) m_hud->applyConfig(cfg);
    });
    connect(m_settings, &SettingsPage::sigHudVisibilityChanged, this, &MainWindow::onHudToggle);
    connect(m_settings, &SettingsPage::sigHudPositionChanged, this, &MainWindow::onHudPositionChanged);

    statusBar()->showMessage(QStringLiteral("Phase 1A | Mock data | UI refresh 500ms"));
    switchPage(DashboardPage);
}

QWidget* MainWindow::buildSidebar() {
    auto* side = new QFrame;
    side->setObjectName("SidebarNav");
    side->setFixedWidth(180);
    auto* layout = new QVBoxLayout(side);
    layout->setContentsMargins(10, 14, 10, 12);
    layout->setSpacing(6);

    auto* brand = textLabel(QStringLiteral("GamePerfMonitor"), "brandTitle");
    auto* version = textLabel(QStringLiteral("1A Mock"), "brandVersion");
    layout->addWidget(brand);
    layout->addWidget(version);
    layout->addSpacing(12);

    const QList<QPair<QString, int>> nav = {
        { QStringLiteral("仪表盘"), DashboardPage },
        { QStringLiteral("游戏 HUD"), OverlayPage },
        { QStringLiteral("游戏配置"), ProfilesPage },
        { QStringLiteral("设置"), SettingsPageIndex },
        { QStringLiteral("会话报告"), ReportsPage },
    };
    for (const auto& item : nav) {
        auto* button = new QPushButton(item.first, side);
        button->setObjectName("navItem");
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        m_navButtons.append(button);
        connect(button, &QPushButton::clicked, this, [this, index = item.second] {
            switchPage(index);
        });
        layout->addWidget(button);
    }

    layout->addStretch(1);
    layout->addWidget(textLabel(QStringLiteral("Sampler 1s"), "sideStatus"));
    layout->addWidget(textLabel(QStringLiteral("UI 500ms"), "sideStatusGood"));
    return side;
}

QWidget* MainWindow::buildTopBar() {
    auto* top = new QFrame;
    top->setObjectName("TopBar");
    top->setFixedHeight(42);
    auto* layout = new QHBoxLayout(top);
    layout->setContentsMargins(16, 0, 14, 0);
    layout->setSpacing(12);

    m_title = textLabel(QStringLiteral("仪表盘"), "topTitle");
    m_gameInfo = textLabel(QStringLiteral("当前：Mock Session / 未绑定游戏"), "topGame");
    auto* record = new QPushButton(QStringLiteral("录制占位"), top);
    record->setObjectName("topAction");
    record->setEnabled(false);

    layout->addWidget(m_title);
    layout->addWidget(m_gameInfo);
    layout->addStretch(1);
    layout->addWidget(record);
    return top;
}

QWidget* MainWindow::buildOverlayPage() {
    auto* page = new QWidget;
    auto* layout = new QHBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(14);

    auto* preview = new QFrame(page);
    preview->setObjectName("hudPreview");
    auto* previewLayout = new QVBoxLayout(preview);
    previewLayout->setContentsMargins(18, 18, 18, 18);
    previewLayout->setSpacing(10);
    previewLayout->addWidget(textLabel(QStringLiteral("HUD 预览"), "sectionTitle"));
    previewLayout->addStretch(1);
    auto* hudCard = card(QStringLiteral("FPS 142 | 1% 118 | 0.1% 94 | 7.0 ms"),
                         QStringLiteral("GPU 91% 74 C 112 W | CPU 54% N/A 68 W\nRAM 18.2 GB | VRAM 5.8 GB | PING 24 ms"));
    hudCard->setObjectName("hudMini");
    previewLayout->addWidget(hudCard, 0, Qt::AlignLeft | Qt::AlignTop);
    previewLayout->addStretch(4);

    auto* config = new QFrame(page);
    config->setObjectName("configPanel");
    config->setFixedWidth(310);
    auto* cfg = new QVBoxLayout(config);
    cfg->setContentsMargins(14, 14, 14, 14);
    cfg->setSpacing(12);
    cfg->addWidget(textLabel(QStringLiteral("HUD 设置"), "sectionTitle"));

    auto* enable = new QCheckBox(QStringLiteral("启用 HUD"), config);
    auto* lock = new QCheckBox(QStringLiteral("锁定位置"), config);
    auto* direction = new QComboBox(config);
    direction->addItems({ QStringLiteral("vertical"), QStringLiteral("horizontal") });
    auto* opacity = new QSlider(Qt::Horizontal, config);
    opacity->setRange(20, 100);
    opacity->setValue(86);
    auto* fontSize = new QSlider(Qt::Horizontal, config);
    fontSize->setRange(8, 32);
    fontSize->setValue(13);

    cfg->addWidget(enable);
    cfg->addWidget(lock);
    cfg->addWidget(textLabel(QStringLiteral("布局"), "mutedText"));
    cfg->addWidget(direction);
    cfg->addWidget(textLabel(QStringLiteral("透明度"), "mutedText"));
    cfg->addWidget(opacity);
    cfg->addWidget(textLabel(QStringLiteral("字号"), "mutedText"));
    cfg->addWidget(fontSize);
    cfg->addWidget(textLabel(QStringLiteral("缺失字段示例：CPU TEMP N/A / CPU PWR N/A"), "naText"));
    cfg->addStretch(1);

    auto apply = [this, enable, lock, direction, opacity, fontSize] {
        HudConfig cfg = defaultHudConfig();
        cfg.enabled = enable->isChecked();
        cfg.locked = lock->isChecked();
        cfg.direction = direction->currentText();
        cfg.opacity = opacity->value() / 100.0;
        cfg.baseFontSize = fontSize->value();
        for (auto& field : cfg.fields) {
            field.fontSize = qMax(10, fontSize->value());
        }
        if (m_hud) {
            m_hud->applyConfig(cfg);
            cfg.enabled ? m_hud->show() : m_hud->hide();
        }
    };
    connect(enable, &QCheckBox::toggled, this, apply);
    connect(lock, &QCheckBox::toggled, this, apply);
    connect(direction, &QComboBox::currentTextChanged, this, apply);
    connect(opacity, &QSlider::valueChanged, this, apply);
    connect(fontSize, &QSlider::valueChanged, this, apply);

    layout->addWidget(preview, 1);
    layout->addWidget(config);
    return page;
}

QWidget* MainWindow::buildProfilesPage() {
    auto* page = new QWidget;
    auto* layout = new QHBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(14);

    auto* list = new QListWidget(page);
    list->setFixedWidth(230);
    list->addItems({ QStringLiteral("Mock Game"), QStringLiteral("notepad.exe 示例") });

    auto* editor = card(QStringLiteral("游戏配置"),
                        QStringLiteral("1A 阶段仅保存配置，不启动真实游戏识别。\n字段：displayName / exePath / exeName / windowTitleRegex / HUD / 采样间隔 / 告警阈值"));
    layout->addWidget(list);
    layout->addWidget(editor, 1);
    return page;
}

QWidget* MainWindow::buildReportsPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->addWidget(card(QStringLiteral("会话报告"),
                           QStringLiteral("1B 启用：SQLite 会话录制、HTML 报告、平均 FPS、最低 FPS、1% low、0.1% low、温度峰值和功耗峰值。")));
    layout->addStretch(1);
    return page;
}

void MainWindow::switchPage(int index) {
    if (!m_stack || index < 0 || index >= m_stack->count()) return;
    m_stack->setCurrentIndex(index);
    const QStringList titles = {
        QStringLiteral("仪表盘"),
        QStringLiteral("游戏 HUD"),
        QStringLiteral("游戏配置"),
        QStringLiteral("设置"),
        QStringLiteral("会话报告"),
    };
    if (m_title && index < titles.size()) m_title->setText(titles[index]);
    for (int i = 0; i < m_navButtons.size(); ++i) {
        m_navButtons[i]->setChecked(i == index);
    }
}

void MainWindow::onUiTick() {
    const SampleBlock b = DataBus::instance().latest();
    if (m_realtime) m_realtime->updateValues(b);
}

void MainWindow::onHudToggle(bool visible) {
    if (!m_hud) return;
    if (visible) m_hud->show();
    else m_hud->hide();
}

void MainWindow::onHudPositionChanged() {
    if (m_hud) m_hud->persistPosition();
}

} // namespace gpm
