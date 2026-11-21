# Waliduje konfigurację, kompiluje firmware i zapisuje wynik dla wskazanej rewizji.

param(
    [ValidateSet('V1')]
    [string]$BoardRevision = 'V1',
    [ValidateSet('true', 'false')]
    [string]$CanBusEnabled = 'true',
    [ValidateSet('NORMAL', 'LISTENONLY', 'SELFTEST')]
    [string]$CanBusMode = 'NORMAL',
    [switch]$ConfirmBenchOnly,
    [switch]$PassiveCapture,
    [ValidateSet('1ms', '5ms')]
    [string]$CanSelfTestInterval = '5ms',
    [ValidateSet('true', 'false')]
    [string]$ObdPollingEnabled = 'true',
    [ValidateSet('0', '115200')]
    [string]$LoggerBaudRate = '115200'
)

$captureEnabled = if ($PassiveCapture) { 'true' } else { 'false' }
if ($PassiveCapture) { $CanBusMode = 'LISTENONLY'; $ObdPollingEnabled = 'false'; $CanBusEnabled = 'true' }
$ErrorActionPreference = 'Stop'
$project = $PSScriptRoot
$selfTestBenchWarning = 'CanBusMode SELFTEST wysyła syntetyczne ramki przez TWAI. Przed kompilacją wariantu przeznaczonego do wgrania odłącz fizycznie transceiver SN65HVD230 od ESP32 i samochodu; dodaj -ConfirmBenchOnly, aby potwierdzić użycie wyłącznie na stole.'
if ($CanBusMode -eq 'SELFTEST' -and -not $ConfirmBenchOnly) {
    throw $selfTestBenchWarning
}
$esphomePythonCandidates = @(
    $env:ESPHOME_PYTHON,
    (Join-Path $project '.venv\Scripts\python.exe'),
    (Join-Path $env:USERPROFILE 'Documents\Codex\2026-10-01\c\.venv\Scripts\python.exe')
) | Where-Object { $_ -and (Test-Path -LiteralPath $_ -PathType Leaf) }
$esphomePython = $esphomePythonCandidates | Select-Object -First 1
$projectName = 'audi-dpf-monitor'
$buildDirectory = Join-Path $project ".esphome-data\build\$projectName\build"
$cmakeCache = Join-Path $buildDirectory 'CMakeCache.txt'
$cachedIdfRoot = $null
if (Test-Path -LiteralPath $cmakeCache) {
    $cachedPythonLine = Select-String -LiteralPath $cmakeCache -Pattern '^PYTHON:UNINITIALIZED=(.+)$' |
        Select-Object -First 1
    if ($cachedPythonLine -and $cachedPythonLine.Matches[0].Groups[1].Value -match '^(.*)\\penvs\\5\.5\.5\\Scripts\\python\.exe$') {
        $cachedIdfRoot = $Matches[1]
    }
}
$idfRoot = if ($env:ESPRESSIF_TOOLS_PATH) {
    $env:ESPRESSIF_TOOLS_PATH
} elseif ($env:ESPHOME_ESP_IDF_PREFIX) {
    $env:ESPHOME_ESP_IDF_PREFIX
} elseif ($cachedIdfRoot -and (Test-Path -LiteralPath (Join-Path $cachedIdfRoot 'frameworks\5.5.5'))) {
    # Keep the IDF Python/tool roots aligned with the existing CMake build cache.
    $cachedIdfRoot
} elseif (Test-Path -LiteralPath 'C:\Espressif\frameworks\5.5.5') {
    'C:\Espressif'
} else {
    # Use the project-managed toolchain when no existing or system install exists.
    Join-Path $project '.esphome-idf'
}
$idfPath = Join-Path $idfRoot 'frameworks\5.5.5'
$idfPython = Join-Path $idfRoot 'penvs\5.5.5\Scripts\python.exe'
$displayCsPin = 'GPIO9'

if (-not $esphomePython) {
    throw 'Nie znaleziono Pythona z ESPHome. Ustaw ESPHOME_PYTHON albo zainstaluj ESPHome w .venv projektu.'
}
if (-not (Test-Path -LiteralPath (Join-Path $project 'audi-dpf-monitor.yaml'))) {
    throw "Nie znaleziono konfiguracji audi-dpf-monitor.yaml w: $project"
}

