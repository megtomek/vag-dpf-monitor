# Uruchom ten skrypt w PowerShellu, aby utworzyć skrót na Pulpicie.
# Sam skrót otwiera PowerShell przygotowany do pracy z ESPHome/ESP-IDF.

$project = 'C:\Users\Administrator\Documents\ChatGPT\Samochodowy Monitor DPF'
$launcher = Join-Path $project 'Uruchom ESP-IDF.ps1'
$desktop = [Environment]::GetFolderPath('Desktop')
$shortcutPath = Join-Path $desktop 'Uruchom ESP-IDF.lnk'
$powershell = Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe'

if (-not (Test-Path -LiteralPath $launcher)) {
    throw "Nie znaleziono skryptu uruchamiającego: $launcher"
}

$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut($shortcutPath)
$shortcut.TargetPath = $powershell
$shortcut.Arguments = "-NoExit -ExecutionPolicy Bypass -File `"$launcher`""
$shortcut.WorkingDirectory = $project
$shortcut.Description = 'Otwiera PowerShell przygotowany do kompilacji projektu ESPHome/ESP-IDF.'
$shortcut.IconLocation = "$powershell,0"
$shortcut.Save()

Write-Host "Utworzono skrót: $shortcutPath" -ForegroundColor Green
