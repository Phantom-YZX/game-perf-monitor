#include "sampler/mock/MockAdapterBase.h"

namespace gpm {

class MockNetAdapter : public MockAdapterBase {
public:
    MockNetAdapter() : MockAdapterBase("net", "Network (mock)",
        {"netRxKbps","netTxKbps","netLatencyMs"}) {}

    bool tick(SampleBlock& b) override {
        // 基础速率 200 KB/s，偶发 burst 到 2 MB/s
        double burst = (uniform(0,1) < 0.06) ? uniform(800, 2000) : 0;
        b.netRxKbps = 80.0 + uniform(0, 200) + burst;
        b.netTxKbps = 30.0 + uniform(0, 80)  + burst * 0.3;
        b.netLatencyMs = 18.0 + uniform(0, 15) + (uniform(0,1) < 0.02 ? uniform(20, 80) : 0);
        return true;
    }
};

} // namespace gpm
