#!/bin/sh
#

files=`find . -name \*.c -o -name \*.h`
for i in $files
do
  if fgrep 'LICENSE BEGIN' $i
  then
    sed -e '/\/\* LICENSE BEGIN/,/LICENSE END \*\// { d ; }' $i > $i.$$
    sed -e '/^\*\//r license_code2014.txt' $i.$$ > $i.2.$$
    mv $i.2.$$ $i.$$
  else
    sed -e '/^\*\//r license_code2014.txt' $i > $i.$$
  fi
  if [ -s $i.$$ ]
  then
    mv $i.$$ $i
  fi
done

