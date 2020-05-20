sudo apt install -y cmake
# Install Qt dependencies
sudo apt install -y git g++ libgl1-mesa-dev
# Install LinuxDeployQt dependencies
sudo apt install -y libicu-dev

# Download linux deploy QT
cd ..  
git clone  https://github.com/probonopd/linuxdeployqt

# Get elf patcher, needed by linux deploy Qt
wget https://nixos.org/releases/patchelf/patchelf-0.9/patchelf-0.9.tar.bz2
tar xf patchelf-0.9.tar.bz2
( cd patchelf-0.9/ && ./configure  && make && sudo make install )
rm patchelf-0.9.tar.bz2
sudo wget -c "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" -O /usr/local/bin/appimagetool
sudo chmod a+x /usr/local/bin/appimagetool
cd linuxdeployqt
red=`tput setaf 1`
echo -e "${red}All linuxdeployqt dependencies have been collected."
echo -e "${red}To build linuxdeployqt, open the directory's .pro file in Qt Creator."
echo -e "${red}For each Qt build kit that you would like to enable linuxdeployqt on,"
echo -e "${red}build the project in Qt creator, and then open the build directory in the terminal."
echo -e "${red}In the terminal, execute \"sudo make install\"."
