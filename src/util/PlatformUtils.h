#pragma once

#include <QString>
#include <QRect>

namespace gpm {

/**
 * @brief Win32 杂项封装：前台窗口 / 进程名 / DPI 感知 / 进程枚举。
 *
 * 所有方法都使用普通权限可用的 API（ToolHelp32 / GetForegroundWindow）。
 * 对受保护进程 / 系统进程的操作在 Phase 2+ 才需要 SeDebugPrivilege。
 */
class PlatformUtils {
public:
    /// 启用 Per-Monitor V2 DPI 感知（程序启动时调用一次）
    static void enableDpiAwareness();

    /// 当前前台窗口的进程 ID（0 = 无）
    static quint32 foregroundProcessId();

    /// 当前前台窗口的 exe 完整路径（空 = 失败）
    static QString foregroundProcessPath();

    /// 当前前台窗口的 exe 名（不含目录）
    static QString foregroundProcessName();

    /// 当前前台窗口的标题
    static QString foregroundWindowTitle();

    /// 给定 PID 的进程完整路径（空 = 失败或权限不足）
    static QString processPathById(quint32 pid);

    /// 给定 PID 的进程名（不含目录）
    static QString processNameById(quint32 pid);

    /// 进程是否在运行
    static bool isProcessRunning(quint32 pid);

    /// 通过进程名查找 PID（同名多个只返回第一个，Phase 1A 演示够用）
    static quint32 findProcessByName(const QString& name);

    /// 当前是否以管理员身份运行
    static bool isRunAsAdmin();
};

} // namespace gpm
