#!/bin/sh

# This script generates ../data/flags/flags.qrc based on the ../data/flags dir contents

GOLDENDICT_ROOT="${PWD}/.."

FLAGS_DIR="${GOLDENDICT_ROOT}/data/flags"
OUT="${FLAGS_DIR}/flags.qrc"

echo "<RCC>" > $OUT
echo "  <qresource prefix=\"/flags\">" >> $OUT

for x in "${FLAGS_DIR}"/*.png; do
	echo "    <file>$(basename $x)</file>" >> $OUT
done

echo "  </qresource>" >> $OUT
echo "</RCC>" >> $OUT
