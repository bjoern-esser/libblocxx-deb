#!/bin/sh 

THIS=$0

AR=""

while [ $# -gt 0 ]
do 
	case "$1" in
		--)
			shift
			break
			;;	
		*)
			AR="$AR $1"
			;;
	esac
	shift
done
  

ARCHIVES=""
OBJECTS=""

MODE="$1"
shift

case "$MODE" in
	cru)
		TARGET="$1"
		shift
		;;
	x)
		$AR x $@ && exit 0
		;;
	*)
		echo "ERROR: Mode $MODE passed to $THIS"
		exit 1
		;;
esac


while [ $# -gt 0 ]
do 
	case "$1" in
		*.o)
			OBJECTS="$OBJECTS $1"
			;;	
		*.a)
			ARCHIVES="$ARCHIVES $1"
			;;
		*)
			echo "ERROR: Unexpected file type for $1 passed to $THIS"
			exit 1
			;;
	esac
	shift
done

cleanup_archive_dirs()
{
	# clean up each archive dir
	for i in $ARCHIVES;
	do
		name=`basename $i .a`
		ar_dir=".${name}_dir"
		rm -rf $ar_dir
	done
}

# extract each archive into it's own directory so that we
# can safely handle name collisions, i.e. if files are named the
# same, also rename each *.o to be prefixed by it's archive name
# so that when the archive is extracted we don't overwrite files
# that have the same name
for i in $ARCHIVES;
do
	name=`basename $i .a`
	ar_dir=".${name}_dir"
	rm -rf $ar_dir
	full_archive_path=$i
	if [ -f "./$i" ]; then
		full_archive_path=`pwd`/$i
	fi
	if [ -f $full_archive_path ]; then
		mkdir $ar_dir
		cd $ar_dir
		$AR x $full_archive_path

		# If the BLOCXX_NO_OBJECT_LIBRARY_PREFIX variable not set to true,
		# prefix them, otherwise leave it alone. 
		if [ -z "$BLOCXX_NO_OBJECT_LIBRARY_PREFIX" -o "x$BLOCXX_NO_OBJECT_LIBRARY_PREFIX" != "xtrue" ]; then
			for j in *.o; do
				mv $j ${name}_$j
			done
		fi
		cd ..
		NEWOBJS="$NEWOBJS $ar_dir/*.o"
	else
		echo "ERROR: archive $i does not exist."
		cleanup_archive_dirs
		exit 1
	fi		
done

$AR cru $TARGET $NEWOBJS $OBJECTS

cleanup_archive_dirs

