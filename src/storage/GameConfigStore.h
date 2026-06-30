#pragma once

#include "storage/GameProfile.h"

#include <QString>
#include <QList>

namespace gpm {

/**
 * @brief 单游戏配置的 CRUD。
 *
 *   - 文件：AppConfigLocation/games/<gameId>.json
 *   - 索引：扫描目录列出所有 gameId
 *   - 写：原子写
 */
class GameConfigStore {
public:
    /// 读取（不存在 → 默认空配置）
    static GameProfile load(const QString& gameId);

    /// 保存（原子写；自动更新 updatedAtMs）
    static bool save(GameProfile& p);

    /// 列出所有 gameId（按文件名字母序）
    static QStringList listAll();

    /// 删除配置
    static bool remove(const QString& gameId);

    /// 创建一条新配置（生成 gameId / 时间戳；HUD 字段给一组默认）
    static GameProfile createNew(const QString& displayName);
};

} // namespace gpm
