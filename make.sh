#! /bin/bash

#cc -std=c99 -Wall shittyrepl.c -ledit -o shittyrepl
cc -std=c99 -Wall parsing.c mpc.c -ledit -lm -o parsing

