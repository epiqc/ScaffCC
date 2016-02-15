#!/usr/bin/perl

###################################
#   Perl scheduler for ScaffCC
###################################
#   By: Jeff Heckey
#   Copyright: 2014 UCSB
#   NOT INTENDED FOR PUBLIC DISTRIBUTION
###################################
#
# Status (2/Jul/14):
#   Added width to RCP
# Status (3/Jun/14):
#   Added move counts to metrics output
# Status (26/May/14):
#   RCP optimizations round 1
# Status (22/May/14):
#   Added "true" printing, which prints the op schedule of each simd region at a given timestep
# Status (13/May/14):
#   Added tgate counts, modified output metrics to include more raw data
#   Added flags for pretty print
#   Minor optimizations (variable localization)
# Status (10/May/14):
#   update_moves optimized
#   other misc optimizations
# Status (7/May/14):
#   find_lp now uses $obj->{ops} to iterate instead of ACPA schedule. ~33% system time speedup
# Status (5/May/14):
#   Modified lpfs update ready to use hash instead of arrays (>10x speed up!)
# Status (2/May/14):
#   Got rid of moves from stalled SIMD regions
#   LPFS will SIMD sched opp regions
#   Tested SIMD L=1, K=2; L=1, K=4
#   Update 2: regression proofing for reschedule
# Status (1/May/14):
#   Improved find_lp algorithm, now much faster
# Status (29/Apr/14):
#   Enhancements to work with BWT (div/0 errors)
#   Profiled code to speed it up (find_LP() is longest runtime)
# Status (26/Apr/14):
#   Added metrics
# Status (25/Apr/14):
#   RCP now using update_moves and revised update RCPQ and general clean up
#   Update 2: Find next LP for completed SIMD region
#   Update 3: Opportunistic SIMD scheduling added
# Status (24/Apr/14):
#   Some partial fixes to schedule object for repeated passes.
# Status (23/Apr/14):
#   Finished LPFS with moves and cleaned up
# Status (8/Apr/14):
#   Finished a first pass of the LPFS scheduling
#   Non-optimal - could recalculate longest paths from present point if a path is completed.
# Status (18/Mar/14):
#   Contains CPR code for evaluating potential conflicts in SIMD regions
#   Partially completed RCP code, in more structured form
#

#use 5.012;

package Qubit;

sub new {
    my $class = shift;
    my $qubit = { 'name'      => "",
                  'index'     => 0,
                  'size'      => 1,
                  'loc'       => 0,  # RCP [-1 = unalloc, 0 = mem, 1-n = simd-n]
                  'last_op'   => -1,
                  'ops'       => [] };
    $qubit->{name} = shift;
    if ( $qubit->{name} =~ /(.*?)(\d+)/ ) {
        $qubit->{index} = $2;
    } else {
        $qubit->{index} = -1;
    }
    bless $qubit, $class;
    return $qubit;
}

sub add_dep {
    my $qubit = shift;
    push ( @{ $qubit->{ops} }, shift );
}

sub get_dep {
    my $qubit = shift;
    my $idx = shift || -1;  # return last element by default
    return $qubit->{ops}->[$idx];
}

# Fetches qubit at latest timestep
# Return: 1 if qubit needs to be moved, 0 otherwise
sub rcp_fetch_qubit {
    my $qubit = shift;
    my $ts = shift;
    my $simd = shift;
    my $op = shift;
    $qubit->{last_op}++;
    if ( !defined $qubit->{ops}->[ $qubit->{last_op} ] ) {
        printf "E: looking at $qubit->{name} out of range\n";
    } elsif ( ::refaddr( $op ) != ::refaddr( $qubit->{ops}->[$qubit->{last_op}] ) ) {
        printf "E: Out of order scheduling of qubit %s ops '%s' and '%s'\n",
                $qubit->{name}, $op->{text}, $qubit->{ops}->[$qubit->{last_op}]->{text};
    }
    $qubit->{last_ts} = $ts;
    if ( $qubit->{loc} != -1 and $qubit->{loc} != $simd ) {
        $qubit->{loc} = $simd;
        return 1;
    }
    $qubit->{loc} = $simd;
    return 0;
}

# Moves qubit back to memory if it is not used in this timestep
# Return: 1 if qubit returns to memory
sub rcp_store_qubit {
    my $qubit = shift;
    my $curr_ts = shift;
    if ( $qubit->{last_ts} != $curr_ts ) {
        $qubit->{loc} = 0;  # move to memory
        return 1;
    }
    return 0;   # Used in current timestep, so do not move to memory
}


package Op;

my $id = 1;

sub new {
    my $class = shift;
    my $op = {  'op'        => '',
                'rank'      => 0,
                'length'    => 1,
                'dist'      => -1,
                'top'       => 1,
                'bottom'    => 1,
                'args'      => [],
                'text'      => '',
                'id'        => 0,
                'slack'     => 0,
                'asap'      => {'ts'    => -1},
                'alap'      => {
                                'ts'    => -1,
                                'sched' => 0
                            },
                'acap'      => {
                                'ts'    => -1,
                                'sched' => 0
                            },
                'lpfs'      => {
                                'tag'       => 0,
                                'path'      => 0,
                                'followed'  => 0,
                                'simd'      => -1,
                                'ts'        => -1
                            },
                'rcp'       => {
                                'simd'      => -1,
                                'ts'        => -1
                            },
                'p_chaco'   => -1,
                'in_edges'  => [],
                'out_edges' => [] };
    $op->{op} = shift;
    $op->{rank} = shift;
    $op->{length} = shift || 1;
    $op->{id} = $id++   if ( $op->{op} ne "MOV" );
    $op->{text} = "$op->{id}: $op->{op}";
    bless $op, $class;
    return $op;
}

# Add new args and updated dependencies as needed
sub add_qubit {
    my $op = shift;
    my $qubit = shift;
    push @{ $op->{args} }, $qubit;
    if ( my $dep = $qubit->get_dep(-2) ) {
        push ( @{ $op->{'in_edges'} }, $dep );
        push ( @{ $dep->{'out_edges'} }, $op );
        $op->{top} = 0;
        $dep->{bottom} = 0;
    }
    $op->{'text'} .= " $qubit->{name}";
}

sub print_fields {
    my $op = shift;
    foreach my $field ( @_ ) {
        print  "$op->{$field} ";
    }
    print  "\n";
}

sub op_ready {
    my $op = shift;
    my $sched = shift;
    my $ready = 1;
    foreach my $parent ( @{ $op->{in_edges} } ) {
        $ready = 0  if ( ! defined $parent->{$sched}->{ts} or $parent->{$sched}->{ts} == -1 );
    }
    return $ready;
}

sub get_ready_children {
    my $op = shift;
    my $sched = shift;
    my @ready = ();
    foreach my $child ( @{ $op->{out_edges} } ) {
        push @ready, $child     if ( $child->op_ready($sched) );
    }
    return @ready;
}

sub take_path {
    my $op = shift;
    my $path = shift;
    $op->{lpfs}->{dist} = 0;   # Clear distance for next path finding
    $op->{lpfs}->{path} = $path;
    $op->{lpfs}->{simd} = $path;
    $op->{lpfs}->{followed} = 1;
}

# Fetch op's qubits if needed
# Return: list of qubits that need to move
sub rcp_fetch_op {
    my $op = shift;
    my $ts = shift;
    my $simd = shift;
    my @moves = ();
    $op->{rcp}->{ts} = $ts;
    $op->{rcp}->{simd} = $simd;
    foreach my $qubit ( @{ $op->{args} } ) {
        push @moves, $qubit     if ( $qubit->rcp_fetch_qubit($ts, $simd, $op) );
    }
    return @moves;
}

# Store op's qubits if needed
# Return: list of qubits that need to move
sub rcp_store_op {
    my $op = shift;
    my $ts = shift;
    my @moves = ();
    foreach my $qubit ( @{ $op->{args} } ) {
        push @moves, $qubit     if ( $qubit->rcp_store_qubit($ts) );
    }
    return @moves;
}


package Schedule;

sub new {
    my $class = shift;
    my $obj = ();
    $obj->{function} = shift;
    bless $obj, $class;
    $obj->{twidth} = 0; # cell width for printing
    $obj->{threads} = 0; # max number of parallel ops
    $obj->{length} = 0;
    $obj->{top} = [];
    $obj->{bottom} = {};
    $obj->{first_op} = 0;
    $obj->{last_op} = 0;
    $obj->{op_cnt} = 0;
    $obj->{qubits} = ();
    $obj->{active_qubits} = {};
    $obj->{ops} = [];
    $obj->{asap} = {threads => 0};
    $obj->{alap} = {threads => 0};
    $obj->{ss} = $obj->new_ss();
    $obj->{lpfs} = $obj->new_lpfs();
    $obj->{rcp} = $obj->new_rcp();
    return $obj;
}

sub new_ss {
    my $obj = shift;
    my $k = shift || $::SIMD_K;
    my $d = shift || $::SIMD_D;
    return {
                    op_cnt      => 0,   # counts gates and moves
                    moves       => 0,   # counts each move op
                    mts         => 0,   # counts only timesteps with moves
                    len         => 0,   # total time
                    tgates      => 0,   # timesteps that have a T gate
                    width       => 0,   # max SIMD regions
                    simd_k      => $k,
                    simd_d      => $d
                  };
}

