#pragma once

#include <QString>
#include <QJsonObject>

#include <optional>

namespace gpm {

/**
 * @brief 通用 JSON 原子写工具：写临时文件 + 改名，崩溃/断电安全。
 */
class JsonStore {
public:
    /// 读取整个 JSON 文件到 QJsonObject；文件不存在或解析失败返回 std::nullopt
    static std::optional<QJsonObject> read(const QString& path);

    /// 原子写：写 <path>.tmp 后 rename 为 <path>
    /// 返回 true 表示成功
    static bool writeAtomic(const QString& path, const QJsonObject& obj);

    /// 确保父目录存在
    static bool ensureParent(const QString& path);
};

} // namespace gpm
