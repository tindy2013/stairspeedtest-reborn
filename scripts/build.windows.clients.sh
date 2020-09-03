#!/bin/bash
set -xe
mkdir base/tools/clients

curl -LO https://github.com/shadowsocks/libev/archive/mingw.tar.gz
tar xvf mingw.tar.gz
cd libev-mingw
./configure --prefix=$MINGW_PREFIX
make install -j4
cd ..

git clone https://github.com/shadowsocks/simple-obfs --depth=1
cd simple-obfs
git submodule update --init
./autogen.sh
./configure --disable-documentation
make -j4
gcc $(find src/ -name "obfs_local-*.o") $(find . -name "*.a") -o simple-obfs -fstack-protector -static -lev -lws2_32 -s
mv simple-obfs.exe ../base/tools/clients/
cd ..

git clone https://github.com/shadowsocks/shadowsocks-libev --depth=1
cd shadowsocks-libev
git submodule update --init
./autogen.sh
./configure --disable-documentation

# fix codes
sed -i "s/%I/%z/g" src/utils.h

make -j4
gcc $(find src/ -name "ss_local-*.o") $(find . -name "*.a") -o ss-local -fstack-protector -static -lev -lws2_32 -lsodium -lmbedtls -lmbedcrypto -lpcre
mv ss-local.exe ../base/tools/clients/
cd ..

git clone -b Akkariiin/develop --single-branch --depth=1 https://github.com/shadowsocksrr/shadowsocksr-libev
cd shadowsocksr-libev

# build ahead to reconfigure
cd libudns
./autogen.sh
./configure
make -j4
cd ..

./autogen.sh
CFLAGS+="-fstack-protector" ./configure --disable-documentation

# fix codes
sed -i "s/^const/extern const/g" src/tls.h
sed -i "s/^const/extern const/g" src/http.h

make -j4
gcc $(find src/ -name "ss_local-*.o") $(find . -name "*.a") -o ssr-local -fstack-protector -static -lpcre -lssl -lcrypto -lev -lsodium -lws2_32 -s
mv ssr-local.exe ../base/tools/clients/
cd ..

git clone https://github.com/trojan-gfw/trojan --depth=1
cd trojan
cmake -DMYSQL_INCLUDE_DIR=$MINGW_PREFIX/include/mysql -G "Unix Makefiles" .
make -j4
g++ -o trojan $(find CMakeFiles/trojan.dir/src/ -name "*.obj") -static -lmysqlclient -lssl -lcrypto -lz -lws2_32 -lwsock32 -lboost_program_options-mt -lcrypt32  -lsecur32 -lshlwapi -s
mv trojan.exe ../base/tools/clients/
cd ..

if [[ "$MSYSTEM" = "MINGW64" ]];then
    curl -LO https://github.com/v2ray/v2ray-core/releases/latest/download/v2ray-windows-64.zip
    curl -LO https://github.com/joewalnes/websocketd/releases/download/v0.3.1/websocketd-0.3.1-windows_amd64.zip
    curl -LO https://github.com/shadowsocks/v2ray-plugin/releases/download/v1.3.1/v2ray-plugin-windows-amd64-v1.3.1.tar.gz
fi
if [[ "$MSYSTEM" = "MINGW32" ]];then
    curl -LO https://github.com/v2ray/v2ray-core/releases/latest/download/v2ray-windows-32.zip
    curl -LO https://github.com/joewalnes/websocketd/releases/download/v0.3.1/websocketd-0.3.1-windows_386.zip
    curl -LO https://github.com/shadowsocks/v2ray-plugin/releases/download/v1.3.1/v2ray-plugin-windows-386-v1.3.1.tar.gz
fi

unzip v2ray*.zip v2ray.exe v2ctl.exe
unzip websocketd*.zip websocketd
tar xvf v2ray-plugin*.gz
rm v2ray-plugin*.gz
strip -s websocketd
mv v2ray-plugin* base/tools/clients/v2ray-plugin.exe
mv v2ray.exe v2ctl.exe base/tools/clients/
mv websocketd base/tools/gui/websocketd.exe
