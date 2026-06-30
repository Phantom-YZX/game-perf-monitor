#include "sampler/mock/MockAdapterBase.h"

namespace gpm {

// 0-100% 随机游走 + 5% 概率尖峰
class MockCpuAdapter : public MockAdapterBase {
public:
    MockCpuAdapter() : MockAdapterBase("cpu", "CPU (mock)",
        {"cpuUsagePct","cpuFreqMhz","cpuTempC","cpuPowerW"}) {}

    bool tick(SampleBlock& b) override {
        double drift = uniform(-3.0, 3.0);
        m_cpu += drift;
        if (uniform(0,1) < 0.05) m_cpu += uniform(15, 30);   // 5% 尖峰
        m_cpu = std::clamp(m_cpu, 5.0, 99.0);
        m_cpu *= 0.95;                                        // 回归

        b.cpuUsagePct = m_cpu;
        b.cpuFreqMhz  = 3500.0 + (m_cpu - 50.0) * 8.0;       // 频率随负载
        b.cpuTempC    = 40.0 + m_cpu * 0.45;                 // 滞后温度
        b.cpuPowerW   = 25.0 + m_cpu * 2.8;
        return true;
    }

private:
    double m_cpu = 30.0;
};

} // namespace gpm
