#!/bin/zsh
set -e -u -x

REVISION=$1
DATE=`date +%Y_%m_%d`
FOLDER=DataRefTool_${DATE}

mkdir -p ${FOLDER}
cd ${FOLDER}

wget https://datareftool-binaries.s3.amazonaws.com/$REVISION/mac.xpl
wget https://datareftool-binaries.s3.amazonaws.com/$REVISION/lin.xpl
wget https://datareftool-binaries.s3.amazonaws.com/$REVISION/win.xpl
cp ../LICENSE .
cd ..

zip ${FOLDER}.zip ${FOLDER}/*

