#!/usr/bin/perl

$file = $ARGV[0];
@names = split (/\./, $file);
#$name = $names[0];
$scaf = "$names[0]\_noctqg.scaffold";
$merge = "$names[0]\_merged.scaffold";
$module = $names[-2];
@defn = ();
%ancilla = ();

# Find the module declaration we are reinserting
open MQHF, ">", $merge or die "Unable to open $merge for writing!\n";
open SCAF, "<", $scaf or die "Unable to open $scaf!\n";
open QASM, "<", $ARGV[0] or die "Unable to open $ARGV[0]!\n";
while (<SCAF>) {
	if ( /^extern void\s+$module/ ) {
		s/extern void/module/;
		s/\[.*?\]/\[\]/g;	# Remove array notation
		s/;.*/{/;			# Change from prototype to definition
		print MQHF;
		last;				# Process ctqg output and insert first; we'll fill in the rest of the file later
	} else {
		# Copy every line from preprocessed to merged scaffold
		print MQHF;
	}
}

# Load QASM
while (<QASM>) {
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

	# QASM flattening doesn't support n-input toffolis, so 4-input toffolis need to be handled differently
  # Assume: toffoli (control1, control2, target), use simple compute/ copy/ uncompute paradigm (Bennet 73)
	#s/toffoli (.*)/Toffoli ( $1 );/;
	if (/toffoli ([\w\[\]]+) , ([\w\[\]]+) , ([\w\[\]]+) , ([\w\[\]]+)/) {
		# 4-input needs to be decomposed
		$a = $1; $b = $2; $c = $3; $d = $4;
		$ancilla{'I0to0'} = 2;
		$_ = "	Toffoli ( I0to0[0] , $b , $c );\n";    
		$_ .= "	Toffoli ( I0to0[1] , I0to0[0] , $d );\n";
    $_ .= " CNOT ( $a, I0to0[1] );\n";
		$_ .= "	Toffoli ( I0to0[1] , I0to0[0] , $d );\n";
		$_ .= "	Toffoli ( I0to0[0] , $b , $c );\n";    
	}
	s/toffoli (.*)/Toffoli ( $1 );/;

	# Other gate translations go here

	push @defn,$_;
}
push @defn, "}\n";

# Reinsert qubit defs
foreach $anc ( keys %ancilla ) {
	splice @defn, 0, 0, "	qbit $anc\[$ancilla{$anc}\];\n";
}

# Add module definition to the merged file
print MQHF @defn;

# Finish the original scaffold source
while (<SCAF>) {
	print MQHF;
}
close SCAF;
close MQHF;
