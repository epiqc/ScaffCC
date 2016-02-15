#!/usr/bin/perl

%last = ();
@keys = qw(Function SIMDs ts ots mts moves tgates mlist);

foreach $file ( @ARGV ) {
    $algo = '';
    $cfg = '';
    $ext = '';
    $k = 0;
    $d = 0;
    if ( $file =~ /([a-zA-Z0-9]+)_(\w+)\.simd\.(\d)\.(\d+)\.leaves\.(.*)/ ) {
        $algo = $1; $cfg = $2; $k = $3; $d = $4; $ext = $5;
        $comm = "comm_aware_schedule.txt.$algo\_$cfg\_K$k\_D$d\_$ext";
        print "[comm_aware.pl] $file -> $comm\n";
        open COM, ">$comm"  or die "Unable to open $comm: $!\n";
        open RES, $file or die "Unable to open $file: $!\n";
        $last{Function} = "";
        while (<RES>) {
            chomp;
            if ( /^Function: (\w+)/ ) {
                if ( $last{Function} ) {
                    foreach $k ( @keys ) { 
                        print COM "$last{$k} ";
                    }
                    print COM "\n";
                }
                $last{Function} = $1;
            }
            $last{$1} = $2     if ( /^((?:SIMDs|ts|ots|mts|moves|tgates|mlist)) = (.+)/ );
        }
        foreach $k ( @keys ) { 
            print COM "$last{$k} ";
        }
        print COM "\n";
        close COM;
        close RES;
    }
}

