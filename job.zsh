#!/bin/zsh
#PJM -L "rscgrp=lecture"
#PJM -L "node=2"
#PJM -L "elapse=10:00"
#PJM -j

export FLIB_FASTOMP=False
export OMP_NUM_THREADS=16
./app
