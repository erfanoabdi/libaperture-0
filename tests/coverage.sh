#!/bin/sh

echo "Source root: $MESON_SOURCE_ROOT"
echo "Build root: $MESON_BUILD_ROOT"

DIR=$MESON_BUILD_ROOT/meson-logs/test-coverage

mkdir -p $DIR

if [ -x "$(command -v gcovr)" ]; then
  gcovr -r $MESON_SOURCE_ROOT $MESON_BUILD_ROOT -f "../src/*" --html --html-details --print-summary -o $DIR/index.html
  gcovr -r $MESON_SOURCE_ROOT $MESON_BUILD_ROOT -f "../src/*" --xml -o $DIR/coverage.xml
else
  TRACEFILE=$DIR/coverage.info
  lcov --directory $MESON_BUILD_ROOT --capture --initial -o $TRACEFILE.initial
  lcov --directory $MESON_BUILD_ROOT --capture -o $TRACEFILE.run
  lcov -a $TRACEFILE.initial -a $TRACEFILE.run -o $TRACEFILE
  lcov --extract $TRACEFILE $MESON_SOURCE_ROOT/src/\* -o $TRACEFILE
  genhtml --prefix $MESON_SOURCE_ROOT --title "Code Coverage" --legend --show-details --branch-coverage -o $DIR $TRACEFILE
fi
