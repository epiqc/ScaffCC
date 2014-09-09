/cnot/ {
	split($2, args, ",");
	print $1 " ( " args[1] " , " args[2] " );";
}
/toffoli/ {
	split($2, args, ",");
	for (arg in args)
		gensub(/^([^I]*)I([[:digit:]])/ "\\1\[\\2\]", "g");
	print $1 " ( " args[1] " , " args[2] " , " args[3] " );";
}
