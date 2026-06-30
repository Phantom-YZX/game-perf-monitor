#pragma once

#include "storage/GameProfile.h"
#include "util/SampleBlock.h"

namespace gpm {

/**
 * @brief 全局设置读写。
 *
 * 写到 AppConfigLocation/global.json，启动时 load，缺字段用默认值。
 * 缺文件时返回默认 AppSettings，不抛错。
 */
class ConfigStore {
public:
    /// 读取（文件不存在或损坏 → 默认值）
    static AppSettings load();

    /// 保存（原子写）
    static bool save(const AppSettings& s);
};

} // namespace gpm
