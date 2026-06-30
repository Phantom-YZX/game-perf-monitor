# GamePerfMonitor

Windows 游戏性能监测工具，基于 **C++/Qt 6（QWidget + Qt Charts）**。提供独立主窗口和游戏悬浮窗（HUD），实时显示 CPU/GPU/RAM/显存/温度/电压/功耗/FPS/帧时间/1% low/0.1% low/风扇/网络等指标。

## 数据缺失策略
**所有读不到的字段显示 N/A，不允许伪造**。Mock 数据仅在 Debug 构建 + 第一阶段（1A/1B）演示用，Release / 第二阶段及以后编译期忽略。

## 依赖
- **Qt 6.8.3**（QWidget + Qt Charts；不使用 QML/Qt Quick）
- **CMake ≥ 3.21**
- **Visual Studio 2022**（MSVC v143 工具集，Windows 10/11 SDK）
- **vcpkg**（manifest 模式自动拉取 nlohmann/json、spdlog、fmt、inja、SQLiteCpp）

## 构建

```powershell
# 1. 安装 vcpkg（如未安装）
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat

# 2. 配置 + 构建（Debug 启用 mock，Release 不启用）
cd "e:\YZX AI Work data\game-perf-monitor"
cmake -B build -S . `
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake `
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug -j

# 3. Release 构建（无 mock，体积更小、UI 更紧凑）
cmake -B build-release -S . `
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake `
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --config Release -j

# 4. 运行
.\build\Debug\GamePerfMonitor.exe
```

## 阶段规划
- **1A**（当前）：主窗口（实时/设置）+ HUD + Mock 数据 + 配置
- **1B**：SQLite 会话 + HTML 报告 + 游戏识别 + 托盘
- **2**：接入 NVML / LibreHardwareMonitorLib / PresentMon
- **3**：悬浮窗优化（HDR、多屏 DPI）+ 启动器 API
