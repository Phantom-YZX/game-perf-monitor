#include "storage/ConfigStore.h"
#include "storage/JsonStore.h"
#include "util/PathProvider.h"

namespace gpm {

AppSettings ConfigStore::load() {
    auto opt = JsonStore::read(PathProvider::globalConfigPath());
    if (!opt) return AppSettings{};
    return AppSettings::fromJson(*opt);
}

bool ConfigStore::save(const AppSettings& s) {
    return JsonStore::writeAtomic(PathProvider::globalConfigPath(), s.toJson());
}

} // namespace gpm
