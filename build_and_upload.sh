#!/bin/bash

git pull
mkdir -p build
cd build

rm -f darkmark-*

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=~/src/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_PREFIX_PATH=~/src/vcpkg/installed/x64-linux -DVCPKG_TARGET_TRIPLET=x64-linux ..
make -j16
make -j16 package
make -j16 package_source
rm -rf _CPack_Packages

scp -p darkmark-*.{gz,zip} stephane@www.ccoderun.ca:/var/www/html/download/

rm -rf src-dox
make doc
rsync --recursive --times --delete --compress --human-readable --progress src-dox/html/ www.ccoderun.ca:web_site/darkmark/

#cmake -DCMAKE_BUILD_TYPE=Debug ..
#make clean
#make -j16

cloc --autoconf --force-lang=html,dox --skip-uniqueness --exclude-dir=build ..

