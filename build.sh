#!/bin/sh

cd src
make
strip peach-finder
mv peach-finder ..
