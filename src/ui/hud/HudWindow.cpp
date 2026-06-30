#include "ui/hud/HudWindow.h"
#include "ui/hud/HudFieldModel.h"
#include "ui/hud/HudFieldDelegate.h"
#include "ui/hud/HudLayoutEngine.h"
#include "sampler/DataBus.h"
#include "util/Logger.h"

#include <QApplication>
#include <QShortcut>
#include <QKeySequence>
#include <QListView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>

namespace gpm {

HudWindow::HudWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle(QStringLiteral("GamePerfMonitor HUD"));
    setWindowFlags(Qt::FramelessWindowHint
                 | Qt::WindowStaysOnTopHint
                 | Qt::Tool);  // 不在任务栏
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    m_model    = new HudFieldModel(this);
    m_delegate = new HudFieldDelegate(this);
    m_view     = new QListView(this);
    m_view->setModel(m_model);
    m_view->setItemDelegate(m_delegate);
    m_view->setUniformItemSizes(true);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setSelectionMode(QAbstractItemView::NoSelection);
    m_view->setFocusPolicy(Qt::NoFocus);
    m_view->setMouseTracking(false);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_view);

    // 快捷键
    m_scToggleLock = new QShortcut(QKeySequence("Ctrl+L"), this);
    connect(m_scToggleLock, &QShortcut::activated, this, [this]{
        m_cfg.locked = !m_cfg.locked;
        Logger::get()->info("HUD lock = {}", m_cfg.locked);
    });
    m_scToggleDir = new QShortcut(QKeySequence("F9"), this);
    connect(m_scToggleDir, &QShortcut::activated, this, [this]{
        m_cfg.direction = (m_cfg.direction == "vertical") ? "horizontal" : "vertical";
        rebuildFromConfig();
    });
    m_scQuit = new QShortcut(QKeySequence("Esc"), this);
    connect(m_scQuit, &QShortcut::activated, this, [this]{
        hide();
    });
}

HudWindow::~HudWindow() = default;

void HudWindow::applyConfig(const HudConfig& h) {
    m_cfg = h;
    rebuildFromConfig();
    // 位置和大小
    if (m_cfg.x >= 0 && m_cfg.y >= 0) move(m_cfg.x, m_cfg.y);
    setWindowOpacity(m_cfg.opacity);
    if (m_cfg.enabled) {
        // 第一次显示后保持位置；不强制 show，由 SettingsPage 控制
    } else {
        hide();
    }
}

void HudWindow::rebuildFromConfig() {
    m_model->setItems(m_cfg.fields);
    m_delegate->setHorizontal(m_cfg.direction == "horizontal");
    if (m_view) {
        m_view->setFlow(m_cfg.direction == "horizontal" ? QListView::LeftToRight : QListView::TopToBottom);
        m_view->setWrapping(false);
    }
    recomputeGeometry();
}

void HudWindow::recomputeGeometry() {
    QSize sz = HudLayoutEngine::arrange(m_model,
        HudLayoutEngine::fromString(m_cfg.direction), 10, 4);
    if (m_cfg.direction == "vertical") {
        sz.setWidth(std::max(sz.width(), 140));
    } else {
        sz.setHeight(std::max(sz.height(), 36));
    }
    m_view->setMinimumSize(sz);
    m_view->setMaximumSize(sz);
    resize(sz);
    m_view->updateGeometry();
}

void HudWindow::persistPosition() {
    m_cfg.x = x();
    m_cfg.y = y();
}

void HudWindow::onUiTick() {
    const SampleBlock b = DataBus::instance().latest();
    m_model->updateValues(b);
    m_view->viewport()->update();
}

void HudWindow::mousePressEvent(QMouseEvent* e) {
    if (m_cfg.locked || e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }
    m_dragging = true;
    m_dragOffset = e->globalPosition().toPoint() - frameGeometry().topLeft();
    e->accept();
}

void HudWindow::mouseMoveEvent(QMouseEvent* e) {
    if (!m_dragging || m_cfg.locked) {
        e->ignore();
        return;
    }
    move(e->globalPosition().toPoint() - m_dragOffset);
    e->accept();
}

void HudWindow::mouseReleaseEvent(QMouseEvent* e) {
    if (m_dragging) {
        m_dragging = false;
        m_cfg.x = x();
        m_cfg.y = y();
        e->accept();
    } else {
        e->ignore();
    }
}

void HudWindow::mouseDoubleClickEvent(QMouseEvent* e) {
    if (m_cfg.locked) { e->ignore(); return; }
    // 双击切换锁定
    m_cfg.locked = true;
    e->accept();
}

void HudWindow::wheelEvent(QWheelEvent* e) {
    // 滚轮调字号（Ctrl+滚轮调透明度）
    if (e->modifiers() & Qt::ControlModifier) {
        double delta = e->angleDelta().y() / 1200.0;  // ±0.1
        m_cfg.opacity = std::clamp(m_cfg.opacity + delta, 0.2, 1.0);
        setWindowOpacity(m_cfg.opacity);
    } else {
        int delta = e->angleDelta().y() / 120;  // ±1
        for (auto& f : m_cfg.fields) {
            f.fontSize = std::clamp(f.fontSize + delta, 8, 64);
        }
        m_model->setItems(m_cfg.fields);
        recomputeGeometry();
    }
    e->accept();
}

void HudWindow::keyPressEvent(QKeyEvent* e) {
    QWidget::keyPressEvent(e);
}

void HudWindow::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    QColor bg(QColor(m_cfg.backgroundColor));
    bg.setAlphaF(m_cfg.backgroundAlpha);
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(0, 0, -1, -1),
                         m_cfg.backgroundRadius, m_cfg.backgroundRadius);
    p.fillPath(path, bg);
    p.setPen(QColor(255, 255, 255, 20));
    p.drawPath(path);
}

} // namespace gpm
