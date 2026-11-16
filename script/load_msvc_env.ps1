#Requires -Version 5.1
<#
.SYNOPSIS
    在当前 PowerShell 会话里导入 Visual Studio 的 MSVC 环境。

.DESCRIPTION
    用 vswhere 定位 Visual Studio，执行 vcvarsall.bat，把 PATH / INCLUDE / LIB
    等变量导入当前进程。之后可以直接使用：

        cmake --preset=msvc_x64_debug
        cmake --build --preset=msvc_x64_debug_build

    注意：本脚本需要“点源”（dot-source）执行才会影响当前会话：

        Set-ExecutionPolicy -Scope Process Bypass   # 仅当系统禁止运行脚本时
        . .\script\load_msvc_env.ps1
        . .\script\load_msvc_env.ps1 -Architecture x86

.PARAMETER Architecture
    x64（默认）或 x86，对应 32/64 位 MSVC 工具链。

.PARAMETER Quiet
    只导入环境，不打印摘要。
#>
[CmdletBinding()]
param(
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64',

    [switch]$Quiet
)

$ErrorActionPreference = 'Stop'

function Find-VsWhere {
    $candidates = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'),
        (Join-Path $env:ProgramFiles 'Microsoft Visual Studio/Installer/vswhere.exe')
    )
    foreach ($candidate in $candidates) {
        if ($candidate -and (Test-Path $candidate)) { return $candidate }
    }
    return $null
}

$vswhere = Find-VsWhere
if (-not $vswhere) {
    throw 'vswhere.exe not found; install Visual Studio with the C++ workload.'
}

$vsPath = & $vswhere -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath | Select-Object -First 1
if (-not $vsPath) {
    throw 'No Visual Studio installation with the MSVC tools was found.'
}

$vcvarsall = Join-Path $vsPath 'VC/Auxiliary/Build/vcvarsall.bat'
if (-not (Test-Path $vcvarsall)) {
    throw "vcvarsall.bat not found: $vcvarsall"
}

# cmd /c "<vcvarsall> <arch> && set" 会输出该环境下的全部变量
$output = & cmd /c "`"$vcvarsall`" $Architecture >nul 2>&1 && set"
foreach ($line in $output) {
    if ($line -match '^([^=]+)=(.*)$') {
        Set-Item -Path "env:$($Matches[1])" -Value $Matches[2]
    }
}

$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    throw "MSVC environment import failed: cl.exe not found in PATH ($vsPath, $Architecture)"
}

if (-not $Quiet) {
    Write-Host "[env] Visual Studio: $vsPath" -ForegroundColor DarkGray
    Write-Host "[env] architecture : $Architecture" -ForegroundColor DarkGray
    Write-Host "[env] cl.exe       : $($cl.Source)" -ForegroundColor DarkGray
    Write-Host '[env] now run: cmake --preset=msvc_x64_debug' -ForegroundColor Green
}
