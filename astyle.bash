#!/bin/bash

files=`ls *.h *.c *.cpp`

for file in $files
do
echo "Indenting $file..."
astyle --style=java --indent=spaces --attach-classes --attach-inlines --attach-closing-while --indent-classes --indent-modifiers --indent-switches --indent-cases --break-blocks --pad-oper --pad-comma --pad-paren --add-braces --add-one-line-braces --convert-tabs --mode=c --suffix=none --lineend=linux --ascii  --stdin=$file --stdout="$file.indented"
mv $file.indented $file
echo "Indenting $file... Done"
done
