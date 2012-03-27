#!/bin/sh

status_hackery_local_verbose=0

if [ $# -ne 2 ]; then
	 echo "The output filename and input filename must be specified!"
	 echo
	 echo "Usage: $0 <output_filename> <input_filename>"
	 echo
	 exit 1
fi

# This is here to do what 'tail -n1' should do.  Some platforms don't like the
# -n, and newer utilities say that 'tail -1' is deprecated.  This doesn't
# require the '-n 1'.
last_line()
{
	sed -e :a -e '$q;N;2,$D;ba' "$@"
}

# This emulates 'head -nX', where X is some integer.  It exists for the same
# reason as the 'last_line' function.  This requires thaty the first parameter
# is the number of lines (no '-').
first_n_lines()
{
	status_hackery_local_line_count="$1"
	shift

	sed "${status_hackery_local_line_count}q" "$@"
}

# I apologize about the unreadability of this file, but I had to create
# variables that would not likely collide with anything exported in a configure
# script. Should you want to read this, replace 'status_hackery_local_' with
# nothing (it will be much cleaner).  Just don't save it that way.

#
# If the destination file ends in .sed, then the sed expressions are just
# dumped to the file (for use with 'sed -f').  If it has any other extension, a
# script is created. with altogether too many '-e' lines for sed. 
#

status_hackery_local_temp_output_file=/tmp/hackery_replacement.txt.$$
status_hackery_local_output_script="$1"
status_hackery_local_input_file="$2"

# Find sed expressions from within the input file that have
# autoconf-replaceable names as the search portion (@[^@]+@).  This also
# removes lines that would be evaluated in the execution of make --
# $(VARIABLE), and any lines which include '\,', as they will cause errors in a
# shell.  Lines which have backslashes will have those doubled, so they can be
# re-evaluated and still have them appear properly in the output. 
#
# NOTE: This will only find the sed expressions that are located at the
# BEGINNING of a line (that's how they are layed out in configure's
# config.status output file).
egrep '^s.@[A-Za-z0-9_]+@' ${status_hackery_local_input_file} | egrep -v '\$\(.*\)|\\,' | sed 's/\\/\\\\/g' | sed 's/|#_!!_#|//g'> $status_hackery_local_temp_output_file

# Get the delimiter character that was used for the sed expressions.
while read status_hackery_local_line_text; do
	# Get the delimiter character that was used for the sed expressions.
	status_hackery_local_delim_char=`echo "${status_hackery_local_line_text}" | cut -c2`
	echo "${status_hackery_local_line_text}" | cut -f1-3 -d"${status_hackery_local_delim_char}"
done < ${status_hackery_local_temp_output_file} > ${status_hackery_local_temp_output_file}.tweaked
mv -f ${status_hackery_local_temp_output_file}.tweaked ${status_hackery_local_temp_output_file}

# Count the number of lines (expressions) in the file.
status_hackery_local_num_lines=`cat ${status_hackery_local_temp_output_file} | wc -l`

rm -f $status_hackery_local_output_script

# Repeatedly evaluate everything (until no changes are made) to get out-of-order variables set properly.
echo "Repeating evaluation loop until all out-of-order references are resolved."
status_hackery_local_changes_made=1
status_hackery_local_loops_done=0
while [ $status_hackery_local_changes_made -gt 0 ]; do
	status_hackery_local_loops_done=`expr ${status_hackery_local_loops_done} \+ 1`
	echo "Doing evaluation loop #${status_hackery_local_loops_done}"
	status_hackery_local_changes_made=0
	status_hackery_local_line=1
	while [ $status_hackery_local_line -le $status_hackery_local_num_lines ]; do
		status_hackery_local_line_text=`cat ${status_hackery_local_temp_output_file} | first_n_lines $status_hackery_local_line | last_line`
		status_hackery_local_line=`expr ${status_hackery_local_line} \+ 1`
		status_hackery_local_delim_char=`echo "${status_hackery_local_line_text}" | cut -c2`

		status_hackery_local_variable=`echo "$status_hackery_local_line_text" | cut -f2 -d'@'`
		status_hackery_local_value=`echo "$status_hackery_local_line_text" | cut -f3 -d"${status_hackery_local_delim_char}"`

		# Get the value of the variable as it currently exists in the environment.
		eval "status_hackery_local_old_value=\"\$$status_hackery_local_variable\""
		# Evaluate the expression, replacing the old value in the environment.
		eval "$status_hackery_local_variable=\"$status_hackery_local_value\""
		eval "evaled_status_hackery_local_value=\"$status_hackery_local_value\""

		# Check to see if the environment variable's value changed.
		if [ "x$status_hackery_local_old_value" != "x$evaled_status_hackery_local_value" ]; then
			status_hackery_local_changes_made=`expr ${status_hackery_local_changes_made} \+ 1`
			[ $status_hackery_local_verbose -eq 1 ] && echo "Replaced variable $status_hackery_local_variable: old value='$status_hackery_local_old_value' new value='$evaled_status_hackery_local_value'" >&2
		fi
	done
	echo "Found ${status_hackery_local_changes_made} changes this iteration"
done

if [ "`basename $status_hackery_local_output_script .sed`" != "`basename $status_hackery_local_output_script`" ]; then
	status_hackery_local_create_script=0
else
	status_hackery_local_create_script=1
fi

# This round actually spits out expressions.
echo "Creating replacement script $status_hackery_local_output_script"
status_hackery_local_line=1
if [ $status_hackery_local_create_script -eq 1 ]; then
	echo "#!/bin/sh" > $status_hackery_local_output_script
	echo "" >> $status_hackery_local_output_script
	echo "cat \"\$1\" | sed \\" >> $status_hackery_local_output_script
fi

status_hackery_local_temp_evil_hack_file=/tmp/evil_hackery_file.$$

while [ $status_hackery_local_line -le $status_hackery_local_num_lines ]; do
	status_hackery_local_line_text=`cat ${status_hackery_local_temp_output_file} | first_n_lines $status_hackery_local_line | last_line`
	status_hackery_local_variable=`echo "$status_hackery_local_line_text" | cut -f2 -d'@'`
	status_hackery_local_delim_char=`echo "${status_hackery_local_line_text}" | cut -c2`

	# Get the value of the variable as it currently exists in the environment.
	eval "evaled_status_hackery_local_value=\"\$$status_hackery_local_variable\""
	if [ $status_hackery_local_create_script -eq 1 ]; then
		status_hackery_local_output_prefix="	-e \'"
		status_hackery_local_output_suffix="\' \\"
	else
		status_hackery_local_output_prefix=""
		status_hackery_local_output_suffix=""
	fi
	cat > $status_hackery_local_temp_evil_hack_file <<EOF
${status_hackery_local_output_prefix}s${status_hackery_local_delim_char}@${status_hackery_local_variable}@${status_hackery_local_delim_char}${evaled_status_hackery_local_value}${status_hackery_local_delim_char}g${status_hackery_local_output_suffix}
EOF
	cat $status_hackery_local_temp_evil_hack_file >> $status_hackery_local_output_script

	status_hackery_local_line=`expr ${status_hackery_local_line} \+ 1`
done

if [ $status_hackery_local_create_script -eq 1 ]; then
	# There is still a trailing backslash, so put an empty line at the end. and
	# make the script executable.
	echo >> $status_hackery_local_output_script
	chmod +x $status_hackery_local_output_script
fi

rm -f $status_hackery_local_temp_output_file
rm -f $status_hackery_local_temp_evil_hack_file
