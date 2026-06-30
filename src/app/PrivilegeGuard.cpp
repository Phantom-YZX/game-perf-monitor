#include "app/PrivilegeGuard.h"
#include "util/PlatformUtils.h"
#include "util/Logger.h"

#include <QMessageBox>
#include <QWidget>

namespace gpm {

bool PrivilegeGuard::isRunAsAdmin() {
    return PlatformUtils::isRunAsAdmin();
}

QString PrivilegeGuard::describeReason(const QString& reason) {
    if (reason == "presentmon") return QStringLiteral("FPS / 帧时间（PresentMon ETW）");
    if (reason == "lhm")        return QStringLiteral("CPU 温度 / 电压 / 风扇（LibreHardwareMonitor）");
    if (reason == "etw_gpu")    return QStringLiteral("GPU 计数器（ETW）");
    return reason;
}

bool PrivilegeGuard::requireFor(const QString& reason, void* parent) {
    if (isRunAsAdmin()) return true;
    Logger::get()->warn("PrivilegeGuard: need admin for '{}', not elevated", reason.toStdString());

    auto* p = reinterpret_cast<QWidget*>(parent);
    QMessageBox mb(p);
    mb.setIcon(QMessageBox::Warning);
    mb.setWindowTitle(QObject::tr("需要管理员权限"));
    mb.setText(QObject::tr("当前功能需要管理员权限：\n\n  %1\n\n"
                           "请以管理员身份重新启动本程序，"
                           "或将当前用户加入 'Performance Log Users' 组。\n\n"
                           "注意：拒绝后该字段将显示 N/A，"
                           "不会用其他数据冒充。").arg(describeReason(reason)));
    mb.setStandardButtons(QMessageBox::Ok);
    mb.exec();
    return false;
}

} // namespace gpm
