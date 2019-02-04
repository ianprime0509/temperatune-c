#!/bin/sh
retval=0

for input in print-cases/*.in; do
	outfile=$(mktemp temperatune.XXXXXX)
	case=$(basename "$input" .in)
	./print "$input" >"$outfile" 2>&1
	if ! diff "print-cases/$case.out" "$outfile"; then
		echo "FAIL: $case"
		retval=1
	fi
	rm "$outfile"
done

exit $retval
