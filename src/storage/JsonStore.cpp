#include "storage/JsonStore.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSaveFile>
#include <QJsonDocument>

namespace gpm {

std::optional<QJsonObject> JsonStore::read(const QString& path) {
    QFile f(path);
    if (!f.exists()) return std::nullopt;
    if (!f.open(QIODevice::ReadOnly)) return std::nullopt;
    QByteArray bytes = f.readAll();
    f.close();
    QJsonParseError err{};
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return std::nullopt;
    return doc.object();
}

bool JsonStore::ensureParent(const QString& path) {
    QFileInfo fi(path);
    if (fi.exists()) return true;
    QDir().mkpath(fi.absolutePath());
    return QDir(fi.absolutePath()).exists();
}

bool JsonStore::writeAtomic(const QString& path, const QJsonObject& obj) {
    if (!ensureParent(path)) return false;
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    QJsonDocument doc(obj);
    f.write(doc.toJson(QJsonDocument::Indented));
    if (!f.commit()) return false;
    return true;
}

} // namespace gpm
