@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

REM Custom build script (batch file).
REM
REM Usage:
REM    .\build.bat: clean, compile and run
REM    .\build.bat -clean: clean compiled file
REM    .\build.bat -cleanAndCompile: clean compiled file and compile the project
REM    .\build.bat -compile: compile the project
REM    .\build.bat -compileAndRun: compile the project and run the compiled file
REM    .\build.bat -run: run the compiled file
REM
REM Author: Prof. Dr. David Buzatto

PUSHD "%~dp0.."

SET switch=%1
SET currentStep=-1

SET "SrcDir=src"
SET "BuildDir=build"
SET "ScriptsDir=scripts"

SET "BinDir=%BuildDir%\bin"
SET "ObjDir=%BuildDir%\obj"
SET "GeneratedDir=%BuildDir%\generated"

SET "CompiledFile=%BinDir%\aquapura.exe"
SET "BuildResourcesScript=%ScriptsDir%\build-resources.bat"

IF "%switch%"=="" GOTO allSteps
IF "%switch%"=="-clean" GOTO cleanSteps
IF "%switch%"=="-cleanAndCompile" GOTO cleanAndCompileSteps
IF "%switch%"=="-compile" GOTO compileSteps
IF "%switch%"=="-compileAndRun" GOTO compileAndRunSteps
IF "%switch%"=="-run" GOTO runSteps

:allSteps
SET steps[0]=clean
SET steps[1]=compile
SET steps[2]=run
SET steps[3]=end
GOTO nextStep

:cleanSteps
SET steps[0]=clean
SET steps[1]=end
GOTO nextStep

:cleanAndCompileSteps
SET steps[0]=clean
SET steps[1]=compile
SET steps[2]=end
GOTO nextStep

:compileSteps
SET steps[0]=compile
SET steps[1]=end
GOTO nextStep

:compileAndRunSteps
SET steps[0]=compile
SET steps[1]=run
SET steps[2]=end
GOTO nextStep

:runSteps
SET steps[0]=run
SET steps[1]=end
GOTO nextStep

:nextStep
SET /A currentStep=%currentStep%+1
CALL GOTO %%steps[%currentStep%]%%

:clean
ECHO Cleaning...
IF EXIST "%CompiledFile%" DEL "%CompiledFile%"
GOTO nextStep

:compile
ECHO Compiling...

IF NOT EXIST "%ObjDir%" CALL "%BuildResourcesScript%"

IF NOT EXIST "%ObjDir%\windows" MKDIR "%ObjDir%\windows"
windres %SrcDir%\aquapura.rc -o %ObjDir%\windows\aquapura.rc.o --target=pe-x86-64 --codepage=65001

SET SrcFiles=
FOR /R "%SrcDir%" %%F IN (*.c) DO (
    SET "SrcFiles=!SrcFiles! %%F"
)

SET ObjFiles=
FOR /R "%ObjDir%" %%F IN (*.o) DO (
    SET "ObjFiles=!ObjFiles! %%F"
)

IF NOT EXIST "%BinDir%" MKDIR "%BinDir%"

:: --- JULIA DYNAMIC CONFIGURATION ---
ECHO Fetching Julia configuration...
FOR /F "usebackq delims=" %%i IN (`julia -e "print(joinpath(Sys.BINDIR, Base.DATAROOTDIR, \"julia\"))"`) DO SET "JL_SHARE=%%i"

:: ADICIONE ESTA LINHA AQUI PARA PEGAR A PASTA BIN:
FOR /F "usebackq delims=" %%i IN (`julia -e "print(Sys.BINDIR)"`) DO SET "JL_BINDIR=%%i"

FOR /F "usebackq delims=" %%i IN (`julia "!JL_SHARE!\julia-config.jl" --cflags`) DO SET "RAW_JL_CFLAGS=%%i"

FOR /F "usebackq delims=" %%i IN (`julia "!JL_SHARE!\julia-config.jl" --cflags`) DO SET "RAW_JL_CFLAGS=%%i"
FOR /F "usebackq delims=" %%i IN (`julia "!JL_SHARE!\julia-config.jl" --ldflags`) DO SET "RAW_JL_LDFLAGS=%%i"
FOR /F "usebackq delims=" %%i IN (`julia "!JL_SHARE!\julia-config.jl" --ldlibs`) DO SET "RAW_JL_LDLIBS=%%i"
:: Removendo as aspas simples usando Delayed Expansion (!)

SET "JL_CFLAGS=!RAW_JL_CFLAGS:'=!"
SET "JL_LDFLAGS=!RAW_JL_LDFLAGS:'=!"
SET "JL_LDLIBS=!RAW_JL_LDLIBS:'=!"
:: -----------------------------------

ECHO Executing GCC...
gcc !SrcFiles! !ObjFiles! -o "%CompiledFile%" -O2 -Wall -Wextra -Wno-unused-parameter -pedantic-errors -std=c99 -Wno-missing-braces "-I%SrcDir%\include" "-I%GeneratedDir%\include" !JL_CFLAGS! !JL_LDFLAGS! !JL_LDLIBS! -Llib -lraylib -lopengl32 -lgdi32 -lwinmm -lpthread

GOTO nextStep

:run
ECHO Running...

:: Força o Windows a procurar as DLLs da Julia ANTES das do MinGW/GCC
SET "PATH=!JL_BINDIR!;%PATH%"

IF EXIST "%CompiledFile%" ( "%CompiledFile%" ) ELSE ( ECHO "%CompiledFile%" does not exist! )
GOTO nextStep

POPD
ENDLOCAL