sub new_lpfs {
    my $obj = shift;
    my $k = shift || $::SIMD_K;
    my $d = shift || $::SIMD_D;
    my $l = shift || $::SIMD_L;
    my $refill = shift || $::refill;
    my $opp = shift || $::opp;
    foreach my $op ( @{ $obj->{ops} } ) { $op->{lpfs}->{followed} = 0; }
    return {
                    op_cnt      => 0,   # counts gates and moves
                    moves       => 0,   # counts each move op
                    mts         => 0,   # counts only timesteps with moves
                    len         => 0,   # total time
                    tgates      => 0,   # timesteps that have a T gate
                    width       => 0,   # max SIMD regions
                    simd_k      => $k,
                    simd_d      => $d,
                    simd_l      => $l,  # simd regions to allocate only to paths
                    refill_simd => $refill,
                    opp_simd    => $opp,
                    path        => 1    # starting path id
                  };
}

sub new_rcp {
    my $obj = shift;
    my $k = shift || $::SIMD_K;
    my $d = shift || $::SIMD_D;
    my $w_op = shift || $::w_op;
    my $w_dist = shift || $::w_dist;
    my $w_slack = shift || $::w_slack;
    return {
                    op_cnt  => 0,   # counts gates and moves
                    moves   => 0,   # counts each move op
                    mts     => 0,   # counts only timesteps with moves
                    len     => 0,   # last timestep
                    tgates  => 0,   # timesteps that have a T gate
                    width   => 0,   # max SIMD regions
                    simd_k  => $k,
                    simd_d  => $d,
                    w_op    => $w_op,
                    w_dist  => $w_dist,
                    w_slack => $w_slack
                  };
}

# Add an operation to the function
# Operation will be ASAP scheduled
sub add_op {
    my $obj = shift;
    my $op = shift;
    my $time = 0;
    my $dist = 1;
    push @{ $obj->{ops} }, $op;
    if ( $op->{top} ) {
        push @{ $obj->{top} }, $op;
        $op->{dist} = $dist;
    } else {
        foreach my $pre ( @{$op->{in_edges}} ) {
            print  "W: '$pre->{text}'\@$pre->{asap}->{ts} not assigned\n"  if ($pre->{asap}->{ts} == -1);
            $time = ($time, $pre->{asap}->{ts} + $pre->{length})[$time < $pre->{asap}->{ts} + $pre->{length}];
            $dist = ($dist, $pre->{dist} + 1)[$dist < $pre->{dist} + 1];
            $pre->{bottom} = 0  if ( $pre->{bottom} );
            delete $obj->{bottom}->{$pre};
        }
        $op->{dist} = $dist;
    }
    $obj->{bottom}->{$op} = $op   if ( $op->{bottom} );
    $op->{asap}->{ts} = $time;
    $op->{lpfs}->{dist} = $dist;
    push @{ $obj->{asap}->{$time} }, $op;
    #$obj->{asap}->{threads} = ::max ($obj->{asap}->{threads}, scalar @{ $obj->{asap}->{$time} });
    $obj->{first_op} = $op      if ( $obj->{asap}->{first_op} == 0 );
    $obj->{last_op} = $op;
    $obj->{twidth} = ::max ($obj->{twidth}, length $op->{text});
    $obj->{length} = ($obj->{length}, $time+$op->{length})[$obj->{length} < $time+$op->{length}];
    $obj->{asap}->{op_cnt}++;
    $obj->{op_cnt}++;
}

