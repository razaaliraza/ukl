#!/bin/bash

# Output file for QEMU
outfile = "actions.out"

# Build
git submodule update --init
autoreconf -i
./configure --enable-use-ret --with-program=hello
make -j `nproc`

# Boot the machine
sudo timeout 10m make boot-actions

# Check to see that output contains magic string
# If Test fails, print QEMU output
if ! grep -sq "hello, from the UKL world" "$test_output"
then
    echo "TEST FAIL - UKL BOOT"
    cat $outfile
    rm -f $outfile
    exit 1
fi

#Otherwise, print success message
echo "TEST PASS - UKL BOOT"

rm -f $outfile

exit 0

