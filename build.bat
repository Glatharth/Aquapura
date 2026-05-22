@ECHO OFF

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

SET switch=%1
SHIFT

SET currentStep=-1

SET CompiledFile=build\aquapura.exe

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
IF EXIST %CompiledFile% DEL %CompiledFile%
GOTO nextStep

:compile
ECHO Compiling...
windres src/aquapura.rc -o build/obj/aquapura.rc.o --target=pe-x86-64 --codepage=65001

@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

SET SourceFiles=
FOR /R "src\" %%F IN (*.c) DO (
    SET SourceFiles=!SourceFiles! %%F
)

SET ObjFiles=
FOR /R "build\obj\" %%F IN (*.o) DO (
    SET ObjFiles=!ObjFiles! %%F
)

gcc !SourceFiles! !ObjFiles! -o %CompiledFile% -O2 -Wall -Wextra -Wno-unused-parameter -pedantic-errors -std=c99 -Wno-missing-braces -I src/include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows

ENDLOCAL
GOTO nextStep

:run
ECHO Running...
IF EXIST %CompiledFile% ( %CompiledFile% ) ELSE ( ECHO %CompiledFile%% does not exists! )
GOTO nextStep

:end