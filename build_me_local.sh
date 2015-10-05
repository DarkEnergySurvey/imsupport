#!/usr/bin/env bash

source $EUPS_DIR/desdm_eups_setup.sh
source source_me_before_build.csh

export PRODUCT_DIR=/Users/felipe/tmp
export PREFIX=$PRODUCT_DIR
make clean
make
make install
