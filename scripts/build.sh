#!/bin/bash

# Custom build script (linux shell script).
#
# Usage:
#    ./build.sh: clean, compile and run
#    ./build.sh -clean: clean compiled file
#    ./build.sh -cleanAndCompile: clean compiled file and compile the project
#    ./build.sh -compile: compile the project
#    ./build.sh -compileAndRun: compile the project and run the compiled file
#    ./build.sh -run: run the compiled file
#
# Author: Prof. Dr. David Buzatto

pushd "$(dirname "$0")/.." || exit 1

SrcDir="src"
BuildDir="build"
ScriptsDir="scripts"

BinDir="${BuildDir}/bin"
ObjDir="${BuildDir}/obj"
GeneratedDir="${BuildDir}/generated"

CompiledFile="${BinDir}/aquapura"

clean_project() {
    echo "Cleaning..."
    rm -f $CompiledFile
}

compile_project() {
    echo "Compiling..."

    SrcFiles=$(find "$SrcDir" -name "*.c")
    ObjFiles=$(find "$BuildDir" -path "${ObjDir}/windows/*" -prune -o -name "*.o")

    mkdir -p "$BinDir"

    gcc $SrcFiles $ObjFiles -o "$CompiledFile" \
        -O2 \
        -Wall \
        -Wextra \
        -Wno-unused-parameter \
        -pedantic-errors \
        -std=c99 \
        -Wno-missing-braces \
        "-I${SrcDir}/include/" \
        "-I${BuildDir}/generated/include/" \
        -lraylib \
        -lGL \
        -lm \
        -lpthread \
        -ldl \
        -lrt \
        -lX11
}

run_project() {
    echo "Running..."
    if [ -e "$CompiledFile" ]; then
        ./"$CompiledFile"
    else
        echo "$CompiledFile does not exist!"
    fi
}

if [ $# -eq 0 ]; then
    clean_project
    compile_project
    run_project
fi

if [ "$1" = "-clean" ]; then
    clean_project
fi

if [ "$1" = "-cleanAndCompile" ]; then
    clean_project
    compile_project
fi

if [ "$1" = "-compile" ]; then
    compile_project
fi

if [ "$1" = "-compileAndRun" ]; then
    compile_project
    run_project
fi

if [ "$1" = "-run" ]; then
    run_project
fi

popd