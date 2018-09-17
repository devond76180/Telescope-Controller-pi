#!/bin/bash
gcc  -g -o server "server.c" "parseUtils.c" "motionManager.c" "controllerReader.c" "debug.c" "readConfig.c" "align.c"  -lpthread -lm -lwiringPi -lrt
