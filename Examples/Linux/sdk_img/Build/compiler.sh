#!/bin/bash

if [ "$1" == "0" ]; then
	gcc `pkg-config --cflags --libs gtk+-2.0 opencv` $2
fi
if [ "$1" == "1" ];	then
	./a.out
fi
if [ "$1" == "2" ];	then
	gcc `pkg-config --cflags --libs gtk+-2.0 opencv` $2
	./a.out
fi
