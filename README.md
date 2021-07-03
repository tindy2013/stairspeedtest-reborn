# Stair Speedtest Reborn
**Proxy performance batch tester based on Shadowsocks(R), V2Ray and Trojan**  
[![Build Status](https://github.com/tindy2013/stairspeedtest-reborn/actions/workflows/build.yml/badge.svg)](https://github.com/tindy2013/stairspeedtest-reborn/actions/workflows/build.yml)
[![GitHub tag (latest SemVer)](https://img.shields.io/github/tag/tindy2013/stairspeedtest-reborn.svg)](https://github.com/tindy2013/stairspeedtest-reborn/tags)
[![GitHub release](https://img.shields.io/github/release/tindy2013/stairspeedtest-reborn.svg)](https://github.com/tindy2013/stairspeedtest-reborn/releases)
[![GitHub license](https://img.shields.io/github/license/tindy2013/stairspeedtest-reborn.svg)](https://github.com/tindy2013/stairspeedtest-reborn/blob/master/LICENSE)
  
## Intro
This is a C++ remake version of the original [Stair Speedtest](https://github.com/tindy2013/stairspeedtest) script. Despite its similarity to the script verion, this remake version works much more effectively, with faster node parsing, result picture rendering and even cross-platform support.  

## Special Thanks
* [@NyanChanMeow](https://github.com/nyanchanmeow) for the original script [SSRSpeed](https://github.com/nyanchanmeow/ssrspeed)
* [@CareyWong](https://github.com/careywang) for Web GUI design
* [@ang830715](https://github.com/ang830715)  for MacOS support
* ...and a lot of people who have helped me during the testing phase!
  
## Installation  
### Prebuilt release  
Go to [Release Page](https://github.com/tindy2013/stairspeedtest-reborn/releases).  
### Build
In general, you need the following build dependencies:  
* curl
* openssl
* PNGwriter
* libpng
* freetype
* zlib
* yaml-cpp
* libevent
* pcre2
* rapidjson
  
On non-Windows platforms, you also need to have the following clients installed to 'tools/clients/':  
* shadowsocks-libev
* shadowsocksr-libev ('ss-local' installed as the name 'ssr-local')
* v2ray-core
* trojan
  
After installing all these dependencies, you can use CMake to configure and build:  
```bash
cmake .
make -j
```

## Usage
* Run "stairspeedtest" for CLI speedtest, run "webgui" for Web GUI speedtest.
* Results for subscribe link tests will be saved to a log file in "results" folder.
* The result will be exported into a PNG file with the result log.
* You can customize some settings by editing "pref.ini".
## Compatibility
Tested platforms: 
  
* Windows 10 1903 x64, Windows Server 2008 R2 x64, Windows 7 SP1 x64
* Ubuntu 18.10
* Debian 6.3
* CentOS 7.6
* MacOS 10.13.6 High Sierra, 10.14.6 Mojave, 10.15 Catalina
* Android 8.0, 9.0 (with Termux)
* iOS/iPadOS 13 (with iSH Shell) **Bad performance, only for testing purpose**
* Raspberry Pi 4B with Raspbian (armv7l)
  
Supported proxy types:  

 |Proxy|Client|Config Parser|
 |:-:|:-:|:-:|
 |SSR|ShadowsocksR-libev| ShadowsocksR \| Quantumult(X) \| SSTap \| Netch GSF |
 |SS|Shadowsocks-libev| Shadowsocks \| ShadowsocksD \| Shadowsocks Android \| SSTap \| Clash \| Surge 2 \| Surge 3+ \| Quantumult(X) \| Netch GSF |
 |V2RAY|V2Ray-Core| V2RayN \| Quantumult(X) \| Clash \| Surge 4 \| Netch GSF |
 |TROJAN|Trojan-Core| Trojan \| Quantumult(X) \| Surge 4 \| Clash \| Netch GSF |
 |SOCKS5|-| Telegram \| SSTap \| Clash \| Surge 2+ \| Netch GSF |
 
## Known Bugs
* Nothing yet

## TODO
* Nothing yet
