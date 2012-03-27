#!/bin/sh

# This script creates a "fake shared library", which is basically a mapping
# from a function name to another function name, so that runtime lookup may be
# performed on symbols which were placed (possibly renamed) in the main
# executable. 
#
# The first parameter passed to this script, if it isn't a library, will be
# used as a pattern to use when grepping for symbol names.  If this is empty,
# then all symbols that can be exported will be exported.
#
# The remainder of the parameters will be used as the names of libraries from
# which a fake library should be created.
#
# If no libraries parameters were given, then this script will recursively
# locate shared libraries (see SUFFIX2 below), and create fake libraries for
# each of those, with all matching exports (or all, if none were requested).
#
# Note: The exports will NOT act the same if a prefix is given.  An export to
# the name of the prefix will be emitted, as well as special care for symbols
# which contain both the prefix and NO_ID

FAKE_EXTENSION=.fsl

SUFFIX1=.a
SUFFIX2=.so

# This assumes that the libraries have already been created.

EXPORT_PREFIX=
if [ $# -gt 0 ]
then
  if [ `basename $1 $SUFFIX1` = "$1" -a  `basename $1 $SUFFIX2` = "$1" ]
  then
    EXPORT_PREFIX="$1"
    shift
  fi
fi

DEMANGLE_PREVENTION_FLAGS=

if nm --version 2>/dev/null >/dev/null
then
  DEMANGLE_PREVENTION_FLAGS="--no-demangle"
else
  DEMANGLE_PREVENTION_FLAGS="-C"
fi


echo "Using \"$EXPORT_PREFIX\" as the function export prefix."

extract_creation_name_from_lib()
{
  CREATION_FUNCTION_NAME=
  if [ $# -eq 1 ]
  then
    # If the symbol is not undefined, doesn't have a leading '.' or '_', then
    # export it.  This will also be required to match the export prefix (if the
    # export prefix is empty, then all symbols will be exported).
    CREATION_FUNCTION_NAME=`nm -Bpg $DEMANGLE_PREVENTION_FLAGS $1 |
	grep "$EXPORT_PREFIX" |
	awk '{ 
		if ( ($2 != "U") && (substr($3,1,1) != ".") &&  (substr($3,1,1) != "_") )
		{ print $3 } 
	}' | 
	sort -u`
  else
    echo "extract_creation_name_from_lib() requires exactly one argument."
  fi
}

create_fake_library()
{
  if [ $# -eq 1 ]
  then
    
    DIRECTORY=`dirname $1`
    FILENAME=`basename $1`
    BASE_LIBNAME=
    LIB_EXT=

    MINUS_A=`basename $FILENAME $SUFFIX1`
    if [ "x$MINUS_A" != "x$FILENAME" ]
    then
      LIB_EXT=$SUFFIX1
      BASE_LIBNAME=$MINUS_A
    else
      MINUS_SO=`basename $FILENAME $SUFFIX2`
      if [ "x$MINUS_SO" != "x$FILENAME" ]
      then
        LIB_EXT=$SUFFIX2
	BASE_LIBNAME=$MINUS_SO
      fi
    fi

    if [ "x$BASE_LIBNAME" == "x" ]
    then
	MINUS_NLM=`basename $FILENAME .nlm`
	if [ "x$MINUS_NLM" != "x$FILENAME" ]
	then
	  LIB_EXT=.nlm
	  BASE_LIBNAME=$MINUS_NLM
	fi
    fi

    if [ "x$BASE_LIBNAME" != "x" ]
    then
      if [ "x$LIB_EXT" == "x.nlm" ]
      then
        IMPORT_FILE=$BASE_LIBNAME".imp"
        CREATION_FUNCTION_NAME=`grep "$EXPORT_PREFIX" $IMPORT_FILE | sort -u`
      else
        extract_creation_name_from_lib "$DIRECTORY/$FILENAME"
      fi
      
      if [ "x$CREATION_FUNCTION_NAME" != "x" ]
      then
	echo "Creating fake library for $1"
        FAKE_LIBNAME=$DIRECTORY/$BASE_LIBNAME$FAKE_EXTENSION
	echo "FAKE=true" > $FAKE_LIBNAME
	for export_name in $CREATION_FUNCTION_NAME
	do
	  if [ "x$EXPORT_PREFIX" != "x" ]
	  then
 	    echo "	Creating function for $EXPORT_PREFIX as \"$export_name\""
            echo "$EXPORT_PREFIX=$export_name" >> $FAKE_LIBNAME
	    if echo "$export_name" | grep "NO_ID" 2>&1 > /dev/null
	    then
	      echo "	Creating function for ${EXPORT_PREFIX}NO_ID as \"$export_name\""
	      echo "${EXPORT_PREFIX}NO_ID=$export_name" >> $FAKE_LIBNAME	  
	    fi
	  fi
	  echo "	Exporting function $export_name"
	  echo "$export_name=$export_name" >> $FAKE_LIBNAME
	done
	echo "	Creating getOWVersion function"
	echo "getOWVersion=getOWVersion" >> $FAKE_LIBNAME
      else
	echo "Unable to find any matching exports in $1"
      fi
    else
      echo "Unable to determine library type for $DIRECTORY/$FILENAME"
    fi
  else
    echo "create_fake_library() needs exactly one parameter"
  fi
}


# Main portion of the script.  Either create fake libraries for the requested
# libraries, or recursively locate shared libraries (suffix2) and create the
# fake libraries for each of those (putting it in the same directory as the
# library).
if [ $# -gt 0 ]
then
    while [ $# -gt 0 ]
    do
      create_fake_library $1
      shift
    done
else
  for x in `find . -name "*$SUFFIX2"`
  do
    create_fake_library $x
  done
fi
