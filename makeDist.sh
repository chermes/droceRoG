#!/usr/bin/env bash
#
# Compile droceRoG and put everything together in a zip file.
#
# Author: Christoph Hermes (hermes<AT>hausmilbe<DOT>net)

sh makearm.sh

TODAY=`date "+%Y_%m_%d"`
VERSION=`cat CMakeLists.txt | grep DROCEROG_VERSION | sed 's/.*"\([0-9.]\+\)")/\1/' | sed "s/\./_/"`
TMP_DIR=droceRoG-${TODAY}-V$VERSION

echo "Create installation directory $TMP_DIR, zipped"
mkdir $TMP_DIR

cp drocerog.app README.txt LICENSE.txt $TMP_DIR
cp fonts/drocerog.ttf fonts/DejaVuSerif.ttf $TMP_DIR

zip -r $TMP_DIR.zip $TMP_DIR

rm -rf $TMP_DIR

