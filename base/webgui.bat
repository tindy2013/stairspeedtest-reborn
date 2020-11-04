@echo off
chcp 65001>nul
title Stair Speedtest Web GUI Backend
setlocal enabledelayedexpansion
start http://127.0.0.1:65430
tools\gui\websocketd --port=65430 --maxforks=1 --staticdir=tools\gui stairspeedtest /rpc
