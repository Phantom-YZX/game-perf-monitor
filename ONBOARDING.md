# 1A 阶段上手指南

> 本指南配套计划文件 `e:\YZX AI Work data\.qoder\plans\game-perf-monitor.md`。
> 完成本指南后，应用可以跑起来：主窗口 + HUD + 11 个 Mock 适配器 + 配置持久化。

---

## 1. 当前已生成代码的目录结构

```
game-perf-monitor/                              # 项目根
├── CMakeLists.txt                              # 顶层构建脚本（Qt 6.8 + vcpkg + mock 条件编译）
├── vcpkg.json                                  # 依赖清单（baseline + overrides 锁版本）
├── README.md                                   # 简要说明
├── ONBOARDING.md                               # ← 本文件
├── .gitignore
│
├── resources/
│   ├── qss/
│   │   ├── rtss.qss                           # 主窗口 RTSS/Afterburner 深色主题
│   │   └── hud.qss                            # HUD 透明专用
│   ├── icons/
│   │   └── README.md                           # 图标生成说明（缺图不影响构建/运行）
│   └── templates/                              # 1B 阶段存放 inja HTML 模板（当前空）
│
├── src/
│   ├── app/                                    # 应用编排
│   │   ├── main.cpp                            # 入口：QApplication + QSS + AppMain
│   │   ├── AppMain.{h,cpp}                     # 单实例、注册 mock、启动 SamplerService
│   │   ├── SettingsCenter.{h,cpp}              # 全局设置内存镜像 + 通知
│   │   └── PrivilegeGuard.{h,cpp}              # 管理员检测 + UI 提示（1B 才自提升）
│   │
│   ├── util/                                   # 基础工具
│   │   ├── SampleBlock.h                       # 一次采样的全部指标（std::optional<double> 表 N/A）
│   │   ├── Logger.{h,cpp}                      # spdlog 封装（stderr + 滚动文件）
│   │   ├── PathProvider.{h,cpp}                # AppConfig/AppData 路径标准化
│   │   └── PlatformUtils.{h,cpp}               # 前台进程/窗口、DPI、ToolHelp32 枚举
│   │
│   ├── sampler/                                # 采样与数据源
│   │   ├── IDataAdapter.h                      # 适配器抽象接口
│   │   ├── DataBus.{h,cpp}                     # Qt Signal/Slot 单例 + 环形缓冲
│   │   ├── AdapterRegistry.{h,cpp}             # 适配器注册表
│   │   ├── SamplerService.{h,cpp}              # QTimer 调度 + 500ms UI 节拍
│   │   └── mock/                               # 11 个 Mock 适配器（header-only）
│   │       ├── MockAdapterBase.h
│   │       ├── MockCpuAdapter.h
│   │       ├── MockGpuAdapter.h
│   │       ├── MockFpsAdapter.h
│   │       ├── MockFrameTimeAdapter.h
│   │       ├── MockRamAdapter.h
│   │       ├── MockVramAdapter.h
│   │       ├── MockThermalAdapter.h
│   │       ├── MockVoltageAdapter.h
│   │       ├── MockPowerAdapter.h
│   │       ├── MockFanAdapter.h
│   │       └── MockNetAdapter.h
│   │
│   ├── storage/                                # 配置与持久化
│   │   ├── JsonStore.{h,cpp}                   # 通用 JSON 原子写（QSaveFile）
│   │   ├── ConfigStore.{h,cpp}                 # 全局设置读写
│   │   ├── GameConfigStore.{h,cpp}             # 单游戏配置 CRUD
│   │   └── GameProfile.{h,cpp}                 # 数据类：MatchRule + HudFieldConfig + HudConfig + AppSettings
│   │
│   ├── game/                                   # 游戏识别
│   │   └── GameMatcher.{h,cpp}                 # exe 路径 / 进程名 / 窗口标题 三条件任一匹配
│   │
│   └── ui/
│       ├── main/                               # 主窗口
│       │   ├── MainWindow.{h,cpp}              # QTabWidget：实时 + 设置
│       │   ├── RealtimePanel.{h,cpp}           # 8 数字大值 + 3 折线（QChart）
│       │   ├── SettingsPage.{h,cpp}            # 通用 + 游戏库 + HUD + dev(仅 Debug)
│       │   └── GameEditDialog.{h,cpp}          # 新增/编辑单游戏配置的对话框
│       │
│       └── hud/                                # 悬浮窗
│           ├── HudWindow.{h,cpp}               # 透明/置顶/拖动/锁定/字号/横竖切换
│           ├── HudFieldModel.{h,cpp}           # QAbstractListModel 驱动字段
│           ├── HudFieldDelegate.{h,cpp}        # 单字段渲染（label + value）
│           └── HudLayoutEngine.{h,cpp}         # 横/竖布局 sizeHint 计算
│
├── third_party/                                # 第二阶段手动集成 SDK（当前空）
│   ├── nvml/                                   #   nvml.h + nvml.dll（动态加载）
│   ├── presentmon/                             #   PresentMonAPI.h + pmonsdk.dll
│   └── lhm/                                    #   LibreHardwareMonitorLib 子项目占位
│
└── tests/                                     # 单元测试（计划中 5 个，1A 暂不实现）
```

