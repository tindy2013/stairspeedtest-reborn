#!/bin/bash
set -e
#initialize sudo so that no password entering during installation
sudo -v

echo installing brew
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

#install most libraries and client through brew
brew install cmake git wget openssl@1.1 libpng yaml-cpp freetype rapidjson shadowsocks-libev

echo downloading and compiling pngwriter
git clone https://github.com/pngwriter/pngwriter
cd pngwriter
cmake .
sudo make install -j8
#fix install error
#sudo install libPNGwriter.a /usr/local/lib/
cd ..

echo downloading and compiling shadowsocksr-libev
git clone https://github.com/shadowsocksrr/shadowsocksr-libev
cd shadowsocksr-libev
git checkout -b latest origin/Akkariiin/develop
./configure --with-openssl-lib=/usr/local/opt/openssl/lib --with-openssl-include=/usr/local/opt/openssl/include --disable-documentation
make -j8
#fix dynamic lib reference error
install_name_tool -change @rpath/libssl.1.1.dylib /usr/local/opt/openssl@1.1/lib/libssl.1.1.dylib src/ss-local
install_name_tool -change @rpath/libcrypto.1.1.dylib /usr/local/opt/openssl@1.1/lib/libcrypto.1.1.dylib src/ss-local
#will not directly install this one, use another method
sudo install src/ss-local /usr/local/bin/ssr-local
cd ..

echo installing v2ray from brew
brew tap v2ray/v2ray
brew install v2ray-core

echo all done!
