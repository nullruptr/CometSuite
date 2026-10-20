# CometSuite.exe (host) のビルド用スクリプト。
# 使い方 (通常の PowerShell から):
#   .\build.ps1              # compile, compile-debug, compdb をすべて実行
#   .\build.ps1 compile      # Release のみ
#   .\build.ps1 compile-debug
#   .\build.ps1 compdb
param(
    [ValidateSet('all', 'compile', 'compile-debug', 'compdb')]
    [string]$Target = 'all'
)

$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

# "Developer PowerShell for VS 2022" 相当の環境 (cl.exe / ninja.exe を PATH に通す)
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) { throw 'Visual Studio (C++ ツール) が見つかりません。' }
Import-Module (Join-Path $vsPath 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll')
Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64'

# 初回のみ Conan を入れる
if (-not (Test-Path '.venv')) {
    poetry install --no-root
    if ($LASTEXITCODE -ne 0) { throw 'poetry install に失敗しました。' }
}

$targets = if ($Target -eq 'all') { @('compile', 'compile-debug', 'compdb') } else { @($Target) }
foreach ($t in $targets) {
    Write-Host "=== make $t ===" -ForegroundColor Cyan
    make $t
    if ($LASTEXITCODE -ne 0) { throw "make $t に失敗しました。" }
}
