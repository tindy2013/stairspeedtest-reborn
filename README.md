# Stair Speedtest Reborn
**Proxy performance batch tester based on Shadowsocks(R) and V2Ray**  
<img alt="Travis-CI build status" src="https://travis-ci.org/tindy2013/stairspeedtest-reborn.svg?branch=master">
<img alt="GitHub tag (latest SemVer)" src="https://img.shields.io/github/tag/tindy2013/stairspeedtest-reborn.svg">
<img alt="GitHub release" src="https://img.shields.io/github/release/tindy2013/stairspeedtest-reborn.svg">
<img alt="GitHub license" src="https://img.shields.io/github/license/tindy2013/stairspeedtest-reborn.svg">
  
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
* libpng16
* freetype
* zlib
* yaml-cpp
* libevent
  
On non-Windows platforms, you also need to have the following clients installed to 'tools/clients/':  
* shadowsocks-libev
* shadowsocksr-libev ('ss-local' installed as the name 'ssr-local')
* v2ray-core
  
You can also use the auto configure script (experimental):
```bash
# On most Linux platforms  
./config.sh  

# On MacOS:  
./config.macos.sh
```
  
After installing all these dependencies, you can use the quick build script:  
```bash
# On Windows
build.bat  
  
# On most Linux platforms  
./build.sh  

# On MacOS:  
make -f Makefile.macos
```
## Usage
* Run "stairspeedtest" for CLI speedtest, run "webgui" for Web GUI speedtest.
* Results for subscribe link tests will be saved to a log file in "results" folder.
* The result will be exported into a PNG file with the result log. An interactive HTML file with test results will also be generated with the PNG file. In this HTML, you can sort by name, ping, packet loss or speed.
* You can customize some settings by editing "pref.ini".
## Compatibility
Tested platforms: 
  
* Windows 10 1903 x64, Windows Server 2008 R2 x64, Windows 7 SP1 x64
* Ubuntu 18.10
* Debian 6.3
* CentOS 7.6
* MacOS 10.14.6 Mojave
* Android 8.0 (Termux)
  
Supported proxy types:  

 |Proxy|Client|Config Parser|
 |:-:|:-:|:-:|
 |SSR|ShadowsocksR-libev|ShadowsocksR \| SSTap \| Netch GSF |
 |SS|Shadowsocks-libev|Shadowsocks \| ShadowsocksD \| Shadowsocks Android \| SSTap \| Clash \| Surge 2 \| Surge 3+ \| Netch GSF |
 |V2RAY|V2Ray-Core|V2RayN \| Quantumult \| Clash \| Surge 4 \| Netch GSF |
 |SOCKS5|-|Telegram \| SSTap \| Clash \| Surge 2+ \| Netch GSF
 
## Known Bugs
* There is a multi-thread exit problem when running on MacOS, the total file download testing time will exceed 10s.
## TODO
* Nothing yet
