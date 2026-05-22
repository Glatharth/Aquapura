@ECHO OFF

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

SET switch=%1
SHIFT

SET currentStep=-1

SETLOCAL ENABLEDELAYEDEXPANSION

REM Source and output directories
SET "SourceDirectory=resources"
SET "OutputDirectory=build\obj"

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
IF EXIST %OutputDirectory% rmdir /s /q %OutputDirectory%
GOTO nextStep

:compile
ECHO Compiling...

REM Convert resources to objects
FOR /R "%SourceDirectory%" %%F IN (*.png *.mp3) DO (
    REM Absolute path of file
    SET "SourcePath=%%F"

    REM Remove the absolute path so only the relative path remains
    SET "SourcePath=!SourcePath:%CD%\=!"

    REM Replicate the source structure on the output path
    SET "OutputPath=!SourcePath:%SourceDirectory%\=!"
    SET "OutputPath=%OutputDirectory%\!OutputPath!.o"

    REM Ensures that the target directory exists
    FOR %%D IN ("!OutputPath!") DO IF NOT EXIST "%%~DPD" MKDIR "%%~DPD"

    ECHO Converting !SourcePath!...
    ld -r -b binary -o "!OutputPath!" "!SourcePath!"
)

GOTO nextStep

:generateHeader
ECHO Generating header...

SET HeaderFile="src\include\resources.h"

ECHO //Automatically generated with build-resources.bat > %HeaderFile%
(
    ECHO(
    ECHO #ifndef RESOURCES_H
    ECHO #define RESOURCES_H
    ECHO(
) >> %HeaderFile%

FOR /R %SourceDirectory% %%F IN (*.png *.mp3) DO (
    REM Full path of file
    SET "File=%%F"

    REM Remove the source path so only the relative path remains
    SET "RelativePath=!File:%CD%\%SourceDirectory%\=!"

    REM Replace \ and . with _
    SET "RelativePath=!RelativePath:\=_!"
    SET "RelativePath=!RelativePath:.=_!"

    REM Create symbol based on the relative path of the file
    SET "Symbol=_binary_resources_!RelativePath!"

    (
        ECHO extern const unsigned char !Symbol!_start[];
        ECHO extern const unsigned char !Symbol!_end[];
        ECHO(
    ) >> %HeaderFile%
%
)

ECHO #endif >> %HeaderFile%
GOTO nextStep

:end

ENDLOCAL