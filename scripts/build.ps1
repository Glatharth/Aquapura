# Custom build script (windows powershell).
#
# Usage:
#    .\build.ps1: clean, compile and run
#    .\build.ps1 -clean: clean compiled file
#    .\build.ps1 -cleanAndCompile: clean compiled file and compile the project
#    .\build.ps1 -compile: compile the project
#    .\build.ps1 -compileAndRun: compile the project and run the compiled file
#    .\build.ps1 -run: run the compiled file
#
# Author: Prof. Dr. David Buzatto

param(
    [switch]$clean,
    [switch]$cleanAndCompile,
    [switch]$compile,
    [switch]$compileAndRun,
    [switch]$run
);

Push-Location "$PSScriptRoot\.."
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope Process

$SrcDir="src"
$BuildDir="build"
$ScriptsDir="scripts"

$BinDir="${BuildDir}\bin"
$ObjDir="${BuildDir}\obj"
$GeneratedDir="${BuildDir}\generated"

$CompiledFile="${BinDir}\aquapura.exe"
$BuildResourcesScript="${ScriptsDir}\build-resources.bat"

$all = $false
if ( -not( $clean -or $cleanAndCompile -or $compile -or $compileAndRun -or $run ) ) {
    $all = $true
}

# clean
if ( $clean -or $cleanAndCompile -or $all ) {
    Write-Host "Cleaning..."
    if ( Test-Path $CompiledFile ) {
        Remove-Item $CompiledFile
    }
}

# compile
if ( $compile -or $cleanAndCompile -or $compileAndRun -or $all ) {
    Write-Host "Compiling..."

    if ( -not (Test-Path "$ObjDir") ) {
        & .\"$BuildResourcesScript"
    }

    if ( -not (Test-Path "$ObjDir\windows") ) {
        New-Item -ItemType Directory -Force -Path "$ObjDir\windows"
    }

    windres "${SrcDir}\aquapura.rc" -o "${ObjDir}\windows\aquapura.rc.o" --target=pe-x86-64 --codepage=65001

    $SrcFiles = Get-ChildItem -Path "$SrcDir" -Filter "*.c" -Recurse | ForEach-Object { $_.FullName }
    $ObjFiles = Get-ChildItem -Path "$ObjDir" -Filter "*.o" -Recurse | ForEach-Object { $_.FullName }

    if ( -not (Test-Path $BinDir) ) {
        New-Item -ItemType Directory -Force -Path "$BinDir"
    }

    gcc $SrcFiles $ObjFiles -o "$CompiledFile" `
        -O1 `
        -Wall `
        -Wextra `
        -Wno-unused-parameter `
        -pedantic-errors `
        -std=c99 `
        -Wno-missing-braces `
        "-I${SrcDir}\include" `
        "-I${GeneratedDir}\include" `
        -Llib `
        -lraylib `
        -lopengl32 `
        -lgdi32 `
        -lwinmm `
        -mwindows
}

# run
if ( $run -or $compileAndRun -or $all ) {
    Write-Host "Running..."
    if ( Test-Path "$CompiledFile" ) {
        & .\"$CompiledFile"
    } else {
        Write-Host "$CompiledFile does not exist!"
    }
}

Pop-Location