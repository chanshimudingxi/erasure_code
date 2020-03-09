	set -x
	aclocal
	libtoolize --force --copy
	autoconf --force
	automake --foreign --copy --add-missing
