#!/bin/bash

echo "Updating submodules..."
git submodule update --init --recursive

echo "Applying patches..."
git -C ext_lib/SdFat apply ../../patches/sdfat.patch