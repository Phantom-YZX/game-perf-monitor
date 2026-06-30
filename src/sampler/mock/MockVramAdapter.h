#include "sampler/mock/MockAdapterBase.h"

namespace gpm {

// VRAM 真实数据来自 NVML；本适配器只提供非 NV 显卡环境下的占位
// （实际上 VRAM 数据 GPU 适配器已经填了；这里 available()=false 不参与）
class MockVramAdapter : public MockAdapterBase {
public:
    MockVramAdapter() : MockAdapterBase("vram", "VRAM (mock)",
        {"vramUsagePct","vramUsedMb","vramTotalMb"}) {}

    bool tick(SampleBlock& /*b*/) override { return true; }
};

} // namespace gpm
