#!/bin/sh
if [ -e /usr/local/share/aclocal ] ; then
	autoreconf -I /usr/local/share/aclocal -vfi
else
	autoreconf -vfi
fi
