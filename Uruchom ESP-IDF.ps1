$project = 'C:\Users\Administrator\Documents\ChatGPT\Samochodowy Monitor DPF'
$toolRoot = 'C:\Users\Administrator\Documents\Codex\2026-10-01\c'
$buildCache = Join-Path $project '.esphome-data\build\audi-dpf-monitor\build\CMakeCache.txt'
$cachedIdfRoot = $null
if (Test-Path -LiteralPath $buildCache) {
    $cachedPythonLine = Select-String -LiteralPath $buildCache -Pattern '^PYTHON:UNINITIALIZED=(.+)$' |
        Select-Object -First 1
    if ($cachedPythonLine -and $cachedPythonLine.Matches[0].Groups[1].Value -match '^(.*)\\penvs\\5\.5\.5\\Scripts\\python\.exe$') {
        $cachedIdfRoot = $Matches[1]
    }
}
$idfRoot = if ($cachedIdfRoot -and (Test-Path -LiteralPath (Join-Path $cachedIdfRoot 'frameworks\5.5.5'))) {
    $cachedIdfRoot
} elseif (Test-Path -LiteralPath 'C:\Espressif\frameworks\5.5.5') {
    'C:\Espressif'
} else {
    Join-Path $project '.esphome-idf'
}
$env:ESPHOME_DATA_DIR = Join-Path $project '.esphome-data'
$env:ESPHOME_ESP_IDF_PREFIX = $idfRoot
$env:IDF_PATH = Join-Path $idfRoot 'frameworks\5.5.5'
$env:IDF_TOOLS_PATH = $idfRoot
$env:IDF_PYTHON_ENV_PATH = Join-Path $idfRoot 'penvs\5.5.5'
$env:PATH = (Join-Path $toolRoot '.venv\Scripts') + ';' + $env:PATH
Set-Location $project
Write-Host 'ESP-IDF is managed by ESPHome for this project.' -ForegroundColor Cyan
Write-Host 'Project folder:' $project
Write-Host 'ESP-IDF root:' $idfRoot
Write-Host 'To compile and install/check the ESP-IDF environment, run:' -ForegroundColor Yellow
Write-Host "& '$toolRoot\.venv\Scripts\esphome.exe' compile audi-dpf-monitor.yaml"
Write-Host 'The shell is ready. Type exit to close it.'
