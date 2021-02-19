#!/bin/bash
mkdir base/tools/clients

set -xe

apk add gcc g++ build-base linux-headers cmake make autoconf automake libtool curl
apk add openssl-dev openssl-libs-static libev-dev pcre-dev libsodium-dev libsodium-static c-ares-dev libevent-dev libevent-static mbedtls-dev mbedtls-static boost-dev boost-static mariadb-dev mariadb-static

git clone https://github.com/shadowsocks/simple-obfs --depth=1
cd simple-obfs
git submodule update --init
./autogen.sh
./configure --disable-documentation
make -j4
gcc $(find src/ -name "obfs_local-*.o") $(find . -name "*.a") -o simple-obfs -static -lev -s
mv simple-obfs ../base/tools/clients/
cd ..

git clone https://github.com/shadowsocks/shadowsocks-libev --depth=1
cd shadowsocks-libev
git submodule update --init
./autogen.sh
./configure --disable-documentation
make -j4
gcc $(find src/ -name "ss_local-*.o") $(find . -name "*.a") -o ss-local -static -lpcre -lmbedtls -lmbedcrypto -lev -lsodium -s
mv ss-local ../base/tools/clients/
cd ..

git clone -b Akkariiin/develop --single-branch --depth=1 https://github.com/shadowsocksrr/shadowsocksr-libev
cd shadowsocksr-libev
# fix codes
sed -i "s/^const/extern const/g" src/tls.h
sed -i "s/^const/extern const/g" src/http.h
./autogen.sh
./configure --disable-documentation
make -j4
gcc $(find src/ -name "ss_local-*.o") $(find . -name "*.a") -o ssr-local -static -lpcre -lssl -lcrypto -lev -lsodium -s
mv ssr-local ../base/tools/clients/
cd ..

git clone https://github.com/trojan-gfw/trojan --depth=1
cd trojan
cmake -DDEFAULT_CONFIG=config.json -DFORCE_TCP_FASTOPEN=ON .
make -j4
g++ $(find CMakeFiles/trojan.dir/src/ -name "*.o") -o trojan -static -lmysqlclient -lboost_program_options -lssl -lcrypto -lz -s
mv trojan ../base/tools/clients/
cd ..

if [[ "$ARCH" = "x86_64" ]];then
    curl -LO https://github.com/v2fly/v2ray-core/releases/latest/download/v2ray-linux-64.zip
    curl -LO https://github.com/joewalnes/websocketd/releases/download/v0.3.1/websocketd-0.3.1-linux_amd64.zip
    curl -LO https://github.com/shadowsocks/v2ray-plugin/releases/download/v1.3.1/v2ray-plugin-linux-amd64-v1.3.1.tar.gz
    else if [[ "$ARCH" = "x86" ]];then
        curl -LO https://github.com/v2fly/v2ray-core/releases/latest/download/v2ray-linux-32.zip
        curl -LO https://github.com/joewalnes/websocketd/releases/download/v0.3.1/websocketd-0.3.1-linux_386.zip
        curl -LO https://github.com/shadowsocks/v2ray-plugin/releases/download/v1.3.1/v2ray-plugin-linux-386-v1.3.1.tar.gz
    else
        curl -LO https://github.com/joewalnes/websocketd/releases/download/v0.3.1/websocketd-0.3.1-linux_arm.zip
        if [[ "$ARCH" = "aarch64" ]];then
            curl -LO https://github.com/v2fly/v2ray-core/releases/latest/download/v2ray-linux-arm64-v8a.zip
            curl -LO https://github.com/shadowsocks/v2ray-plugin/releases/download/v1.3.1/v2ray-plugin-linux-arm64-v1.3.1.tar.gz
            else if [[ "$ARCH" = "armhf" ]];then
                curl -LO https://github.com/v2fly/v2ray-core/releases/latest/download/v2ray-linux-arm32-v7a.zip
                curl -LO https://github.com/shadowsocks/v2ray-plugin/releases/download/v1.3.1/v2ray-plugin-linux-arm-v1.3.1.tar.gz
            fi
        fi
    fi
fi

unzip v2ray*.zip v2ray v2ctl
unzip websocketd*.zip websocketd
if [[ "$ARCH" = "armhf" ]];then
  tar xvf v2ray-plugin*.gz v2ray-plugin_linux_arm7
else
  tar xvf v2ray-plugin*.gz
fi
strip -s websocketd
mv v2ray-plugin_* base/tools/clients/v2ray-plugin
mv v2ray base/tools/clients/
mv v2ctl base/tools/clients/
mv websocketd base/tools/gui/

chmod +rx base/tools/clients/* base/tools/gui/websocketd
