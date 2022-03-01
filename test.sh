#!/bin/bash

# Trigger all your test cases with this script

#! /usr/bin/env sh
echo "##########################"
echo "### Running e2e tests! ###"
echo "##########################"
count=0 # number of test cases run so far

# Assume all `.in` and `.out` files are located in a separate `tests` directory


for test in tests/objdump_tests/*.x2017; do
    name=$(basename $test .x2017)
    expected=tests/objdump_tests/$name.out
    args=tests/objdump_tests/$name.x2017

    echo running $test
    ./objdump_x2017 $args 2>&1 | diff - $expected || echo "Test $name: failed!"


    count=$((count+1))
done

for test in tests/objdump_EXIT_tests/*.x2017; do
    name=$(basename $test .x2017)
    expected=tests/objdump_EXIT_tests/$name.out
    args=tests/objdump_EXIT_tests/$name.x2017

    echo running $test
    ./objdump_x2017 $args &> /dev/null
    check=$?
    diff $expected <(echo "$check")|| echo "Test $name: failed!"


    count=$((count+1))
done


for test in tests/vm_tests/*.x2017; do
    name=$(basename $test .x2017)
    expected=tests/vm_tests/$name.out
    args=tests/vm_tests/$name.x2017

    echo running $test
    ./vm_x2017 $args 2>&1 | diff - $expected || echo "Test $name: failed!"


    count=$((count+1))
done

for test in tests/VM_EXIT_tests/*.x2017; do
    name=$(basename $test .x2017)
    expected=tests/VM_EXIT_tests/$name.out
    args=tests/VM_EXIT_tests/$name.x2017

    echo running $test
    ./vm_x2017 $args &> /dev/null
    check=$?
    diff $expected <(echo "$check")|| echo "Test $name: failed!"


    count=$((count+1))
done


echo "Finished running $count tests!"