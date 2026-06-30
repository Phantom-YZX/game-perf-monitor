#include "sampler/mock/MockAdapterBase.h"

namespace gpm {

class MockGpuAdapter : public MockAdapterBase {
public:
    MockGpuAdapter() : MockAdapterBase("gpu", "GPU (mock)",
        {"gpuUsagePct","gpuFreqMhz","gpuTempC","gpuPowerW","vramUsagePct","vramUsedMb","vramTotalMb"}) {}

    bool tick(SampleBlock& b) override {
        if (!b.fps) {            // 没 FPS 就走中速负载
            m_load += uniform(-5, 5);
        } else {
            // GPU 负载大致跟 FPS 相关（cap 144）
            double target = std::min(99.0, (*b.fps / 144.0) * 95.0);
            m_load += (target - m_load) * 0.4 + uniform(-4, 4);
        }
        m_load = std::clamp(m_load, 3.0, 99.5);
        b.gpuUsagePct = m_load;
        b.gpuFreqMhz  = 1500.0 + m_load * 12.0;
        b.gpuTempC    = 42.0 + m_load * 0.50;
        b.gpuPowerW   = 35.0 + m_load * 3.0;

        const double totalMb = 12288.0;
        b.vramTotalMb = totalMb;
        b.vramUsedMb  = totalMb * (m_load / 100.0);
        b.vramUsagePct = m_load;
        return true;
    }

private:
    double m_load = 40.0;
};

} // namespace gpm
