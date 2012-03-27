#!/bin/sh

MAKE=${MAKE:-make}

if [ $# -eq 0 ] || [ x$1 = xall ]; then
	${MAKE} check
	exit $?
fi

for testname in "$@"; do
	if [ -f ${testname}TestCases ]; then
		# Rebuild it to ensure we're current.
		${MAKE} ${testname}TestCases || exit $?
		echo "Running tests in ${testname}TestCases"
		./set_test_libpath.sh ./${testname}TestCases || exit $?
	elif [ -f ${testname} ]; then
		# Rebuild it to ensure we're current.
		${MAKE} ${testname} || exit $?
		echo "Running tests in ${testname}"
		./set_test_libpath.sh ./${testname} || exit $?
	elif [ -f ./${testname}TestCases.cpp ] || [ -f ./${testname}Test.cpp ]; then
		if grep ${testname}TestCases.hpp UnitMain.cpp >/dev/null 2>&1; then
			make unitMain
			echo "Running tests for ${testname} (inside unitMain)"
			./set_test_libpath.sh ./unitMain ${testname}
		else
			# Build and rerun...
			${MAKE} || exit $?
			exec "$0" "$@"
		fi
	else
		echo "No test for ${testname}" >&2
		exit 1
	fi
done


