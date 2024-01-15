#!/bin/bash
#Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3

file=$1
echo "Indenting $file..."
astyle --style=java --indent=spaces --attach-classes --attach-inlines --attach-closing-while --indent-classes --indent-modifiers --indent-switches --indent-cases --break-blocks --pad-oper --pad-comma --pad-paren --add-braces --add-one-line-braces --convert-tabs --mode=c --suffix=none --lineend=linux --ascii  --stdin=$file --stdout="$file.indented"
mv $file.indented $file
echo "Indenting $file... Done"
