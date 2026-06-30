#pragma once

#include "storage/GameProfile.h"
#include "util/SampleBlock.h"

#include <QAbstractListModel>
#include <QHash>
#include <QList>

namespace gpm {

/**
 * @brief HUD 字段列表模型。
 *
 * 角色：LabelRole / ValueRole / ColorRole / FontSizeRole
 *
 * 数据源：HudFieldConfig 列表（来自 GameProfile::hud.fields）
 * 数值：来自最新的 SampleBlock（按 source/id 查表）
 */
class HudFieldModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        LabelRole = Qt::UserRole + 1,
        ValueRole,
        ColorRole,
        FontSizeRole,
        EnabledRole,
        NaRole   // 当前显示的是 N/A
    };

    explicit HudFieldModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const QList<HudFieldConfig>& items);
    const QList<HudFieldConfig>& items() const { return m_items; }

    /// 用最新 SampleBlock 刷新数值（无效字段置 N/A）
    void updateValues(const SampleBlock& b);

private:
    QString valueAsString(const HudFieldConfig& f, const SampleBlock& b, bool& na) const;

    QList<HudFieldConfig> m_items;
    QHash<QString, int>   m_idIndex;  // 字段 id → row
    SampleBlock           lastBlock;  // 最近一次 updateValues 推入的快照
};

} // namespace gpm
