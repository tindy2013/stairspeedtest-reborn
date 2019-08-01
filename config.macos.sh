#!/bin/bash
set -e
#initialize sudo so that no password entering during installation
sudo -v

echo installing brew
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

echo preconfigure compiler
brew install gcc

#don't remove these for non-xcode user
#cd /usr/bin
#sudo rm -f g++ gcc c++ cpp cc
#sudo ln -s /usr/local/Cellar/gcc/9.1.0/bin/c++-9 /usr/bin/c++
#sudo ln -s /usr/local/Cellar/gcc/9.1.0/bin/cpp-9 /usr/bin/cpp
#sudo ln -s /usr/local/Cellar/gcc/9.1.0/bin/g++-9 /usr/bin/g++
#sudo ln -s /usr/local/Cellar/gcc/9.1.0/bin/gcc-9 /usr/bin/gcc
#sudo ln -s /usr/local/Cellar/gcc/9.1.0/bin/gcc-9 /usr/bin/cc
#fix for cmake which uses a different path
#sudo rm -f g++ gcc c++ cpp cc
#sudo ln -s /usr/local/Cellar/gcc/9.1.0/bin/c++-9 /Library/Developer/CommandLineTools/usr/bin/c++
#sudo ln -s /usr/local/Cellar/gcc/9.1.0/bin/cpp-9 /Library/Developer/CommandLineTools/usr/bin/cpp
#sudo ln -s /usr/local/Cellar/gcc/9.1.0/bin/g++-9 /Library/Developer/CommandLineTools/usr/bin/g++
#sudo ln -s /usr/local/Cellar/gcc/9.1.0/bin/gcc-9 /Library/Developer/CommandLineTools/usr/bin/gcc
#sudo ln -s /usr/local/Cellar/gcc/9.1.0/bin/gcc-9 /Library/Developer/CommandLineTools/usr/bin/cc

echo installing tools with brew
brew install wget cmake zlib

echo downloading and compiling openssl-1.1.1c
wget https://www.openssl.org/source/openssl-1.1.1c.tar.gz
tar xvf openssl-1.1.1c.tar.gz
cd openssl-1.1.1c
#override compiler
CC=gcc ./config --prefix=/usr/local
make -j8
#don't install docs since we don't need them
sudo make install_sw -j8
#fix install error
sudo install libssl.a /usr/local/lib/
sudo install libssl.dylib /usr/local/lib/
sudo install libcrypto.a /usr/local/lib/
sudo install libcrypto.dylib /usr/local/lib/
cd ..

#using system-default libcurl
#echo downloading and compiling curl-7.65.1
#wget https://curl.haxx.se/download/curl-7.65.1.tar.gz
#tar xvf curl-7.65.1.tar.gz
#cd curl-7.65.1
#./configure --disable-shared --disable-ldap --disable-ldaps --disable-smtp --disable-pop3 --disable-rtmp --disable-imap --disable-gopher --disable-telnet --disable-tftp
#make -j8
#sudo make install -j8
#cd ..

echo downloading and compiling libpng-1.6.37
wget https://nchc.dl.sourceforge.net/project/libpng/libpng16/1.6.37/libpng-1.6.37.tar.gz
tar xvf libpng-1.6.37.tar.gz
cd libpng-1.6.37
./configure
make -j8
sudo make install -j8
cd ..

echo downloading and compiling freetype-2.10.1
wget https://nchc.dl.sourceforge.net/project/freetype/freetype2/2.10.1/freetype-2.10.1.tar.xz
tar xvf freetype-2.10.1.tar.xz
cd freetype-2.10.1
./configure
make -j8
sudo make install -j8
#a fix for incorrect file path
sudo mv /usr/local/include/freetype2/* /usr/local/include
sudo rmdir /usr/local/include/freetype2
cd ..

echo downloading and compiling pngwriter
git clone https://github.com/pngwriter/pngwriter
cd pngwriter
cmake .
make -j8
sudo make install -j8
#fix install error
#sudo install libPNGwriter.a /usr/local/lib/
cd ..

echo downloading and installing rapidjson
git clone https://github.com/tencent/rapidjson
cd rapidjson
cmake .
make -j8
sudo make install -j8
cd ..

#install ss-local from brew
echo installing shadowsocks-libev from brew
brew install shadowsocks-libev

#extra tools for ssr-local
#but ss-libev has already covered these

#brew install asciidoc xmlto pcre

#use brew version is okay
#echo downloading and compiling pcre-8.43
#wget https://ftp.pcre.org/pub/pcre/pcre-8.43.tar.gz
#tar xvf pcre-8.43.tar.gz
#cd pcre-8.43
#./configure
#make -j8
#sudo make install -j8
#cd ..

echo downloading and compiling shadowsocksr-libev
git clone https://github.com/shadowsocksrr/shadowsocksr-libev
cd shadowsocksr-libev
#override some warning settings
CFLAGS+="-Wno-format-overflow -Wno-format-truncation -Wno-sizeof-pointer-memaccess" ./configure --with-openssl-lib=/usr/local/lib --with-openssl-include=/usr/local/include --disable-documentation
make -j8
#will not directly install this one, use another method
sudo install src/ss-local /usr/local/bin/ssr-local
cd ..

echo installing v2ray from brew
brew tap v2ray/v2ray
brew install v2ray-core

echo all done!
