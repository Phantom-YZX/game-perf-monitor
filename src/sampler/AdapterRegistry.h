#pragma once

#include "sampler/IDataAdapter.h"

#include <QObject>
#include <QStringList>

#include <memory>
#include <vector>

namespace gpm {

/**
 * @brief 适配器注册表：根据 GameProfile::enabledAdapters 决定启用哪些适配器。
 *
 * 第二阶段：所有 Mock* 不再注册；第一阶段 1A/1B：仅 Mock* 注册。
 * 这层抽象让 SamplerService 不用关心真实/演示。
 */
class AdapterRegistry {
public:
    static AdapterRegistry& instance();

    /// 注册一个适配器（通常 main.cpp 里在创建 SamplerService 之前调用一次）
    void registerAdapter(IDataAdapterPtr a);

    /// 清空
    void clear();

    /// 返回所有已注册的适配器
    const std::vector<IDataAdapterPtr>& all() const { return m_all; }

    /// 按 GameProfile::enabledAdapters 过滤；若 enabledAdapters 为空 = 全部
    std::vector<IDataAdapterPtr> active(const QStringList& enabledAdapters) const;

    /// 按 id 查找
    IDataAdapterPtr find(const QString& id) const;

    /// 字段 ID → 可产出该字段的适配器 ID 列表（用于 Settings UI 校验）
    QStringList adapterIdsForField(const QString& fieldId) const;

private:
    AdapterRegistry() = default;
    std::vector<IDataAdapterPtr> m_all;
};

} // namespace gpm
