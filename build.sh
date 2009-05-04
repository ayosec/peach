#!/bin/sh

cd src
make
mv peach-finder ..
strip peach-finder
