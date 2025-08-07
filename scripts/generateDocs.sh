#!/bin/bash

basePath=$(pwd)
scriptPath=$(dirname "$0")

cd $scriptPath

# Generate a documentation based on the configuration file
cd ../doc
doxygen config

# Show the resulted documentation
firefox html/annotated.html

# Return to the base directory
cd $basePath
