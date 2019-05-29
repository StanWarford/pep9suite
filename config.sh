sudo apt install -y cmake
sudo apt-get -y install git g++ libgl1-mesa-dev
cd ..  
git clone  https://github.com/probonopd/linuxdeployqt
wget https://nixos.org/releases/patchelf/patchelf-0.9/patchelf-0.9.tar.bz2
tar xf patchelf-0.9.tar.bz2
( cd patchelf-0.9/ && ./configure  && make && sudo make install )
rm patchelf-0.9.tar.bz2
sudo wget -c "https://github.com/AppImage/AppImageKit/releases/download/continu$
sudo chmod a+x /usr/local/bin/appimagetool
cd linuxdeployqt
echo -e "$(tput setaf 1)All linuxdeployqt dependencies have been collected.\n" \
        "To build linuxdeployqt, open the directory's .pro file in Qt Creator.\$
        "For each Qt build kit that you would like to enable linuxdeployqt on,\$
        "build the project in Qt creator, and then open the build directory in $
        "In the terminal, execute \"sudo make install\"."
