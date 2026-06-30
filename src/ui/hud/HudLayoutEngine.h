#pragma once

#include <QSize>
#include <QString>

namespace gpm {

class HudFieldModel;

/**
 * @brief HUD 布局计算：根据 direction（vertical/horizontal）重算 sizeHint。
 */
class HudLayoutEngine {
public:
    enum Direction { Vertical, Horizontal };

    /// 给定模型 + 方向，返回建议的窗口大小（包含内边距）
    static QSize arrange(const HudFieldModel* model, Direction dir,
                         int padding = 10, int spacing = 4);

    static Direction fromString(const QString& s) {
        return s.compare("horizontal", Qt::CaseInsensitive) == 0 ? Horizontal : Vertical;
    }
};

} // namespace gpm
