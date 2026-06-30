#include "storage/GameProfile.h"

#include <QJsonArray>
#include <QUuid>
#include <QDateTime>

namespace gpm {

QJsonObject GameProfile::toJson() const {
    QJsonObject o;
    o["version"]      = 1;
    o["gameId"]       = gameId;
    o["displayName"]  = displayName;
    QJsonObject m;
    m["exePath"]          = match.exePath;
    m["exeName"]          = match.exeName;
    m["windowTitleRegex"] = match.windowTitleRegex;
    o["match"]            = m;
    o["enabledAdapters"]  = QJsonArray::fromStringList(enabledAdapters);
    o["intervalMs"]       = intervalMs;
    o["historyPoints"]    = historyPoints;
    o["uiRefreshMs"]      = uiRefreshMs;
    o["createdAtMs"]      = createdAtMs;
    o["updatedAtMs"]      = updatedAtMs;

    QJsonObject h;
    h["enabled"]  = hud.enabled;
    h["locked"]   = hud.locked;
    h["opacity"]  = hud.opacity;
    h["direction"]= hud.direction;
    QJsonObject pos; pos["x"] = hud.x; pos["y"] = hud.y; h["position"] = pos;
    h["baseFontSize"] = hud.baseFontSize;
    QJsonObject bg;
    bg["color"] = hud.backgroundColor;
    bg["alpha"] = hud.backgroundAlpha;
    bg["radius"]= hud.backgroundRadius;
    h["background"] = bg;
    QJsonArray fs;
    for (const auto& f : hud.fields) {
        QJsonObject fj;
        fj["id"]       = f.id;
        fj["source"]   = f.source;
        fj["label"]    = f.label;
        fj["format"]   = f.format;
        fj["color"]    = f.color;
        fj["fontSize"] = f.fontSize;
        fj["enabled"]  = f.enabled;
        fs.append(fj);
    }
    h["fields"] = fs;
    o["hud"] = h;
    return o;
}

GameProfile GameProfile::fromJson(const QJsonObject& o) {
    GameProfile p;
    p.gameId        = o.value("gameId").toString(QUuid::createUuid().toString(QUuid::WithoutBraces));
    p.displayName   = o.value("displayName").toString();
    auto m          = o.value("match").toObject();
    p.match.exePath          = m.value("exePath").toString();
    p.match.exeName          = m.value("exeName").toString();
    p.match.windowTitleRegex = m.value("windowTitleRegex").toString();
    for (auto v : o.value("enabledAdapters").toArray())
        p.enabledAdapters << v.toString();
    p.intervalMs     = o.value("intervalMs").toInt(1000);
    p.historyPoints  = o.value("historyPoints").toInt(600);
    p.uiRefreshMs    = o.value("uiRefreshMs").toInt(500);
    p.createdAtMs    = o.value("createdAtMs").toVariant().toLongLong();
    p.updatedAtMs    = o.value("updatedAtMs").toVariant().toLongLong();

    auto h = o.value("hud").toObject();
    p.hud.enabled  = h.value("enabled").toBool(true);
    p.hud.locked   = h.value("locked").toBool(false);
    p.hud.opacity  = h.value("opacity").toDouble(0.85);
    p.hud.direction= h.value("direction").toString("vertical");
    auto pos       = h.value("position").toObject();
    p.hud.x        = pos.value("x").toInt(80);
    p.hud.y        = pos.value("y").toInt(80);
    p.hud.baseFontSize = h.value("baseFontSize").toInt(14);
    auto bg        = h.value("background").toObject();
    p.hud.backgroundColor = bg.value("color").toString("#000000");
    p.hud.backgroundAlpha = bg.value("alpha").toDouble(0.35);
    p.hud.backgroundRadius= bg.value("radius").toInt(6);
    for (auto v : h.value("fields").toArray()) {
        auto fj = v.toObject();
        HudFieldConfig f;
        f.id       = fj.value("id").toString();
        f.source   = fj.value("source").toString();
        f.label    = fj.value("label").toString();
        f.format   = fj.value("format").toString("%.0f");
        f.color    = fj.value("color").toString("#FFFFFF");
        f.fontSize = fj.value("fontSize").toInt(14);
        f.enabled  = fj.value("enabled").toBool(true);
        p.hud.fields.append(f);
    }
    return p;
}

QJsonObject AppSettings::toJson() const {
    QJsonObject o;
    o["version"] = version;
    QJsonObject g;
    g["startWithWindows"] = startWithWindows;
    g["minimizeToTray"]   = minimizeToTray;
    g["language"]         = language;
    o["general"] = g;
    QJsonObject s;
    s["defaultIntervalMs"]    = defaultIntervalMs;
    s["defaultHistoryPoints"] = defaultHistoryPoints;
    s["defaultUiRefreshMs"]   = defaultUiRefreshMs;
    o["sampling"] = s;
    QJsonObject st;
    st["retentionDays"] = retentionDays;
    st["autoVacuum"]    = autoVacuum;
    o["storage"] = st;
    QJsonObject r;
    r["template"] = reportTemplate;
    o["report"] = r;
    QJsonObject e;
    e["autoElevateFor"] = QJsonArray::fromStringList(autoElevateFor);
    e["lastReason"]     = "";
    o["elevation"] = e;
    QJsonObject d;
#ifndef NDEBUG
    d["mockMode"] = devMockMode;
#else
    d["mockMode"] = false;
#endif
    o["dev"] = d;
    return o;
}

AppSettings AppSettings::fromJson(const QJsonObject& o) {
    AppSettings s;
    s.version = o.value("version").toInt(1);
    auto g = o.value("general").toObject();
    s.startWithWindows = g.value("startWithWindows").toBool(false);
    s.minimizeToTray   = g.value("minimizeToTray").toBool(true);
    s.language         = g.value("language").toString("zh-CN");
    auto sj = o.value("sampling").toObject();
    s.defaultIntervalMs    = sj.value("defaultIntervalMs").toInt(1000);
    s.defaultHistoryPoints = sj.value("defaultHistoryPoints").toInt(600);
    s.defaultUiRefreshMs   = sj.value("defaultUiRefreshMs").toInt(500);
    auto st = o.value("storage").toObject();
    s.retentionDays = st.value("retentionDays").toInt(90);
    s.autoVacuum    = st.value("autoVacuum").toBool(true);
    auto r = o.value("report").toObject();
    s.reportTemplate = r.value("template").toString("default");
    auto e = o.value("elevation").toObject();
    for (auto v : e.value("autoElevateFor").toArray())
        s.autoElevateFor << v.toString();
#ifndef NDEBUG
    auto d = o.value("dev").toObject();
    s.devMockMode = d.value("mockMode").toBool(false);
#endif
    return s;
}

} // namespace gpm
