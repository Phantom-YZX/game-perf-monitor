#pragma once

#include <QObject>
#include <QString>

namespace gpm {

/**
 * @brief Phase 1A：仅检测管理员 + 占位 UI 提示；不自提升（自提升是 1B 的事）。
 *
 * 第二阶段真实适配器（NVML/LHM/PresentMon）若需要管理员，应调用
 *   PrivilegeGuard::requireFor(reason, parentWidget)
 * 检查并提示用户；不强制自提升。
 */
class PrivilegeGuard {
public:
    /// 是否以管理员身份运行
    static bool isRunAsAdmin();

    /// 给出简短原因（"presentmon" / "lhm" / ...），返回用户可读的说明
    static QString describeReason(const QString& reason);

    /// 检查依赖：若需要管理员但当前不是管理员，弹出 QMessageBox 提示，返回 false
    /// reason 取值："presentmon" | "lhm" | "etw_gpu" | ...
    /// parent 可空（nullptr = 无父窗口）
    static bool requireFor(const QString& reason, void* parent = nullptr);
};

} // namespace gpm
