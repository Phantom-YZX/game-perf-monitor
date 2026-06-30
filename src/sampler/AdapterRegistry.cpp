#include "sampler/AdapterRegistry.h"

namespace gpm {

AdapterRegistry& AdapterRegistry::instance() {
    static AdapterRegistry r;
    return r;
}

void AdapterRegistry::registerAdapter(IDataAdapterPtr a) {
    if (!a) return;
    m_all.push_back(std::move(a));
}

void AdapterRegistry::clear() {
    m_all.clear();
}

std::vector<IDataAdapterPtr> AdapterRegistry::active(const QStringList& enabledAdapters) const {
    std::vector<IDataAdapterPtr> out;
    if (enabledAdapters.isEmpty()) {
        for (const auto& a : m_all) if (a->available()) out.push_back(a);
        return out;
    }
    for (const auto& a : m_all) {
        if (!a->available()) continue;
        if (enabledAdapters.contains(a->id(), Qt::CaseInsensitive)) {
            out.push_back(a);
        }
    }
    return out;
}

IDataAdapterPtr AdapterRegistry::find(const QString& id) const {
    for (const auto& a : m_all) {
        if (a->id().compare(id, Qt::CaseInsensitive) == 0) return a;
    }
    return nullptr;
}

QStringList AdapterRegistry::adapterIdsForField(const QString& fieldId) const {
    Q_UNUSED(fieldId);
    // 简化：每个适配器负责自己的字段，目前不提供反向索引。
    // 实际配置 UI 用 GameConfig 静态映射即可。
    return {};
}

} // namespace gpm
