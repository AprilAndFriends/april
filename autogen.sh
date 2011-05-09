#!/bin/sh
if [ -e /usr/local/share/aclocal ] ; then
	export ACLOCAL_PATH="/usr/local/share/aclocal"
	autoreconf -I /usr/local/share/aclocal -vfi
else
	autoreconf -vfi
fi
