#!/usr/bin/perl

use File::Basename;

# Generate overview page

open OV, "> overview.dox";
print OV "/**\n";
print OV ' * @page overview Overview'."\n";
print OV " *\n";
print OV " * This page gives an outline of all components in the libraries.\n";
print OV " *\n";
print OV ' * @section core Core'."\n";
print OV ' * @htmlonly'."\n";
print OV ' * <div style="position: absolute; left: 0px; top: 200px"><img id="info_image" src="" border="0" /></div>'."\n";
print OV " * <div class=\"contents\">\n";
print OV " * <table border=\"0\">\n";
my %core = ( "1_blank" => [
                 [ "core/gate.hpp", "file" ],
                 [ "core/circuit.hpp", "file" ],
                 [ "core/functor.hpp", "file" ],
                 [ "core/pattern.hpp", "class" ],
                 [ "core/properties.hpp", "file" ],
                 [ "core/target_tags.hpp", "file" ],
                 [ "core/truth_table.hpp", "file" ],
                 [ "core/version.hpp", "file" ]
                ],
             "Functions" => [
                 [ "core/functions/active_controls.hpp", "file" ],
                 [ "core/functions/add_circuit.hpp", "file" ],
                 [ "core/functions/add_gates.hpp", "file" ],
                 [ "core/functions/add_line_to_circuit.hpp", "file" ],
                 [ "core/functions/circuit_hierarchy.hpp", "file" ],
                 [ "core/functions/circuit_to_truth_table.hpp", "file" ],
                 [ "core/functions/clear_circuit.hpp", "file" ],
                 [ "core/functions/control_lines.hpp", "file" ],
                 [ "core/functions/copy_circuit.hpp", "file" ],
                 [ "core/functions/copy_metadata.hpp", "file" ],
                 [ "core/functions/create_simulation_pattern.hpp", "file" ],
                 [ "core/functions/expand_circuit.hpp", "file" ],
                 [ "core/functions/extend_truth_table.hpp", "file" ],
                 [ "core/functions/find_lines.hpp", "file" ],
                 [ "core/functions/flatten_circuit.hpp", "file" ],
                 [ "core/functions/fully_specified.hpp", "file" ],
                 [ "core/functions/reverse_circuit.hpp", "file" ],
                 [ "core/functions/target_lines.hpp", "file" ],
                 [ "core/functions/transposition_to_circuit.hpp", "file" ]
               ],
             "I/O" => [
                 [ "core/io/create_image.hpp", "file" ],
                 [ "core/io/print_circuit.hpp", "file" ],
                 [ "core/io/print_statistics.hpp", "file" ],
                 [ "core/io/read_pattern.hpp", "file" ],
                 [ "core/io/read_realization.hpp", "file" ],
                 [ "core/io/read_specification.hpp", "file" ],
                 [ "core/io/read_pla.hpp", "file" ],
                 [ "core/io/revlib_parser.hpp", "file" ],
                 [ "core/io/revlib_processor.hpp", "class" ],
                 [ "core/io/write_blif.hpp", "file" ],
                 [ "core/io/write_realization.hpp", "file" ],
                 [ "core/io/write_specification.hpp", "file" ],
                 [ "core/io/write_verilog.hpp", "file" ]
               ],
             "Meta" => [
                 [ "core/meta/bus_collection.hpp", "class" ]
               ],
             "Utils" => [
                 [ "core/utils/costs.hpp", "file" ],
                 [ "core/utils/program_options.hpp", "file" ],
                 [ "core/utils/timer.hpp", "file" ]
               ]
);

foreach $key (sort keys %core) {
    if ( !( $key eq "1_blank" ) ) {
        print OV " * <tr><td class=\"indexkey\" colspan=\"2\" style=\"background-color:#a0a0a0\"><i>$key</i></td></tr>\n";
    }
    foreach $arr (@{$core{$key}}) {
        $filename = "../../src/".$$arr[0];
        $name = substr basename( $filename ), 0, -4;

        $type     = $$arr[1];
        $new_name = $name;
        $new_name =~ s/_/__/g;
        if ( $type eq "class" ) {
            $link = "classrevkit_1_1$new_name.html";
        } elsif ( $type eq "file" ) {
            $link = $new_name."_8hpp.html";
        }

        $info = `./read_file_brief.sh $filename`;
        $is_new = `./is_new.sh $filename`;
        print OV " * <tr><td class=\"indexkey\"><a class=\"el\" href=\"$link\">$name</a></td><td class=\"indexvalue\">$info <small style=\"font-weight: bold; color: red\">$is_new</small></td></tr>\n";
    }
}