sub draw_graph {
    my $obj = shift;
    my $file = shift;
    my @colors = qw(white red orange yellow green blue indigo violet pink 
                    darkorange gold limegreen cyan navy magenta coral 
                    orangered darkgoldenrod chartreuse skyblue turquoise 
                    plum);
    open DOT, ">$file" or die "Unable to open Dot file $file\n";
    # Print some header stuff
    # Make the function name the root node and an "exit" node
    print DOT <<HEADING;
digraph $obj->{function} {
    graph [ rankdir=LR ];

    Top \t[ label=\"$obj->{function}\", rank=0 ];
    Bottom\t[ label=\"Return\", rank=$obj->{length} ];
HEADING
    # Loop over {ops} for all intermediate nodes
    foreach my $op ( @{ $obj->{ops} } ) {
        #print DOT "    $op->{id}\t[ label=\"$op->{op}\" ];\n";
        print DOT "    $op->{id}\t[ label=\"$op->{op}$op->{id}\"";
        print DOT ", style=filled, fillcolor=$colors[$op->{lpfs}->{path}]";
        print DOT ", rank=$op->{dist}";
        print DOT " ];\n";
    }
    print DOT "\n";
    # Loop over all qubits for each edge (safer than trying to infer/rebuild from op perspective)
    while ( my ($name, $qubit) = each %{ $obj->{qubits} } ) {
        my $last = {'id' => 'Top'};
        foreach my $op ( @{ $qubit->{ops} } ) {
            print DOT "    $last->{id} -> $op->{id}\t[ label=\"$name\" ];\n";
            $last = $op;
        }
        print DOT "    $last->{id} -> Bottom\t[ label=\"$name\" ];\n";
    }
    print DOT "}";
    close DOT;
}

# Start by sceduling all of the bottom-most times
# Then recursively call all available dependencies
sub alap {
    my $obj = shift;
    my @recurse = ();
    $obj->{alap}->{op_cnt} = 0;
    foreach my $op_ref ( keys $obj->{bottom} ) {
        my $op = $obj->{bottom}->{$op_ref};
        $op->{alap}->{sched} = 1;
        $op->{alap}->{ts} = $obj->{length} - $op->{length};
        $op->{slack} = $op->{alap}->{ts} - $op->{asap}->{ts};
        print "W: $op->{text} has negative slack $op->{slack} (ASAP: $op->{asap}->{ts}, ALAP: $op->{alap}->{ts})\n" if ( $op->{slack} < 0 );
        push @{ $obj->{alap}->{$op->{alap}->{ts}} }, $op;
        $obj->{alap}->{threads} = ::max ($obj->{alap}->{threads}, scalar @{ $obj->{alap}->{$op->{alap}->{ts}} });
        $obj->{alap}->{op_cnt}++;
        foreach my $pre ( @{ $op->{in_edges} } ) {
            my $next = 1;
            foreach my $dep ( @{ $pre->{out_edges} } ) {
                $next &= $dep->{alap}->{sched};
            }
            if ( $next ) {
                $pre->{alap}->{ts} = $obj->{length};
                # Search preferred to hash because most lists will be short 
                # (<3 elements, max <20)
                push(@recurse, $pre) unless grep{$_ == $pre} @recurse;
            }
        }
    }
    $obj->alap_recurse("alap",@recurse);
    if ($obj->{op_cnt} != $obj->{alap}->{op_cnt}) {
        print  "W: $obj->{function} op counts mismatch (Sched: $obj->{op_cnt}, ALAP: $obj->{alap}->{op_cnt})\n";
    }
}

sub alap_recurse {
    my $obj = shift;
    my $type = shift;
    my @recurse = ();
    foreach my $op ( @_ ) {
        $op->{$type}->{sched} = 1;
        foreach my $dep ( @{ $op->{out_edges} } ) {
            $op->{$type}->{ts} = min ($op->{$type}->{ts}, $dep->{$type}->{ts} - $op->{length});
        }
        if ($type eq "alap") {
            $op->{slack} = $op->{alap}->{ts} - $op->{asap}->{ts};
            print "W: $op->{text} has negative slack $op->{slack} (ASAP: $op->{asap}->{ts}, ALAP: $op->{alap}->{ts})\n" if ( $op->{slack} < 0 );
        }
        push @{ $obj->{$type}->{$op->{$type}->{ts}} }, $op;
        $obj->{$type}->{threads} = ::max ($obj->{$type}->{threads}, scalar @{ $obj->{$type}->{$op->{$type}->{ts}} });
        $obj->{$type}->{op_cnt}++;
        foreach my $pre ( @{ $op->{in_edges} } ) {
            my $next = 1;
            foreach my $dep ( @{ $pre->{out_edges} } ) {
                $next &= $dep->{$type}->{sched};
            }
            if ( $next ) {
                $pre->{$type}->{ts} = $obj->{length};
                # Search preferred to hash because most lists will be short (<3 elements)
                push(@recurse, $pre) unless grep{$_ == $pre} @recurse;
            }
        }
    }
    $obj->alap_recurse($type,@recurse)    if ( scalar @recurse );
}

# As Centered As Possble scheduling
sub acap {
    my $obj = shift;
    my $type = "acap";
    my $mid = $obj->{length} >> 1;  # divide by two, without decimal
    my @recurse = ();
    # Copy second half of ASAP schedule to end of ACAP schedule
    for (my $ts=$mid; $ts < $obj->{length}; $ts++) {
        foreach my $op ( @{ $obj->{asap}->{$ts} } ) {
            push @{ $obj->{$type}->{$ts} }, $op;
            $op->{$type}->{ts} = $ts;
            $op->{$type}->{sched} = 1;
            $obj->{$type}->{threads} = ::max ($obj->{$type}->{threads}, scalar @{ $obj->{$type}->{$ts} });
            $obj->{$type}->{op_cnt}++;
            # Build recursive list of next operations
            foreach my $pre ( @{ $op->{in_edges} } ) {
                if ( ! $pre->{acap}->{sched} ) {
                    my $next = 1;
                    foreach my $dep ( @{ $pre->{out_edges} } ) {
                        $next &= $dep->{$type}->{sched};
                    }
                    if ( $next ) {
                        $pre->{$type}->{ts} = $obj->{length};
                        # Search preferred to hash because most lists will be short 
                        # (<3 elements, max <20) and hashes are expensive in memory
                        push(@recurse, $pre) unless grep{$_ == $pre} @recurse;
                    }
                }
            }
        }
    }
    $obj->alap_recurse($type,@recurse)    if ( scalar @recurse );
}

# LPFS Longest Path First Scheduling. Performs a breadth first search to 
# find the longest path(s) in the graph and assign those directly to SIMD 
# regions. All other operations are scheduled to remaining SIMD regions 
# as available.
sub lpfs {
    my $obj = shift;
    my $ts = 0;
    my $op_cnt = 0; # only ops, no moves
    my $tgate = 0; # set if T/Tdag found
    my $t_cnt = 0; # count of timesteps with a T/Tdag
    my $width = 0;
    my $ready = {};
    my $simds = {};
    my $pathsearch = 1;    # 0 if all paths are discovered
    my $simd_l = $obj->{lpfs}->{simd_l};
    my $opp_simd = $obj->{lpfs}->{opp_simd};
    my $refill = $obj->{lpfs}->{refill_simd};
    # Find longest paths
    for ( my $i=1; $pathsearch and $i<=$simd_l; $i++ ) {
        @{ $simds->{$i} } = $obj->find_lp( $obj->{top} );
        $pathsearch = 0    if ( ! @{ $simds->{$i} } );
    }
    $ready = $obj->lpfs_init();
    while ( ( keys %$ready ) or $op_cnt < $obj->{op_cnt} ) {
        $tgate = 0;
        if ( $ts > $obj->{op_cnt} ) {
            print "E: LPFS timestep $ts > op count $obj->{op_cnt}. Aborting.\n";
            return;
        }
        my $next = {};
        # Schedule all assigned paths if ready
        foreach my $simd ( 1..$simd_l ) {
            if ( $pathsearch and $refill and ! scalar @{ $simds->{$simd} } ) {
                @{ $simds->{$simd} } = $obj->find_lp( ref values $ready );
                $pathsearch = 0    if ( ! @{ $simds->{$simd} } );
            }
            my $op = $simds->{$simd}->[0];
            if ( $obj->sched_op( "lpfs", $op, $ts, $simd ) ) {
                shift @{ $simds->{$simd} };
                $width = ($width,$simd)[$width < $simd];
                $op_cnt++;
                $tgate |= ( $op->{op} =~ /^(?:T|Tdag)$/ );
                # add opportunistic scheduling to a scehduled simd region already doing the same op
                if ( $opp_simd ) {
                    foreach ( $obj->lpfs_extract_optype( "lpfs", $op->{op}, $ready ) ) {
                        $obj->sched_op( "lpfs", $_, $ts, $simd );
                        $op_cnt++;
                    }
                }
            }
        }
        # Schedule any remaing ready tasks
        for ( my $simd=$simd_l+1; $simd<=$obj->{lpfs}->{simd_k}; $simd++ ) {
            # Remove all longest path ops from the front
            my $scheduled = 0;
            while ( ! $scheduled and ( keys $ready ) ) {
                my $id = 0;
                while ( $id = ::min(keys %$ready) and $ready->{$id}->{followed} ) {
                    print "W: followed!\n";
                    delete $ready->{$id};
                }
                if ( $opp_simd ) {
                    foreach ( $obj->lpfs_extract_optype( "lpfs", $ready->{$id}->{op}, $ready ) ) {
                        $obj->sched_op( "lpfs", $_, $ts, $simd );
                        $width = ($width,$simd)[$width < $simd];
                        $scheduled = 1;
                        $op_cnt++;
                        $tgate |= ( $_->{op} =~ /^(?:T|Tdag)$/ );
                    }
                } else {
                    $scheduled = $obj->sched_op( "lpfs", $ready->{$id}, $ts, $simd );
                    if ( $scheduled ) {
                        $op_cnt++;
                        $tgate |= ( $ready->{$id}->{op} =~ /^(?:T|Tdag)$/ );
                        delete $ready->{$id};
                        $width = ($width,$simd)[$width < $simd];
                    }
                }
            }
        }
        # Update data moves
        $obj->{lpfs}->{$ts}->{0} = $obj->update_moves("lpfs", $ts);
        # Update ready list
        $ready = $obj->lpfs_update_ready( $ts, $ready );
        $ts++;
        $t_cnt += $tgate;
    }
    print "E: ops mis-scheduled ($op_cnt out of $obj->{op_cnt})\n"   if ( $op_cnt != $obj->{op_cnt} );
    $obj->{lpfs}->{len} = $ts;
    $obj->{lpfs}->{tgates} = $t_cnt;
    $obj->{lpfs}->{width} = $width;
}

sub lpfs_init {
    my $obj = shift;
    my $ready = {};
    $obj->{lpfs}->{op_cnt} = 0;
    foreach $op ( @{ $obj->{top} } ) {
        $ready->{$op->{id}} = $op   if ( ! $op->{lpfs}->{followed} );
    }
    foreach my $op ( @{ $obj->{ops} } ) {
        $op->{lpfs}->{ts}    = -1;
        $op->{lpfs}->{simd}  = -1;
        $op->{lpfs}->{dist}  = 0;
        $op->{lpfs}->{tag}   = 0;
    }
    while ( my ($q, $qubit) = each %{ $obj->{qubits} } ) {
        $qubit->{loc}       = 0;
        $qubit->{last_op}   = -1;
    }
    return $ready;
}

sub lpfs_update_ready {
    my $obj = shift;
    my $ts = shift;
    my $ready = shift;
    my $next = {};
    foreach my $simd ( keys %{ $obj->{lpfs}->{$ts} } ) {
        if ( $simd != 0 ) {
            foreach my $op ( @{ $obj->{lpfs}->{$ts}->{$simd} } ) {
                foreach my $child ( $op->get_ready_children("lpfs") ) {
                    $ready->{$child->{id}} = $child    if ( ! $child->{lpfs}->{followed} );
                }
            }
        }
    }
    return $ready;
}

sub lpfs_extract_optype {
    my $obj = shift;
    my $sched = shift;
    my $optype = shift;
    my $ready = shift;
    my @ret = ();
    my $cnt = 0;
    foreach $id ( keys %$ready ) {
        if ( $ready->{$id}->{op} eq $optype and $cnt < $obj->{$sched}->{simd_d} ) {
            push @ret, $ready->{$id};
            delete $ready->{$id};
            $cnt++;
        }
    }
    return @ret;
}


sub find_lp {
    my $obj = shift;
    my $top = shift;
    return ()   if ( ! @$top );
    my @path = ();
    my $id = 0x7fffffff;
    my $op = ();
    foreach my $op ( @{ $obj->{ops} } ) {
        $op->{lpfs}->{dist} = 1     if ( ! $op->{lpfs}->{followed} );
    };
    foreach $op ( @$top ) {
        $id = ($id, $op->{id})[$id > $op->{id}];
    };
    foreach $op ( @{ $obj->{ops} } ) {
        if ( $op->{id} >= $id ) {
            if ( $op->{lpfs}->{followed} ) {
                $op->{lpfs}->{dist} = 0;
            } else {
                foreach my $child ( @{ $op->{out_edges} } ) {
                    my $dist = $op->{lpfs}->{dist} + 1;
                    $child->{lpfs}->{dist} = ($dist, $child->{lpfs}->{dist})[$dist < $child->{lpfs}->{dist}]
                }
            }
            $path[0] = $op  if ( ! $op->{lpfs}->{followed} 
                                 and $op->{lpfs}->{dist} > $path[0]->{lpfs}->{dist} );
        }
    }
    return  if ( ! @path );
    PATH: while ( $path[0]->{lpfs}->{dist} > 1 ) {
        my $dist = $path[0]->{lpfs}->{dist} - 1;
        $path[0]->take_path( $obj->{lpfs}->{path} );
        foreach my $parent ( @{ $path[0]->{in_edges} } ) {
            if ( $parent->{lpfs}->{dist} == $dist ) {
                unshift @path, $parent;
                next PATH;
            }
        }
        print "E: unable to find path from $path[0]->{text} (dist: $path[0]->{lpfs}->{dist})\n";
    }
    $path[0]->take_path( $obj->{lpfs}->{path} );
    $obj->{lpfs}->{path}++;
    return @path;
}

sub find_lp_orig {
    my $obj = shift;
    my $top = shift;
    return ()   if ( ! @$top );
    my $next = ();
    my $curr = ();
    my $tag = 0;
    my @path = ();
    my $ops = 0;
    my $children = 0;
    my $parents = 0;
    my $acc = 0;
    my $maxfan = 0;
    @$next = @$top; # copy list instead of modifying
    # Reset all distances before recomputing
    foreach my $op ( @{ $obj->{ops} } ) {
        $op->{lpfs}->{dist} = 1     if ( ! $op->{lpfs}->{followed} );
        $op->{lpfs}->{tag} = 0;
    };
    while ( (my $len = scalar @$next) > 0 ) {
        $acc += $len;
        $max_fan = ($max_fan, $len)[$max_fan < $len];
        $tag++;
        $curr = $next;
        $next = ();
        foreach my $op ( @{ $curr } ) {
            $ops++;
            my $dist = $op->{lpfs}->{dist} + 1;
            foreach my $child ( @{ $op->{out_edges} } ) {
                $children++;
                if ( ! $child->{lpfs}->{followed} ) {
                    $child->{lpfs}->{dist} = ($dist, $child->{lpfs}->{dist})[$dist < $child->{lpfs}->{dist}];
                }
                if ( ! $child->{lpfs}->{tag} ) {
                    $child->{lpfs}->{tag} = $tag;
                    push @$next, $child;
                }
            }
            $path[0] = $op  if ( ! $op->{lpfs}->{followed} 
                                 and $op->{lpfs}->{dist} > $path[0]->{lpfs}->{dist} );
        }
        print "Total ops: $obj->{op_cnt}; visited ops: $ops; children: $children; parents: $parents\n";
        print "Levels: $tag; avg: ".($acc/$tag)."; max: $max_fan\n";
    }
    return  if ( ! @path );
    PATH: while ( $path[0]->{lpfs}->{dist} > 1 ) {
        my $dist = $path[0]->{lpfs}->{dist} - 1;
        $path[0]->take_path( $obj->{lpfs}->{path} );
        foreach my $parent ( @{ $path[0]->{in_edges} } ) {
            $parents++;
            if ( $parent->{lpfs}->{dist} == $dist ) {
                unshift @path, $parent;
                $dist--;
                next PATH;
            }
        }
        print "E: unable to find path from $path[0]->{text} (dist: $path[0]->{lpfs}->{dist})\n";
    }
    if ( $::DEBUG > 20 ) {
        print "Total ops: $obj->{op_cnt}; visited ops: $ops; children: $children; parents: $parents\n";
        print "Levels: $tag; avg: ".($acc/$tag)."; max: $max_fan\n";
    }
    $path[0]->take_path( $obj->{lpfs}->{path} );
    $obj->{lpfs}->{path}++;
    return @path;
}

sub sched_op {
    my $obj = shift;
    my $sched = shift;
    my $op = shift;
    my $ts = shift;
    my $simd = shift;
    if ( defined $op and $op->op_ready($sched) ) {
        push @{ $obj->{$sched}->{$ts}->{$simd} }, $op;
        $obj->{$sched}->{op_cnt}++;
        $op->{$sched}->{simd} = $simd;
        $op->{$sched}->{ts} = $ts;
        $op->{$sched}->{followed} = 1   if ( $sched eq "lpfs" );
        return 1;
    }
    return 0;
}

sub extract_optype {
    my $obj = shift;
    my $sched = shift;
    my $optype = shift;
    my $ready = shift;
    my $i = 0;
    # Sort the ready list by optype and scan for start and end of desired optype
    @$ready = sort {$a->{op} cmp $b->{op}} @$ready;
    while ( $i < scalar @$ready and $ready->[$i]->{op} ne $optype ) { $i++; }
    return  if ( $i >= scalar @$ready );
    my $s = $i;
    # Make sure that we don't schedule too many ops to the SIMD region
    my $size = scalar @{ $ready->[$i]->{args} };
    my $cnt = $size;
    while ( $i < scalar @$ready 
            and $ready->[$i]->{op} eq $optype 
            and $cnt+$size < $obj->{$sched}->{simd_d} ) {
        $i++;
        $cnt += $size;
    }
    my $l = $i - $s;
    return splice @$ready, $s, $l;
}

sub update_moves {
    my $obj = shift;
    my $sched = shift;
    my $ts = shift;
    my $s = $obj->{$sched};
    my $simd = 1;
    my $op = ();
    my $qubit = ();
    my $moves = ();
    my $active = $obj->{active_qubits};
    my $curr = {};
    my $next = {};
    my @simd_active = (0) x ($s->{simd_k}+1);
    # Get all current 
    foreach $simd ( 1..$s->{simd_k} ) {
        $simd_active[$simd] = 1  if ( scalar @{ $s->{$ts}->{$simd} } );
        foreach $op ( @{ $s->{$ts}->{$simd} } ) {
            foreach $qubit ( @{ $op->{args} } ) {
                $curr->{$qubit->{name}} = $simd;
            }
        }
    }
    foreach my $name ( keys %$active ) {
        my $src = $active->{$name};
        my $dst = $curr->{$name} || 0;
        if ( $dst ) {
            # qubit is reused, keep it
            $next->{$name} = $dst;
            delete $curr->{$name};  # remove for fetching
        }
        if ( ! $dst and ! $simd_active[ $src ] ) {
            # Qubit doesn't need to move
            $next->{$name} = $src;
        }
        if ( ( ( $dst ) and $dst != $src )
             or ( ( ! $dst ) and $simd_active[ $src ] ) ) {
            # Moved into new location or SIMD is active and need to move to memory
            $qubit = $obj->{qubits}->{$name};
            push @$moves, bless({
                'op'    => 'MOV',
                'src'   => $src,
                'dst'   => $dst,
                'text'  => "MOV $dst $src $name",
                'args'  => [$qubit]
            },'Op');
            $qubit->{loc} = $dst;
            $s->{op_cnt}++;
            $s->{moves}++;
        }
    }
    foreach my $name ( keys %$curr ) {
        # Qubit not active, fetch from mem
        my $dst = $curr->{$name};
        $next->{$name} = $dst;
        $qubit = $obj->{qubits}->{$name};
        push @$moves, bless({
            'op'    => 'MOV',
            'src'   => 0,
            'dst'   => $dst,
            'text'  => "MOV $dst 0 $name",
            'args'  => [$qubit]
        },'Op');
        $qubit->{loc} = $dst;  # Qubit moved to SIMD
        $s->{op_cnt}++;
        $s->{moves}++;
    }
    $s->{mts}++     if ( @$moves );
    $obj->{active_qubits} = $next;
    @$moves = sort { $a->{text} cmp $b->{text} } @$moves;
    return $moves;
}

sub update_moves_hold_for_stall {
    my $obj = shift;
    my $sched = shift;
    my $ts = shift;
    my $s = $obj->{$sched};
    my $simd = 1;
    my $op = ();
    my $qubit = ();
    my $moves = ();
    my $curr = {};
    my $prev = {};
    foreach $simd ( 1..$s->{simd_k} ) {
        my $pts = $ts-1;
        # Find previous non-empty timestep for this simd region
        while ( $pts >= 0 and ! scalar @{ $s->{$pts}->{$simd} } ) { $pts--; }
        # get all previous qubits
        if ( $pts >= 0 ) {
            foreach $op ( @{ $s->{$pts}->{$simd} } ) {
                foreach $qubit ( @{ $op->{args} } ) {
                    $prev->{$qubit->{name}} = $qubit->{loc};
                }
            }
        }
        # get all current qubits
        foreach $op ( @{ $s->{$ts}->{$simd} } ) {
            foreach $qubit ( @{ $op->{args} } ) {
                $curr->{$qubit->{name}} = [ $simd, $qubit->{loc} ]; #dst, src
            }
        }
    }
    foreach my $name ( sort keys %$curr ) {
        my $src = $curr->{$name}->[1];
        my $dst = $curr->{$name}->[0];
        $qubit = $obj->{qubits}->{$name};
        if ( $src != $dst ) {
            push @$moves, bless({
                'op'    => 'MOV',
                'src'   => $src,
                'dst'   => $dst,
                'args'  => ($qubit),
                'text'  => "MOV $dst $src $name",
                'args'  => [$qubit]
            },'Op');
            $qubit->{loc} = $dst;  # Qubit moved to SIMD
            $obj->{$sched}->{op_cnt}++;
            $obj->{$sched}->{moves}++;
        }
        delete $prev->{$name}   if ( exists $prev->{$name} );
    }
    foreach my $name ( sort {$b cmp $a} keys %$prev ) {
        my $src = $prev->{$name};
        my $dst = 0;
        $qubit = $obj->{qubits}->{$name};
        if ( @{ $s->{$ts}->{$src} } ) {
            push @$moves, bless({
                'op'    => 'MOV',
                'src'   => $src,
                'dst'   => $dst,
                'args'  => ($qubit),
                'text'  => "MOV $dst $src $name",
                'args'  => [$qubit]
            },'Op');
            $qubit->{loc} = $dst;  # Qubit moved to SIMD
            $obj->{$sched}->{op_cnt}++;
            $obj->{$sched}->{moves}++;
        }
    }
    $obj->{$sched}->{mts}++     if ( @$moves );
    @$moves = sort { $a->{text} cmp $b->{text} } @$moves;
    return $moves;
}

sub update_moves_orig {
    my $obj = shift;
    my $sched = shift;
    my $ts = shift;
    my $moves = ();
    my $prev = {};
    # Get qubits from previous timestep
    if ( $ts > 0 ) {
        foreach my $simd ( sort keys %{ $obj->{$sched}->{$ts-1} } ) {
            if ( $simd > 0 and scalar @{ $obj->{$sched}->{$ts}->{$simd} } ) {
                foreach my $op ( @{ $obj->{$sched}->{$ts-1}->{$simd} } ) {
                    foreach my $qubit ( @{ $op->{args} } ) {
                        $prev->{$qubit->{name}} = $qubit;
                    }
                }
            }
        }
    }
    foreach my $simd ( sort keys %{ $obj->{$sched}->{$ts} } ) {
        foreach my $op ( @{ $obj->{$sched}->{$ts}->{$simd} } ) {
            foreach my $qubit ( @{ $op->{args} } ) {
                delete $prev->{$qubit->{name}}; # remove if used in previous timestep
                if ( $qubit->{loc} == -1 ) {
                    #print STDERR "alloc\n";
                    $qubit->{loc} = $simd;  # Qubit allocated to first SIMD use
                } elsif ( $qubit->{loc} != $simd ) {
                    #print STDERR "moved#\n";
                    push @$moves, {
                        'op'    => 'MOV',
                        'src'   => $qubit->{loc},
                        'dst'   => $simd,
                        'args'  => ($qubit),
                        'text'  => "MOV $simd $qubit->{loc} $qubit->{name}",
                        'args'  => [$qubit]
                    };
                    $qubit->{loc} = $simd;  # Qubit moved to SIMD
                    $obj->{$sched}->{op_cnt}++;
                    $obj->{$sched}->{moves}++;
                }
            }
        }
    }
    # Any remaining qubits from previous timestep should be moved to memory
    foreach my $q ( keys %$prev ) {
        my $qubit = $prev->{$q};
        push @$moves, {
            'text'  => "MOV 0 $qubit->{loc} $qubit->{name}",
            'args'  => [$qubit]
        };
        $qubit->{loc} = 0;  # Qubit moved to memory
        $obj->{$sched}->{op_cnt}++;
        $obj->{$sched}->{moves}++;
    }
    $obj->{$sched}->{mts}++     if ( @$moves );
    return $moves;
}

sub print_rcpq {
    my $obj = shift;
    foreach my $op ( @{ $obj->{rcp}->{rcpq} } ) {
        print "$op->{text}\n";
    }
}

sub rcp {
    my $obj = shift;
    my $s = $obj->{rcp};
    my $ts = 0;
    my $op_cnt = 0; # Track gates ops (not moves) scheduled
    my %simds = ();
    my $tgate = 0;
    my $t_cnt = 0;
    my $rcpq = $obj->rcp_init();
    while ( ( keys %{ $rcpq } ) && $op_cnt < $obj->{op_cnt} ) {
        $tgate = 0;
        if ( $ts > $obj->{op_cnt} ) {
            print "E: RCP timestep $ts > op count $obj->{op_cnt}. Aborting.\n";
            return;
        }
        #if ( $::DEBUG > 50 ) {
        #    print "\nRCPQ @ $ts\n-------------\n";
        #    $obj->print_rcpq();
        #}
        foreach ( 1..$s->{simd_k} ) { $simds{$_} = 1; }
        while ( scalar keys %{ $rcpq } && scalar keys %simds ) {
            my $simd = $obj->rcp_sched_next_simd( $rcpq, $ts, keys %simds );
            $op_cnt += scalar @{ $s->{$ts}->{$simd} };
            $tgate |= ( $s->{$ts}->{$simd}->[0]->{op} =~ /^(?:T|Tdag)$/ );
            delete $simds{$simd};
        }
        print "E: Scheduled too many ops ($op_cnt instead of $obj->{op_cnt})\n" if ( $op_cnt > $obj->{op_cnt} );
        $s->{$ts}->{0} = $obj->update_moves( "rcp", $ts );
        $rcpq = $obj->rcpq_update( $ts, $rcpq );
        $ts++;
        $t_cnt += $tgate
    }
    $s->{len} = $ts;
    $s->{width} = $obj->{rcp}->{simd_k};
    $s->{tgates} = $t_cnt;
}

# Initialize RCPQ
# Update: RCPQ
sub rcp_init {
    my $obj = shift;
    my $s = $obj->{rcp};
    my $rcpq = ();
    # copy top to RCPQ
    $s->{op_cnt} = 0;
    foreach my $op ( @{ $obj->{top} } ) {
        $rcpq->{$op->{id}} = $op;
    }
    foreach my $op ( @{ $obj->{ops} } ) {
        $s->{ts}    = -1;
        $s->{simd}  = -1;
    }
    while ( my ($q, $qubit) = each %{ $obj->{qubits} } ) {
        $qubit->{loc}       = 0;
        $qubit->{last_op}   = -1;
    }
    return $rcpq;
}

# Calculate all weights in the RCPQ
# Input: available SIMD regions
# Return: \%{$simd} = {$max, $op, %weights{op} = $weight}
sub rcp_calc_weights {
    my $obj = shift;
    my $rcpq = shift;
    my $ret = {};
    # init $ret
    foreach my $simd (@_) {
        $ret->{$simd} = {
                            max => -2**63,  # set to very low value
                            op  => ''
                        };
    }
    foreach my $op ( values %$rcpq ) {
        my $tmp = $obj->rcp_op_weight( $op, @_ );
        while ( my ($simd, $weight) = each %$tmp ) {
            my $tmp = $ret->{$simd};
            $tmp->{ $op->{op} } += $weight;
            if ( $tmp->{ $op->{op} } > $tmp->{max} ) {
                $tmp->{max} = $tmp->{ $op->{op} };
                $tmp->{op} = $op->{op};
            }
        }
    }
    return $ret;
}

# Calculate operation weight for a simd region
# Inputs: $op = Op, @simd = available SIMD regions
# Return: $weight
sub rcp_op_weight {
    my $obj = shift;
    my $op = shift;
    my $w_op = $obj->{rcp}->{w_op};
    my $w_slack = $obj->{rcp}->{w_slack};
    my $w_dist = $obj->{rcp}->{w_dist};
    my $ret = {};
    foreach my $simd (@_) {
            # Weight for the communication distance
            my $dist = 1;
            foreach my $qubit ( @{ $op->{args} } ) {
                # Check that all qubits are in the current simd region
                $dist &= ($qubit->{loc} == -1 || $qubit->{loc} == $simd);
            }
            my $weight = $w_op +
                         $w_slack * $op->{slack} +
                         $w_dist * $dist;
            #$op->{rcp}->{simd_weight}->{$simd} = $weight;
            $ret->{$simd} = $weight;
    }
    return $ret;
}

# Get the next highest SIMD weight
# Input: timestep
# Side-effect: Schedule next highest SIMD region to timestep
#   $obj->{rcp}->{$ts}->{$simd} is populated
# Return: SIMD region populated
sub rcp_sched_next_simd {
    my $obj = shift;
    my $rcpq = shift;
    my $ts = shift;
    my $weights = $obj->rcp_calc_weights( $rcpq, @_ );
    # Sort by highest weight, then by lowest simd_k region
    my $simd = (sort { $weights->{a}->{max} <=> $weights->{b}->{max} || $a <=> $b } keys %$weights )[0];
    print "E: $simd already scheduled at timestep $ts!\n"  if ( @{ $obj->{rcp}->{$ts}->{$simd} } );
    foreach my $op ( $obj->rcpq_extract_optype( $rcpq, $weights->{$simd}->{op} ) ) {
        $obj->sched_op( "rcp", $op, $ts, $simd );
    }
    return $simd;
}

# Gets all operations of the requested type from the RCPQ and removes matches 
# from RCPQ
# Input: $optype
# Update: RCPQ
# Return: \@ = [Op]
sub rcpq_extract_optype {
    my $obj = shift;
    my $rcpq = shift;
    my $optype = shift;
    my $i = 0;
    my $size = 0;
    my $simd_d = $obj->{rcp}->{simd_d};
    my @ret = ();
    while ( ( my ($id, $op) = each %$rcpq ) and
            $size * ( 1 + scalar @ret ) < $simd_d ) {
        if ( $op->{op} eq $optype ) {
            $size |= scalar @{ $op->{args} };
            push @ret, $op;
            delete $rcpq->{$id};
        }
    }
    return @ret;
    # Sort the RCPQ by optype and scan for start and end of desired optype
    #@{$obj->{rcp}->{rcpq}} = sort {$a->{op} cmp $b->{op}} @{ $obj->{rcp}->{rcpq} };
    #while ( $i < scalar @{$obj->{rcp}->{rcpq}} and $obj->{rcp}->{rcpq}->[$i]->{op} ne $optype ) { $i++; }
    #my $s = $i;
    ## Make sure that we don't schedule too many ops to the SIMD region
    #my $size = scalar @{ $obj->{rcp}->{rcpq}->[$i]->{args} };
    #my $cnt = $size;
    #while ( $i < scalar @{$obj->{rcp}->{rcpq}} 
    #        and $obj->{rcp}->{rcpq}->[$i]->{op} eq $optype 
    #        and $cnt+$size < $obj->{rcp}->{simd_d} ) {
    #    $i++;
    #    $cnt += $size;
    #}
    #my $l = $i - $s;
    #return splice @{$obj->{rcp}->{rcpq}}, $s, $l;
}

# Checks all scheduled ops and adds them to the RCPQ
# Input: $ts, $rcpq pointer
# Update: RCPQ
# Return: null
sub rcpq_update {
    my $obj = shift;
    my $ts = shift;
    my $rcpq = shift;
    my $s = $obj->{rcp};
    my $simd_k = $s->{simd_k};
    foreach my $simd ( 1..$simd_k ) {
        foreach my $op ( @{ $s->{$ts}->{$simd} } ) {
            foreach my $child ( $op->get_ready_children("rcp") ) {
                $rcpq->{$child->{id}} = $child;
            }
        }
    }
    return $rcpq;
}

# Determine fetches from memory and other simd regions
# Input: current timestep and SIMD region
# Side-effect: propagates scheduling to operations
# Output: @[Qubit] = all qubits moving to SIMD region
sub rcp_simd_fetch {
    my $obj = shift;
    my $ts = shift;
    my $simd = shift;
    my @qubits = ();
    foreach my $op ( @{ $obj->{rcp}->{$ts}->{$simd} } ) {
        push @qubits, $op->rcp_fetch_op( $ts, $simd );
    }
    return @qubits;
}

# Determine stores to memory
# NOTE: must be run after rcp_simd_fetch - depends on scheduling information 
#       for current cycle
# Input: current timestep
# Output: @[Qubit] = all qubits moving out of SIMD region
sub rcp_simd_store {
    my $obj = shift;
    my $ts = shift;
    my $simd = shift;
    my @qubits = ();
    # look at the previous cycle's ops in the simd region and remove the ones
    # that aren't scheduled for this cycle
    $ts--;
    if ( exists $obj->{rcp}->{$ts} ) {
        foreach my $op ( @{ $obj->{rcp}->{$ts}->{$simd} } ) {
            push @qubits, $op->rcp_store_op( $ts );
        }
    }
    return @qubits;
}

sub ss_init {
    my $obj = shift;
    foreach my $op ( @{ $obj->{ops} } ) {
        $op->{ss}->{ts}     = -1;
        $op->{ss}->{simd}   = -1;
    }
    while ( my ($q, $qubit) = each %{ $obj->{qubits} } ) {
        $qubit->{loc}       = 0;
        $qubit->{last_op}   = -1;
    }
}

sub ss {
    my $obj = shift;
    my $ss = $obj->{ss};
    my $len = 0;
    my $width = 0;
    $obj->ss_init();
    foreach my $op ( @{ $obj->{ops} } ) {
        my $ts = $op->{rank}-1;
        my $simd = 1;
        my $done = 0;
        while ( ! $done and $simd <= $ss->{simd_k} ) {
            if ( ! @{ $ss->{$ts}->{$simd} } 
                 or ( scalar @{ $ss->{$ts}->{$simd} } < $ss->{simd_d}
                 and $ss->{$ts}->{$simd}->[0]->{op} eq $op->{op} ) ) {
                $done = $obj->sched_op( "ss", $op, $ts, $simd );;
                $width = ($width, $simd)[$width < $simd]    if ($done);
            }
            $simd++;
        }
        print "E: unable to place $op->{text} at $ts\n"     if ( ! $done );
        $len = ::max($len, $ts+1);
    }
    for ( my $ts=0; $ts < $len; $ts++ ) {
        $ss->{$ts}->{0} = $obj->update_moves("ss", $ts);
    }
    $ss->{len} = $len;
    $ss->{width} = $width;
    $obj->tgate_cnt("ss");
}

sub tgate_cnt {
    my $obj = shift;
    my $sched = shift;
    my $s = $obj->{$sched};
    my $tgate = 0;
    my $t_cnt = 0;
    foreach my $ts ( 0..( $s->{len}-1 ) ) {
        $tgate = 0;
        foreach my $simd ( 1..$s->{simd_k} ) {
            if ( defined $s->{$ts}->{$simd}->[0] and
                 $s->{$ts}->{$simd}->[0]->{op} =~ /^(?:T|Tdag)$/ ) {
                $tgate = 1;
            }
        }
        $t_cnt += $tgate;
    }
    $s->{tgates} = $t_cnt;
}

sub cpr {
    my $obj = shift;
    my $threshold = shift || 4;
    my @qconflicts = ();
    my @conflicts = ();
    my %stats = (
        'count'     => 0,
        'mean'      => 0,
        'median'    => 0,
        'mode'      => 0,
        'mode_cnt'  => 0,
        'hist'      => {},
        'var'       => 0,
        'min'       => 2147483647,
        'max'       => 0,
        'range'     => 0,
        'acc'       => 0,
        'raw'       => []
    );
    # Calculate cpr for each qubits' ops, collect intermediate stats on the way
    foreach my $qref ( keys %{ $obj->{qubits} } ) {
        my $qubit = $obj->{qubits}->{$qref};
        $qubit->{cpr}->[0] = 0;
        $qubit->{last_op} = 0;
        for ( my $i=1; $i<scalar @{ $qubit->{ops} }; $i++ ) {
            $qubit->{cpr}->[$i] = $qubit->{ops}->[$i]->{asap}->{ts} - $qubit->{ops}->[$i-1]->{asap}->{ts};
            $stats{acc} += $qubit->{cpr}->[$i];
            push @{ $stats{raw} }, $qubit->{cpr}->[$i];
            $stats{min} = ::min( $stats{min}, $qubit->{cpr}->[$i] );
            $stats{max} = ::max( $stats{max}, $qubit->{cpr}->[$i] );
            $stats{count}++;
        }
        if ( scalar @{$qubit->{cpr}} != scalar @{$qubit->{ops}} ) {
            printf "Warning: CPR count doesn't match op count (%d != %d)\n", scalar @{$qubit->{cpr}}, scalar @{$qubit->{ops}};
        }
    }
    # Calculate remaining stats
    if ( $stats{count} > 0 ) {
        $stats{mean} = $stats{acc} / $stats{count};
        $stats{median} = $stats{raw}->[int ( $stats{count}/2 + 0.5 )];
        foreach my $cpr ( @{ $stats{raw} } ) {
            $stats{var} += (abs ($cpr - $stats{mean}))**2;
            $stats{hist}->{$cpr}++;
            if ( $stats{hist}->{$cpr} > $stats{mode_cnt} ) {
                $stats{mode} = $cpr;
                $stats{mode_cnt} = $stats{hist}->{$cpr};
            }
        }
        $stats{var} /= $stats{count};
        $stats{range} = $stats{max} - $stats{min};
    }
    # Check ops for conflicts
    foreach my $ts ( sort {$a <=> $b} keys %{ $obj->{asap} } ) {
        if ( $ts =~ /^-?\d+$/ ) {
            foreach my $op ( @{ $obj->{asap}->{$ts} } ) {
                # If there are more than two arguments, check for a move conflict
                if ( scalar @{ $op->{args} } > 1 ) {
                    # Foreach qubit, check that the CPR value for the current operation is below the threshold and add it to @qconflict
                    foreach my $qubit ( @{ $op->{args} } ) {
                        my $idx = $qubit->{last_op};
                        if ( ::refaddr($op) == ::refaddr($qubit->{ops}->[$idx]) ) {
                            $qubit->{last_op}++;
                        } else {
                            # Start at next index and search through list - it's probably after the current one, not before
                            $idx++;
                            while ( $idx != $qubit->{last_op} && ::refaddr($op) != ::refaddr($qubit->{ops}->[$idx]) ) {
                                $idx = ($idx + 1) % scalar @{ $qubit->{ops} };
                            }
                            die "Qubit $qubit->{name} not found in operation '$op->{text}'\n"   if ( $idx == $qubit->{last_op} );
                            $qubit->{last_op} = $idx+1;
                        }
                        push @qconflicts, $qubit    if ( $qubit->{cpr}->[$idx] > 0 && $qubit->{cpr}->[$idx] <= $threshold );
                    }
                    # If $#qconflict > 0, add operation to @conflict
                    push @conflicts, $op    if ( scalar @qconflicts > 1 );
                }
                undef @qconflicts;  # free memory
            }
        }
    }
    # Print results
    my $msg = "$obj->{function}:";
    print "$msg\n";
    $msg =~ s/./-/g;
    print "$msg\n";
    foreach ( qw(mean median mode var range) ) { printf "%9s = %d\n", $_, $stats{$_}; }
    print "Histogram:\n";
    foreach ( sort {$a <=> $b} keys %{ $stats{hist} } ) { printf "%9s = %d\n", $_, $stats{hist}->{$_}; }
    printf "%s (%d / %d):\n", "conflicts", scalar @conflicts, $obj->{op_cnt};
    foreach my $op ( @conflicts ) { print "    $op->{text}\n"; }
    print "\n";
}

sub sched_check {
    my $obj = shift;
    my $type = shift || "asap";
    foreach my $ts ( sort keys %{ $obj->{$type} } ) {
        if ( $ts =~ /^-?\d+$/ ) {
            foreach my $op ( @{ $obj->{$type}->{$ts} } ) {
                foreach my $pre ( @{ $op->{in_edges} } ) {
                    print  "E: Dependency error; '$op->{text}'\@$op->{$type}->{ts}, not after '$pre->{text}'\@$pre->{$type}->{ts}\n"
                            if ( $pre->{$type}->{ts} >= $op->{$type}->{ts} );
                }
            }
        }
    }
}

sub header_print {
    my $obj = shift;
    my $sched = shift;
    my $msg = "Function: $obj->{function} ";
    $msg .= "(sched: $sched, op_cnt: $obj->{$sched}->{op_cnt}, ";
    $msg .= "k: $obj->{$sched}->{simd_k}, d: $obj->{$sched}->{simd_d}";
    $msg .= ", l: $obj->{$sched}->{simd_l}"           if ( $sched eq "lpfs" );
    $msg .= ", opp: $obj->{$sched}->{opp_simd}"       if ( $sched eq "lpfs" );
    $msg .= ", refill: $obj->{$sched}->{refill_simd}" if ( $sched eq "lpfs" );
    $msg .= ", w_op: $obj->{$sched}->{w_op}"          if ( $sched eq "rcp" );
    $msg .= ", w_dist: $obj->{$sched}->{w_dist}"      if ( $sched eq "rcp" );
    $msg .= ", w_slack: $obj->{$sched}->{w_slack}"    if ( $sched eq "rcp" );
    $msg .= ")";
    print "$msg\n";
    $msg =~ s/./=/g;
    print "$msg\n";
}

sub metrics_print {
    my $obj = shift;
    my $sched = shift;
    my $s = $obj->{$sched};
    my $k = $s->{simd_k};
    my $d = $s->{simd_d};
    my $w = $s->{width};
    my $ops = $s->{op_cnt} - $s->{moves};
    my $t1 = $obj->{op_cnt} * 2.0;
    my $w1 = $obj->{op_cnt} * 3.0;
    my $tinf = $obj->{length}; # inf means no comms.?
    my $tk = $s->{tk} = $s->{len} + 4*$s->{mts};
    my $wk = $s->{wk} = $s->{op_cnt};
    my $sk = $s->{sk} = $t1 / $tk;
    my $ek = $s->{ek} = $t1 / ($k * $tk);
    my $uk = $s->{uk} = $wk / ($k * $tk);
    my $qk = $s->{qk} = $t1**3 / ($k * $tk**2 * $wk);
    my $ck = $s->{ck} = 100.0 * $s->{moves} / $s->{op_cnt};
    my $sav = 200/3 - $ck;
    my $avg = $s->{avg} = ($s->{mts}) ? $s->{moves} / $s->{mts} : "inf";
    my $max = 0;
    my @moves = ();
    foreach my $ts ( 0..($s->{len} - 1) ) {
        push( @moves, scalar @{ $s->{$ts}->{0} } );
        $max = ( $max, $moves[-1] )[$max < $moves[-1]];
    }
    $s->{max} = $max;
    print "ops = $ops\n";
    print "moves = $s->{moves}\n";
    print "total = $s->{op_cnt}\n";
    print "ots = $s->{len}\n";
    print "mts = $s->{mts}\n";
    print "ts = $tk\n";
    print "SIMDs = $w\n";
    print "tgates = $s->{tgates}\n";
    print "T(1) = $t1\n";
    print "T(inf) = $tinf\n";
    print "T($k,$d) = $tk\n";
    print "Speedup = $sk\n";
    print "Efficiency = $ek\n";
    print "Utility = $uk\n";
    print "Quality = $uk\n";
    print "Overhead = $ck% (reduction: $sav)\n";
    print "Avg load = $avg\n";
    print "Peak load = $max\n";
    print "mlist = ".(join " ", @moves)."\n";
    print "\n";
}

sub sched_print {
    my $obj = shift;
    my $sched = shift;
    my $op_cnt = 0;
    my $i = 0;
    foreach my $ts ( 0..$obj->{$sched}->{len} ) {
        foreach my $simd ( 0..$obj->{$sched}->{simd_k} ) {
            foreach my $op ( sort { $a->{text} cmp $b->{text} } @{ $obj->{$sched}->{$ts}->{$simd} } ) {
                print "$ts,$simd $op->{text}\n";
                $op_cnt++;
            }
        }
    }
    print "E: Printed $op_cnt, expected $obj->{$sched}->{op_cnt}\n"  if ( $op_cnt != $obj->{$sched}->{op_cnt} );
    print "\n";
}

sub sched_pretty_print {
    my $obj = shift;
    my $sched = shift;
    my $op_cnt = 0;
    my $i = 0;
    my $cw = 1 + $obj->{twidth};
    my @ts = undef;
    if ( @_ ) {
        @ts = @_;
    } else {
        @ts = sort {$a <=> $b} keys %{ $obj->{$sched} };
    }
    my $at = ::max( 4, length sprintf "%d", $ts[-1] ); # 4 is length of string "Time"
    # Print column headings
    $msg = sprintf "%${at}s | %-${cw}s", "Time", "Moves";
    for ($i=1; $i<=$obj->{$sched}->{simd_k}; $i++) {
        $msg .= sprintf "| %-${cw}s", "SIMD $i";
    }
    print "$msg\n";
    $msg =~ s/./-/g;
    print "$msg\n";
    foreach my $time ( @ts ) {
        if ( $time =~ /^-?\d+$/ ) {
            my $rows = 0;
            my $ts = $time;
            # Determine number of rows that will be printed for this timestep
            for ($i=0; $i<=$obj->{$sched}->{simd_k}; $i++) {
                $rows = ::max( $rows, scalar @{$obj->{$sched}->{$ts}->{$i}} );
            }
            # Print each operation in the proper SIMD column
            for ($i=0; $i<$rows; $i++) {
                printf "%${at}s ", $time;
                $op_cnt++   if ( defined $obj->{$sched}->{$ts}->{0}->[$i] );
                printf "| %-${cw}s", $obj->{$sched}->{$ts}->{0}->[$i]->{text} || "";
                for (my $simd=1; $simd<=$obj->{$sched}->{simd_k}; $simd++) {
                    $op_cnt++   if ( defined $obj->{$sched}->{$ts}->{$simd}->[$i] );
                    printf "| %-${cw}s", $obj->{$sched}->{$ts}->{$simd}->[$i]->{text} || "";
                }
                print "\n";
                $time = ""; # only print $time on first row
            }
            print "$msg\n";
        }
    }
    print "E: Printed $op_cnt, expected $obj->{$sched}->{op_cnt}\n"  if ( $op_cnt != $obj->{$sched}->{op_cnt} );
    print "\n";
}

sub sched_true_print {
    my $obj = shift;
    my $sched = shift;
    my $s = $obj->{$sched};
    my $simd_k = $s->{simd_k};
    my $op = '';
    my $simd = 0;
    # real timestep - includes moves
    my $rts = 0;
    foreach my $fts ( 0..$s->{len} ) {
        my $curr = $s->{$fts};
        if ( exists $curr->{0} && scalar @{ $curr->{0} } ) {
            # Track all source and dest moves
            my @m_src = (0) x ( $simd_k + 1 );
            my @m_dst = (0) x ( $simd_k + 1 );
            foreach my $mov ( @{ $curr->{0} } ) {
                $m_src[$mov->{src}] = 1;
                $m_dst[$mov->{dst}] = 1;
            }
            # Print 3 timesteps where src is active (ignore memory SIMD)
            #foreach $op ( qw(H CNOT MEAS) ) {
            foreach $op ( qw(H CNOT) ) {
                foreach $simd ( 1..$simd_k ) {
                    print "$rts $simd MOV($op)\n"    if ( $m_src[$simd] );
                }
                $rts++;
            }
            # Print 2 timestep where dest is active
            foreach $op ( qw(X Z) ) {
                foreach $simd ( 1..$simd_k ) {
                    print "$rts $simd MOV($op)\n"    if ( $m_dst[$simd] );
                }
                $rts++;
            }
        }
        foreach $simd ( 1..$simd_k ) {
            # Print operation of the current SIMD for current timestep
            print "$rts $simd $curr->{$simd}->[0]->{op}\n"  if ( UNIVERSAL::isa( $curr->{$simd}->[0], "Op" ) );
        }
        $rts++;
    }
}

sub print {
    my $obj = shift;
    my $sched = shift || "asap";
    my $msg = "Function: $obj->{function} (op_cnt: $obj->{$sched}->{op_cnt}, max: $obj->{$sched}->{threads})";
    my @ts = sort {$a <=> $b} keys %{ $obj->{$sched} };
    my $op_cnt = 0;
    #print join " ", @ts;
    #print "\n";
    my $at = length sprintf "%d", $ts[-1];
    my $cw = 3 + $obj->{twidth};
    print "$msg\n";
    $msg =~ s/./=/g;
    print "$msg\n";
    foreach my $time ( @ts ) {
        if ( $time =~ /^-?\d+$/ ) {
            printf "%${at}d ", $time;
            @{$obj->{$sched}->{$time}} = sort { $a->{id} <=> $b->{id} } @{ $obj->{$sched}->{$time} };
            my $i=0;
            # Print operations
            foreach my $op ( @{$obj->{$sched}->{$time}} ) {
                printf "%-${cw}s", "| $op->{text}";
                $i++;
                $op_cnt++;
            }
            # Print formatting for unscheduled ops
            for (; $i<$obj->{$sched}->{threads}; $i++) {
                printf "%-${cw}s", "| "
            }
            print "\n";
        }
    }
    print "E: Printed $op_cnt, expected $obj->{$sched}->{op_cnt}\n"  if ( $op_cnt != $obj->{$sched}->{op_cnt} );
    print "\n";
}


package main;

use Getopt::Long;
use List::Util qw(max min);
use Scalar::Util qw(refaddr);
use Data::Dumper;
sub dm {$Data::Dumper::Maxdepth = shift || 3};

# Schedule parameters
$::DEBUG = 100;
$::SIMD_K = 4;
$::SIMD_D = 1024;
$::SIMD_L = 2;  # Number of SIMD regions allocated to longest paths
$::refill = 0;
$::opp = 0;
$::w_op = 1;
$::w_dist = -1;
$::w_slack = 1;
my $name = "lpfs";
my $metrics = 0;
my $schedule = 0;
my $pretty = 0;
my $true = 0;
my $dot = 0;
my $all = 0;
our %opts = (
    'asap'          => 0,
    'alap'          => 0,
    'acap'          => 0,
    'ss'            => 0,
    'rcp'           => 0,
    'lpfs'          => 0,
    'lpfs_debug'    => 0,
    'cpr'           => 0,
    'dot'           => 0,
    'leaves_only'   => 1,
    'conflict'      => 0
);

GetOptions ("d=i"       => \$::SIMD_D,
            "k=i"       => \$::SIMD_K,
            "l=i"       => \$::SIMD_L,
            "refill"    => \$::refill,
            "opp"       => \$::opp,
            "op:i"      => \$::w_op,
            "dist:i"    => \$::w_dist,
            "slack:i"   => \$::w_slack,
            "n=s"       => \$name,
            "g"         => \$dot,
            "m"         => \$metrics,
            "s"         => \$schedule,
            "p"         => \$pretty,
            "t"         => \$true,
            "a"         => \$all,
            "DEBUG=i"   => \$::DEBUG);
if ( $::SIMD_L >= $::SIMD_K ) {
    $::SIMD_L = $::SIMD_K >> 1; # half, rounded down
}
if ( $all ) { $metrics = 1; $schedule = 1; }
if ( ! ( $metrics or $schedule or $pretty or $dot or $true ) ) { $metrics = 1; }
if ( $::DEBUG >= 10 ) {
    print "M: \$::SIMD_K=$::SIMD_K; \$::SIMD_D=$::SIMD_D; \$::SIMD_L=$::SIMD_L\n";
}
if ( ! exists $opts{$name} ) {
    die "Invalid sched name $name!\n";
} else {
    $opts{$name} = 1;
}

our %func_sched = (
    'START'   => {'length' => 0},
    'END'     => {'length' => 0},
    'PrepZ'   => {'length' => 1},
    'MeasX'   => {'length' => 1},
    'MeasZ'   => {'length' => 1},
    'CNOT'    => {'length' => 1},
    'H'       => {'length' => 1},
    'S'       => {'length' => 1},
    'Sdag'    => {'length' => 1},
    'T'       => {'length' => 1},
    'Tdag'    => {'length' => 1},
    'X'       => {'length' => 1},
    'Y'       => {'length' => 1},
    'Z'       => {'length' => 1}
);
our @func_names = ();

sub max_key {
    my $hash = shift;
    my ($max, $ret) = each %$hash;
    while ( my ($k, $v) = each %$hash ) {
        if ( $max < $v ) {
            $max = $v;
            $ret = $k;
        }
    }
    return $ret;
}

sub main {
    my $file = $ARGV[0];
    my $function = '';

    open SCHED, $ARGV[0] or die "Unable to open file '$ARGV[0]': $!\n";

    while (<SCHED>) {
        chomp;
        if (/#Function (\w+)/) {
            $function = $1;
            push @func_names, $function;
            $func_sched{$function} = Schedule->new($function);

        } elsif (/#EndFunction/) {
            if ( $opts{asap} ) {
                print  "ASAP:\n";
                $func_sched{$function}->sched_check("asap");
                $func_sched{$function}->print("asap");
            }
            if ( $opts{alap} ) {
                print  "ALAP:\n";
                $func_sched{$function}->alap();
                $func_sched{$function}->sched_check("alap");
                $func_sched{$function}->print("alap");
            }
            if ( $opts{acap} ) {
                print  "ACAP:\n";
                $func_sched{$function}->acap();
                #print ::Dumper $func_sched{$function}->{acap};
                $func_sched{$function}->sched_check("acap");
                $func_sched{$function}->print("acap");
                $function = '';
            }
            if ( $opts{ss} ) {
                print  "SS:\n";
                $func_sched{$function}->ss();
                $func_sched{$function}->header_print("ss")          if ( $metrics or $schedule or $pretty or $true );
                $func_sched{$function}->metrics_print("ss")         if ( $metrics );
                $func_sched{$function}->sched_print("ss")           if ( $schedule );
                $func_sched{$function}->sched_pretty_print("ss")    if ( $pretty );
                $func_sched{$function}->sched_true_print("ss")      if ( $true );
            }
            if ( $opts{rcp} ) {
                print  "RCP:\n";
                $func_sched{$function}->rcp();
                $func_sched{$function}->header_print("rcp")         if ( $metrics or $schedule or $pretty or $true );
                $func_sched{$function}->metrics_print("rcp")        if ( $metrics );
                $func_sched{$function}->sched_print("rcp")          if ( $schedule );
                $func_sched{$function}->sched_pretty_print("rcp")   if ( $pretty );
                $func_sched{$function}->sched_true_print("rcp")     if ( $true );
            }
            if ( $opts{lpfs} ) {
                print  "LPFS:\n";
                $func_sched{$function}->lpfs();
                $func_sched{$function}->header_print("lpfs")        if ( $metrics or $schedule or $pretty or $true );
                $func_sched{$function}->metrics_print("lpfs")       if ( $metrics );
                $func_sched{$function}->sched_print("lpfs")         if ( $schedule );
                $func_sched{$function}->sched_pretty_print("lpfs")  if ( $pretty );
                $func_sched{$function}->sched_true_print("lpfs")    if ( $true );

            }
            if ( $opts{lpfs_debug} ) {
                my @path = $func_sched{$function}->find_lp( $func_sched{$function}->{top} );
	 	$func_sched{$function}->header_print("lpfs"); 
                while ( 3 < scalar @path ) {
                    foreach my $op ( @path ) {
                        print "$op->{dist} - $op->{text}\n";
                    }
		    @path = $func_sched{$function}->find_lp( $func_sched{$function}->{top} );
		    $func_sched{$function}->header_print("lpfs");
                }
                foreach my $op ( @path ) {
                    print "$op->{dist} - $op->{text}\n";
                }
                print "\n";
            }
            if ( $opts{cpr} ) {
                print  "Critical Path Report:\n";
                $func_sched{$function}->cpr();
            }
            if ( $opts{dot} ) {
                $func_sched{$function}->draw_graph("$ARGV[0]\.$function\.dot");
            }
            if ( $opts{conflict} ) {
            }
            if ( $opts{leaves_only} ) {
                # If only processing leaves, do not keep old functions
                delete $func_sched{$function};
            }
            #print "\n";

        # Check for defined function region, parse command, and verify that the command is defined
        } elsif ($function ne '' and /^\d+/ ) {
            my ($rank, $oper, @args) = split / /;
            if (! exists $func_sched{$oper}) {
                print  "Unknown command in function $function: $oper (line: $_)\n";
                next;
            }
            my $op = Op->new($oper, $rank, $func_sched{$oper}->{length});
            foreach my $arg (@args) {
                if ( ! exists($func_sched{$function}->{qubits}->{$arg}) ) {
                    $func_sched{$function}->{qubits}->{$arg} = Qubit->new($arg);
                }
                $func_sched{$function}->{qubits}->{$arg}->add_dep($op);
                $op->add_qubit($func_sched{$function}->{qubits}->{$arg});
            }
            $func_sched{$function}->add_op( $op );

        } elsif (/^#/ or /^\s*$/) {
            # comment or blank line, do nothing

        } else {
            print  "Unparsed line(func: $function, cmd: $1): $_\n";
        }
    }
    close SCHED;
}

dm();
main();

