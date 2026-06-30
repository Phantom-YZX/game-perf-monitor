#pragma once

#include "storage/GameProfile.h"

#include <QString>
#include <QRegularExpression>

namespace gpm {

/**
 * @brief 多条件"任一匹配"的规则引擎。
 *
 * 规则：
 *   - match.exePath 用 QRegularExpression::wildcardToRegularExpression 转换
 *     （不区分大小写）匹配完整路径
 *   - match.exeName 同样 wildcard 匹配 basename
 *   - match.windowTitleRegex 直接当 ECMAScript 正则
 *   - 任一非空规则命中即视为匹配
 *   - 若所有规则都为空 → 永不匹配
 */
class GameMatcher {
public:
    explicit GameMatcher(const MatchRule& rule);

    /// 是否匹配（完整路径 + 进程名 + 窗口标题）
    bool matches(const QString& exePath, const QString& windowTitle) const;

    /// 仅靠进程名匹配（不依赖完整路径）
    bool matchesByName(const QString& exeName, const QString& windowTitle) const;

    bool isUsable() const { return !m_rule.isEmpty(); }

private:
    MatchRule m_rule;
    QRegularExpression m_pathRx;
    QRegularExpression m_nameRx;
    QRegularExpression m_titleRx;
    bool m_hasPath = false;
    bool m_hasName = false;
    bool m_hasTitle = false;

    static QRegularExpression wildcard(const QString& pat, bool& ok);
};

} // namespace gpm
