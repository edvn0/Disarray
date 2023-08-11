#! /usr/bin/env bash

set -e

function compile_clang() {
  CXX=clang++-16 CC=clang-16 python3 disarray.py -bf -g Ninja -t App -m RelWithDebInfo
}

function compile_gnu() {
  CXX=g++ CC=gcc python3 disarray.py -bf -g Ninja -t App -m RelWithDebInfo
}

compile_clang & compile_gnu
