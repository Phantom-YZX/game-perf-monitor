#include "game/GameMatcher.h"

#include <QFileInfo>

namespace gpm {

QRegularExpression GameMatcher::wildcard(const QString& pat, bool& ok) {
    Q_UNUSED(ok);
    return QRegularExpression(
        QRegularExpression::wildcardToRegularExpression(pat, QRegularExpression::NonPathWildcardConversion),
        QRegularExpression::CaseInsensitiveOption);
}

GameMatcher::GameMatcher(const MatchRule& rule) : m_rule(rule) {
    if (!m_rule.exePath.isEmpty()) {
        m_pathRx = wildcard(m_rule.exePath, m_hasPath);
        m_hasPath = m_pathRx.isValid() && !m_rule.exePath.isEmpty();
    }
    if (!m_rule.exeName.isEmpty()) {
        m_nameRx = wildcard(m_rule.exeName, m_hasName);
        m_hasName = m_nameRx.isValid() && !m_rule.exeName.isEmpty();
    }
    if (!m_rule.windowTitleRegex.isEmpty()) {
        m_titleRx = QRegularExpression(m_rule.windowTitleRegex,
            QRegularExpression::CaseInsensitiveOption);
        m_hasTitle = m_titleRx.isValid();
    }
}

bool GameMatcher::matches(const QString& exePath, const QString& windowTitle) const {
    if (!isUsable()) return false;
    if (m_hasPath) {
        if (m_pathRx.match(exePath).hasMatch()) return true;
    }
    if (m_hasName) {
        QString name = QFileInfo(exePath).fileName();
        if (m_nameRx.match(name).hasMatch()) return true;
    }
    if (m_hasTitle) {
        if (m_titleRx.match(windowTitle).hasMatch()) return true;
    }
    return false;
}

bool GameMatcher::matchesByName(const QString& exeName, const QString& windowTitle) const {
    if (!isUsable()) return false;
    if (m_hasName) {
        if (m_nameRx.match(exeName).hasMatch()) return true;
    }
    if (m_hasTitle) {
        if (m_titleRx.match(windowTitle).hasMatch()) return true;
    }
    return false;
}

} // namespace gpm
