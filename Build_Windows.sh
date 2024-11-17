#!/bin/bash

i686-w64-mingw32-gcc main.c -I./windows-deps/include -L./windows-deps/lib -lserialport -lhid -lsetupapi -lmingw32 -mwindows -o GB_Duino.exe
mkdir Windows_Build
cp GB_Duino.exe ./Windows_Build/
