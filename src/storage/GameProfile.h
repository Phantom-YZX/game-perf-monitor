#pragma once

#include "util/SampleBlock.h"

#include <QString>
#include <QStringList>
#include <QJsonObject>

namespace gpm {

/**
 * @brief 单游戏的全部配置（数据类，Qt signal 友好）。
 *
 * match 规则：exePath / exeName / windowTitleRegex 任一命中即视为匹配；
 * 任意字段为空表示该条件不参与。
 */
struct MatchRule {
    QString exePath;           ///< 通配符（* ?），不区分大小写
    QString exeName;           ///< 进程名通配符
    QString windowTitleRegex;  ///< ECMAScript 正则
    bool isEmpty() const { return exePath.isEmpty() && exeName.isEmpty() && windowTitleRegex.isEmpty(); }
};

struct HudFieldConfig {
    QString id;            // "fps" / "cpu" / "gpu" / ...
    QString source;        // 数据源 adapter id（与 IDataAdapter::id 对应）
    QString label;         // 显示标签
    QString format;        // QString::arg 格式串，如 "%.0f"
    QString color;         // "#RRGGBB"
    int     fontSize = 14; // 像素
    bool    enabled = true;
};

struct HudConfig {
    bool    enabled  = true;
    bool    locked   = false;
    double  opacity  = 0.85;          // 0.0 - 1.0
    QString direction = "vertical";    // "vertical" / "horizontal"
    int     x = 80, y = 80;
    int     baseFontSize = 14;
    QString backgroundColor  = "#000000";
    double  backgroundAlpha  = 0.35;
    int     backgroundRadius = 6;
    QList<HudFieldConfig> fields;
};

struct GameProfile {
    QString    gameId;     // UUID
    QString    displayName;
    MatchRule  match;
    QStringList enabledAdapters;   // 空 = 全部
    int        intervalMs      = 1000;
    int        historyPoints   = 600;
    int        uiRefreshMs     = 500;
    HudConfig  hud;
    qint64     createdAtMs = 0;
    qint64     updatedAtMs = 0;

    QJsonObject toJson() const;
    static GameProfile fromJson(const QJsonObject& o);
};

/// 全局设置数据类
struct AppSettings {
    int     version = 1;
    bool    startWithWindows = false;
    bool    minimizeToTray   = true;
    QString language         = "zh-CN";
    int     defaultIntervalMs   = 1000;
    int     defaultHistoryPoints= 600;
    int     defaultUiRefreshMs  = 500;
    int     retentionDays       = 90;
    bool    autoVacuum          = true;
    QString reportTemplate      = "default";
    QStringList autoElevateFor  = { "presentmon", "lhm" };

    // dev.mockMode：仅 Debug 构建生效；Release / Phase 2 起该字段被忽略
#ifndef NDEBUG
    bool    devMockMode         = false;
#endif

    QJsonObject toJson() const;
    static AppSettings fromJson(const QJsonObject& o);
};

} // namespace gpm
