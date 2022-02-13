#!/bin/bash

set -e

DATA=tools/benchmark/data.txt
N_REPEAT=5

for N_LINES in 10 100 1000 10000 100000
do
nori_sum=0
nori_clone_cc_sum=0
nori_clone_go_sum=0
nori_clone_py_sum=0
kiwipiepy_sum=0

    for (( c=1; c<=$N_REPEAT; c++ )) do
        num=$(./tools/benchmark/nori_runner $DATA $N_LINES)
        echo "Nori: $num"
        nori_sum=$((nori_sum + num))
    done
    nori_sum=$((nori_sum / 5))

    for (( c=1; c<=$N_REPEAT; c++ )) do
        num=$(./tools/benchmark/nori_clone_runner_cc -input $DATA -n $N_LINES)
        echo "Nori clone cc: $num"
        nori_clone_cc_sum=$((nori_clone_cc_sum + num))
    done
    nori_clone_cc_sum=$((nori_clone_cc_sum / 5))

    for (( c=1; c<=$N_REPEAT; c++ )) do
        num=$(./tools/benchmark/nori_clone_runner_py $DATA $N_LINES;)
        echo "Nori clone py: $num"
        nori_clone_py_sum=$((nori_clone_py_sum + num))
    done
    nori_clone_py_sum=$((nori_clone_py_sum / 5))

    for (( c=1; c<=$N_REPEAT; c++ )) do
        num=$(./tools/benchmark/nori_clone_runner_go_/nori_clone_runner_go $DATA $N_LINES;)
        echo "Nori clone go: $num"
        nori_clone_go_sum=$((nori_clone_go_sum + num))
    done
    nori_clone_go_sum=$((nori_clone_go_sum / 5))

    for (( c=1; c<=$N_REPEAT; c++ )) do
        num=$(./tools/benchmark/kiwipiepy_runner $DATA $N_LINES;)
        echo "Kiwipiepy: $num"
        kiwipiepy_sum=$((kiwipiepy_sum + num))
    done
    kiwipiepy_sum=$((kiwipiepy_sum / 5))

    echo "=========================="
    echo "N: $N_LINES"
    echo "Nori runner: $nori_sum ms"
    echo "Nori clone runner(C++): $nori_clone_cc_sum ms"
    echo "Nori clone runner(Py): $nori_clone_py_sum ms"
    echo "Nori clone runner(Go): $nori_clone_go_sum ms"
    echo "Kiwipiepy runner(Py): $kiwipiepy_sum ms"
    echo "=========================="
done
