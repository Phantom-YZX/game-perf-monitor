#include "sampler/SamplerService.h"
#include "sampler/AdapterRegistry.h"
#include "sampler/DataBus.h"
#include "util/Logger.h"

#include <QDateTime>

namespace gpm {

SamplerService::SamplerService(QObject* parent) : QObject(parent) {
    m_sampleTimer = new QTimer(this);
    m_sampleTimer->setTimerType(Qt::PreciseTimer);
    connect(m_sampleTimer, &QTimer::timeout, this, &SamplerService::onSamplingTick);

    m_uiTimer = new QTimer(this);
    m_uiTimer->setTimerType(Qt::PreciseTimer);
    connect(m_uiTimer, &QTimer::timeout, this, &SamplerService::onUiTick);
}

SamplerService::~SamplerService() {
    stop();
}

void SamplerService::start(const QStringList& enabledAdapters, int intervalMs, int uiRefreshMs) {
    if (m_running) return;
    buildActive(enabledAdapters);
    if (m_active.empty()) {
        Logger::get()->warn("SamplerService: no active adapter (enabledAdapters={})",
                            enabledAdapters.join(",").toStdString());
    } else {
        Logger::get()->info("SamplerService: started with {} active adapter(s), interval={}ms, uiTick={}ms",
                            m_active.size(), intervalMs, uiRefreshMs);
    }

    m_sampleTimer->start(qMax(50, intervalMs));
    m_uiTimer->start(qMax(50, uiRefreshMs));
    m_seq = 0;
    m_running = true;
    emit started();
}

void SamplerService::stop() {
    if (!m_running) return;
    m_sampleTimer->stop();
    m_uiTimer->stop();
    m_active.clear();
    m_running = false;
    Logger::get()->info("SamplerService: stopped after {} samples", m_seq);
    emit stopped();
}

void SamplerService::buildActive(const QStringList& enabledAdapters) {
    m_active = AdapterRegistry::instance().active(enabledAdapters);
}

void SamplerService::onSamplingTick() {
    SampleBlock b;
    b.seq  = ++m_seq;
    b.tsMs = QDateTime::currentMSecsSinceEpoch();

    bool anyOk = false;
    for (const auto& a : m_active) {
        try {
            if (a->tick(b)) anyOk = true;
        } catch (const std::exception& e) {
            Logger::get()->error("Adapter {} threw: {}", a->id().toStdString(), e.what());
        } catch (...) {
            Logger::get()->error("Adapter {} threw unknown exception", a->id().toStdString());
        }
    }
    Q_UNUSED(anyOk);

    DataBus::instance().push(b);
}

void SamplerService::onUiTick() {
    emit DataBus::instance().sigUiTick();
}

} // namespace gpm
