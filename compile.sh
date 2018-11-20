#!/bin/sh
# build the program!
# note: there will eventually be a separate build step for your bot, but for now it counts against your runtime.
LIBRARIES="-lutil -ldl -lrt -lpthread -lgcc_s -lc -lm -L/battlecode/battlecode/c/lib/ -lbattlecode-linux"
INCLUDES="-I/battlecode/battlecode/c/include"
g++ -std=c++14 -O2 globals.cpp utility.cpp unit_behav.cpp scalar_field.cpp main.cpp -o main $LIBRARIES $INCLUDES