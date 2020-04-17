#!/bin/bash
export filepath=`realpath "$0"`
cd `dirname -- $filepath`
./stairspeedtest /web
