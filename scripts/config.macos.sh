#!/bin/bash
set -e
#initialize sudo so that no password entering during installation
sudo -v

#install most libraries and client through brew
brew install cmake git wget openssl@1.1 libpng yaml-cpp freetype rapidjson libevent pcre2

echo downloading and compiling pngwriter
git clone https://github.com/pngwriter/pngwriter
cd pngwriter
cmake .
sudo make install -j8
#fix install error
#sudo install libPNGwriter.a /usr/local/lib/
cd ..

echo all done!
