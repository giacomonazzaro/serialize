# make update script 
echo "cp -R $(pwd)/* \$(dirname \$0)" > update.sh
chmod +x update.sh

# make bin directory
mkdir example/bin
mkdir example/output