#!/bin/bash

set -e

DATA=tools/benchmark/data.txt
N_REPEAT=5
N_LINES=1000

echo "Nori runner"
for (( c=1; c<=$N_REPEAT; c++ )) do ./tools/benchmark/nori_runner $DATA $N_LINES; done

echo "Nori-clone runner (C++)"
for (( c=1; c<=$N_REPEAT; c++ )) do ./tools/benchmark/nori_clone_runner_cc -input $DATA -n $N_LINES; done

echo "Nori-clone runner (Py)"
for (( c=1; c<=$N_REPEAT; c++ )) do ./tools/benchmark/nori_clone_runner_py $DATA $N_LINES; done

echo "Nori-clone runner (Go)"
for (( c=1; c<=$N_REPEAT; c++ )) do ./tools/benchmark/nori_clone_runner_go_/nori_clone_runner_go $DATA $N_LINES; done
