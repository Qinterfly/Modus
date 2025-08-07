#!/bin/bash

# Set the optinos
buildDir="../KC-L/install"
installDir="./lib/kcl"
keyword="\-cpp"

# Remove previously installed builds
echo "Cleaning the installation directory..."
if [ -d "$installDir" ]; then
	files=($(ls $installDir -I CMakeLists.txt))
	if [ -n "$files" ]; then
		files=("${files[@]/#/$installDir/}")
		rm -r "${files[@]}"
	fi
else
	mkdir $installDir
fi

# Copy all the builds
names=($(ls $buildDir | grep $keyword))
for name in "${names[@]}"; do
	# Create the destination directory
	targetName="${name//$keyword/}"
	targetDir="$installDir/$targetName"
	mkdir $targetDir
	echo "Installing $targetName"
	
	# Copy compiled files
	find "$buildDir/$name" -maxdepth 1 -type f -exec cp "{}" $targetDir \;
	
	# Copy header files
	find "$buildDir/$name/include" -type f -exec cp "{}" $installDir \;
done
echo "Installation completed"