print OV " * </table>\n";
print OV " * </div>\n";
print OV ' * @endhtmlonly'."\n";

print OV ' * @section algorithms Algorithms'."\n";
print OV ' * @htmlonly'."\n";
print OV " * <div class=\"contents\">\n";
print OV " * <table border=\"0\">\n";
my %algorithms = (
    "Optimization" => [
        [ "algorithms/optimization/adding_lines.hpp", "file" ],
        [ "algorithms/optimization/line_reduction.hpp", "file" ],
        [ "algorithms/optimization/window_optimization.hpp", "file" ]
    ],
    "Simulation" => [
        [ "algorithms/simulation/partial_simulation.hpp", "file" ],
        [ "algorithms/simulation/sequential_simulation.hpp", "file" ],
        [ "algorithms/simulation/simple_simulation.hpp", "file" ]
    ],
    "Synthesis" => [
        [ "algorithms/synthesis/bdd_synthesis.hpp", "file" ],
        [ "algorithms/synthesis/embed_truth_table.hpp", "file" ],
        [ "algorithms/synthesis/esop_synthesis.hpp", "file" ],
        [ "algorithms/synthesis/exact_synthesis.hpp", "file" ],
        [ "algorithms/synthesis/kfdd_synthesis.hpp", "file" ],
        [ "algorithms/synthesis/quantum_decomposition.hpp", "file" ],
        [ "algorithms/synthesis/reed_muller_synthesis.hpp", "file" ],
        [ "algorithms/synthesis/swop.hpp", "file" ],
        [ "algorithms/synthesis/transformation_based_synthesis.hpp", "file" ],
        [ "algorithms/synthesis/transposition_based_synthesis.hpp", "file" ]
    ],
    "Verification" => [
        [ "algorithms/verification/equivalence_check.hpp", "file" ]
    ]
);

foreach $key (keys %algorithms) {
    if ( !( $key eq "_blank" ) ) {
        print OV " * <tr><td class=\"indexkey\" colspan=\"2\" style=\"background-color:#a0a0a0\"><i>$key</i></td></tr>\n";
    }
    foreach $arr (@{$algorithms{$key}}) {
        $filename = "../../src/".$$arr[0];
        $name = substr basename( $filename ), 0, -4;

        $type     = $$arr[1];
        $new_name = $name;
        $new_name =~ s/_/__/g;
        if ( $type eq "class" ) {
            $link = "classrevkit_1_1$new_name.html";
        } elsif ( $type eq "file" ) {
            $link = $new_name."_8hpp.html";
        }

        $info = `./read_file_brief.sh $filename`;
        $is_new = `./is_new.sh $filename`;
        print OV " * <tr><td class=\"indexkey\"><a class=\"el\" href=\"$link\">$name</a></td><td class=\"indexvalue\">$info <small style=\"font-weight: bold; color: red\">$is_new</small></td></tr>\n";
    }
}

print OV " * </table>\n";
print OV " * </div>\n";
print OV ' * @endhtmlonly'."\n";

print OV " */\n";
close OV;

# Generage mainpage.dox

open MP, "> mainpage.dox";

print MP "/**\n";
print MP ' * @mainpage RevKit 1.3 Developer Documentation'."\n";

#@pages = ( "getting_started", "first_steps", "tutorial_window_optimization", "enhance_core", "adding_algorithms", "acknowledgement", "references" );
@pages = ( "getting_started", "first_steps", "tutorial_window_optimization", "enhance_core", "acknowledgement", "references" );

foreach (@pages) {
    open PAGE, "< $_.dox";
    while (<PAGE>) {
        if ( /\@section ([\w\d_]+)/ ) {
            print MP " * -# \\ref $1\n";
        }
        elsif ( /\@subsection ([\w\d_]+)/ ) {
            print MP " *   -# \\ref $1\n";
        }
    }
    close PAGE;
}

print MP " */";

close MP;

my $old_file = "";
my $new_file = "";

open CMD, "doxygen 2>&1 |";
while (<CMD>) {
    if ( /(.*):\d+: [Ww]arning: (.*)/ ) {
        $new_file = basename($1);

        if ( !( $old_file eq $new_file ) ) {
            print "$new_file:\n";
            $old_file = $new_file;
        }

        print " * $2\n";
    }
}
close CMD;

system "./insert_overview_page.sh";

system "rm overview.dox";
system "rm mainpage.dox";


