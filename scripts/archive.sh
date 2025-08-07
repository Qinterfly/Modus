#!/bin/bash

function parseSetCommand(){
    local result=$(cat "$1" | grep "$2" | sed 's/[^0-9]*//g')
    echo $result
}

# Get to the root folder
pathScript=$(dirname "$0")
cd $pathScript/..

# Retrieve the version of the program
projectFile="CMakeLists.txt"
varNameVersion="PROJECT_VERSION"
versionMajor=$(parseSetCommand $projectFile "VERSION_MAJOR")
versionMinor=$(parseSetCommand $projectFile "VERSION_MINOR")
versionPatch=$(parseSetCommand $projectFile "VERSION_PATCH")
version="$versionMajor.$versionMinor.$versionPatch"
echo $version

# Create a list of files to archive
listArchive=$(ls -a --ignore=. --ignore=.. | grep -Ev "archive|build|releases")

# Create an archive
dirArchive="archive"
tempNameArchive="tempArchive.zip"
echo $listArchive | xargs zip -r "$dirArchive/$tempNameArchive" 

# Set the archive name
cd $dirArchive
nameArchive="$(date +"%Y.%m.%d") - v$version.zip"
if [[ ! -f $nameArchive ]]; then
    mv $tempNameArchive "$nameArchive"
    echo "$nameArchive was successfully created"
else
    echo "$nameArchive already exists. The name of the created archive is $tempNameArchive"
fi