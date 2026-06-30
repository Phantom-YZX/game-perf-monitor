#pragma once

#include "util/SampleBlock.h"

#include <QString>
#include <QObject>
#include <QStringList>

#include <memory>

namespace gpm {

/**
 * @brief 数据源适配器抽象接口。
 *
 * 适配器职责：
 *   1. tick() 在工作线程上执行（不能直接操作 QWidget）
 *   2. 读不到的数据保留 std::nullopt，**绝不**降级到 mock / 估算
 *   3. 适配器只产出自己负责的字段；其它字段保持 nullopt
 *
 * 适配器实现原则：
 *   - 第一阶段 1A/1B：实现 Mock* 子类，标记 available() = true
 *   - 第二阶段：实现 Nvml* / Lhm* / PresentMon* / Pdh* 子类
 *   - 编译期由 CMake 的 GAME_PERF_ENABLE_MOCK 控制是否注册 mock
 */
class IDataAdapter {
public:
    virtual ~IDataAdapter() = default;

    /// 唯一 ID（"cpu" / "gpu" / "fps" / "thermal" ...），用于配置里启用/禁用
    virtual QString id() const = 0;

    /// 用户可读名称
    virtual QString displayName() const = 0;

    /// 当前是否可用：第二阶段真实适配器若驱动/库不可用应返回 false
    virtual bool available() const = 0;

    /// 是否需要管理员权限（仅用于状态栏提示，不影响 tick）
    virtual bool needsAdmin() const { return false; }

    /// 该适配器能产出的字段 ID 列表（用于配置"启用哪些适配器"的合法性校验）
    virtual QStringList producedFields() const = 0;

    /// 单次采样：读取实时数据，填到 block 的对应字段
    /// 返回 true 表示成功（即便所有字段为 nullopt 也算成功）
    virtual bool tick(SampleBlock& block) = 0;
};

using IDataAdapterPtr = std::shared_ptr<IDataAdapter>;

} // namespace gpm
