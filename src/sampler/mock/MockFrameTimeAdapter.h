#include "sampler/mock/MockAdapterBase.h"

namespace gpm {

class MockFrameTimeAdapter : public MockAdapterBase {
public:
    MockFrameTimeAdapter() : MockAdapterBase("frameTime", "Frame Time (mock)",
        {"frameTimeMs","oneLowPct","pointOneLowPct"}) {}

    bool tick(SampleBlock& b) override {
        if (!b.fps) { b.frameTimeMs = 16.6; return true; }
        double ft = 1000.0 / *b.fps;
        b.frameTimeMs = ft;

        // 1% low / 0.1% low 用简化模型：取 fps 的最小滑窗近似
        b.oneLowPct       = std::max(0.0, *b.fps - uniform(2.0, 6.0));
        b.pointOneLowPct  = std::max(0.0, *b.fps - uniform(5.0, 12.0));
        return true;
    }
};

} // namespace gpm
