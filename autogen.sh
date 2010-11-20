#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

REQUIRED_AUTOMAKE_VERSION=1.9
REQUIRED_INTLTOOL_VERSION=0.40.4

PKG_NAME="typing-break"

which gnome-autogen.sh || {
    echo "You need to install gnome-common from the GNOME SVN"
    exit 1
}

. gnome-autogen.sh
