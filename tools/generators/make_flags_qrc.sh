#!/bin/sh

# This script generates ../flags.qrc based on the ../flags dir contents

cd ..

OUT=flags.qrc

echo "<RCC>" > $OUT
echo "  <qresource>" >> $OUT

for x in flags/*.png; do
	echo "    <file>$x</file>" >> $OUT
done

echo "  </qresource>" >> $OUT
echo "</RCC>" >> $OUT
