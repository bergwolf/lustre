#! /bin/bash

MY_PWD=`pwd`
LUSTRE_SOURCE_DIR=`pwd`


TDIR=$MY_PWD/.cleanupdefs
CONFIGH=$LUSTRE_SOURCE_DIR/config.h
COAN=`which coan`
COAN_ARGS_FILE=$TDIR/args
NEWSDIR=$MY_PWD/lustre-client
MAKEFILE_DIR=$LUSTRE_SOURCE_DIR/build/upstream-kbuild

LUSTRE_CLIENT_KMOD_DIRS="$LUSTRE_SOURCE_DIR/lnet $LUSTRE_SOURCE_DIR/libcfs
			 $LUSTRE_SOURCE_DIR/lustre"
LUSTRE_SERVER_DIRS="mds ost mgs mdt cmm mdd ofd quota osp lod osd-ldiskfs
		    osd-zfs"
UNUSED_DIRNAMES="$LUSTRE_SERVER_DIRS conf autoconf contrib doc kernel_patches
		 tests utils obdfilter darwin liblustre ulnds scripts winnt
		 mxlnd ptllnd qswlnd ralnd .deps libcfs.xcode ksocklnd.xcode
		 portals.xcode .tmp_versions util posix"
UNUSED_FILENAMES="auto Makefile LICENSE ChangeLog BUILDING BUGS FDL
		  nodist modules.order Kernelenv Info.plist .gitignore
		  socklnd_lib-darwin socklnd_lib-winnt cygwin-ioctl.h
		  .empty lustre_ver.h.in user- libcfs_pack.h libcfs_unpack.h
		  lltrace.h libcfsutil.h"

# 0th, sanity check

if [ ! -x $COAN ]; then
	echo "    $0: cannot executable coan at $COAN... exiting." >&2
	exit 1
fi

if ! [ -e $CONFIGH ]; then
	echo "    $0: you did not run configure..." >&2
	echo "    $0: run configure with --with-linux=$KERNEL_DIR first." >&2
	exit 1
fi

echo -n "make clean Lustre tree..."
cd $LUSTRE_SOURCE_DIR
make clean > /dev/null 2>&1
echo "...done"

# 1st, create coan args
echo -n "creating coan arguments file ..."

rm -rf $TDIR
mkdir $TDIR
cp $CONFIGH $TDIR
cd $TDIR

grep -v MDT_MAX_THREADS $CONFIGH | egrep '(^#define|^\/\* #undef)' | sed -e 's/#define /-D/' -e 's/\/\* #undef /-U/' -e 's/ *\*\///' | awk '{print $1}' > args

echo "-D__KERNEL__" >> args
echo "-D__linux__" >> args
echo "-U__APPLE__" >> args
echo "-U__WINNT__" >> args
echo "-ULIBLUSTRE" >> args

cd $MY_PWD
echo "... done"
echo "coan args file $COAN_ARGS_FILE"

# 2nd, copy Lustre client source files
echo -n "copying Lustre client source files..."
rm -rf $NEWSDIR
mkdir $NEWSDIR

for I in $LUSTRE_CLIENT_KMOD_DIRS; do
	cp -r $I $NEWSDIR
done

cd $NEWSDIR
echo "... done"

# 3rd, clean up Lustre source tree
echo "cleaning up Lustre source tree ..."

cd $NEWSDIR
# dirs
for I in $UNUSED_DIRNAMES; do
	find . -name "$I" -a -type d | xargs rm -rf
done
# files
for I in $UNUSED_FILENAMES; do
	find . -name "$I*" -a -type f |xargs rm -rf
done

# 3.2 replace macros
COAN_ARGS="source -R --file $COAN_ARGS_FILE -kd -xe -Fc,h -ge"
echo "$COAN $COAN_ARGS $NEWSDIR"
$COAN $COAN_ARGS $NEWSDIR

# 3.2.x one special case...
egrep -q '^#define.*HAVE_D_DELETE_CONST' $CONFIGH
[ $? -eq 0 ] && sed -i 's/HAVE_D_DELETE_CONST/const/' $NEWSDIR/lustre/llite/dcache.c

# 3.3 copy Makefile
egrep "^#.*CONFIG_*" $CONFIGH > $NEWSDIR/config.h
cp -r $MAKEFILE_DIR/* $NEWSDIR

cd $NEWSDIR
for I in `find . -name Makefile.kbuild`;do
	tmp=`dirname $I`; mv $I $tmp/Makefile;
done

cd $MY_PWD
echo "... done"
