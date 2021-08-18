#!/bin/sh

rm -f *.zip
chmod 777 espack* 
chmod 777 esp.* 

ver=$(grep  'define.*ESPACK_VERSION' src/espack.c  | awk -F'"' '{print $2}')
dir="espack-$ver"

mkdir -p $dir/tool

cp -a ESP $dir/
cp -a espack64* $dir/tool/
cp -a espack32* $dir/tool/
cp -a esp.* $dir/

zip -r "$dir.zip" $dir
rm -rf $dir


