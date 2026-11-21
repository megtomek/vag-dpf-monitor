# Kompiluje wskazany wariant i wgrywa go na płytkę przez USB.
param(
    [ValidateSet('V1')]
    [string]$BoardRevision = 'V1',
    [Parameter(Mandatory = $true)]
    [ValidateSet('true', 'false')]
    [string]$CanBusEnabled,
    [ValidateSet('NORMAL', 'LISTENONLY', 'SELFTEST')]
    [string]$CanBusMode = 'NORMAL',
    [switch]$ConfirmBenchOnly,
    [switch]$PassiveCapture,
    [ValidateSet('1ms', '5ms')]
    [string]$CanSelfTestInterval = '5ms',
    [ValidateSet('true', 'false')]
    [string]$ObdPollingEnabled = 'true',
    [ValidateSet('0', '115200')]
    [string]$LoggerBaudRate = '115200',
    [ValidateSet('USB', 'OTA')]
    [string]$Transport = 'OTA',
    [string]$Device = 'audi-dpf-monitor.local'
)

$captureEnabled = if ($PassiveCapture) { 'true' } else { 'false' }
if ($PassiveCapture) { $CanBusMode = 'LISTENONLY'; $ObdPollingEnabled = 'false'; $CanBusEnabled = 'true' }
$ErrorActionPreference = 'Stop'
$selfTestBenchWarning = 'CanBusMode SELFTEST wysyła syntetyczne ramki przez TWAI. Nie wgrywaj go, gdy transceiver SN65HVD230 jest połączony z autem. Odłącz transceiver od ESP32 i samochodu oraz użyj -ConfirmBenchOnly, aby jawnie potwierdzić test wyłącznie na stole.'
if ($CanBusMode -eq 'SELFTEST' -and -not $ConfirmBenchOnly) {
    throw $selfTestBenchWarning
}
$project = $PSScriptRoot
$esphomePythonCandidates = @(
    $env:ESPHOME_PYTHON,
    (Join-Path $project '.venv\Scripts\python.exe'),
    (Join-Path $env:USERPROFILE 'Documents\Codex\2026-10-01\c\.venv\Scripts\python.exe')
) | Where-Object { $_ -and (Test-Path -LiteralPath $_ -PathType Leaf) }
$esphomePython = $esphomePythonCandidates | Select-Object -First 1
$compileScript = Join-Path $project 'Kompiluj-monitor-DPF.ps1'
$factoryImage = Join-Path $project '.esphome-data\build\audi-dpf-monitor\build\firmware.factory.bin'
$otaImage = Join-Path $project '.esphome-data\build\audi-dpf-monitor\build\firmware.ota.bin'

if (-not $esphomePython) {
	throw 'Nie znaleziono Pythona z ESPHome. Ustaw ESPHOME_PYTHON albo zainstaluj ESPHome w .venv projektu.'
}

if ($Transport -eq 'OTA' -and $Device -match '^COM\d+$') {
    throw 'Dla OTA podaj adres IP lub nazwę urządzenia, np. -Transport OTA -Device 192.168.4.1.'
}

Write-Host ("Kompilacja i wgrywanie: PCB {0}, CAN={1}, tryb={2}, OBD={3}, logger USB={4}, transport={5}, cel={6}" -f $BoardRevision, $CanBusEnabled, $CanBusMode, $ObdPollingEnabled, $LoggerBaudRate, $Transport, $Device) -ForegroundColor Cyan
$compileArguments = @{
    BoardRevision = $BoardRevision
    CanBusEnabled = $CanBusEnabled
    CanBusMode = $CanBusMode
    CanSelfTestInterval = $CanSelfTestInterval
    ObdPollingEnabled = $ObdPollingEnabled
    LoggerBaudRate = $LoggerBaudRate
}
$compileCliArguments = @()
foreach ($key in $compileArguments.Keys) {
    $compileCliArguments += "-$key"
    $compileCliArguments += [string]$compileArguments[$key]
}
if ($PassiveCapture) { $compileCliArguments += '-PassiveCapture' }
if ($ConfirmBenchOnly) { $compileCliArguments += '-ConfirmBenchOnly' }
& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $compileScript @compileCliArguments
if ($LASTEXITCODE -ne 0) {
    throw "Kompilacja zakończyła się kodem $LASTEXITCODE. Firmware nie został wgrany."
}
$uploadImage = if ($Transport -eq 'OTA') { $otaImage } else { $factoryImage }
if (-not (Test-Path -LiteralPath $uploadImage -PathType Leaf)) {
    throw "Nie znaleziono obrazu dla transportu $Transport`: $uploadImage. Firmware nie został wgrany."
}

$oldDataDir = [Environment]::GetEnvironmentVariable('ESPHOME_DATA_DIR', 'Process')
try {
    # ESPHome stores this project's real build under .esphome-data. Setting this
    # before upload prevents flashing an older image from the default .esphome folder.
    $env:ESPHOME_DATA_DIR = Join-Path $project '.esphome-data'
    Push-Location $project
    try {
        & $esphomePython -m esphome --substitution can_capture_enabled $captureEnabled --substitution display_cs_pin GPIO9 --substitution can_bus_enabled $CanBusEnabled --substitution can_bus_mode $CanBusMode --substitution can_selftest_interval $CanSelfTestInterval --substitution obd_polling_enabled $ObdPollingEnabled --substitution logger_baud_rate $LoggerBaudRate upload audi-dpf-monitor.yaml --device $Device --file $uploadImage
        if ($LASTEXITCODE -ne 0) {
            if ($Transport -eq 'USB') {
                throw "Wgrywanie USB na $Device zakończyło się kodem $LASTEXITCODE. Jeśli ESPHome zgłosiło 'Access is denied' lub 'port is busy', zamknij monitor szeregowy ESPHome/Arduino/terminal korzystający z tego portu i ponów próbę. Nie kończę procesów automatycznie. Zwykłe aktualizacje wykonuj przez OTA, aby ominąć COM."
            }
            throw "Wgrywanie OTA na $Device zakończyło się kodem $LASTEXITCODE. Sprawdź, czy monitor jest w tej samej sieci i jaki ma aktualny adres; dla AP użyj -Device 192.168.4.1."
        }
    }
    finally {
        Pop-Location
    }
}
finally {
    [Environment]::SetEnvironmentVariable('ESPHOME_DATA_DIR', $oldDataDir, 'Process')
}

if ($Transport -eq 'USB') {
    Write-Host 'Wgranie USB zakończone; ESPHome zweryfikowało zapis flash hashem.' -ForegroundColor Green
} else {
    Write-Host 'Wgranie OTA zakończone; ESPHome potwierdziło zakończenie transferu.' -ForegroundColor Green
}
