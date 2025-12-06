###############################################################################
# Unreal Engine Plugin Setup Script (Windows Only)
# Clean, ASCII-only, Compatible with all PowerShell encodings
###############################################################################

param(
    [string]$UEPathOverride
)

# Ensure script runs from project root
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Set-Location "$ScriptDir/.."


###############################################################################
# CONFIGURATION
###############################################################################

# Unreal Engine version to search for on the system
$UEVersion = "UE_5.7"

# UE Plugins (paths inside Engine installation)
$Plugins = @(
    "Engine/Plugins/Animation/PoseSearch",
    "Engine/Plugins/Experimental/ChaosMover",
    "Engine/Plugins/Runtime/DataRegistry",
    "Engine/Plugins/Chooser",
    "Engine/Plugins/Editor/GameplayTagsEditor",
    "Engine/Plugins/Runtime/MassGameplay",
    "Engine/Plugins/Runtime/SmartObjects",
    "Engine/Plugins/Experimental/GameplayTargetingSystem",
    "Engine/Plugins/Experimental/Mover",
    "Engine/Plugins/Experimental/MoverExamples",
    "Engine/Plugins/Experimental/MoverIntegrations"
)

# GAS-NPP
$GasRepo = "https://github.com/Sabri-Kai/GAS-NPP-Simulation.git"
$GasBranch = "GAS-NPP-Simulation-5.7"
$GasPath = "External/GAS-NPP-Simulation"
$GasPluginSource = "$GasPath/AbilitySystemSimulation"
$GasPluginDestination = "Plugins/AbilitySystemSimulation"

# UnrealEngineNPP
$NppRepo = "https://github.com/Sabri-Kai/UnrealEngineNPP.git"
$NppBranch = "GAS-NPP-5.7"
$NppPath = "External/UnrealEngineNPP"
$NppPlugins = @(
    "GameplayAbilities",
    "NetworkPrediction",
    "NetworkPredictionExtras",
    "NetworkPredictionInsights"
)


###############################################################################
# LOCATE UNREAL ENGINE INSTALLATION
###############################################################################

Write-Host ""
Write-Host "=== Locating Unreal Engine installation ($UEVersion) ==="

$EnginePaths = @()

# If override provided, use it
if ($UEPathOverride) {

    if (Test-Path $UEPathOverride) {
        $EnginePaths += $UEPathOverride
    }
    else {
        Write-Error "Override path not found: $UEPathOverride"
        exit 1
    }
}
else {

    # Auto-discover across all fixed drives
    $drives = Get-PSDrive -PSProvider FileSystem |
              Where-Object { $_.DisplayRoot -eq $null -and $_.Free -gt 0 }

    foreach ($drive in $drives) {

        $root = $drive.Root

        # Launcher install
        $LauncherPath = Join-Path $root "Program Files\Epic Games\$UEVersion"
        if (Test-Path $LauncherPath) {
            $EnginePaths += $LauncherPath
        }

        # Source builds under UnrealEngine directory
        $SourceRoot = Join-Path $root "UnrealEngine"
        if (Test-Path $SourceRoot) {
            $matches = Get-ChildItem $SourceRoot -Directory -Recurse -ErrorAction SilentlyContinue |
                       Where-Object { $_.Name -eq $UEVersion }

            foreach ($match in $matches) {
                $EnginePaths += $match.FullName
            }
        }
    }

    if ($EnginePaths.Count -eq 0) {
        Write-Host ""
        Write-Host "ERROR: Could not locate Unreal Engine version $UEVersion on any drive."
        Write-Host ""
        Write-Host "You can specify the engine path manually:"
        Write-Host "    .\plugin-setup.ps1 -UEPathOverride ""D:\UE_5.7"""
        Write-Host ""
        exit 1
    }
}

Write-Host ""
Write-Host "Found Unreal Engine installation(s):"
foreach ($found in $EnginePaths) {
    Write-Host " - $found"
}
Write-Host ""


###############################################################################
# COPY UE PLUGINS FROM LOCAL ENGINE INSTALL
###############################################################################

Write-Host ""
Write-Host "=== Copying Unreal Engine plugins from local installation ==="

foreach ($PluginPath in $Plugins) {

    $PluginName = Split-Path $PluginPath -Leaf
    Write-Host ""
    Write-Host "Processing plugin: $PluginName"

    $PluginSource = $null

    # Try to find plugin in each engine installation found
    foreach ($EnginePath in $EnginePaths) {

        $Candidate = Join-Path $EnginePath $PluginPath

        if (Test-Path $Candidate) {
            $PluginSource = $Candidate
            break
        }
    }

    if (-not $PluginSource) {
        Write-Host "Plugin not found: $PluginName"
        continue
    }

    $PluginDest = "Plugins/$PluginName"

    # Remove existing version
    if (Test-Path $PluginDest) {
        Remove-Item -Recurse -Force $PluginDest
    }

    # Copy entire folder (including C++, Content, Config, Resources)
    Write-Host "Copying plugin folder to: $PluginDest"
    Copy-Item -Recurse -Force $PluginSource $PluginDest
}

Write-Host ""
Write-Host "=== Finished copying built-in UE plugins ==="


###############################################################################
# FETCH GAS-NPP-Simulation
###############################################################################

Write-Host ""
Write-Host "=== Fetching GAS-NPP-Simulation plugin ==="

if (-not (Test-Path $GasPath)) {
    git clone `
        --depth 1 `
        --branch $GasBranch `
        $GasRepo `
        $GasPath
}

if (Test-Path $GasPluginDestination) {
    Remove-Item -Recurse -Force $GasPluginDestination
}

Copy-Item -Recurse -Force $GasPluginSource $GasPluginDestination


###############################################################################
# FETCH UnrealEngineNPP
###############################################################################

Write-Host ""
Write-Host "=== Fetching UnrealEngineNPP plugins ==="

if (-not (Test-Path $NppPath)) {
    git clone `
        --depth 1 `
        --branch $NppBranch `
        $NppRepo `
        $NppPath
}

foreach ($PluginName in $NppPlugins) {

    $Source = "$NppPath/$PluginName"
    $Destination = "Plugins/$PluginName"

    Write-Host "Copying $PluginName into project Plugins folder..."

    if (Test-Path $Destination) {
        Remove-Item -Recurse -Force $Destination
    }

    Copy-Item -Recurse -Force $Source $Destination
}

Write-Host ""
Write-Host "=== UnrealEngineNPP plugins installed ==="
Write-Host ""
Write-Host "=== ALL PLUGINS INSTALLED SUCCESSFULLY ==="
Write-Host ""
