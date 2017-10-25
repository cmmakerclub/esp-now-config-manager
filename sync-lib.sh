#!/bin/bash

rsync -av --delete --exclude=.git --exclude=.pioenvs ~/Documents/Arduino/libraries/CMMC_* ./espnow-controller/lib
rsync -av --delete --exclude=.git --exclude=.pioenvs ~/Documents/Arduino/libraries/CMMC_* ./espnow-slave-conf/lib
rsync -av --delete --exclude=.git --exclude=.pioenvs ~/Documents/Arduino/libraries/CMMC_* ./espnow-slave-always/lib
