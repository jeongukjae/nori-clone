#!/usr/bin/env bash

set -ex

time ./tools/comparison/nori_runner ./tools/comparison/data.txt > nori_runner.dump
time ./tools/comparison/nori_clone_runner ./tools/comparison/data.txt > nori_clone_runner.dump
diff -u nori_runner.dump nori_clone_runner.dump
