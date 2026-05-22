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

$CompiledFile = "build\aquapura.exe"

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

    windres "src\aquapura.rc" -o "build\obj\aquapura.rc.o" --target=pe-x86-64 --codepage=65001

    $SourceFiles = Get-ChildItem -Path "src" -Filter "*.c" -Recurse | ForEach-Object { $_.FullName }
    $ObjFiles = Get-ChildItem -Path "build\obj" -Filter "*.o" -Recurse | ForEach-Object { $_.FullName }

    gcc $SourceFiles $ObjFiles -o $CompiledFile `
        -O1 `
        -Wall `
        -Wextra `
        -Wno-unused-parameter `
        -pedantic-errors `
        -std=c99 `
        -Wno-missing-braces `
        -I src/include/ `
        -L lib/ `
        -lraylib `
        -lopengl32 `
        -lgdi32 `
        -lwinmm `
        -mwindows
}

# run
if ( $run -or $compileAndRun -or $all ) {
    Write-Host "Running..."
    if ( Test-Path $CompiledFile ) {
        & .\$CompiledFile
    } else {
        Write-Host "$CompiledFile does not exist!"
    }
}