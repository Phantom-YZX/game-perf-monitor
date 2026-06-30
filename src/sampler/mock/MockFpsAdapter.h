#include "sampler/mock/MockAdapterBase.h"

namespace gpm {

class MockFpsAdapter : public MockAdapterBase {
public:
    MockFpsAdapter() : MockAdapterBase("fps", "FPS (mock)", {"fps","presentMode"}) {}

    bool tick(SampleBlock& b) override {
        const double target = 60.0;
        double v = m_fps + (target - m_fps) * 0.3 + uniform(-2.5, 2.5);
        if (uniform(0,1) < 0.04) v -= uniform(8, 20);          // 偶发卡顿
        m_fps = std::clamp(v, 12.0, 75.0);
        b.fps = m_fps;
        b.presentMode = QStringLiteral("窗口化");
        return true;
    }

private:
    double m_fps = 58.0;
};

} // namespace gpm
