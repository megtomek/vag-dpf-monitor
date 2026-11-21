param(
    [string]$Device = 'audi-dpf-monitor.local',
    [ValidateRange(1, 65535)]
    [int]$Port = 80,
    [ValidateRange(0, 86400)]
    [double]$Seconds = 0,
    [string]$OutputFile = ''
)

$ErrorActionPreference = 'Stop'
$project = $PSScriptRoot
$pythonCandidates = @(
    $env:ESPHOME_PYTHON,
    (Join-Path $project '.venv\Scripts\python.exe'),
    (Join-Path $env:USERPROFILE 'Documents\Codex\2026-10-01\c\.venv\Scripts\python.exe')
) | Where-Object { $_ -and (Test-Path -LiteralPath $_ -PathType Leaf) }
$python = $pythonCandidates | Select-Object -First 1
if (-not $python) {
    throw 'ESPHome Python was not found. Set ESPHOME_PYTHON or check the installation under Documents\Codex\2026-10-01\c.'
}
$reader = Join-Path $project 'czytaj_logi_dpf.py'
if (-not (Test-Path -LiteralPath $reader -PathType Leaf)) {
    throw "Nie znaleziono czytaj_logi_dpf.py w $project"
}

$readerArguments = @($reader, '--device', $Device, '--port', $Port, '--seconds', $Seconds)
if ($OutputFile) {
    $readerArguments += @('--output', $OutputFile)
}
& $python @readerArguments
if ($LASTEXITCODE -ne 0) {
    throw "Log reader exited with code $LASTEXITCODE."
}
