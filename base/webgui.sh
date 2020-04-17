#!/bin/bash
export filepath=`realpath "$0"`
cd `dirname -- $filepath`
tools/gui/websocketd --port=65430 --maxforks=1 --staticdir=tools/gui ./stairspeedtest /rpc
