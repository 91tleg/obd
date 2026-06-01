#!/usr/bin/env bash

cmake -B build/release -DCMAKE_BUILD_TYPE=Release -DBOARD=nucleo_h753zi
cmake --build build/release