$cmakeHome = Get-ChildItem -LiteralPath (Join-Path $idfRoot 'tools\cmake') -Directory |
    Sort-Object Name -Descending | Select-Object -First 1
$ninjaHome = Get-ChildItem -LiteralPath (Join-Path $idfRoot 'tools\ninja') -Directory |
    Sort-Object Name -Descending | Select-Object -First 1
$xtensaHome = Get-ChildItem -LiteralPath (Join-Path $idfRoot 'tools\xtensa-esp-elf') -Directory |
    Sort-Object Name -Descending | Select-Object -First 1
$cmakeBin = if ($cmakeHome) { Join-Path $cmakeHome.FullName 'bin' }
$ninjaBin = if ($ninjaHome) { $ninjaHome.FullName }
$xtensaBin = if ($xtensaHome) { Join-Path $xtensaHome.FullName 'xtensa-esp-elf\bin' }

foreach ($requiredPath in @($idfPath, $idfPython, $cmakeBin, $ninjaBin, $xtensaBin)) {
    if (-not $requiredPath -or -not (Test-Path -LiteralPath $requiredPath)) {
        throw "Brakuje lokalnego narzędzia ESP-IDF: $requiredPath"
    }
}

$environmentNames = @(
    'ESPHOME_DATA_DIR',
    'ESPHOME_ESP_IDF_PREFIX',
    'IDF_PATH',
    'IDF_TOOLS_PATH',
    'IDF_PYTHON_ENV_PATH',
    'ESP_ROM_ELF_DIR',
    'XTENSA_GNU_CONFIG',
    'PATH'
)
$oldEnvironment = @{}
foreach ($name in $environmentNames) {
    $oldEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
}

$env:ESPHOME_DATA_DIR = Join-Path $project '.esphome-data'
$env:ESPHOME_ESP_IDF_PREFIX = $idfRoot
$env:IDF_PATH = $idfPath
$env:IDF_TOOLS_PATH = $idfRoot
$env:IDF_PYTHON_ENV_PATH = Split-Path -Parent (Split-Path -Parent $idfPython)
$romElfHome = Get-ChildItem -LiteralPath (Join-Path $idfRoot 'tools\esp-rom-elfs') -Directory -ErrorAction SilentlyContinue |
    Sort-Object Name -Descending | Select-Object -First 1
Remove-Item Env:ESP_ROM_ELF_DIR -ErrorAction SilentlyContinue
if ($romElfHome) {
    $env:ESP_ROM_ELF_DIR = $romElfHome.FullName
}
$env:PATH = (@(
    (Split-Path -Parent $idfPython),
    $cmakeBin,
    $ninjaBin,
    $xtensaBin
) + $oldEnvironment['PATH']) -join ';'

# ESPHome supplies its own Xtensa -dynconfig; remove a conflicting legacy value.
Remove-Item Env:XTENSA_GNU_CONFIG -ErrorAction SilentlyContinue

# Check the actual cross-compiler before starting ESP-IDF. A broken Windows
# loader can otherwise leave idf.py/ninja waiting without a useful error.
$xtensaCompiler = Join-Path $xtensaBin 'xtensa-esp32s3-elf-g++.exe'
try {
    $compilerProbeOutput = & $xtensaCompiler --version 2>&1
    $compilerProbeExitCode = $LASTEXITCODE
    if ($compilerProbeExitCode -ne 0) {
        throw "Kompilator zwrócił kod $compilerProbeExitCode."
    }
}
catch {
    throw "Cannot start ESP32-S3 compiler: $xtensaCompiler. If Windows reports DLL relocation (error 623), run this script from a normal PowerShell terminal outside the restricted sandbox before reinstalling ESP-IDF. Details: $($_.Exception.Message)"
}

