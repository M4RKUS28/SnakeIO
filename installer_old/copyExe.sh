
echo "Delete old exe File..."
rm ./packages/de.snakeio.main/data/SnakeIO.exe

echo "Copying new exe File..."
cp ../release/SnakeIO.exe ./packages/de.snakeio.main/data/SnakeIO.exe


# Prompt the user to press Enter
read -p "Press Enter to close the terminal"