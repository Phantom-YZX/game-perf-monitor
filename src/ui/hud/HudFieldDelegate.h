#pragma once

#include <QStyledItemDelegate>

namespace gpm {

/**
 * @brief HUD 字段渲染：行/列布局由 HudLayoutEngine 决定，Delegate 只画一项。
 *
 * 每项 = "label value" 两段文本，标签偏灰，值按字段色着色。
 */
class HudFieldDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit HudFieldDelegate(QObject* parent = nullptr);

    void paint(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx) const override;
    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& idx) const override;

    void setHorizontal(bool h) { m_horizontal = h; }
    bool isHorizontal() const { return m_horizontal; }

private:
    bool m_horizontal = false;
};

} // namespace gpm
