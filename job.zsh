#!/bin/zsh
#PJM -L "rscgrp=lecture"
#PJM -L "node=1"
#PJM -L "elapse=10:00"
##PJM --mpi “procc=28
##PJM -g xx1
#PJM -j

export FLIB_FASTOMP=False
mpiexec ./app
