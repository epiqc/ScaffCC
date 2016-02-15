#!/usr/bin/perl

sub max {
    $a = shift;
    $b = shift;
    return $b   if ($b>$a);
    return $a;
}

@defn = ();
%ancilla = ('tof_anc' => 0);

while (<>) {
	next	if (/qubit [^I]/); # Skip non-ancilla qbit definitions (ancilla start with I)
	# Collapse ancilla defs to array
	if ( /qubit (I\w+?)I(\d+)$/ ) {
		$ancilla{$1} = $2 + 1;	# increment by 1 for size
		next;
	}
	# Restore array notation
    s/,/ , /g;
	s/(\s+)(\w*)I(\d+)([,\s])/$1$2\[$3\]$4/g;

	# Gate translations
	s/cnot (.*)/CNOT ( $1 );/;
	s/X (.*)/X ( $1 );/;

	# Other gate translations go here

	# QASM flattening doesn't support n-input toffolis, so 4-input toffolis need to be handled differently
    if (/toffoli (.*)/) {
        s/toffoli (.*)/Toffoli ( $1 );/;
        @bits = split ',', $1;
        $line = $_;
        if ($#bits > 2) {
            # preserve original as a comment
            $line =~ s/Toffoli/\/\/ Toffoli/;
            push @defn, $line;

            $ancilla{'tof_anc'} = max($#bits-2, $ancilla{'tof_anc'});
            $line = "	Toffoli ($bits[0],$bits[1],tof_anc[0]);\n";
            push @defn, $line;
            unshift @d_toff, $line;

            $i = 2;
            while ($i < $#bits-1) {
                $line = "	Toffoli ($bits[$i],tof_anc[".($i-2)."],tof_anc[".($i-1)."]);\n";
                push @defn, $line;
                unshift @d_toff, $line; # push onto the head of a stack for reversing out ancilla
                $i++;
            }
            $line = "	Toffoli ($bits[-2],tof_anc[".($i-2)."],$bits[-1]);\n";
            push @defn, $line;
            push @defn, @d_toff;
            @d_toff = ();
        } else {
            push @defn, $line;
        }
    } else {
        push @defn,$_;
    }
}

# Reinsert qubit defs
foreach $anc ( sort( keys %ancilla ) ) {
    #splice @defn, 0, 0, "	qbit $anc\[$ancilla{$anc}\];\n";
    print "	qbit $anc\[$ancilla{$anc}\];\n";
}
print @defn;

