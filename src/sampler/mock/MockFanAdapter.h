#include "sampler/mock/MockAdapterBase.h"

namespace gpm {

class MockFanAdapter : public MockAdapterBase {
public:
    MockFanAdapter() : MockAdapterBase("fan", "Fan (mock)",
        {"cpuFanRpm","cpuFanPct","gpuFanRpm","gpuFanPct"}) {}

    bool tick(SampleBlock& b) override {
        // 温度-转速查表：40℃ → 800 RPM，80℃ → 2400 RPM
        auto rpmFromTemp = [](double t) {
            if (t < 40.0) return 600.0;
            if (t > 80.0) return 2400.0;
            return 600.0 + (t - 40.0) * 40.0;
        };
        double cpuT = b.cpuTempC.value_or(50.0);
        double gpuT = b.gpuTempC.value_or(52.0);
        b.cpuFanRpm = rpmFromTemp(cpuT) + uniform(-30, 30);
        b.gpuFanRpm = rpmFromTemp(gpuT) + uniform(-30, 30);
        b.cpuFanPct = (b.cpuFanRpm.value() / 3000.0) * 100.0;
        b.gpuFanPct = (b.gpuFanRpm.value() / 3000.0) * 100.0;
        return true;
    }
};

} // namespace gpm
