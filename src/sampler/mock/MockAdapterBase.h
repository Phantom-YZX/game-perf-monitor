#pragma once

#include "sampler/IDataAdapter.h"

#include <QString>
#include <QStringList>

#include <random>

namespace gpm {

/**
 * @brief Mock 适配器基类：提供随机数引擎、统一 id/displayName 模板。
 *
 * 真正产出数据由子类 tick() 实现。
 */
class MockAdapterBase : public IDataAdapter {
public:
    explicit MockAdapterBase(QString id, QString display, QStringList produced)
        : m_id(std::move(id)), m_display(std::move(display)), m_produced(std::move(produced)) {}

    QString id() const override { return m_id; }
    QString displayName() const override { return m_display; }
    QStringList producedFields() const override { return m_produced; }
    bool available() const override { return true; }
    bool needsAdmin() const override { return false; }

    /// 子类用：返回 [lo, hi] 均匀分布
    double uniform(double lo, double hi) {
        std::uniform_real_distribution<double> d(lo, hi);
        return d(m_rng);
    }
    int uniformInt(int lo, int hi) {
        std::uniform_int_distribution<int> d(lo, hi);
        return d(m_rng);
    }

private:
    QString m_id;
    QString m_display;
    QStringList m_produced;
    std::mt19937_64 m_rng { std::random_device{}() };
};

} // namespace gpm
