# 图标说明

`app.ico` 和 `tray.ico` 是可选资源。缺失时编译/运行不报错，Qt 使用内置默认图标。

## 推荐的临时方案
1. 用任意图片转 .ico 在线工具生成（16x16 / 32x32 / 48x48 多分辨率）
2. 放到本目录后重新 cmake 即可

## 简单程序生成（PowerShell）
```powershell
Add-Type -AssemblyName System.Drawing
$bmp = New-Object System.Drawing.Bitmap 32,32
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.Clear([System.Drawing.Color]::FromArgb(124,252,0))   # RTSS 绿
$font = New-Object System.Drawing.Font("Segoe UI", 14, [System.Drawing.FontStyle]::Bold)
$brush = [System.Drawing.Brushes]::Black
$g.DrawString("G", $font, $brush, 4, 4)
# 写入 .ico（简化：直接用 Icon.ExtractAssociatedIcon 不易，保留 bmp 也可被 Qt 接受）
$bmp.Save("app.bmp", [System.Drawing.Imaging.ImageFormat]::Bmp)
# 真正 .ico 需要多帧 + 头，1A 阶段先跳过 — 项目编译/运行不需要图标
```

## 1A 阶段结论
**图标资源不影响功能**。功能全部由代码完成。
