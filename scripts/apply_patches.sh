#!/bin/bash

echo "Updating submodules..."
git submodule update --init --recursive

echo "Applying patches..."
git -C lib/SdFat apply ../../patches/sdfat.patch