#include "sampler/mock/MockAdapterBase.h"

namespace gpm {

class MockPowerAdapter : public MockAdapterBase {
public:
    MockPowerAdapter() : MockAdapterBase("power", "Power (mock)",
        {"cpuPowerW","gpuPowerW"}) {}

    bool tick(SampleBlock& b) override {
        if (b.cpuUsagePct) b.cpuPowerW = 25.0 + *b.cpuUsagePct * 2.8;
        if (b.gpuUsagePct) b.gpuPowerW = 35.0 + *b.gpuUsagePct * 3.0;
        return true;
    }
};

} // namespace gpm
