#!/usr/bin/env bash

openocd -f interface/stlink.cfg \         
        -f target/stm32h7x.cfg \
        -c "program build/release/src/firmware.elf verify reset exit"
