#!/bin/bash

if [ $# != 1 ]; then
  echo Usage: $0 infile.nsi
  exit 1
fi

echo > uninst.nsh || exit 1

( makensis "-XSetCompress off" "$1" | tee log; exit ${PIPESTATUS[0]} ) || exit 1

./gen_uninstall/gen_uninstall < log > uninst.nsh || exit 1

exec makensis "$1"
