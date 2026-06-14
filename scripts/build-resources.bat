@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

REM Custom build script (batch file).
REM
REM Usage:
REM    .\build-resources.bat: clean, compile and generate header
REM    .\build-resources.bat -clean: clean compiled file
REM    .\build-resources.bat -cleanAndCompile: clean compiled files and compile the resources
REM    .\build-resources.bat -compile: compile the resources
REM    .\build-resources.bat -generateHeader: generate header based on the resources' filenames
REM
REM Based on Prof. Dr. David Buzatto's work

PUSHD "%~dp0.."

SET switch=%1
SET currentStep=-1

REM Source and output directories
SET "ResourceDir=resources"
SET "BuildDir=build"

SET "ObjDir=%BuildDir%\obj"
SET "GeneratedDir=%BuildDir%\generated"

IF "%switch%"=="" GOTO allSteps
IF "%switch%"=="-clean" GOTO cleanSteps
IF "%switch%"=="-cleanAndCompile" GOTO cleanAndCompileSteps
IF "%switch%"=="-compile" GOTO compileSteps
IF "%switch%"=="-generateHeader" GOTO generateHeaderSteps

:allSteps
SET steps[0]=clean
SET steps[1]=compile
SET steps[2]=generateHeader
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

:generateHeaderSteps
SET steps[0]=generateHeader
SET steps[1]=end
GOTO nextStep

:nextStep
SET /A currentStep=%currentStep%+1
CALL GOTO %%steps[%currentStep%]%%

:clean
ECHO Cleaning...
IF EXIST "%ObjDir%" rmdir /s /q "%ObjDir%"
GOTO nextStep

:compile
ECHO Compiling...

REM Convert resources to objects
FOR /R "%ResourceDir%" %%F IN (*.png *.mp3) DO (
    REM Absolute path of file
    SET "InputFile=%%F"

    REM Remove the absolute path so only the relative path remains
    SET "InputFile=!InputFile:%CD%\=!"

    REM Replicate the source structure on the output path
    SET "OutputFile=!InputFile:%ResourceDir%\=!"
    SET "OutputFile=%ObjDir%\!OutputFile!.o"

    REM Ensures that the target directory exists
    FOR %%D IN ("!OutputFile!") DO IF NOT EXIST "%%~DPD" MKDIR "%%~DPD"

    ECHO Converting !InputFile!...
    ld -r -b binary -o "!OutputFile!" "!InputFile!"
)

GOTO nextStep

:generateHeader
ECHO Generating header...

SET "HeaderDir=%GeneratedDir%\include"
SET "HeaderFile=%HeaderDir%\resources.h"
IF NOT EXIST "%HeaderDir%" MKDIR "%HeaderDir%"

ECHO //Automatically generated with build-resources.bat > "%HeaderFile%"
(
    ECHO(
    ECHO #ifndef RESOURCES_H
    ECHO #define RESOURCES_H
    ECHO(
) >> "%HeaderFile%"

FOR /R %ResourceDir% %%F IN (*.png *.mp3) DO (
    REM Full path of file
    SET "File=%%F"

    REM Remove the source path so only the relative path remains
    SET "RelativePath=!File:%CD%\%ResourceDir%\=!"

    REM Replace \ and . with _
    SET "RelativePath=!RelativePath:\=_!"
    SET "RelativePath=!RelativePath:.=_!"

    REM Create symbol based on the relative path of the file
    SET "Symbol=_binary_resources_!RelativePath!"

    (
        ECHO extern const unsigned char !Symbol!_start[];
        ECHO extern const unsigned char !Symbol!_end[];
        ECHO(
    ) >> "%HeaderFile%"
)

ECHO #endif >> "%HeaderFile%"
GOTO nextStep

:end

POPD
ENDLOCAL