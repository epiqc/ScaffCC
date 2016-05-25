#!/usr/bin/perl
use warnings;

use File::Basename;

# Only argument is input scaffold file
# Output filenames are taken from that
$basename = basename( $ARGV[0], qw(.scaffold) );
$scafOut = "$basename\_noctqg.scaffold";
#$scafOut =~ s#.*/(.*)\.scaffold$#$1\_noctqg.scaffold#g;
open FIN, "<", $ARGV[0] or die "Couldn't open source file $ARGV[0]: $!\n";
open SCAF, ">", $scafOut or die "Unable to open scaffold output: $!\n";

$line=0;
@buffer=<FIN>;
$last_name='';
@ctqg=();

# Duplicate all global lines between Scaffold and CTQG output
# Any CTQG module is changed to an 'extern void' prototype in Scaffold
# Any Scaffold module is not added to the CTQG output
# All global variables need to be #defines for CTQG
while ($#buffer >= $line) {
  print "$line: $buffer[$line]";

  # Extract CTQG: it could either be regular ctqg function or a Parameterized one
  if ( $buffer[$line] =~ /^ctqg\s+(\w+).*{/ || $buffer[$line] =~ /^ctqg\s+(\<.*?\>)?\s?(\w+).*{/ ) {
    if ( $buffer[$line] =~ /^ctqg\s+(\w+).*{/ ) {
      $module = $1;
      #print "ctqg: $line: $buffer[$line]\n";
      #print CTQG $buffer[$line];
      $buffer[$line] =~ s/ctqg\s+(\w+)/module $1/;
      $last_name = $1;
    }
    elsif ( $buffer[$line] =~ /^ctqg\s+(\<.*?\>)?\s?(\w+).*{/ ) {
      $module = $2;
      #print "ctqg: $line: $buffer[$line]\n";
      #print CTQG $buffer[$line];
      $buffer[$line] =~ s/ctqg\s+(\<.*?\>)?\s?(\w+)/module $1 $2/;
      $last_name = $2;      
    }
    push @ctqg, $buffer[$line];
    $buffer[$line] =~ s/module/extern void/;
    $buffer[$line] =~ s/qint\[(\s*\d*\s*)\]\s+(\w+)/qbit $2\[$1\]/g;
    $buffer[$line] =~ s/{/;/;    
    if ($buffer[$line] =~ /\<.*?\>/) { 
      $buffer[$line] =~ s/.*//;  
    }
    print SCAF $buffer[$line];
    $braces = 1;
    while ($braces > 0 and $#buffer >= $line) {
      $line++;
      #print "ctqg: $line, $braces\n";
      #print CTQG $buffer[$line];
      push @ctqg, $buffer[$line];
      $braces++ if ( $buffer[$line] =~ /{/ );
      $braces-- if ( $buffer[$line] =~ /}/ );
    }
    if ( $line > $#buffer ) {
      die "Unable to find end of CTQG module '$module'\n";
    }

    # Preserve all modules and other functions in Scaffold
  } elsif ( $buffer[$line] =~ /^\w+\s+(\w+).*{/ ) {
    $module = $1;
    #print "module: $line: $buffer[$line]\n";
    print SCAF $buffer[$line];
    $braces = 1;
    while ($braces > 0 and $#buffer >= $line) {
      $line++;
      #print "module: $line, $braces: \n";
      print SCAF $buffer[$line];
      $braces++ if ( $buffer[$line] =~ /{/ );
      $braces-- if ( $buffer[$line] =~ /}/ );
    }
    if ( $line > $#buffer ) {
      die "Unable to find end of CTQG module '$module'\n";
    }

    # Duplicate all compiler directives for scaffold, but only #define for ctqg
  } elsif ( $buffer[$line] =~ /^#/ ) {
    print SCAF $buffer[$line];
    push @ctqg, $buffer[$line]	if ( $buffer[$line] =~ /^#define/ );

    # Clone all global lines
  } else {
    #print "$line\n";
    print SCAF $buffer[$line];
    #print CTQG $buffer[$line];
    push @ctqg, $buffer[$line];
  }
  $line++;
}

close FIN;
close SCAF;

# Generate CTQG for translation
# Name file with the module to be replaced in the Scaffold
$ctqgOut = $ARGV[0];
$ctqgOut = "$basename\.$last_name\.ctqg";
#$ctqgOut =~ s#.*/(.*)\.scaffold$#$1\.$last_name\.ctqg#g;
open CTQG, ">", $ctqgOut or die "Unable to open ctqg output: $!\n";
foreach $line (@ctqg) {
  # Replace the last module with 'main_module' for ctqg
  print CTQG "// $last_name = main_module\n"    if ( $line =~ s/module\s*$last_name/module main_module/ );
  print CTQG $line
};
close CTQG;

