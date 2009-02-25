#!/bin/sh
#

files=`find . -name \*.c`
for i in $files
do
  sed -e '/^\*\//r license_code.txt' $i > $i.$$
  if [ -s $i.$$ ]
  then
    mv $i.$$ $i
  fi
done

files=`find . -name \*.h`
for i in $files
do
  sed -e '/^\*\//r license_code.txt' $i > $i.$$
  if [ -s $i.$$ ]
  then
    mv $i.$$ $i
  fi
done
