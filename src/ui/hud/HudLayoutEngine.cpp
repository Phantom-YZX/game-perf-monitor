#include "ui/hud/HudLayoutEngine.h"
#include "ui/hud/HudFieldModel.h"

#include <QFontMetrics>
#include <QFont>

namespace gpm {

QSize HudLayoutEngine::arrange(const HudFieldModel* model, Direction dir, int padding, int spacing) {
    if (!model || model->rowCount() == 0) return QSize(160, 60);
    int rows = model->rowCount();

    int maxW = 0;
    int sumH = 0;
    for (int i = 0; i < rows; ++i) {
        QModelIndex idx = model->index(i, 0);
        int fs = idx.data(HudFieldModel::FontSizeRole).toInt();
        QFont valFont("Segoe UI", fs, QFont::Bold);
        QFontMetrics vm(valFont);
        QString label  = idx.data(HudFieldModel::LabelRole).toString();
        QString value  = idx.data(HudFieldModel::ValueRole).toString();
        int w = std::max(vm.horizontalAdvance(label), vm.horizontalAdvance(value));
        int h = (dir == Vertical) ? (vm.height() * 2 + 8) : (vm.height() + 6);
        maxW = std::max(maxW, w);
        sumH += h;
    }
    sumH += spacing * (rows - 1);

    int totalW = maxW + padding * 2 + 4;  // 边框
    int totalH = sumH + padding * 2;
    if (dir == Horizontal) {
        // 横向：每行一个字段，行宽累加
        int w = (maxW + spacing) * rows + padding * 2;
        int h = maxW > 0 ? (maxW * 0 + 40) : 40;
        return QSize(w, h);
    }
    return QSize(totalW, totalH);
}

} // namespace gpm
