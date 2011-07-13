#!/bin/sh
set -x
libtoolize --automake --force --copy \
&& aclocal -I config \
&& autoheader \
&& automake --gnu --add-missing --copy \
&& autoconf

