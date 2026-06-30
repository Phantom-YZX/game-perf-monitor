<#
.SYNOPSIS
    构建并运行 GamePerfMonitor 1A（Debug + Mock 适配器）。
.DESCRIPTION
    1. 调用 vcvars64.bat 加载 MSVC 环境
    2. cmake -B build 配置（Debug, GAME_PERF_ENABLE_MOCK=ON）
    3. cmake --build build --config Debug -j 并行编译
    4. 启动 GamePerfMonitor.exe，2>&1 + Tee-Object 输出到 run.log
.PARAMETER Clean
    强制清空 build/ 后重新配置
.PARAMETER VcpkgRoot
    vcpkg 安装根目录，默认 C:\vcpkg
.EXAMPLE
    .\build-and-run.ps1
    .\build-and-run.ps1 -Clean
    .\build-and-run.ps1 -VcpkgRoot "D:\vcpkg"
#>
[CmdletBinding()]
param(
    [switch]$Clean,
    [string]$VcpkgRoot = "C:\vcpkg"
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ProjectRoot

# -------- 1. 检测工具链 --------
Write-Host "[1/4] Detecting toolchain..." -ForegroundColor Cyan

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsInstallPath = $null
if (Test-Path $vswhere) {
    $vsInstallPath = & $vswhere -latest -property installationPath 2>$null
}
if (-not $vsInstallPath) {
    Write-Host "  - Visual Studio 2022 not found via vswhere" -ForegroundColor Yellow
    $vsInstallPath = "C:\Program Files\Microsoft Visual Studio\2022\Community"
}

$vcvars = Join-Path $vsInstallPath "VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvars)) {
    Write-Host "  ✗ vcvars64.bat not found at: $vcvars" -ForegroundColor Red
    Write-Host "    安装 Visual Studio 2022 + 'Desktop development with C++'" -ForegroundColor Yellow
    exit 1
}
Write-Host "  - VS install: $vsInstallPath" -ForegroundColor Green

if (-not (Test-Path $VcpkgRoot)) {
    Write-Host "  ✗ vcpkg not found at: $VcpkgRoot" -ForegroundColor Red
    Write-Host "    git clone https://github.com/microsoft/vcpkg.git $VcpkgRoot" -ForegroundColor Yellow
    exit 1
}
$env:VCPKG_ROOT = $VcpkgRoot
Write-Host "  - VCPKG_ROOT: $VcpkgRoot" -ForegroundColor Green

# -------- 2. cmake 配置 --------
Write-Host "[2/4] Configuring CMake (Debug + GAME_PERF_ENABLE_MOCK=ON)..." -ForegroundColor Cyan

if ($Clean -and (Test-Path build)) {
    Remove-Item -Recurse -Force build
    Write-Host "  - Cleaned build/" -ForegroundColor Yellow
}

# 加载 MSVC 环境
$tempVcvars = Join-Path $env:TEMP "vcvars_setup_$PID.bat"
@"
@echo off
call "$vcvars" > nul 2>&1
set
"@ | Out-File -Encoding ASCII $tempVcvars
cmd /c $tempVcvars 2>&1 | ForEach-Object {
    if ($_ -match '^(PATH|INCLUDE|LIB|LIBPATH|VCINSTALLDIR|VSINSTALLDIR|WindowsSdkDir|UCRTVersion|WindowsSDKVersion|VCToolsInstallDir)=') {
        $name, $val = $_ -split '=', 2
        Set-Item -Path "Env:$name" -Value $val
    }
}
Remove-Item $tempVcvars -ErrorAction SilentlyContinue

if ($Clean -or -not (Test-Path build)) {
    cmake -B build -S . `
        -DCMAKE_TOOLCHAIN_FILE="$VcpkgRoot\scripts\buildsystems\vcpkg.cmake" `
        -DCMAKE_BUILD_TYPE=Debug `
        -G "Visual Studio 17 2022" -A x64
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  ✗ cmake configure failed" -ForegroundColor Red
        exit 1
    }
    Write-Host "  - Configured." -ForegroundColor Green
} else {
    Write-Host "  - build/ exists, skip configure" -ForegroundColor Yellow
}

# -------- 3. 构建 --------
Write-Host "[3/4] Building (Debug, parallel)..." -ForegroundColor Cyan
cmake --build build --config Debug -j 8
if ($LASTEXITCODE -ne 0) {
    Write-Host "  ✗ Build failed." -ForegroundColor Red
    Write-Host "  查看 build\GamePerfMonitor.dir\*.log 找具体错误" -ForegroundColor Yellow
    exit 1
}
Write-Host "  - Built successfully." -ForegroundColor Green

$exe = Join-Path $ProjectRoot "build\Debug\GamePerfMonitor.exe"
if (-not (Test-Path $exe)) {
    Write-Host "  ✗ $exe not found" -ForegroundColor Red
    exit 1
}

# -------- 4. 运行 --------
Write-Host "[4/4] Running $exe ..." -ForegroundColor Cyan
Write-Host "    日志: %APPDATA%\MiniMax\GamePerfMonitor\GamePerfMonitor\logs\game-perf-monitor.log" -ForegroundColor Yellow
Write-Host "    配置: %APPDATA%\MiniMax\GamePerfMonitor\GamePerfMonitor\global.json" -ForegroundColor Yellow
Write-Host "    按 Ctrl+C 退出" -ForegroundColor Yellow
Write-Host ""

# 复制 windeployqt 输出的 Qt DLL（首次或缺 Qt6Core.dll 时）
$qtDir = Join-Path $VcpkgRoot "installed\x64-windows\bin"
if (Test-Path $qtDir) {
    $missing = @("Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", "Qt6Charts.dll") | Where-Object {
        -not (Test-Path (Join-Path (Split-Path $exe) $_))
    }
    if ($missing.Count -gt 0) {
        Write-Host "    复制 Qt 运行时 DLL（$($missing -join ', ')）..." -ForegroundColor Yellow
        Get-ChildItem $qtDir -Filter "Qt6*.dll" -ErrorAction SilentlyContinue | Copy-Item -Destination (Split-Path $exe) -Force
        $plugins = Join-Path $qtDir "..\plugins"
        if (Test-Path $plugins) {
            $platforms = Join-Path (Split-Path $exe) "platforms"
            New-Item -ItemType Directory -Force -Path $platforms | Out-Null
            Copy-Item -Path (Join-Path $plugins "platforms\qwindows.dll") -Destination $platforms -Force
        }
    }
}

# 启动（带 Tee）
& $exe 2>&1 | Tee-Object run.log