Push-Location $project
try {
    Write-Host ('Płytka Waveshare {0}: LCD_CS={1}, CAN={2}, tryb={3}, OBD={4}, logger USB={5}' -f $BoardRevision, $displayCsPin, $CanBusEnabled, $CanBusMode, $ObdPollingEnabled, $LoggerBaudRate) -ForegroundColor Cyan
    Write-Host 'Krok 1/2: walidacja konfiguracji ESPHome...' -ForegroundColor Cyan
    # `esphome config` prints the fully expanded YAML on success. Capture it so
    # the build script stays readable and never dumps resolved settings to the console.
    $previousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        $configOutput = & $esphomePython -m esphome --substitution can_capture_enabled $captureEnabled --substitution display_cs_pin $displayCsPin --substitution can_bus_enabled $CanBusEnabled --substitution can_bus_mode $CanBusMode --substitution can_selftest_interval $CanSelfTestInterval --substitution obd_polling_enabled $ObdPollingEnabled --substitution logger_baud_rate $LoggerBaudRate config audi-dpf-monitor.yaml 2>&1
        $configExitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }
    if ($configExitCode -ne 0) {
        $configOutput |
            Where-Object { [string]$_ -match '(?i)(error|invalid|not defined|failed)' } |
            Select-Object -Last 15 |
            ForEach-Object { Write-Host $_ -ForegroundColor Red }
        throw "Walidacja ESPHome zakończyła się kodem $configExitCode."
    }
    Write-Host 'Konfiguracja ESPHome jest poprawna.' -ForegroundColor Green

    Write-Host 'Krok 2/2: kompilacja firmware...' -ForegroundColor Cyan
    & $esphomePython -m esphome --substitution can_capture_enabled $captureEnabled --substitution display_cs_pin $displayCsPin --substitution can_bus_enabled $CanBusEnabled --substitution can_bus_mode $CanBusMode --substitution can_selftest_interval $CanSelfTestInterval --substitution obd_polling_enabled $ObdPollingEnabled --substitution logger_baud_rate $LoggerBaudRate compile audi-dpf-monitor.yaml
    if ($LASTEXITCODE -ne 0) {
        throw "Kompilacja ESPHome zakończyła się kodem $LASTEXITCODE."
    }

    $revisionFirmwareDirectory = Join-Path $project "firmware\$BoardRevision"
    New-Item -ItemType Directory -Path $revisionFirmwareDirectory -Force | Out-Null
    $artifactSuffix = ''
    if ($PassiveCapture) { $artifactSuffix += '-passive-capture' }
    if ($CanBusEnabled -eq 'false') { $artifactSuffix += '-can-off-test' }
    if ($CanBusMode -eq 'LISTENONLY') { $artifactSuffix += '-can-listenonly-test' }
    if ($CanBusMode -eq 'SELFTEST') { $artifactSuffix += ('-can-selftest-' + $CanSelfTestInterval) }
    if ($ObdPollingEnabled -eq 'false') { $artifactSuffix += '-obd-poll-off-test' }
    if ($LoggerBaudRate -eq '0') { $artifactSuffix += '-logger-off-test' }
    foreach ($artifact in @(
        @{ Source = 'firmware.factory.bin'; Destination = "$projectName-$BoardRevision$artifactSuffix.factory.bin" },
        @{ Source = 'firmware.ota.bin'; Destination = "$projectName-$BoardRevision$artifactSuffix.ota.bin" },
        # ESPHome zawsze zapisuje ELF jako firmware.elf, niezależnie od nazwy urządzenia.
        @{ Source = 'firmware.elf'; Destination = "$projectName-$BoardRevision$artifactSuffix.elf" }
    )) {
        $sourcePath = Join-Path $buildDirectory $artifact.Source
        if (-not (Test-Path -LiteralPath $sourcePath)) {
            throw "Nie znaleziono artefaktu kompilacji: $sourcePath"
        }
        Copy-Item -LiteralPath $sourcePath -Destination (Join-Path $revisionFirmwareDirectory $artifact.Destination) -Force
    }
    Write-Host "Zapisano obrazy rewizji $BoardRevision w firmware\$BoardRevision (CAN=$CanBusEnabled, tryb=$CanBusMode, OBD=$ObdPollingEnabled, logger USB=$LoggerBaudRate)." -ForegroundColor Green
}
finally {
    Pop-Location
    foreach ($name in $environmentNames) {
        [Environment]::SetEnvironmentVariable($name, $oldEnvironment[$name], 'Process')
    }
}
