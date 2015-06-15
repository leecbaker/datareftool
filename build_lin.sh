#!/bin/bash
set -e -v -u
ssh sg.leecb.com rm -r /home/lee/dev/datareftool/build/*
ssh sg.leecb.com "cd /home/lee/dev/datareftool/build/ && cmake -DCMAKE_BUILD_TYPE=Release .."
ssh sg.leecb.com "make -C /home/lee/dev/datareftool/build/"
scp sg.leecb.com:/home/lee/dev/datareftool/bin/datareftool/lin.xpl bin/datareftool/lin.xpl
