#!/bin/sh
qemu-system-x86_64 \
    -smp 2 \
    -cpu host \
    -m 4G \
    -kernel x32/x32_kernel \
    -enable-kvm \
    -nographic \
    -monitor telnet:127.0.0.1:1235,server,nowait \
    -s -S
