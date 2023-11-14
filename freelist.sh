#!/bin/bash
#########################################################
# EEE3535 Operating Systems                             #
# Written by William J. Song                            #
# School of Electrical Engineering, Yonsei University   #
#########################################################

hw="Assignment 4"
rm_sh="tar.sh install.sh shell.sh syscall.sh sched.sh"
patch_tar="freelist.tar"
patch_path="https://icsl.yonsei.ac.kr/wp-content/uploads"

install() {
    make clean
    mv user/sid.h ./
    rm -rf Makefile user kernel $rm_sh
    wget $patch_path/$patch_tar
    tar xf $patch_tar
    rm -f $patch_tar
    mv sid.h user/
}

if [[ `basename $PWD` == "xv6-riscv" && -d user ]]; then
    if [[ ! -f user/malloctest.c ]]; then
        install
    else
        echo "xv6-riscv is already up to date for $hw"
        while true; do
            read -p "Do you want to update again? This will reset all the past work in xv6-riscv [y/n]: " yn
            case $yn in
                [Yy]* )
                    install
                    break;;
                [Nn]* )
                    break;;
            esac
        done
    fi
    if [[ `grep sname user/sid.h` =~ Unknown ]]; then
        read -p "Enter your 10-digit student ID: " sid
        read -p "Enter your name in English: " sname
        echo ""
        echo "#define sid $sid" > user/sid.h;
        echo "#define sname \"$sname\"" >> user/sid.h;
    fi
else
    echo "Error: $0 must run in the xv6-riscv/ directory"
fi

