#include "app/AppMain.h"
#include "util/Logger.h"
#include "util/PathProvider.h"
#include "util/PlatformUtils.h"

#include <QApplication>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    // High-DPI 必须在 QApplication 构造前
    QCoreApplication::setOrganizationName("MiniMax");
    QCoreApplication::setOrganizationDomain("minimaxi.com");
    QCoreApplication::setApplicationName("GamePerfMonitor");
    QCoreApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);  // 常驻托盘

    // QSS 主题：编译进二进制 (/qss/rtss.qss)
    QFile qssFile(":/qss/rtss.qss");
    if (qssFile.open(QIODevice::ReadOnly)) {
        app.setStyleSheet(QString::fromUtf8(qssFile.readAll()));
        qssFile.close();
    }

    gpm::AppMain m;
    int rc = m.run(argc, argv);

    int appRc = app.exec();
    m.shutdown();
    return appRc == 0 ? rc : appRc;
}
