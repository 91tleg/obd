#!/usr/bin/env bash

cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug -DBOARD=nucleo_h753zi
cmake --build build/debug
