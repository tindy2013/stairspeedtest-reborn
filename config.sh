#!/bin/bash
set -e
echo prep up basic env
apt update -y
apt upgrade -y

echo installing toolchain
apt install g++ libssl-dev git wget curl make cmake -y

echo downloading and compiling curl-7.65.1
wget https://curl.haxx.se/download/curl-7.65.1.tar.gz
tar xvf curl-7.65.1.tar.gz
cd curl-7.65.1
./configure --with-ssl --disable-shared
make install -j8
cd ..

echo downloading and compiling zlib-1.2.11
wget http://www.zlib.net/zlib-1.2.11.tar.gz
tar xvf zlib-1.2.11.tar.gz
cd zlib-1.2.11
./configure
make install -j8
cd ..

echo downloading and compiling libpng-1.6.37
wget https://nchc.dl.sourceforge.net/project/libpng/libpng16/1.6.37/libpng-1.6.37.tar.gz
tar xvf libpng-1.6.37.tar.gz
cd libpng-1.6.37
./configure
make install -j8
cd ..

echo downloading and compiling freetype-2.10.1
wget https://nchc.dl.sourceforge.net/project/freetype/freetype2/2.10.1/freetype-2.10.1.tar.xz
tar xvf freetype-2.10.1.tar.xz
cd freetype-2.10.1
./configure
make install -j8
#fix incorrect path
mv /usr/local/include/freetype2/* /usr/local/include/
rm -r /usr/local/include/freetype2
cd ..

echo downloading and compiling pngwriter
git clone https://github.com/pngwriter/pngwriter
cd pngwriter
cmake .
make install -j8
cd ..

echo downloading and installing rapidjson
git clone https://github.com/tencent/rapidjson
cd rapidjson
cmake .
make install -j8
cd ..

echo downloading and compiling yaml-cpp
git clone https://github.com/jbeder/yaml-cpp
cd yaml-cpp
cmake .
make install -j8
cd ..

echo installing shadowsocks-libev
apt install shadowsocks-libev -y

echo downloading and compiling shadowsocksr-libev
git clone https://github.com/shadowsocksrr/shadowsocksr-libev
cd shadowsocksr-libev
git checkout -b latest origin/Akkariiin/develop
./autogen.sh
#override some warning settings
CFLAGS+="-Wno-format-overflow -Wno-format-truncation -Wno-sizeof-pointer-memaccess" ./configure --with-openssl-lib=/usr/local/lib --with-openssl-include=/usr/local/include --disable-documentation
make -j8
#will not directly install this one, use another method
sudo install src/ss-local /usr/local/bin/ssr-local
cd ..

echo installing v2ray
bash <(curl -Ls https://install.direct/go.sh)
#make some link
sudo ln -s /usr/bin/v2ray/v2ray /usr/local/bin/v2ray
sudo ln -s /usr/bin/v2ray/v2ctl /usr/local/bin/v2ctl

echo all done!