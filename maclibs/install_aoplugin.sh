#!/bin/sh
#
# Run this script for properly play of the sounds under Mac OS X
#
if test -f "/usr/local/lib/ao/plugins-4/libmacosx.so"; then :
else
  mkdir -p /usr/local/lib/ao/plugins-4
  cp lib/libmacosx.so /usr/local/lib/ao/plugins-4/
fi
