param(
    [ValidateRange(1, 8)]
    [int]$DurationHours = 8,
    [string]$Device = 'COM6'
)

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$toolRoot = 'C:\Users\Administrator\Documents\Codex\2026-10-01\c'
$esphome = Join-Path $toolRoot '.venv\Scripts\esphome.exe'
$firmware = Join-Path $projectRoot '.esphome-data\build\audi-dpf-monitor\build\firmware.factory.bin'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$stdoutPath = Join-Path $PSScriptRoot "normal-usb-$stamp.stdout.log"
$stderrPath = Join-Path $PSScriptRoot "normal-usb-$stamp.stderr.log"
$metaPath = Join-Path $PSScriptRoot "normal-usb-$stamp.meta.txt"

if (-not (Test-Path -LiteralPath $esphome)) { throw "Nie znaleziono ESPHome: $esphome" }
if (-not (Test-Path -LiteralPath $firmware)) { throw "Nie znaleziono obrazu firmware: $firmware" }
if (-not ([System.IO.Ports.SerialPort]::GetPortNames() -contains $Device)) {
    throw "Port $Device nie jest obecnie dostępny."
}

$env:ESPHOME_DATA_DIR = Join-Path $projectRoot '.esphome-data'
$startedUtc = [DateTime]::UtcNow
$deadlineUtc = $startedUtc.AddHours($DurationHours)
$firmwareHash = (Get-FileHash -LiteralPath $firmware -Algorithm SHA256).Hash

@(
    "Started UTC: $($startedUtc.ToString('o'))"
    "Duration hours: $DurationHours"
    "Device: $Device"
    "Firmware factory SHA256: $firmwareHash"
    "Firmware size bytes: $((Get-Item -LiteralPath $firmware).Length)"
    "Stdout log: $stdoutPath"
    "Stderr log: $stderrPath"
    'Mode: read-only ESPHome serial log capture; this script does not compile or flash firmware.'
) | Set-Content -LiteralPath $metaPath -Encoding utf8
Write-Host "Zbieram logi NORMAL z $Device przez $DurationHours h; zapis: $stdoutPath"

$logger = $null
try {
    $logger = Start-Process -FilePath $esphome `
        -ArgumentList @('logs', 'audi-dpf-monitor.yaml', '--device', $Device) `
        -WorkingDirectory $projectRoot `
        -RedirectStandardOutput $stdoutPath `
        -RedirectStandardError $stderrPath `
        -WindowStyle Hidden `
        -PassThru

    while ([DateTime]::UtcNow -lt $deadlineUtc) {
        Start-Sleep -Seconds 20
        $logger.Refresh()
        if ($logger.HasExited) { break }
    }

    $logger.Refresh()
    if (-not $logger.HasExited) {
        Stop-Process -Id $logger.Id -Force
        $logger.WaitForExit()
    }
    $logger.Refresh()
    $finishedUtc = [DateTime]::UtcNow
    @(
        "Finished UTC: $($finishedUtc.ToString('o'))"
        "Elapsed minutes: $([Math]::Round(($finishedUtc - $startedUtc).TotalMinutes, 1))"
        "Logger process exit code: $($logger.ExitCode)"
    ) | Add-Content -LiteralPath $metaPath -Encoding utf8
}
catch {
    if ($null -ne $logger) {
        $logger.Refresh()
        if (-not $logger.HasExited) { Stop-Process -Id $logger.Id -Force }
    }
    "Capture error UTC: $([DateTime]::UtcNow.ToString('o'))`r`n$($_.Exception.Message)" |
        Add-Content -LiteralPath $metaPath -Encoding utf8
    throw
}
