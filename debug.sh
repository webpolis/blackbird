#!/bin/bash
#this script is intended to run inside the docker container
#it compiles the app with debug symbols and launches it via GDB server
#
cd "$(dirname "$0")"

#compile with debug flags on
make -B DEBUG=true VERBOSE=true
#launch GDB server for remote debugger to attach
gdbserver :9091 /debug-app/blackbird