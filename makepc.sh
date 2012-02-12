#!/bin/sh
cmake -DCMAKE_BUILD_TYPE=Debug -DTARGET_TYPE=Linux
make
if [ -e drocerog ]; then
    mv drocerog drocerog.exe
fi
