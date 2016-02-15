#!/usr/bin/perl

open FIN, $ARGV[0] or die "Unable to open $ARGV[0]! $!\n";
my $in_func = 0;
my $leaf = 1;
my $buffer = [];
while (<FIN>) {
    if (/#Function (\w+)/) {
        $in_func = 1;
        push @$buffer, $_;

    } elsif (/#EndFunction/) {
        push @$buffer, $_;
        if ( $leaf ) {
            print @$buffer;
            print "\n";
        }
        $in_func = 0;
        $leaf = 1;
        $buffer = ();

    } elsif ( $in_func and $leaf ) {
        my ($ts, $op, @args) = split / /;
        $leaf = 0           if ( $op !~ /^(?:GateName|PrepZ|MeasX|MeasZ|CNOT|H|S|Sdag|T|Tdag|X|Y|Z|Fredkin)$/ );
        push @$buffer, $_   if ( $leaf );
    }
}
