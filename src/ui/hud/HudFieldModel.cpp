#include "ui/hud/HudFieldModel.h"

#include <QColor>
#include <QFont>

namespace gpm {

HudFieldModel::HudFieldModel(QObject* parent) : QAbstractListModel(parent) {}

int HudFieldModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant HudFieldModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_items.size()) return {};
    const auto& f = m_items[index.row()];
    bool na = true;
    QString v = valueAsString(f, lastBlock, na);
    switch (role) {
        case LabelRole:    return f.label;
        case ValueRole:    return v;
        case ColorRole:    return f.color;
        case FontSizeRole: return f.fontSize;
        case EnabledRole:  return f.enabled;
        case NaRole:       return na;
        default:           return {};
    }
}

QHash<int, QByteArray> HudFieldModel::roleNames() const {
    return {
        { LabelRole,    "label" },
        { ValueRole,    "value" },
        { ColorRole,    "color" },
        { FontSizeRole, "fontSize" },
        { EnabledRole,  "enabled" },
        { NaRole,       "isNa" }
    };
}

void HudFieldModel::setItems(const QList<HudFieldConfig>& items) {
    beginResetModel();
    m_items = items;
    m_idIndex.clear();
    for (int i = 0; i < m_items.size(); ++i) {
        m_idIndex.insert(m_items[i].id, i);
    }
    endResetModel();
}

void HudFieldModel::updateValues(const SampleBlock& b) {
    lastBlock = b;
    if (m_items.isEmpty()) return;
    emit dataChanged(index(0), index(m_items.size() - 1),
                     { ValueRole, NaRole });
}

QString HudFieldModel::valueAsString(const HudFieldConfig& f, const SampleBlock& b, bool& na) const {
    na = true;
    // 按 source + id 联合查表
    auto tryFmt = [&](const std::optional<double>& v) -> QString {
        if (!v) return QStringLiteral("N/A");
        na = false;
        return QString::asprintf(f.format.toUtf8().constData(), *v);
    };
    const QString s = f.source;
    const QString id = f.id;
    if (id == "fps"         || s == "fps")         return tryFmt(b.fps);
    if (id == "frametime"   || s == "frameTime")   return tryFmt(b.frameTimeMs);
    if (id == "cpu"         || s == "cpu")         return tryFmt(b.cpuUsagePct);
    if (id == "gpu"         || s == "gpu")         return tryFmt(b.gpuUsagePct);
    if (id == "ram"         || s == "ram")         return tryFmt(b.ramUsagePct);
    if (id == "vram"        || s == "vram")        return tryFmt(b.vramUsagePct);
    if (id == "cpufreq"     || s == "cpu")         return tryFmt(b.cpuFreqMhz);
    if (id == "gpufreq"     || s == "gpu")         return tryFmt(b.gpuFreqMhz);
    if (id == "cputemp"     || s == "thermal")     return tryFmt(b.cpuTempC);
    if (id == "gputemp"     || s == "thermal")     return tryFmt(b.gpuTempC);
    if (id == "cpuvolt"     || s == "voltage")     return tryFmt(b.cpuVoltageV);
    if (id == "gpuvolt"     || s == "voltage")     return tryFmt(b.gpuVoltageV);
    if (id == "cpupower"    || s == "power")       return tryFmt(b.cpuPowerW);
    if (id == "gpupower"    || s == "power")       return tryFmt(b.gpuPowerW);
    if (id == "cpufan"      || s == "fan")         return tryFmt(b.cpuFanRpm);
    if (id == "gpufan"      || s == "fan")         return tryFmt(b.gpuFanRpm);
    if (id == "onelow"      || s == "frameTime")   return tryFmt(b.oneLowPct);
    if (id == "pointonelow" || s == "frameTime")   return tryFmt(b.pointOneLowPct);
    if (id == "netrx"       || s == "net")         return tryFmt(b.netRxKbps);
    if (id == "nettx"       || s == "net")         return tryFmt(b.netTxKbps);
    if (id == "netlatency"  || s == "net")         return tryFmt(b.netLatencyMs);
    if (id == "ramused"     || s == "ram")         return tryFmt(b.ramUsedMb);
    if (id == "vramused"    || s == "vram")        return tryFmt(b.vramUsedMb);
    return QStringLiteral("N/A");
}

} // namespace gpm
