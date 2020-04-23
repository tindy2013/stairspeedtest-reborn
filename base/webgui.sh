#!/bin/bash
cd "$(dirname "$0")"
tools/gui/websocketd --port=65430 --maxforks=1 --staticdir=tools/gui ./stairspeedtest /rpc
