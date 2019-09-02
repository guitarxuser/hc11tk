#!/bin/sh

#build

cd src
make clean
make

cd ../tty0tty/pts
make clean
make
cd ../..

cd test
make clean
make

cd ../

#install within /usr/local/bin with    sudo sh install.sh
