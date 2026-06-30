#include "sampler/mock/MockAdapterBase.h"

namespace gpm {

class MockVoltageAdapter : public MockAdapterBase {
public:
    MockVoltageAdapter() : MockAdapterBase("voltage", "Voltage (mock)",
        {"cpuVoltageV","gpuVoltageV"}) {}

    bool tick(SampleBlock& b) override {
        b.cpuVoltageV = 1.250 + uniform(-0.04, 0.04);
        b.gpuVoltageV = 1.050 + uniform(-0.03, 0.03);
        return true;
    }
};

} // namespace gpm
