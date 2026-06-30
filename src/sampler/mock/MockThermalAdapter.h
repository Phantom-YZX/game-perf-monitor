#include "sampler/mock/MockAdapterBase.h"

namespace gpm {

class MockThermalAdapter : public MockAdapterBase {
public:
    MockThermalAdapter() : MockAdapterBase("thermal", "Thermal (mock)",
        {"cpuTempC","gpuTempC"}) {}

    bool tick(SampleBlock& b) override {
        if (b.cpuUsagePct) {
            m_cpuT += ((*b.cpuUsagePct * 0.45 + 40.0) - m_cpuT) * 0.3;
        }
        if (b.gpuUsagePct) {
            m_gpuT += ((*b.gpuUsagePct * 0.50 + 42.0) - m_gpuT) * 0.3;
        }
        b.cpuTempC = m_cpuT;
        b.gpuTempC = m_gpuT;
        return true;
    }

private:
    double m_cpuT = 50.0, m_gpuT = 52.0;
};

} // namespace gpm
