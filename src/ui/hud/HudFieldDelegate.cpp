#include "ui/hud/HudFieldDelegate.h"
#include "ui/hud/HudFieldModel.h"

#include <QPainter>
#include <QApplication>
#include <QFontMetrics>

namespace gpm {

HudFieldDelegate::HudFieldDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void HudFieldDelegate::paint(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx) const {
    p->save();
    p->setRenderHint(QPainter::TextAntialiasing, true);

    const QString label  = idx.data(HudFieldModel::LabelRole).toString();
    const QString value  = idx.data(HudFieldModel::ValueRole).toString();
    const QColor  color  = QColor(idx.data(HudFieldModel::ColorRole).toString());
    const int     fs     = idx.data(HudFieldModel::FontSizeRole).toInt();
    const bool    na     = idx.data(HudFieldModel::NaRole).toBool();

    QFont labFont("Segoe UI", std::max(8, fs - 2));
    QFont valFont("Segoe UI", fs, QFont::Bold);

    QFontMetrics lm(labFont);
    QFontMetrics vm(valFont);

    QRect r = opt.rect;
    if (m_horizontal) {
        // 横排：左 label，右 value
        QRect lr(r.left() + 6, r.top(), r.width()/2, r.height());
        QRect vr(r.left() + r.width()/2, r.top(), r.width()/2 - 6, r.height());
        p->setPen(QColor("#A0A0A0"));
        p->setFont(labFont);
        p->drawText(lr, Qt::AlignVCenter | Qt::AlignLeft, label);

        p->setPen(na ? QColor("#888888") : color);
        p->setFont(valFont);
        p->drawText(vr, Qt::AlignVCenter | Qt::AlignRight, value);
    } else {
        // 竖排：label 在上（灰色小字），value 在下（彩色大字）
        QRect lr(r.left(), r.top() + 2, r.width(), r.height()/2);
        QRect vr(r.left(), r.top() + r.height()/2, r.width(), r.height()/2 - 2);
        p->setPen(QColor("#A0A0A0"));
        p->setFont(labFont);
        p->drawText(lr, Qt::AlignHCenter | Qt::AlignTop, label);

        p->setPen(na ? QColor("#888888") : color);
        p->setFont(valFont);
        p->drawText(vr, Qt::AlignHCenter | Qt::AlignBottom, value);
    }

    p->restore();
}

QSize HudFieldDelegate::sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& idx) const {
    const int fs = idx.data(HudFieldModel::FontSizeRole).toInt();
    QFont valFont("Segoe UI", fs, QFont::Bold);
    QFontMetrics vm(valFont);
    int h = vm.height() * 2 + 8;  // label + value + padding
    int w = 80;                    // 默认宽度
    if (m_horizontal) {
        // 横向时高度小
        h = std::max(28, vm.height() + 6);
        w = 160;
    }
    return QSize(w, h);
}

} // namespace gpm
