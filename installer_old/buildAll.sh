#!/bin/bash


qt_path=""
installer_version="4.6"


if [ -d "/C/Qt/" ]; then
	qt_path="/C/Qt/"
else
	qt_path="/D/QtNeu/"
fi
echo "Qt Directory: $qt_path"

# Start program 1 in the background
echo "Starting creating online installer..."
${qt_path}Tools/QtInstallerFramework/$installer_version/bin/binarycreator.exe --online-only -c config/config.xml -p packages SnakeIO-OnlineInstaller.exe &

# Start program 2 in the background
echo "Starting creating offline installer..."
${qt_path}Tools/QtInstallerFramework/$installer_version/bin/binarycreator.exe --offline-only -c config/config.xml -p packages SnakeIO-OfflineInstaller.exe &


if [ -d "./repository" ]; then
	echo "Staring updating repository...."
	${qt_path}Tools/QtInstallerFramework/$installer_version/bin/repogen.exe --update -p packages repository
else
	echo "Staring creating repository...."
	${qt_path}Tools/QtInstallerFramework/$installer_version/bin/repogen.exe -p packages repository
fi

# Wait for program 1 and program 2 to finish
wait

#update git repo...
echo "Updating git repo..."

echo "	>>	git add ./repository/"
sleep 1
git add ./repository/

echo "	>>	git commit -m \"update repository\""
sleep 1
git commit -m "update repository"

echo "	>>	git push"
sleep 1
git push


# Continue with the rest of your script after program 1 and program 2 have finished
echo "Finished!"


# Prompt the user to press Enter
read -p "Press Enter to close the terminal"