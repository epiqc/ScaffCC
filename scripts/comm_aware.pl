#!/usr/bin/perl

%last = ();
@keys = qw(Function SIMDs ts ots mts moves tgates mlist);

foreach $file ( @ARGV ) {
    $name = '';
    $size = '';
    $th = '';
    $ext = '';
    $k = 0;
    $d = 0;
    if ( $file =~ /([a-zA-Z0-9_]+)\.(\w+)\.(\w+)\.simd\.(\d)\.(\d+)\.leaves\.(.*)/ ) {
        $name = $1; $size = $2; $th = $3; $k = $4; $d = $5; $ext = $6;
        $comm = "comm_aware_schedule.txt.$name\.$size\.$th\_K$k\_D$d\_$ext";
        print "\t[comm_aware.pl] $file -> $comm\n";
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

