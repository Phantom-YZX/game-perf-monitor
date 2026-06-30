#include "sampler/mock/MockAdapterBase.h"

namespace gpm {

class MockRamAdapter : public MockAdapterBase {
public:
    MockRamAdapter() : MockAdapterBase("ram", "Memory (mock)",
        {"ramUsagePct","ramUsedMb","ramTotalMb","ramFreqMhz"}) {}

    bool tick(SampleBlock& b) override {
        m_used += uniform(-30, 80);                   // MB
        m_used = std::clamp(m_used, 4500.0, 13800.0); // 4.5 ~ 13.8 GB
        const double total = 16384.0;                  // 16 GB
        b.ramTotalMb = total;
        b.ramUsedMb  = m_used;
        b.ramUsagePct = (m_used / total) * 100.0;
        b.ramFreqMhz = 3200.0;
        return true;
    }

private:
    double m_used = 6800.0;
};

} // namespace gpm
