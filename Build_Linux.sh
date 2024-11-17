#!/bin/bash

gcc main.c -I./linux-deps/include -L./linux-deps/lib -o GB_Duino -lserialport
