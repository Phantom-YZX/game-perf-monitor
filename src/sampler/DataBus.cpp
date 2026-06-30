#include "sampler/DataBus.h"
#include "util/Logger.h"

#include <QTimer>

namespace gpm {

DataBus::DataBus(QObject* parent) : QObject(parent) {
    qRegisterMetaType<SampleBlock>("gpm::SampleBlock");
    qRegisterMetaType<SampleBlock>("SampleBlock");
}

DataBus& DataBus::instance() {
    static DataBus bus;
    return bus;
}

void DataBus::setBufferSize(int n) {
    QMutexLocker lk(&m_mtx);
    m_bufSize = qMax(10, n);
    while (static_cast<int>(m_buf.size()) > m_bufSize) m_buf.pop_front();
}

void DataBus::push(const SampleBlock& b) {
    QMutexLocker lk(&m_mtx);
    m_buf.push_back(b);
    while (static_cast<int>(m_buf.size()) > m_bufSize) m_buf.pop_front();
    // 注意：emit 在锁外做（信号可能跨线程，会被 Qt 异步排队）
    const SampleBlock copy = b;
    lk.unlock();
    emit sigBlock(copy);
}

SampleBlock DataBus::latest() const {
    QMutexLocker lk(&m_mtx);
    if (m_buf.empty()) return {};
    return m_buf.back();
}

std::deque<SampleBlock> DataBus::snapshot(int n) const {
    QMutexLocker lk(&m_mtx);
    if (n <= 0 || n >= static_cast<int>(m_buf.size())) return m_buf;
    return std::deque<SampleBlock>(m_buf.end() - n, m_buf.end());
}

void DataBus::clear() {
    QMutexLocker lk(&m_mtx);
    m_buf.clear();
}

} // namespace gpm
