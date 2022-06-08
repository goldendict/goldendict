#!/bin/bash
# $1 - GoldenDict source directory

if [ ${OSTYPE:0:6} != "darwin" ]; then
  echo "OSX Only"
  exit 1
fi


if [[ -n "$1" && ! -d "$1" ]]; then
  echo "Invalid GoldenDict source directory"
  exit 1
fi

./configure \
  CFLAGS="-arch i386 -arch x86_64 -DCUSTOM_AO_PLUGIN_PATH=\\\"@executable_path/../Frameworks/ao\\\"" \
  LDFLAGS="-arch i386 -arch x86_64" \
  --disable-dependency-tracking

make clean && make


install_name_tool \
  -id @executable_path/../Frameworks/libao.dylib \
  src/.libs/libao.dylib

for PLUGIN_DIR in `find src/plugins -type d -maxdepth 1`; do
  PLUGIN_NAME=$(basename "$PLUGIN_DIR")
  PLUGIN_PATH=$PLUGIN_DIR/.libs/lib$PLUGIN_NAME.so
  if [ -f "$PLUGIN_PATH" ]; then
    install_name_tool \
      -id @executable_path/../Frameworks/ao/$PLUGIN_NAME.so \
      $PLUGIN_PATH
  fi
done

if [ -n "$1" ]; then
  cp src/.libs/libao.dylib $1/maclibs/lib
  # Copy plugins
  mkdir -p $1/maclibs/lib/ao
  find src/plugins -type f -maxdepth 3 -name "*.so" -exec cp {} $1/maclibs/lib/ao \;
fi
