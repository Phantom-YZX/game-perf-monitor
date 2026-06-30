#pragma once

#include "storage/GameProfile.h"
#include "util/SampleBlock.h"

#include <QWidget>

class QListView;
class QVBoxLayout;
class QShortcut;

namespace gpm {

class HudFieldModel;
class HudFieldDelegate;

/**
 * @brief HUD 悬浮窗：透明、置顶、可拖、可锁定；支持横/竖布局。
 *
 *   - 透明：Qt::WA_TranslucentBackground
 *   - 置顶：Qt::WindowStaysOnTopHint
 *   - 鼠标穿透：默认 false（拖动时临时开）
 *   - 锁定：m_locked 时忽略 mousePressEvent
 *   - 字号：滚轮 + / -（加 Ctrl 调透明度）
 *   - 横/竖：F9
 *   - 位置：拖动 mouseMove 写入 HudConfig.x/y
 */
class HudWindow : public QWidget {
    Q_OBJECT
public:
    explicit HudWindow(QWidget* parent = nullptr);
    ~HudWindow() override;

    void applyConfig(const HudConfig& h);
    void persistPosition();

public slots:
    void onUiTick();

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void rebuildFromConfig();
    void recomputeGeometry();

    HudFieldModel*    m_model     = nullptr;
    HudFieldDelegate* m_delegate  = nullptr;
    QListView*        m_view      = nullptr;

    HudConfig  m_cfg;
    bool       m_dragging = false;
    QPoint     m_dragOffset;

    QShortcut* m_scToggleLock = nullptr;
    QShortcut* m_scToggleDir  = nullptr;
    QShortcut* m_scQuit       = nullptr;
};

} // namespace gpm