**1A 阶段代码量**：约 66 个文件、~3500 行 C++/QSS/Markdown。已完整覆盖：UI + Mock 数据 + 配置 + 权限检测 + HUD 全部功能。

---

## 2. Windows 环境安装步骤

### 2.1 Visual Studio 2022（MSVC v143）

1. 下载 [Visual Studio Community 2022](https://visualstudio.microsoft.com/zh-hans/downloads/)（免费）
2. 运行安装器，在"工作负荷"中勾选：
   - ✅ **使用 C++ 的桌面开发**
3. 在右侧"安装详细信息"勾选：
   - ✅ MSVC v143 - VS 2022 C++ x64/x86 生成工具
   - ✅ Windows 11 SDK (10.0.22621.0) 或最新
   - ✅ C++ CMake 工具 for Windows
4. 安装路径默认 `C:\Program Files\Microsoft Visual Studio\2022\Community` 即可
5. 安装完成后**首次启动一次 VS**，让 vcvarsall.bat 注册到环境

验证：
```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cl /? | findstr Version
```
应显示 MSVC 版本号（如 19.36+）。

### 2.2 CMake（≥ 3.21）

**推荐方式**：VS 2022 安装时已自带 CMake（`C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`），版本足够。

**独立安装**（可选）：[CMake 官网](https://cmake.org/download/) 下载 `cmake-3.28.x-windows-x86_64.msi`，安装时勾选"Add CMake to system PATH"。

验证：
```powershell
cmake --version
```
应显示 3.21+。

### 2.3 vcpkg

```powershell
# 选个目录（推荐 C:\vcpkg）
cd C:\
git clone https://github.com/microsoft/vcpkg.git
C:\vcpkg\bootstrap-vcpkg.bat

# 集成到本用户（可选，但推荐）
C:\vcpkg\vcpkg integrate install
```

设置环境变量（PowerShell 用户级）：
```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
```

验证：
```powershell
& "$env:VCPKG_ROOT\vcpkg" --version
```

### 2.4 Qt 6.8.3

**关键说明**：本项目 vcpkg.json 已声明 `qtbase` 和 `qt-charts` 作为依赖，vcpkg 会**自动从源码编译 Qt 6.8.3**（约 30–60 分钟首次构建）。**你不需要单独下载 Qt**。

> 如不想等，可手动从 Qt 官方安装：
> 1. 注册 Qt 账号（[https://www.qt.io/download-qt-installer](https://www.qt.io/download-qt-installer)）
> 2. 安装 Qt 6.8.3 → 勾选：
>    - MSVC 2022 64-bit
>    - Qt Charts
> 3. 把 `C:\Qt\6.8.3\msvc2022_64\bin` 加到 PATH
>
> 然后在 cmake 命令里加 `-DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64`，跳过 vcpkg 编译 Qt。

**推荐方案：让 vcpkg 自动构建 Qt**——版本和 lock 完全一致，CI 友好。

---

## 3. 完整构建命令

### 3.1 首次构建（Debug + Mock 模式）

```powershell
# 设置 MSVC 环境（每次新 shell 都要）
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

# 进入项目目录
cd "e:\YZX AI Work data\game-perf-monitor"

# 配置（首次会编译 vcpkg 依赖：nlohmann/json, spdlog, fmt, inja, sqlitecpp, qtbase, qt-charts）
# 预计耗时：30–90 分钟（视 CPU 而定；其中 Qt 编译占大头）
cmake -B build -S . `
    -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" `
    -DCMAKE_BUILD_TYPE=Debug `
    -G "Visual Studio 17 2022" -A x64

# 编译（约 3–10 分钟）
cmake --build build --config Debug -j 8
```

> 💡 `-G "Visual Studio 17 2022" -A x64` 会生成 `.sln`，可双击用 VS 打开调试。
> 不用 VS 解决方案、只想用 Ninja 也可以：先 `vcpkg install ninja`，再把 `-G` 换成 `Ninja`。
> 不用 MSBuild 多配置的话，**`--config Debug` 也要带**，因为 MSVC 生成器是多配置的。

### 3.2 Release 构建（无 Mock）

```powershell
cmake -B build-release -S . `
    -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" `
    -DCMAKE_BUILD_TYPE=Release `
    -G "Visual Studio 17 2022" -A x64
cmake --build build-release --config Release -j 8
```

Release 包内 `GAME_PERF_MOCK_ENABLED=0`，没有任何 mock 类被编译进二进制，体积更小。

### 3.3 运行

```powershell
# Debug
.\build\Debug\GamePerfMonitor.exe

# Release
.\build-release\Release\GamePerfMonitor.exe
```

### 3.4 清理与重建

```powershell
# 仅清理项目（保留 vcpkg 缓存）
Remove-Item -Recurse -Force build, build-release

# 完全清理（连 vcpkg 编译的 Qt 也删，下次构建会重编）
Remove-Item -Recurse -Force build, build-release, "$env:VCPKG_ROOT\installed"
```

### 3.5 调试技巧

- **VS 调试**：用 VS 打开 `build\GamePerfMonitor.sln`，设 `GamePerfMonitor` 为启动项目，F5
- **日志位置**：`%APPDATA%\MiniMax\GamePerfMonitor\GamePerfMonitor\logs\game-perf-monitor.log`
- **配置文件**：`%APPDATA%\MiniMax\GamePerfMonitor\GamePerfMonitor\global.json` + `games\*.json`

---

## 4. 运行后应该看到的界面和行为

### 4.1 主窗口

启动后弹出 **1100×720** 的窗口：
- 标题：`GamePerfMonitor 1A`
- 深色背景（#1A1A1A）+ 绿色高亮（#7CFC00）
- 状态栏底部：`Phase 1A · Mock 数据 · UI 500ms 节拍`

### 4.2 "实时" Tab

**上半部分：当前指标网格**（4 列 × 2 行 = 8 个值，每 500ms 刷新）

| FPS | 1% low | CPU | GPU |
|-----|--------|-----|-----|
| 帧时间 | 0.1% low | CPU℃ | VRAM |

- 数值用青/绿/橙/粉/黄等颜色编码
- 数据由 11 个 Mock 适配器产生
- FPS 在 60 附近抖动、CPU/GPU 在 30–80% 之间、CPU℃ 45–75℃、VRAM 0–12GB 波动

**下半部分：折线图**（QChart 离屏渲染）
- 三条线：CPU%（蓝）、GPU%（粉）、FPS（绿）
- 横轴 = 采样序号（环形缓冲 600 点 ≈ 5 分钟）
- 滚动更新

### 4.3 "设置" Tab

四个分组：

1. **通用**：随 Windows 启动、最小化到托盘、语言、采样间隔（ms）、历史点数、UI 刷新（ms）
2. **游戏库**：
   - 列表显示已加入的游戏（首次启动为空）
   - 按钮：
     - `新建…`：手动创建配置
     - `从前台进程加入`：一键把当前前台 exe + 窗口标题填入新游戏
     - `从 EXE 文件…`：浏览 exe 文件
     - `编辑…`：打开详细配置对话框
     - `删除`：删除配置
3. **HUD 悬浮窗**：
   - 启用 HUD（勾选后会显示悬浮窗）
   - 透明度（10–100%）
   - 字号（8–64 px）
   - 布局（vertical / horizontal）
   - 字段列表（勾选启用/禁用）
4. **开发（仅 Debug 构建可见）**：
   - `dev.mockMode` 开关
   - 提示信息：Release 构建 / 第二阶段后此开关被忽略

### 4.4 HUD 悬浮窗

在设置页勾选"启用 HUD"后显示：
- **位置**：默认 (80, 80)
- **大小**：根据字段数量、字号、布局方向自动计算
- **外观**：深色半透明圆角（#000 35% alpha），文字青/绿高亮
- **快捷键**（在 HUD 窗口聚焦时）：
  - `Ctrl+L`：切换锁定（锁定后不可拖动）
  - `F9`：切换竖/横布局
  - `Esc`：隐藏 HUD
  - **滚轮**：调字号（每个字段 +1/-1 px，8–64 范围）
  - **Ctrl+滚轮**：调透明度（±0.1，0.2–1.0 范围）
- **拖动**：鼠标左键按住标题区拖动
- **双击**：切换为锁定状态

### 4.5 配置持久化验证

1. 在"设置"页改任意值（如把 UI 刷新改为 250ms、勾选 HUD）
2. 点"保存"
3. 关闭应用
4. 重新启动
5. 状态栏应仍显示你设的 250ms、HUD 仍在原位置
6. 文件已落盘：
   - `%APPDATA%\MiniMax\GamePerfMonitor\GamePerfMonitor\global.json`
   - `%APPDATA%\MiniMax\GamePerfMonitor\GamePerfMonitor\games\<gameId>.json`（如加了游戏）

### 4.6 日志

- `Logger` 同时输出到 stderr 和 `%APPDATA%\MiniMax\GamePerfMonitor\GamePerfMonitor\logs\game-perf-monitor.log`
- 滚动策略：10MB × 5 个文件
- 启动时会打印：mock 适配器列表、采样参数、UI 节拍

---

## 5. 常见构建错误排查

### 5.1 找不到 vcpkg toolchain

```
CMake Error: Could not find toolchain file:
C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

**排查**：
```powershell
Test-Path "$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
# 应返回 True
```
如 False：检查 `C:\vcpkg` 目录是否存在，或重新 `git clone`。

### 5.2 vcpkg 编译 Qt 超时 / 中断

vcpkg 编译 Qt 6.8.3 第一次需要 30–90 分钟，可能被杀毒软件 / Windows 更新中断。

**排查**：
```powershell
# 重新跑（vcpkg 会从断点续编）
cmake --build build --config Debug
```

**加速**：
- 关掉杀毒软件实时保护
- 确保电源计划为"高性能"
- 增加 `-j` 并发（vcpkg 自动用所有核心）

### 5.3 MSVC 编译器找不到

```
error: Microsoft Visual Studio not found; please run vcvarsall.bat
```

**排查**：在 PowerShell 顶部先运行
```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
```
或者改用 `-G "Visual Studio 17 2022" -A x64` 走 MSBuild 路径（CMake 会自动调用 VS 的 cl.exe）。

### 5.4 Qt 头文件找不到（链接错误）

```
fatal error C1083: 无法打开包括文件: "QtWidgets/QMainWindow": No such file or directory
```

**排查**：
1. 检查 vcpkg 是否成功编译 `qtbase:x64-windows`：
   ```powershell
   & "$env:VCPKG_ROOT\vcpkg" list qtbase
   ```
2. 确认 `cmake -B build` 时输出的 `vcpkg-manifest-install` 步骤成功（`Installing 1/8 qtbase ...`）
3. 如果编译报缺 ICU / zlib 等，让 vcpkg 自动拉（manifest 模式已配 `builtin-baseline`）

### 5.5 链接 LNK2019 / LNK2001（找不到符号）

```
unresolved external symbol "public: __cdecl gpm::..."
```

**原因**：常见于 CMakeLists.txt 里某文件没被加进 target_sources。
**排查**：
- 对照 `e:\YZX AI Work data\game-perf-monitor\CMakeLists.txt` 里的 `qt_add_executable` 列表，确认新加的文件已加入
- mock 适配器都是 header-only，已通过 `if(GAME_PERF_ENABLE_MOCK)` 加入

### 5.6 moc 错误（Q_OBJECT 相关）

```
moc: ... error: No Q_OBJECT in class
```

**原因**：CMakeLists.txt 中 `CMAKE_AUTOMOC` 没开，或类没继承 QObject。
**排查**：本项目顶层 CMakeLists.txt 已开 `qt_standard_project_setup(REQUIRES 6.6)`，自动启用 AUTOMOC/UIC/RCC。如有自定义类没继承 QObject 又有 Q_OBJECT 宏，删 Q_OBJECT 即可。

### 5.7 qOverload 编译失败

```
error: 'qOverload' was not declared in this scope
```

**原因**：Qt 6 中 qOverload 在 `<QtCore>`，但需要 `#include <QtCore>` 或具体头。
**排查**：SettingsPage.cpp 用了 `qOverload<int>(&QSpinBox::valueChanged)`，已通过 `#include <QSpinBox>` 间接引入。如还报错，在 SettingsPage.cpp 顶部加 `#include <QtCore>`。

### 5.8 Qt Charts 找不到

```
fatal error C1083: 无法打开包括文件: "QtCharts/QChart"
```

**原因**：vcpkg 没装 `qt-charts`。
**排查**：
```powershell
& "$env:VCPKG_ROOT\vcpkg" list | Select-String "qt"
# 应看到 qtbase, qt-charts, qt5compat, qtdeclarative (Qt6 namespace)
```
如缺 `qt-charts`：检查 `vcpkg.json` 的 `overrides` 段。

### 5.9 运行时立即闪退

**排查**：
1. 用 PowerShell 直接运行（不要双击）：
   ```powershell
   cd "e:\YZX AI Work data\game-perf-monitor"
   .\build\Debug\GamePerfMonitor.exe 2>&1 | Tee-Object -FilePath run.log
   ```
2. 查看 `run.log` 末尾的 spdlog 输出和 stderr
3. 看 `%APPDATA%\MiniMax\GamePerfMonitor\GamePerfMonitor\logs\game-perf-monitor.log`

### 5.10 vcpkg manifest 重新解析

如果修改了 `vcpkg.json`，CMake 会自动重新解析依赖：
```powershell
Remove-Item -Recurse -Force build\vcpkg_installed
cmake --build build --config Debug
```

---

## 6. 1A 阶段验收 checklist

完成下面所有项，1A 就算 OK：

- [ ] 构建成功（Debug + Release 都跑通）
- [ ] 启动后主窗口正常显示，状态栏显示"Phase 1A · Mock 数据"
- [ ] 实时面板 8 个数字每 500ms 变化
- [ ] 三条折线滚动
- [ ] 设置页能改采样间隔、保存、关闭、重启后值保留
- [ ] 新建一个游戏配置（任意名字）+ 保存 → `%APPDATA%` 下出现对应 JSON
- [ ] 启用 HUD → 浮窗出现
- [ ] 拖动 HUD 到新位置 → 关闭 → 重启后位置保留
- [ ] F9 切换横/竖布局
- [ ] Ctrl+滚轮调透明度生效
- [ ] 滚轮调字号生效
- [ ] Ctrl+L 锁定 → 拖动无效 → 再按一次解锁
- [ ] 日志文件正常落盘

完成上述后我们进入 **1B：SQLite 会话 + HTML 报告 + 游戏识别 + 托盘**。
