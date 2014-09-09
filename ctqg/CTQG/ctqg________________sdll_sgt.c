#include <ctqg________________sdll_sgt.h>
#include <ctqg__________memory_manager.h>
/*       <stdio.h>  is included in "ctqg________________sdll_sgt.h" */
/*       <stdint.h> is included in "ctqg________________sdll_sgt.h" */
#include <stdlib.h> /* `malloc' */



typedef struct {
  ptr_to_entry_of_sgt_TOKEN p;
  int                         descend_to_the_left;
} path_element_for_sgt_insert;



ptr_to_el_of_sll_of_unused_el_of_heap_TOKEN
  sll_of_unused_el_of_sdll_TOKEN = NULL;
union abstract_el_of_heap_TOKEN *
  temp_for_internal_use_TOKEN;
struct elem_of_stack_of_allocated_blocks_TOKEN *
  stack_of_allocated_blocks_TOKEN = NULL;



/*
A small program for "bc" which generates
"min_num_elements_for_sgt_TOKEN_of_given_height".
Use "floor" of the printed numbers only (i.e. the digits before decimal point).

{
  scale = 80
  alpha = 2/3
  print "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
  for (i = 0; e(- l(alpha) * i) <= 2^63; i++) {}
  print "  #define MAX_NUM_LEVELS_IN_SGT_TOKEN ";
  i
  print "  int64_t min_num_elements_for_sgt_TOKEN_of_given_height[\n"
  print "    MAX_NUM_LEVELS_IN_SGT_TOKEN + 7] = {\n\n"
  for (i = 0; e(- l(alpha) * i) <= 2^63; i++) {
    i
    e(- l(alpha) * i) + 1 - 10^(-74)
    print "\n"
  }
}
*/



#define MAX_NUM_LEVELS_IN_SGT_TOKEN 108
int64_t min_num_elements_for_sgt_TOKEN_of_given_height[
  MAX_NUM_LEVELS_IN_SGT_TOKEN + 7] = {
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)      1),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)      2),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)      3),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)      4),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)      6),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)      8),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)     12),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)     18),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)     26),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)     39),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)     58),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)     87),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)    130),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)    195),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)    292),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)    438),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)    657),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)    986),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)   1478),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)   2217),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)   3326),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)   4988),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)   7482),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)  11223),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)  16835),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)  25252),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)  37877),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)  56816),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)  85223),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t) 127835),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t) 191752),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t) 287627),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t) 431440),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t) 647160),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t) 970740),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)1456110),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)2184165),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)3276247),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)4914370),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      0) + ((int64_t)7371555),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      1) + ((int64_t)1057333),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      1) + ((int64_t)6585999),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      2) + ((int64_t)4878998),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      3) + ((int64_t)7318497),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      5) + ((int64_t)5977745),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)      8) + ((int64_t)3966618),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)     12) + ((int64_t)5949926),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)     18) + ((int64_t)8924889),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)     28) + ((int64_t)3387334),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)     42) + ((int64_t)5081001),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)     63) + ((int64_t)7621501),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)     95) + ((int64_t)6432251),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)    143) + ((int64_t)4648376),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)    215) + ((int64_t)1972564),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)    322) + ((int64_t)7958845),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)    484) + ((int64_t)1938268),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)    726) + ((int64_t)2907401),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)   1089) + ((int64_t)4361102),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)   1634) + ((int64_t)1541652),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)   2451) + ((int64_t)2312478),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)   3676) + ((int64_t)8468717),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)   5515) + ((int64_t)2703076),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)   8272) + ((int64_t)9054614),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)  12409) + ((int64_t)3581920),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)  18614) + ((int64_t) 372880),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)  27921) + ((int64_t) 559320),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)  41881) + ((int64_t)5838979),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)  62822) + ((int64_t)3758469),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)  94233) + ((int64_t)5637703),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t) 141350) + ((int64_t)3456554),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t) 212025) + ((int64_t)5184831),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t) 318038) + ((int64_t)2777246),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t) 477057) + ((int64_t)4165869),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t) 715586) + ((int64_t)1248803),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)1073379) + ((int64_t)1873204),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)1610068) + ((int64_t)7809805),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)2415103) + ((int64_t)1714708),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)3622654) + ((int64_t)7572061),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)5433982) + ((int64_t)1358091),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    0) +
  ((int64_t)10000000)*((int64_t)8150973) + ((int64_t)2037137),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    1) +
  ((int64_t)10000000)*((int64_t)2226459) + ((int64_t)8055705),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    1) +
  ((int64_t)10000000)*((int64_t)8339689) + ((int64_t)7083557),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    2) +
  ((int64_t)10000000)*((int64_t)7509534) + ((int64_t)5625336),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    4) +
  ((int64_t)10000000)*((int64_t)1264301) + ((int64_t)8438004),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    6) +
  ((int64_t)10000000)*((int64_t)1896452) + ((int64_t)7657005),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)    9) +
  ((int64_t)10000000)*((int64_t)2844679) + ((int64_t)1485508),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)   13) +
  ((int64_t)10000000)*((int64_t)9267018) + ((int64_t)7228261),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)   20) +
  ((int64_t)10000000)*((int64_t)8900528) + ((int64_t) 842391),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)   31) +
  ((int64_t)10000000)*((int64_t)3350792) + ((int64_t)1263587),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)   47) +
  ((int64_t)10000000)*((int64_t)  26188) + ((int64_t)1895380),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)   70) +
  ((int64_t)10000000)*((int64_t)5039282) + ((int64_t)2843070),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)  105) +
  ((int64_t)10000000)*((int64_t)7558923) + ((int64_t)4264605),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)  158) +
  ((int64_t)10000000)*((int64_t)6338385) + ((int64_t)1396907),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)  237) +
  ((int64_t)10000000)*((int64_t)9507577) + ((int64_t)7095360),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)  356) +
  ((int64_t)10000000)*((int64_t)9261366) + ((int64_t)5643039),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)  535) +
  ((int64_t)10000000)*((int64_t)3892049) + ((int64_t)8464559),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)  803) +
  ((int64_t)10000000)*((int64_t) 838074) + ((int64_t)7696838),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t) 1204) +
  ((int64_t)10000000)*((int64_t)6257112) + ((int64_t)1545256),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t) 1806) +
  ((int64_t)10000000)*((int64_t)9385668) + ((int64_t)2317884),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t) 2710) +
  ((int64_t)10000000)*((int64_t)4078502) + ((int64_t)3476825),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t) 4065) +
  ((int64_t)10000000)*((int64_t)6117753) + ((int64_t)5215238),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t) 6098) +
  ((int64_t)10000000)*((int64_t)4176630) + ((int64_t)2822857),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t) 9147) +
  ((int64_t)10000000)*((int64_t)6264945) + ((int64_t)4234285),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)13721) +
  ((int64_t)10000000)*((int64_t)4397418) + ((int64_t)1351427),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)20582) +
  ((int64_t)10000000)*((int64_t)1596127) + ((int64_t)2027140),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)30873) +
  ((int64_t)10000000)*((int64_t)2394190) + ((int64_t)8040709),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)46309) +
  ((int64_t)10000000)*((int64_t)8591286) + ((int64_t)2061064),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)69464) +
  ((int64_t)10000000)*((int64_t)7886929) + ((int64_t)3091596),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)92233) +
  ((int64_t)10000000)*((int64_t)7203685) + ((int64_t)4775807),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)92233) +
  ((int64_t)10000000)*((int64_t)7203685) + ((int64_t)4775807),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)92233) +
  ((int64_t)10000000)*((int64_t)7203685) + ((int64_t)4775807),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)92233) +
  ((int64_t)10000000)*((int64_t)7203685) + ((int64_t)4775807),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)92233) +
  ((int64_t)10000000)*((int64_t)7203685) + ((int64_t)4775807),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)92233) +
  ((int64_t)10000000)*((int64_t)7203685) + ((int64_t)4775807),
  ((int64_t)10000000)*((int64_t)10000000)*((int64_t)92233) +
  ((int64_t)10000000)*((int64_t)7203685) + ((int64_t)4775807),
};

static ptr_to_entry_of_sgt_TOKEN
  stack_for_subtree_rebuild[MAX_NUM_LEVELS_IN_SGT_TOKEN + 7];
static ptr_to_entry_of_sgt_TOKEN *
  ptr_to_last_el_of_stack_for_subtree_rebuild;
static ptr_to_entry_of_sgt_TOKEN *
  ptr_to_ptr_to_the_element_ready_for_rm;
static sgt_TOKEN *
  ptr_to_home_sgt_of_the_element_ready_for_rm;
static ptr_to_entry_of_sgt_TOKEN
  NULL_ptr_to_entry_of_sgt_TOKEN_having_an_address = NULL;



void
print_to_file_TOKEN(
FILE * f, TOKEN * arg) {
  if (f == stdout) fprintf(f, "\033[34;40m");

  fprintf(f, "%3d[%d]", arg->line, arg->brackets_depth);

  switch(arg->type) {
    case ID____________TT:
      fprintf(f, "ID____________");
    break;

    case DECIMAL_CONST_TT:
      fprintf(f, "DECIMAL_CONST_");
    break;

    case SPACE_________TT:
      fprintf(f, "SPACE_________");
    break;

    case PERIODS_______TT:
      fprintf(f, "PERIODS_______");
    break;

    case OPERATOR______TT:
      fprintf(f, "OPERATOR______");
    break;

    case END_OF_LINE___TT:
      fprintf(f, "--------------------------------------------------");
    break;

    case BRACKETS______TT:
      fprintf(f, "BRACKETS______");
    break;

    case STDALN_SYM____TT:
      fprintf(f, "STDALN_SYM____");
    break;

    default:
      fprintf(stderr,
              "Error.\n"
              "An internal CTQG error occurred: "
              "An attempt to print an INVALID_______TT token.\n");
      exit(-2);
  }

  if (f == stdout) fprintf(f, "\033[0m");

  if (arg->type != END_OF_LINE___TT) {
    h_TOKEN_fprintf_str(f, arg->value, 0, ' ', 'l');
  }

  fprintf(f, "\n");
}



void
print_TOKEN(
TOKEN * arg) {
  print_to_file_TOKEN(stdout, arg);
}



void
new_block_of_99_elem_of_sdll_TOKEN(
void) {
  static int is_the_first_call = 1;
  int i;
  struct elem_of_stack_of_allocated_blocks_TOKEN *
    ptr_to_new_block;

  if (is_the_first_call != 0) {
    is_the_first_call = 0;

    if (sizeof(struct node_of_sdll_sgt_TOKEN)
        !=
        sizeof(union abstract_el_of_heap_TOKEN)) {
      fprintf(stderr,
              "WARNING : sdll_sgt.c\n");
      fprintf(stderr,
              "sizeof(struct node_of_sdll_sgt_TOKEN) = %d.\n",
              (int) sizeof(struct node_of_sdll_sgt_TOKEN));
      fprintf(stderr,
              "sizeof(union abstract_el_of_heap_TOKEN) = %d.\n",
              (int) sizeof(union abstract_el_of_heap_TOKEN));
      fprintf(stderr,
              "This only can happen on a very exotic platform.\n"
              "SDLL_SGT will still work correctly but will "
              "be less memory efficient.\n"
              "Exiting for now.\n"
              "It is save to remove the `exit' command in function "
              "`new_block_of_99_elem_of_sdll_TOKEN' to ignore "
              "this warning.\n");
      exit(-1);
    }

    if (sizeof(struct s_block_TOKEN)
        !=
        sizeof(union abstract_el_of_heap_TOKEN)) {
      fprintf(stderr,
              "WARNING : sdll_sgt.c\n");
      fprintf(stderr,
              "sizeof(struct s_block_TOKEN) = %d.\n",
              (int) sizeof(struct s_block_TOKEN));
      fprintf(stderr,
              "sizeof(union abstract_el_of_heap_TOKEN) = %d.\n",
              (int) sizeof(union abstract_el_of_heap_TOKEN));
      fprintf(stderr,
              "This only can happen on a very exotic platform.\n"
              "SDLL_SGT will still work correctly but will "
              "be less memory efficient.\n"
              "Exiting for now.\n"
              "It is save to remove the `exit' command in function "
              "`new_block_of_99_elem_of_sdll_TOKEN' to ignore "
              "this warning.\n");
      exit(-1);
    }

    if (sizeof(char) != 1) {
      fprintf(stderr,
              "Internal error in sdll_sgt implementation: "
              "sizeof(char) != 1.\n\n");
      exit(-1);
    }
  }

  /* --- BEGIN allocate new block and add it to stack --- */

  ptr_to_new_block =
    (struct elem_of_stack_of_allocated_blocks_TOKEN *)
    malloc(sizeof(struct elem_of_stack_of_allocated_blocks_TOKEN));

  if (ptr_to_new_block == NULL) {
    fprintf(stderr, "ERROR : `sdll_sgt' can not allocate memory.\n");
    exit(-1);
  }

  update_memory_usage_info(
    &heap_size_TOKEN,
    sizeof(struct elem_of_stack_of_allocated_blocks_TOKEN)
  );

  ptr_to_new_block->ptr_to_next_elem_of_stack =
    stack_of_allocated_blocks_TOKEN;

  stack_of_allocated_blocks_TOKEN =
    ptr_to_new_block;

  /* ---  END  allocate new block and add it to stack --- */

  /* --- BEGIN add all new el. to deleted_el_of_sdll_TOKEN --- */
  for (i = 99 - 2; i >= 0; i--) {
    ptr_to_new_block->body[i].u.ptr_to_next_el_of_sll =
      &(ptr_to_new_block->body[i + 1]);
  }
  ptr_to_new_block->body[98].u.ptr_to_next_el_of_sll =
    sll_of_unused_el_of_sdll_TOKEN;
  sll_of_unused_el_of_sdll_TOKEN =
    &(ptr_to_new_block->body[0]);
  /* ---  END  add all new el. to deleted_el_of_sdll_TOKEN --- */
}



int
is_heap_TOKEN_empty(
void) {
  int64_t int64_t_constant99 = 99;
  int64_t num_alloc_blocks = 0;
  int64_t num_unused_elems = 0;
  struct elem_of_stack_of_allocated_blocks_TOKEN * stack_it =
    stack_of_allocated_blocks_TOKEN;
  ptr_to_el_of_sll_of_unused_el_of_heap_TOKEN iter =
    sll_of_unused_el_of_sdll_TOKEN;
  while (stack_it != NULL) {
    num_alloc_blocks++;
    stack_it = stack_it->ptr_to_next_elem_of_stack;
  }
  while (iter != NULL) {
    num_unused_elems++;
    iter = iter->u.ptr_to_next_el_of_sll;
  }

  if (0) printf("%d elements are allocated on heap.\n",
                (int)
                (int64_t_constant99 * num_alloc_blocks
                 -
                 num_unused_elems));

  return num_unused_elems == int64_t_constant99 * num_alloc_blocks;
}



void
reset_heap_TOKEN(
void) {
  struct elem_of_stack_of_allocated_blocks_TOKEN * it;
  while(stack_of_allocated_blocks_TOKEN != NULL) {
    it = stack_of_allocated_blocks_TOKEN;
    stack_of_allocated_blocks_TOKEN =
      stack_of_allocated_blocks_TOKEN->ptr_to_next_elem_of_stack;
    free(it);
  }
  sll_of_unused_el_of_sdll_TOKEN = NULL;
}



void
print_to_file_sdll_TOKEN(
FILE * f, sdll_TOKEN sdll) {
  iter_sdll_TOKEN h;
  set_iter_to_leftmost_el_sdll_TOKEN(h, sdll)
  while (h != NULL) {
    print_to_file_TOKEN(f, &(h->e.body));
    printf("\n");
    move_iter_right_sdll_TOKEN(h)
  }
}



void
print_sdll_TOKEN(
sdll_TOKEN sdll) {
  print_to_file_sdll_TOKEN(stdout, sdll);
}



int64_t
count_elements_sdll_TOKEN(
sdll_TOKEN sdll) {
  int64_t rv = 0;
  iter_sdll_TOKEN h;
  set_iter_to_leftmost_el_sdll_TOKEN(h, sdll)
  while (h != NULL) {
    rv++;
    move_iter_right_sdll_TOKEN(h)
  }
  return rv;
}



void
h_TOKEN_alloc_strcpy_hs(
str_on_heap_TOKEN * target, char const * source) {
  size_t i;
  char * write_head;
  str_on_heap_TOKEN last_b;

  if (sll_of_unused_el_of_sdll_TOKEN == NULL)
    new_block_of_99_elem_of_sdll_TOKEN();
  last_b = *target = sll_of_unused_el_of_sdll_TOKEN;
  sll_of_unused_el_of_sdll_TOKEN =
    sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;
  write_head = &(last_b->s.body[0]);
  i = sizeof(struct node_of_sdll_sgt_TOKEN)
      -
      sizeof(str_on_heap_TOKEN);
  while ( (*(write_head++) = *(source++)) != '\0' ) {
    if (--i == 0) {
      if (sll_of_unused_el_of_sdll_TOKEN == NULL)
        new_block_of_99_elem_of_sdll_TOKEN();
      last_b->s.ptr_to_next_s_block =
        sll_of_unused_el_of_sdll_TOKEN;
      sll_of_unused_el_of_sdll_TOKEN =
        sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;
      last_b = last_b->s.ptr_to_next_s_block;
      write_head = &(last_b->s.body[0]);
      i = sizeof(struct node_of_sdll_sgt_TOKEN)
          -
          sizeof(str_on_heap_TOKEN);
    }
  }
  last_b->s.ptr_to_next_s_block = NULL;
}



void
h_TOKEN_alloc_strcpy_hh(
str_on_heap_TOKEN * target, str_on_heap_TOKEN source) {
  size_t i;
  char * write_head;
  char * read_head;
  str_on_heap_TOKEN last_b;

  if (sll_of_unused_el_of_sdll_TOKEN == NULL)
    new_block_of_99_elem_of_sdll_TOKEN();
  last_b = *target = sll_of_unused_el_of_sdll_TOKEN;
  sll_of_unused_el_of_sdll_TOKEN =
    sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;
  write_head = &(last_b->s.body[0]);
  read_head = &(source->s.body[0]);
  i = sizeof(struct node_of_sdll_sgt_TOKEN)
      -
      sizeof(str_on_heap_TOKEN);
  while ( (*(write_head++) = *(read_head++)) != '\0' ) {
    if (--i == 0) {
      if (sll_of_unused_el_of_sdll_TOKEN == NULL)
        new_block_of_99_elem_of_sdll_TOKEN();
      last_b->s.ptr_to_next_s_block = sll_of_unused_el_of_sdll_TOKEN;
      sll_of_unused_el_of_sdll_TOKEN =
        sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;
      last_b = last_b->s.ptr_to_next_s_block;
      write_head = &(last_b->s.body[0]);
      source = source->s.ptr_to_next_s_block;
      read_head = &(source->s.body[0]);
      i = sizeof(struct node_of_sdll_sgt_TOKEN)
          -
          sizeof(str_on_heap_TOKEN);
    }
  }
  last_b->s.ptr_to_next_s_block = NULL;
}



void
h_TOKEN_strcpy_to_prealloc_sh(
char * target, str_on_heap_TOKEN source) {
  size_t i;
  char * read_head;

  read_head = &(source->s.body[0]);
  i = sizeof(struct node_of_sdll_sgt_TOKEN)
      -
      sizeof(str_on_heap_TOKEN);
  while ( (*(target++) = *(read_head++)) != '\0' ) {
    if (--i == 0) {
      source = source->s.ptr_to_next_s_block;
      read_head = &(source->s.body[0]);
      i = sizeof(struct node_of_sdll_sgt_TOKEN)
          -
          sizeof(str_on_heap_TOKEN);
    }
  }
}



void
h_TOKEN_str_delete(
str_on_heap_TOKEN arg) {
  str_on_heap_TOKEN temp;
  while(arg != NULL) {
    temp = arg->s.ptr_to_next_s_block;
    arg->u.ptr_to_next_el_of_sll = sll_of_unused_el_of_sdll_TOKEN;
    sll_of_unused_el_of_sdll_TOKEN = arg;
    arg = temp;
  }
}



int64_t
h_TOKEN_strlen(
str_on_heap_TOKEN arg) {
  int64_t rv = 0;
  char * read_head;
  while (arg->s.ptr_to_next_s_block != NULL) {
    arg = arg->s.ptr_to_next_s_block;
    rv += (int64_t)
          (sizeof(struct node_of_sdll_sgt_TOKEN)
          -
          sizeof(str_on_heap_TOKEN));
  }
  read_head = &(arg->s.body[0]);
  while (*(read_head++) != '\0') rv++;
  return rv;
}



void /* Works correctly even when `arg1' and `arg2' are the same ID. */
h_TOKEN_strcat(
str_on_heap_TOKEN arg1, str_on_heap_TOKEN arg2) {
  char * read_head;
  char * write_head;
  str_on_heap_TOKEN last_b;
  size_t write_i;
  size_t read_i;
  char * ptr_to_former_terminating_char;

  if (arg1 == arg2) { /* Compare as pointers to resolve strcat with self. */
    /* BEGIN Search for the end of `arg1'. */
    while (arg1->s.ptr_to_next_s_block != NULL)
      arg1 = arg1->s.ptr_to_next_s_block;
    last_b = arg1;
    write_i = sizeof(struct node_of_sdll_sgt_TOKEN)
              -
              sizeof(str_on_heap_TOKEN);
    write_head = &(arg1->s.body[0]);
    while (*write_head != '\0') {write_head++; write_i--;}
    /*  END  Search for the end of `arg1'. */

    ptr_to_former_terminating_char = write_head;

    read_head = &(arg2->s.body[0]);
    read_i = sizeof(struct node_of_sdll_sgt_TOKEN)
             -
             sizeof(str_on_heap_TOKEN);
    while (read_head != ptr_to_former_terminating_char) {
      *(write_head++) = *(read_head++);
      if (--write_i == 0) {
        if (sll_of_unused_el_of_sdll_TOKEN == NULL)
          new_block_of_99_elem_of_sdll_TOKEN();
        last_b->s.ptr_to_next_s_block = sll_of_unused_el_of_sdll_TOKEN;
        sll_of_unused_el_of_sdll_TOKEN =
          sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;
        last_b = last_b->s.ptr_to_next_s_block;
        write_head = &(last_b->s.body[0]);
        write_i = sizeof(struct node_of_sdll_sgt_TOKEN)
                  -
                  sizeof(str_on_heap_TOKEN);
      }
      if (--read_i == 0) {
        arg2 = arg2->s.ptr_to_next_s_block;
        read_head = &(arg2->s.body[0]);
        read_i = sizeof(struct node_of_sdll_sgt_TOKEN)
                 -
                 sizeof(str_on_heap_TOKEN);
      }
    }
    *write_head = '\0';
    last_b->s.ptr_to_next_s_block = NULL;
  } else {
    /* BEGIN Search for the end of `arg1'. */
    while (arg1->s.ptr_to_next_s_block != NULL)
      arg1 = arg1->s.ptr_to_next_s_block;
    last_b = arg1;
    write_i = sizeof(struct node_of_sdll_sgt_TOKEN)
              -
              sizeof(str_on_heap_TOKEN);
    write_head = &(arg1->s.body[0]);
    while (*write_head != '\0') {write_head++; write_i--;}
    /*  END  Search for the end of `arg1'. */

    read_head = &(arg2->s.body[0]);
    read_i = sizeof(struct node_of_sdll_sgt_TOKEN)
             -
             sizeof(str_on_heap_TOKEN);
    while ( (*(write_head++) = *(read_head++)) != '\0' ) {
      if (--write_i == 0) {
        if (sll_of_unused_el_of_sdll_TOKEN == NULL)
          new_block_of_99_elem_of_sdll_TOKEN();
        last_b->s.ptr_to_next_s_block = sll_of_unused_el_of_sdll_TOKEN;
        sll_of_unused_el_of_sdll_TOKEN =
          sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;
        last_b = last_b->s.ptr_to_next_s_block;
        write_head = &(last_b->s.body[0]);
        write_i = sizeof(struct node_of_sdll_sgt_TOKEN)
                  -
                  sizeof(str_on_heap_TOKEN);
      }
      if (--read_i == 0) {
        arg2 = arg2->s.ptr_to_next_s_block;
        read_head = &(arg2->s.body[0]);
        read_i = sizeof(struct node_of_sdll_sgt_TOKEN)
                 -
                 sizeof(str_on_heap_TOKEN);
      }
    }
    last_b->s.ptr_to_next_s_block = NULL;
  }
}



int
h_TOKEN_strcmp_hs(
str_on_heap_TOKEN arg1, char const * arg2) {
  size_t i;
  char * read_head1 = &(arg1->s.body[0]);
  i = sizeof(struct node_of_sdll_sgt_TOKEN)
      -
      sizeof(str_on_heap_TOKEN);
  while (*read_head1 == *arg2) {
    if (*read_head1 == '\0') return 0;
    read_head1++;
    arg2++;
    if (--i == 0) {
      arg1 = arg1->s.ptr_to_next_s_block;
      read_head1 = &(arg1->s.body[0]);
      i = sizeof(struct node_of_sdll_sgt_TOKEN)
          -
          sizeof(str_on_heap_TOKEN);
    }
  }
  return
    (  *(unsigned char *) read_head1
       <
       *(unsigned char *) arg2 )  ? -1 : 1;
}



int
h_TOKEN_strcmp_hh(
str_on_heap_TOKEN arg1, str_on_heap_TOKEN arg2) {
  size_t i;
  char * read_head1 = &(arg1->s.body[0]);
  char * read_head2 = &(arg2->s.body[0]);
  i = sizeof(struct node_of_sdll_sgt_TOKEN)
      -
      sizeof(str_on_heap_TOKEN);
  while (*read_head1 == *read_head2) {
    if (*read_head1 == '\0') return 0;
    read_head1++;
    read_head2++;
    if (--i == 0) {
      arg1 = arg1->s.ptr_to_next_s_block;
      read_head1 = &(arg1->s.body[0]);
      arg2 = arg2->s.ptr_to_next_s_block;
      read_head2 = &(arg2->s.body[0]);
      i = sizeof(struct node_of_sdll_sgt_TOKEN)
          -
          sizeof(str_on_heap_TOKEN);
    }
  }
  return
    ( *(unsigned char *) read_head1
      <
      *(unsigned char *) read_head2 ) ? -1 : 1;
}



int
h_TOKEN_alloc_fscan_str(
FILE * stream, str_on_heap_TOKEN * target) {
  int ic;
  size_t i;
  char * write_head;
  str_on_heap_TOKEN last_b;

  do {
    ic = fgetc(stream);
    if (ic == EOF) return 0;
  } while ( (ic < 33) || (ic > 255) );

  if (sll_of_unused_el_of_sdll_TOKEN == NULL)
    new_block_of_99_elem_of_sdll_TOKEN();
  last_b = *target = sll_of_unused_el_of_sdll_TOKEN;
  sll_of_unused_el_of_sdll_TOKEN =
    sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;
  write_head = &(last_b->s.body[0]);
  i = sizeof(struct node_of_sdll_sgt_TOKEN)
      -
      sizeof(str_on_heap_TOKEN);
  do {
    *(write_head++) = (char) ic;
    if (--i == 0) {
      if (sll_of_unused_el_of_sdll_TOKEN == NULL)
        new_block_of_99_elem_of_sdll_TOKEN();
      last_b->s.ptr_to_next_s_block =
        sll_of_unused_el_of_sdll_TOKEN;
      sll_of_unused_el_of_sdll_TOKEN =
        sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;
      last_b = last_b->s.ptr_to_next_s_block;
      write_head = &(last_b->s.body[0]);
      i = sizeof(struct node_of_sdll_sgt_TOKEN)
          -
          sizeof(str_on_heap_TOKEN);
    }
    ic = fgetc(stream);
  } while ( (ic != EOF) && (ic >= 33) && (ic <= 255) );
  *write_head = '\0';
  last_b->s.ptr_to_next_s_block = NULL;
  return 1;
}



int
h_TOKEN_alloc_scan_str(
str_on_heap_TOKEN * target) {
  return h_TOKEN_alloc_fscan_str(stdin, target);
}



int64_t
h_TOKEN_fprintf_str(
FILE * stream, str_on_heap_TOKEN s, int64_t ml, char pad, char just) {
  int     pad_char_as_int = (int) pad;
  int64_t leading_padding;
  int64_t trailing_padding;
  int64_t string_length = h_TOKEN_strlen(s);
  int64_t total_padding;
  int64_t total_number_of_characters_written = 0;
  int64_t k;
  size_t  n = 0;

  total_padding = ml - string_length;
  if (total_padding < 0) total_padding = 0;

  switch (just) {

    case 'l':
      trailing_padding = total_padding;
      leading_padding = 0;
    break;

    case 'c':
      trailing_padding = total_padding >> 1;
      leading_padding = total_padding - trailing_padding;
    break;

    case 'r':
      trailing_padding = 0;
      leading_padding = total_padding;
    break;

    default:
      fprintf(stderr,
              "Error! Function h_TOKEN_fprintf_str(...) "
              "Bad value of `just'. Must be 'l', 'c', or 'r'.\n");
      exit(-1);

  }

  for (k = leading_padding; k > 0; k--) {
    if (fputc(pad_char_as_int, stream) == EOF) return -1;
    total_number_of_characters_written++;
  }

  while (s->s.ptr_to_next_s_block != NULL) {
    if (fwrite(
          &(s->s.body),
          sizeof(char),
          sizeof(struct node_of_sdll_sgt_TOKEN)
          -
          sizeof(str_on_heap_TOKEN),
          stream
        )
        !=
        sizeof(struct node_of_sdll_sgt_TOKEN)
        -
        sizeof(str_on_heap_TOKEN)) {
      return -1;
    }
    total_number_of_characters_written +=
      sizeof(struct node_of_sdll_sgt_TOKEN)
      -
      sizeof(str_on_heap_TOKEN);
    s = s->s.ptr_to_next_s_block;
  }
  while (s->s.body[n] != '\0') n++;
  if (fwrite(&(s->s.body), sizeof(char), n, stream) != n) return -1;
  total_number_of_characters_written += n;

  for (k = trailing_padding; k > 0; k--) {
    if (fputc(pad_char_as_int, stream) == EOF) return -1;
    total_number_of_characters_written++;
  }

  return total_number_of_characters_written;
}



int64_t
h_TOKEN_printf_str(
str_on_heap_TOKEN s, int64_t ml, char pad, char just) {
  return h_TOKEN_fprintf_str(stdout, s, ml, pad, just);
}



int /* Ret. 0 != 0 if a symbol other than `0' or `1' enc. Otherw. ret 1 != 0. */
h_TOKEN_count_zeros_and_ones_in_str(
str_on_heap_TOKEN s, uint64_t * n_zeros, uint64_t * n_ones) {
  uint32_t i;

  *n_zeros = 0;
  *n_ones = 0;

  while (s->s.ptr_to_next_s_block != NULL) {
    for (i = 0; i < sizeof(struct node_of_sdll_sgt_TOKEN)
                    -
                    sizeof(str_on_heap_TOKEN); i++) {
      if (s->s.body[i] == '0') {
        (*n_zeros)++;
      } else
      if (s->s.body[i] == '1') {
        (*n_ones)++;
      } else {
        return 0 != 0;
      }
    }
    s = s->s.ptr_to_next_s_block;
  }

  i = 0;
  while (s->s.body[i] != '\0') {
    if (s->s.body[i] == '0') {
      (*n_zeros)++;
    } else
    if (s->s.body[i] == '1') {
      (*n_ones)++;
    } else {
      return 0 != 0;
    }
    i++;
  }

  return 1 != 0;
}







#define \
append_to_sll_dont_change_content(\
ptr_to_last_ptr_to_right, iter) {\
*(ptr_to_last_ptr_to_right) = (iter);\
(ptr_to_last_ptr_to_right) = &((iter)->e.ptr_to_right);}







void
sort_smaller_to_the_left_sdll_TOKEN(
sdll_TOKEN * sdll) {
  ptr_to_node_of_sdll_sgt_TOKEN   left_rh;
  ptr_to_node_of_sdll_sgt_TOKEN   right_rh;
  ptr_to_node_of_sdll_sgt_TOKEN   left_block_end;
  ptr_to_node_of_sdll_sgt_TOKEN   right_block_end;
  ptr_to_node_of_sdll_sgt_TOKEN   ptr_to_first_el_of_sll;
  ptr_to_node_of_sdll_sgt_TOKEN * ptr_to_last_ptr_to_right_of_sll;
  ptr_to_node_of_sdll_sgt_TOKEN * ptr_to_ptr_to_next_block;
  ptr_to_node_of_sdll_sgt_TOKEN   ptr_to_last_el_inserted;



  set_iter_to_leftmost_el_sdll_TOKEN(left_rh, *sdll)
  if (left_rh  == NULL) return; /* 0 elements in the list. */
  right_rh = left_rh->e.ptr_to_right;
  if (right_rh == NULL) return; /* 1 element in the list. */

  ptr_to_last_ptr_to_right_of_sll = &ptr_to_first_el_of_sll;

  if ( TOKEN_arg1_GREATER_THAN_arg2(
         (left_rh->e.body), (right_rh->e.body)) ) {
    append_to_sll_dont_change_content(
      ptr_to_last_ptr_to_right_of_sll, right_rh)
    ptr_to_ptr_to_next_block = &(right_rh->e.ptr_to_left);
    right_rh = right_rh->e.ptr_to_right;
    append_to_sll_dont_change_content(
      ptr_to_last_ptr_to_right_of_sll, left_rh)
    left_rh = right_rh;
  } else {
    append_to_sll_dont_change_content(
      ptr_to_last_ptr_to_right_of_sll, left_rh)
    ptr_to_ptr_to_next_block = &(left_rh->e.ptr_to_left);
    left_rh = right_rh->e.ptr_to_right;
    append_to_sll_dont_change_content(
      ptr_to_last_ptr_to_right_of_sll, right_rh)
  }
  if (left_rh == NULL) {
    /* 2 elements in the list. */
    /* --- BEGIN Make sll a vaild dll --- */
    *ptr_to_last_ptr_to_right_of_sll = NULL;
    sdll->ptr_to_leftmost =
      ptr_to_first_el_of_sll;
    ptr_to_first_el_of_sll->e.ptr_to_left = NULL;
    (sdll->ptr_to_rightmost
     =
     ptr_to_first_el_of_sll->e.ptr_to_right
    )->e.ptr_to_left = ptr_to_first_el_of_sll;
    /* ---  END  Make sll a vaild dll --- */
    return;
  }
  right_rh = left_rh->e.ptr_to_right;

  do { /* while (1). Terminates by `break'. */
    /* Now left_rh is not NULL, right_rh may be NULL. */
    if (right_rh == NULL) {
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, left_rh)
      *ptr_to_ptr_to_next_block = left_rh;
      left_rh->e.ptr_to_left = NULL;
      break;
    }
    if ( TOKEN_arg1_GREATER_THAN_arg2(
           (left_rh->e.body), (right_rh->e.body)) ) {
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, right_rh)
      *ptr_to_ptr_to_next_block = right_rh;
      ptr_to_ptr_to_next_block = &(right_rh->e.ptr_to_left);
      right_rh = right_rh->e.ptr_to_right;
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, left_rh)
      left_rh = right_rh;
    } else {
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, left_rh)
      *ptr_to_ptr_to_next_block = left_rh;
      ptr_to_ptr_to_next_block = &(left_rh->e.ptr_to_left);
      left_rh = right_rh->e.ptr_to_right;
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, right_rh)
    }
    if (left_rh == NULL) {
      *ptr_to_ptr_to_next_block = NULL;
      break;
    }
    right_rh = left_rh->e.ptr_to_right;
  } while (1);



  /* Now sll is filled with sorted pairs.  */
  /* It has at least 3 elements.           */
  /* It is not terminated with NULL yet.   */

  do { /* while (1). Terminates by `break'. */
    /* At least two blocks left.               */
    /* ptr_to_first_el_of_sll->e.ptr_to_left     */
    /* is guaranteed non-NULL.                 */
    if ( (right_block_end = (
           left_block_end =
           right_rh =
           (left_rh = ptr_to_first_el_of_sll)->e.ptr_to_left
         )->e.ptr_to_left) == NULL) {
      /* Exactly two blocks left. */
      /* Go to last pass.         */
      break;
    }
    *ptr_to_last_ptr_to_right_of_sll = NULL;
    ptr_to_last_ptr_to_right_of_sll = &ptr_to_first_el_of_sll;
    ptr_to_ptr_to_next_block = &temp_for_internal_use_TOKEN;

    do { /* while (1). Terminates by `break'. */

      /* --- BEGIN compare first two elements --- */
      if ( TOKEN_arg1_GREATER_THAN_arg2(
             (left_rh->e.body), (right_rh->e.body)) ) {
        append_to_sll_dont_change_content(
          ptr_to_last_ptr_to_right_of_sll, right_rh)
        *ptr_to_ptr_to_next_block = right_rh;
        ptr_to_ptr_to_next_block = &(right_rh->e.ptr_to_left);
        if ((right_rh = right_rh->e.ptr_to_right)
             ==
             NULL) { /* This is equiv. to "... == right_block_end" */
          *ptr_to_last_ptr_to_right_of_sll = left_rh;
          while (left_rh->e.ptr_to_right != left_block_end) {
            left_rh = left_rh->e.ptr_to_right;
          }
          ptr_to_last_ptr_to_right_of_sll = &(left_rh->e.ptr_to_right);
          *ptr_to_ptr_to_next_block = NULL;
          break; /* Go to "LABEL_1" */
        }
      } else {
        append_to_sll_dont_change_content(
          ptr_to_last_ptr_to_right_of_sll, left_rh)
        *ptr_to_ptr_to_next_block = left_rh;
        ptr_to_ptr_to_next_block = &(left_rh->e.ptr_to_left);
        /* The left block is full and has at least two el. */
        left_rh = left_rh->e.ptr_to_right;
      }
      /* ---  END  compare first two elements --- */

      /* --- BEGIN compare the rest of the pair of blocks --- */
      do {
        if ( TOKEN_arg1_GREATER_THAN_arg2(
               (left_rh->e.body), (right_rh->e.body)) ) {
          append_to_sll_dont_change_content(
            ptr_to_last_ptr_to_right_of_sll, right_rh)
          if ((right_rh = right_rh->e.ptr_to_right)
              ==
              right_block_end) {
            *ptr_to_last_ptr_to_right_of_sll = left_rh;
            while (left_rh->e.ptr_to_right != left_block_end) {
              left_rh = left_rh->e.ptr_to_right;
            }
            ptr_to_last_ptr_to_right_of_sll = &(left_rh->e.ptr_to_right);
            break;
          }
        } else {
          append_to_sll_dont_change_content(
            ptr_to_last_ptr_to_right_of_sll, left_rh)
          if ((left_rh = left_rh->e.ptr_to_right)
               ==
               left_block_end) {
            *ptr_to_last_ptr_to_right_of_sll = right_rh;
            while (right_rh->e.ptr_to_right != right_block_end) {
              right_rh = right_rh->e.ptr_to_right;
            }
            ptr_to_last_ptr_to_right_of_sll = &(right_rh->e.ptr_to_right);
            break;
          }
        }
      } while (1);
      /* ---  END  compare the rest of the pair of blocks --- */

      if ((left_rh = right_block_end) == NULL) {
        *ptr_to_ptr_to_next_block = NULL;
        break; /* Go to "LABEL_1" */
      }
      if ((right_rh =
           left_block_end =
           right_block_end->e.ptr_to_left) == NULL) {
        *ptr_to_ptr_to_next_block = left_rh;
        left_rh->e.ptr_to_left = NULL;
        *ptr_to_last_ptr_to_right_of_sll = left_rh;
        while (left_rh->e.ptr_to_right != left_block_end) {
          left_rh = left_rh->e.ptr_to_right;
        }
        ptr_to_last_ptr_to_right_of_sll = &(left_rh->e.ptr_to_right);
        break; /* Go to "LABEL_1" */
      }
      right_block_end = left_block_end ->e.ptr_to_left;
    } while (1);
    /* "LABEL_1" */
  } while (1);
  *ptr_to_last_ptr_to_right_of_sll = NULL;



  /* Print as sll.
  printf("--- beginning of sll ---\n");
  left_rh = ptr_to_first_el_of_sll;
  while (left_rh) {
    printf("[%03X]: %c %02d ->next_bl = %03X\n",
           ((int) left_rh) & 0xFFF,
           left_rh->e.body.c,
           left_rh->e.body.i,
           ((int) left_rh->e.ptr_to_left) & 0xFFF);
    left_rh = left_rh->e.ptr_to_right;
  }
  printf("---    end    of sll ---\n\n");
  getchar();
  */



  /* Now sll is filled with exactly 2 large sorted blocks.   */
  /* It has at least 3 elements. It is terminated with NULL. */
  /* ptr_to_first_el_of_sll->e.ptr_to_left     */
  /* is guaranteed non-NULL.                 */

  left_block_end = right_rh =
    (left_rh = ptr_to_first_el_of_sll)->e.ptr_to_left;
  /* "right_block_end" is not used because it is for sure NULL. */

  if (sll_of_unused_el_of_sdll_TOKEN == NULL)
    new_block_of_99_elem_of_sdll_TOKEN();
  ptr_to_last_el_inserted = sll_of_unused_el_of_sdll_TOKEN;
  sll_of_unused_el_of_sdll_TOKEN =
    sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;

  sdll->ptr_to_leftmost = ptr_to_last_el_inserted;

  do {
    if ( TOKEN_arg1_GREATER_THAN_arg2(
           (left_rh->e.body), (right_rh->e.body)) ) {
      ptr_to_last_el_inserted->e.ptr_to_right = right_rh;
      right_rh->e.ptr_to_left = ptr_to_last_el_inserted;
      ptr_to_last_el_inserted = right_rh;
      if ((right_rh = right_rh->e.ptr_to_right) == NULL) {
        do {
          ptr_to_last_el_inserted->e.ptr_to_right = left_rh;
          left_rh->e.ptr_to_left = ptr_to_last_el_inserted;
          ptr_to_last_el_inserted = left_rh;
        } while ((left_rh = left_rh->e.ptr_to_right)
                 !=
                 left_block_end);
        break;
      }
    } else {
      ptr_to_last_el_inserted->e.ptr_to_right = left_rh;
      left_rh->e.ptr_to_left = ptr_to_last_el_inserted;
      ptr_to_last_el_inserted = left_rh;
      if ((left_rh = left_rh->e.ptr_to_right) == left_block_end) {
        do {
          ptr_to_last_el_inserted->e.ptr_to_right = right_rh;
          right_rh->e.ptr_to_left = ptr_to_last_el_inserted;
          ptr_to_last_el_inserted = right_rh;
        } while ((right_rh = right_rh->e.ptr_to_right)
                 !=
                 NULL);
        break;
      }
    }
  } while (1);

  ptr_to_last_el_inserted->e.ptr_to_right = NULL;
  sdll->ptr_to_rightmost = ptr_to_last_el_inserted;

  /* Delete the leftmost element.          */
  /* It's ok that its left ptr is not set. */
  temp_for_internal_use_TOKEN = (*sdll).ptr_to_leftmost;
  temp_for_internal_use_TOKEN->e.ptr_to_right->e.ptr_to_left
    = NULL;
  (*sdll).ptr_to_leftmost =
    temp_for_internal_use_TOKEN->e.ptr_to_right;
  temp_for_internal_use_TOKEN->u.ptr_to_next_el_of_sll =
    sll_of_unused_el_of_sdll_TOKEN;
  sll_of_unused_el_of_sdll_TOKEN =
    temp_for_internal_use_TOKEN;
}







void
sort_smaller_to_the_right_sdll_TOKEN(
sdll_TOKEN * sdll) {
  ptr_to_node_of_sdll_sgt_TOKEN   left_rh;
  ptr_to_node_of_sdll_sgt_TOKEN   right_rh;
  ptr_to_node_of_sdll_sgt_TOKEN   left_block_end;
  ptr_to_node_of_sdll_sgt_TOKEN   right_block_end;
  ptr_to_node_of_sdll_sgt_TOKEN   ptr_to_first_el_of_sll;
  ptr_to_node_of_sdll_sgt_TOKEN * ptr_to_last_ptr_to_right_of_sll;
  ptr_to_node_of_sdll_sgt_TOKEN * ptr_to_ptr_to_next_block;
  ptr_to_node_of_sdll_sgt_TOKEN   ptr_to_last_el_inserted;



  set_iter_to_leftmost_el_sdll_TOKEN(left_rh, *sdll)
  if (left_rh  == NULL) return; /* 0 elements in the list. */
  right_rh = left_rh->e.ptr_to_right;
  if (right_rh == NULL) return; /* 1 element in the list. */

  ptr_to_last_ptr_to_right_of_sll = &ptr_to_first_el_of_sll;

  if ( TOKEN_arg1_GREATER_THAN_arg2(
         (right_rh->e.body), (left_rh->e.body)) ) {
    append_to_sll_dont_change_content(
      ptr_to_last_ptr_to_right_of_sll, right_rh)
    ptr_to_ptr_to_next_block = &(right_rh->e.ptr_to_left);
    right_rh = right_rh->e.ptr_to_right;
    append_to_sll_dont_change_content(
      ptr_to_last_ptr_to_right_of_sll, left_rh)
    left_rh = right_rh;
  } else {
    append_to_sll_dont_change_content(
      ptr_to_last_ptr_to_right_of_sll, left_rh)
    ptr_to_ptr_to_next_block = &(left_rh->e.ptr_to_left);
    left_rh = right_rh->e.ptr_to_right;
    append_to_sll_dont_change_content(
      ptr_to_last_ptr_to_right_of_sll, right_rh)
  }
  if (left_rh == NULL) {
    /* 2 elements in the list. */
    /* --- BEGIN Make sll a vaild dll --- */
    *ptr_to_last_ptr_to_right_of_sll = NULL;
    sdll->ptr_to_leftmost =
      ptr_to_first_el_of_sll;
    ptr_to_first_el_of_sll->e.ptr_to_left = NULL;
    (sdll->ptr_to_rightmost
     =
     ptr_to_first_el_of_sll->e.ptr_to_right
    )->e.ptr_to_left = ptr_to_first_el_of_sll;
    /* ---  END  Make sll a vaild dll --- */
    return;
  }
  right_rh = left_rh->e.ptr_to_right;

  do { /* while (1). Terminates by `break'. */
    /* Now left_rh is not NULL, right_rh may be NULL. */
    if (right_rh == NULL) {
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, left_rh)
      *ptr_to_ptr_to_next_block = left_rh;
      left_rh->e.ptr_to_left = NULL;
      break;
    }
    if ( TOKEN_arg1_GREATER_THAN_arg2(
           (right_rh->e.body), (left_rh->e.body)) ) {
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, right_rh)
      *ptr_to_ptr_to_next_block = right_rh;
      ptr_to_ptr_to_next_block = &(right_rh->e.ptr_to_left);
      right_rh = right_rh->e.ptr_to_right;
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, left_rh)
      left_rh = right_rh;
    } else {
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, left_rh)
      *ptr_to_ptr_to_next_block = left_rh;
      ptr_to_ptr_to_next_block = &(left_rh->e.ptr_to_left);
      left_rh = right_rh->e.ptr_to_right;
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, right_rh)
    }
    if (left_rh == NULL) {
      *ptr_to_ptr_to_next_block = NULL;
      break;
    }
    right_rh = left_rh->e.ptr_to_right;
  } while (1);



  /* Now sll is filled with sorted pairs.  */
  /* It has at least 3 elements.           */
  /* It is not terminated with NULL yet.   */

  do { /* while (1). Terminates by `break'. */
    /* At least two blocks left.               */
    /* ptr_to_first_el_of_sll->e.ptr_to_left     */
    /* is guaranteed non-NULL.                 */
    if ( (right_block_end = (
           left_block_end =
           right_rh =
           (left_rh = ptr_to_first_el_of_sll)->e.ptr_to_left
         )->e.ptr_to_left) == NULL) {
      /* Exactly two blocks left. */
      /* Go to last pass.         */
      break;
    }
    *ptr_to_last_ptr_to_right_of_sll = NULL;
    ptr_to_last_ptr_to_right_of_sll = &ptr_to_first_el_of_sll;
    ptr_to_ptr_to_next_block = &temp_for_internal_use_TOKEN;

    do { /* while (1). Terminates by `break'. */

      /* --- BEGIN compare first two elements --- */
      if ( TOKEN_arg1_GREATER_THAN_arg2(
             (right_rh->e.body), (left_rh->e.body)) ) {
        append_to_sll_dont_change_content(
          ptr_to_last_ptr_to_right_of_sll, right_rh)
        *ptr_to_ptr_to_next_block = right_rh;
        ptr_to_ptr_to_next_block = &(right_rh->e.ptr_to_left);
        if ((right_rh = right_rh->e.ptr_to_right)
             ==
             NULL) { /* This is equiv. to "... == right_block_end" */
          *ptr_to_last_ptr_to_right_of_sll = left_rh;
          while (left_rh->e.ptr_to_right != left_block_end) {
            left_rh = left_rh->e.ptr_to_right;
          }
          ptr_to_last_ptr_to_right_of_sll = &(left_rh->e.ptr_to_right);
          *ptr_to_ptr_to_next_block = NULL;
          break; /* Go to "LABEL_1" */
        }
      } else {
        append_to_sll_dont_change_content(
          ptr_to_last_ptr_to_right_of_sll, left_rh)
        *ptr_to_ptr_to_next_block = left_rh;
        ptr_to_ptr_to_next_block = &(left_rh->e.ptr_to_left);
        /* The left block is full and has at least two el. */
        left_rh = left_rh->e.ptr_to_right;
      }
      /* ---  END  compare first two elements --- */

      /* --- BEGIN compare the rest of the pair of blocks --- */
      do {
        if ( TOKEN_arg1_GREATER_THAN_arg2(
               (right_rh->e.body), (left_rh->e.body)) ) {
          append_to_sll_dont_change_content(
            ptr_to_last_ptr_to_right_of_sll, right_rh)
          if ((right_rh = right_rh->e.ptr_to_right)
              ==
              right_block_end) {
            *ptr_to_last_ptr_to_right_of_sll = left_rh;
            while (left_rh->e.ptr_to_right != left_block_end) {
              left_rh = left_rh->e.ptr_to_right;
            }
            ptr_to_last_ptr_to_right_of_sll = &(left_rh->e.ptr_to_right);
            break;
          }
        } else {
          append_to_sll_dont_change_content(
            ptr_to_last_ptr_to_right_of_sll, left_rh)
          if ((left_rh = left_rh->e.ptr_to_right)
               ==
               left_block_end) {
            *ptr_to_last_ptr_to_right_of_sll = right_rh;
            while (right_rh->e.ptr_to_right != right_block_end) {
              right_rh = right_rh->e.ptr_to_right;
            }
            ptr_to_last_ptr_to_right_of_sll = &(right_rh->e.ptr_to_right);
            break;
          }
        }
      } while (1);
      /* ---  END  compare the rest of the pair of blocks --- */

      if ((left_rh = right_block_end) == NULL) {
        *ptr_to_ptr_to_next_block = NULL;
        break; /* Go to "LABEL_1" */
      }
      if ((right_rh =
           left_block_end =
           right_block_end->e.ptr_to_left) == NULL) {
        *ptr_to_ptr_to_next_block = left_rh;
        left_rh->e.ptr_to_left = NULL;
        *ptr_to_last_ptr_to_right_of_sll = left_rh;
        while (left_rh->e.ptr_to_right != left_block_end) {
          left_rh = left_rh->e.ptr_to_right;
        }
        ptr_to_last_ptr_to_right_of_sll = &(left_rh->e.ptr_to_right);
        break; /* Go to "LABEL_1" */
      }
      right_block_end = left_block_end ->e.ptr_to_left;
    } while (1);
    /* "LABEL_1" */
  } while (1);
  *ptr_to_last_ptr_to_right_of_sll = NULL;



  /* Print as sll.
  printf("--- beginning of sll ---\n");
  left_rh = ptr_to_first_el_of_sll;
  while (left_rh) {
    printf("[%03X]: %c %02d ->next_bl = %03X\n",
           ((int) left_rh) & 0xFFF,
           left_rh->e.body.c,
           left_rh->e.body.i,
           ((int) left_rh->e.ptr_to_left) & 0xFFF);
    left_rh = left_rh->e.ptr_to_right;
  }
  printf("---    end    of sll ---\n\n");
  getchar();
  */



  /* Now sll is filled with exactly 2 large sorted blocks.   */
  /* It has at least 3 elements. It is terminated with NULL. */
  /* ptr_to_first_el_of_sll->e.ptr_to_left     */
  /* is guaranteed non-NULL.                 */

  left_block_end = right_rh =
    (left_rh = ptr_to_first_el_of_sll)->e.ptr_to_left;
  /* "right_block_end" is not used because it is for sure NULL. */

  if (sll_of_unused_el_of_sdll_TOKEN == NULL)
    new_block_of_99_elem_of_sdll_TOKEN();
  ptr_to_last_el_inserted = sll_of_unused_el_of_sdll_TOKEN;
  sll_of_unused_el_of_sdll_TOKEN =
    sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;

  sdll->ptr_to_leftmost = ptr_to_last_el_inserted;

  do {
    if ( TOKEN_arg1_GREATER_THAN_arg2(
           (right_rh->e.body), (left_rh->e.body)) ) {
      ptr_to_last_el_inserted->e.ptr_to_right = right_rh;
      right_rh->e.ptr_to_left = ptr_to_last_el_inserted;
      ptr_to_last_el_inserted = right_rh;
      if ((right_rh = right_rh->e.ptr_to_right) == NULL) {
        do {
          ptr_to_last_el_inserted->e.ptr_to_right = left_rh;
          left_rh->e.ptr_to_left = ptr_to_last_el_inserted;
          ptr_to_last_el_inserted = left_rh;
        } while ((left_rh = left_rh->e.ptr_to_right)
                 !=
                 left_block_end);
        break;
      }
    } else {
      ptr_to_last_el_inserted->e.ptr_to_right = left_rh;
      left_rh->e.ptr_to_left = ptr_to_last_el_inserted;
      ptr_to_last_el_inserted = left_rh;
      if ((left_rh = left_rh->e.ptr_to_right) == left_block_end) {
        do {
          ptr_to_last_el_inserted->e.ptr_to_right = right_rh;
          right_rh->e.ptr_to_left = ptr_to_last_el_inserted;
          ptr_to_last_el_inserted = right_rh;
        } while ((right_rh = right_rh->e.ptr_to_right)
                 !=
                 NULL);
        break;
      }
    }
  } while (1);

  ptr_to_last_el_inserted->e.ptr_to_right = NULL;
  sdll->ptr_to_rightmost = ptr_to_last_el_inserted;

  /* Delete the leftmost element.          */
  /* It's ok that its left ptr is not set. */
  temp_for_internal_use_TOKEN = (*sdll).ptr_to_leftmost;
  temp_for_internal_use_TOKEN->e.ptr_to_right->e.ptr_to_left
    = NULL;
  (*sdll).ptr_to_leftmost =
    temp_for_internal_use_TOKEN->e.ptr_to_right;
  temp_for_internal_use_TOKEN->u.ptr_to_next_el_of_sll =
    sll_of_unused_el_of_sdll_TOKEN;
  sll_of_unused_el_of_sdll_TOKEN =
    temp_for_internal_use_TOKEN;
}







static ptr_to_node_of_sdll_sgt_TOKEN
build_2balanced_tree_from_sll(
int64_t n) {
  /* `temp_for_internal_use_TOKEN' is the read head.            */
  /* The argument is always >= 2 (including all recursive calls). */

  ptr_to_node_of_sdll_sgt_TOKEN rv;
  ptr_to_node_of_sdll_sgt_TOKEN t;
  int64_t left_branch_n;

  switch (n) {

    case 2:
    rv = temp_for_internal_use_TOKEN;
    rv->e.ptr_to_right = t = temp_for_internal_use_TOKEN =
      temp_for_internal_use_TOKEN->e.ptr_to_right;
    rv->e.ptr_to_left = NULL;
    temp_for_internal_use_TOKEN =
      temp_for_internal_use_TOKEN->e.ptr_to_right;
    t->e.ptr_to_left = t->e.ptr_to_right = NULL;
    return rv;

    case 3:
    t = temp_for_internal_use_TOKEN;
    rv = temp_for_internal_use_TOKEN =
      temp_for_internal_use_TOKEN->e.ptr_to_right;
    t->e.ptr_to_left = t->e.ptr_to_right = NULL;
    temp_for_internal_use_TOKEN =
      temp_for_internal_use_TOKEN->e.ptr_to_right;
    rv->e.ptr_to_left = t;
    t = rv->e.ptr_to_right = temp_for_internal_use_TOKEN;
    temp_for_internal_use_TOKEN =
      temp_for_internal_use_TOKEN->e.ptr_to_right;
    t->e.ptr_to_left = t->e.ptr_to_right = NULL;
    return rv;

    case 4:
    t = temp_for_internal_use_TOKEN;
    rv = temp_for_internal_use_TOKEN =
      temp_for_internal_use_TOKEN->e.ptr_to_right;
    t->e.ptr_to_left = t->e.ptr_to_right = NULL;
    temp_for_internal_use_TOKEN =
      temp_for_internal_use_TOKEN->e.ptr_to_right;
    rv->e.ptr_to_left = t;
    t = rv->e.ptr_to_right = temp_for_internal_use_TOKEN;
    temp_for_internal_use_TOKEN =
      temp_for_internal_use_TOKEN->e.ptr_to_right;
    t->e.ptr_to_left = NULL;
    t = t->e.ptr_to_right = temp_for_internal_use_TOKEN;
    temp_for_internal_use_TOKEN =
      temp_for_internal_use_TOKEN->e.ptr_to_right;
    t->e.ptr_to_left = t->e.ptr_to_right = NULL;
    return rv;

    default:
    t = build_2balanced_tree_from_sll(left_branch_n = (n >> 1));
    rv = temp_for_internal_use_TOKEN;
    temp_for_internal_use_TOKEN =
      temp_for_internal_use_TOKEN->e.ptr_to_right;
    rv->e.ptr_to_left = t;
    rv->e.ptr_to_right = build_2balanced_tree_from_sll(n - left_branch_n - 1);
    return rv;

  }
}



void
move_all_elements_to_empty_sgt_TOKEN(
sgt_TOKEN * sgt, sdll_TOKEN * sdll) {
  ptr_to_node_of_sdll_sgt_TOKEN   left_rh;
  ptr_to_node_of_sdll_sgt_TOKEN   right_rh;
  ptr_to_node_of_sdll_sgt_TOKEN   left_block_end;
  ptr_to_node_of_sdll_sgt_TOKEN   right_block_end;
  ptr_to_node_of_sdll_sgt_TOKEN   ptr_to_first_el_of_sll;
  ptr_to_node_of_sdll_sgt_TOKEN * ptr_to_last_ptr_to_right_of_sll;
  ptr_to_node_of_sdll_sgt_TOKEN * ptr_to_ptr_to_next_block;
  int64_t                           num_entries;

  set_iter_to_leftmost_el_sdll_TOKEN(left_rh, *sdll)
  if (left_rh  == NULL) return; /* 0 elements in the list. */
  right_rh = left_rh->e.ptr_to_right;
  if (right_rh == NULL) {
    /* 1 element in the list. */
    sgt->ptr_to_root = left_rh;
    sgt->max_size_since_full_rebuild = sgt->num_entries = 1;
    return;
  }

  ptr_to_last_ptr_to_right_of_sll = &ptr_to_first_el_of_sll;

  if ( TOKEN_arg1_GREATER_THAN_arg2(
         (left_rh->e.body), (right_rh->e.body)) ) {
    append_to_sll_dont_change_content(
      ptr_to_last_ptr_to_right_of_sll, right_rh)
    ptr_to_ptr_to_next_block = &(right_rh->e.ptr_to_left);
    right_rh = right_rh->e.ptr_to_right;
    append_to_sll_dont_change_content(
      ptr_to_last_ptr_to_right_of_sll, left_rh)
    left_rh = right_rh;
  } else {
    append_to_sll_dont_change_content(
      ptr_to_last_ptr_to_right_of_sll, left_rh)
    ptr_to_ptr_to_next_block = &(left_rh->e.ptr_to_left);
    left_rh = right_rh->e.ptr_to_right;
    append_to_sll_dont_change_content(
      ptr_to_last_ptr_to_right_of_sll, right_rh)
  }
  if (left_rh == NULL) {
    /* 2 elements in the list. */
    /* --- BEGIN Make sll a vaild tree --- */
    (sgt->ptr_to_root = ptr_to_first_el_of_sll)->e.ptr_to_left = NULL;
    ptr_to_first_el_of_sll->e.ptr_to_right->e.ptr_to_left = NULL;
    *ptr_to_last_ptr_to_right_of_sll = NULL;
    sgt->max_size_since_full_rebuild = sgt->num_entries = 2;
    /* ---  END  Make sll a vaild tree --- */
    return;
  }
  right_rh = left_rh->e.ptr_to_right;

  num_entries = 2;

  do { /* while (1). Terminates by `break'. */
    /* Now left_rh is not NULL, right_rh may be NULL. */
    if (right_rh == NULL) {
      num_entries++;
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, left_rh)
      *ptr_to_ptr_to_next_block = left_rh;
      left_rh->e.ptr_to_left = NULL;
      break;
    }
    if ( TOKEN_arg1_GREATER_THAN_arg2(
           (left_rh->e.body), (right_rh->e.body)) ) {
      num_entries += 2;
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, right_rh)
      *ptr_to_ptr_to_next_block = right_rh;
      ptr_to_ptr_to_next_block = &(right_rh->e.ptr_to_left);
      right_rh = right_rh->e.ptr_to_right;
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, left_rh)
      left_rh = right_rh;
    } else {
      num_entries += 2;
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, left_rh)
      *ptr_to_ptr_to_next_block = left_rh;
      ptr_to_ptr_to_next_block = &(left_rh->e.ptr_to_left);
      left_rh = right_rh->e.ptr_to_right;
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, right_rh)
    }
    if (left_rh == NULL) {
      *ptr_to_ptr_to_next_block = NULL;
      break;
    }
    right_rh = left_rh->e.ptr_to_right;
  } while (1);



  /* Now sll is filled with sorted pairs.  */
  /* It has at least 3 elements.           */
  /* It is not terminated with NULL yet.   */

  do { /* while (1). Terminates by `break'. */
    /* At least two blocks left.               */
    /* ptr_to_first_el_of_sll->e.ptr_to_left     */
    /* is guaranteed non-NULL.                 */
    if ( (right_block_end = (
           left_block_end =
           right_rh =
           (left_rh = ptr_to_first_el_of_sll)->e.ptr_to_left
         )->e.ptr_to_left) == NULL) {
      /* Exactly two blocks left. */
      /* Go to last pass.         */
      break;
    }
    *ptr_to_last_ptr_to_right_of_sll = NULL;
    ptr_to_last_ptr_to_right_of_sll = &ptr_to_first_el_of_sll;
    ptr_to_ptr_to_next_block = &temp_for_internal_use_TOKEN;

    do { /* while (1). Terminates by `break'. */

      /* --- BEGIN compare first two elements --- */
      if ( TOKEN_arg1_GREATER_THAN_arg2(
             (left_rh->e.body), (right_rh->e.body)) ) {
        append_to_sll_dont_change_content(
          ptr_to_last_ptr_to_right_of_sll, right_rh)
        *ptr_to_ptr_to_next_block = right_rh;
        ptr_to_ptr_to_next_block = &(right_rh->e.ptr_to_left);
        if ((right_rh = right_rh->e.ptr_to_right)
             ==
             NULL) { /* This is equiv. to "... == right_block_end" */
          *ptr_to_last_ptr_to_right_of_sll = left_rh;
          while (left_rh->e.ptr_to_right != left_block_end) {
            left_rh = left_rh->e.ptr_to_right;
          }
          ptr_to_last_ptr_to_right_of_sll = &(left_rh->e.ptr_to_right);
          *ptr_to_ptr_to_next_block = NULL;
          break; /* Go to "LABEL_1" */
        }
      } else {
        append_to_sll_dont_change_content(
          ptr_to_last_ptr_to_right_of_sll, left_rh)
        *ptr_to_ptr_to_next_block = left_rh;
        ptr_to_ptr_to_next_block = &(left_rh->e.ptr_to_left);
        /* The left block is full and has at least two el. */
        left_rh = left_rh->e.ptr_to_right;
      }
      /* ---  END  compare first two elements --- */

      /* --- BEGIN compare the rest of the pair of blocks --- */
      do {
        if ( TOKEN_arg1_GREATER_THAN_arg2(
               (left_rh->e.body), (right_rh->e.body)) ) {
          append_to_sll_dont_change_content(
            ptr_to_last_ptr_to_right_of_sll, right_rh)
          if ((right_rh = right_rh->e.ptr_to_right)
              ==
              right_block_end) {
            *ptr_to_last_ptr_to_right_of_sll = left_rh;
            while (left_rh->e.ptr_to_right != left_block_end) {
              left_rh = left_rh->e.ptr_to_right;
            }
            ptr_to_last_ptr_to_right_of_sll = &(left_rh->e.ptr_to_right);
            break;
          }
        } else {
          append_to_sll_dont_change_content(
            ptr_to_last_ptr_to_right_of_sll, left_rh)
          if ((left_rh = left_rh->e.ptr_to_right)
               ==
               left_block_end) {
            *ptr_to_last_ptr_to_right_of_sll = right_rh;
            while (right_rh->e.ptr_to_right != right_block_end) {
              right_rh = right_rh->e.ptr_to_right;
            }
            ptr_to_last_ptr_to_right_of_sll = &(right_rh->e.ptr_to_right);
            break;
          }
        }
      } while (1);
      /* ---  END  compare the rest of the pair of blocks --- */

      if ((left_rh = right_block_end) == NULL) {
        *ptr_to_ptr_to_next_block = NULL;
        break; /* Go to "LABEL_1" */
      }
      if ((right_rh =
           left_block_end =
           right_block_end->e.ptr_to_left) == NULL) {
        *ptr_to_ptr_to_next_block = left_rh;
        left_rh->e.ptr_to_left = NULL;
        *ptr_to_last_ptr_to_right_of_sll = left_rh;
        while (left_rh->e.ptr_to_right != left_block_end) {
          left_rh = left_rh->e.ptr_to_right;
        }
        ptr_to_last_ptr_to_right_of_sll = &(left_rh->e.ptr_to_right);
        break; /* Go to "LABEL_1" */
      }
      right_block_end = left_block_end ->e.ptr_to_left;
    } while (1);
    /* "LABEL_1" */
  } while (1);
  *ptr_to_last_ptr_to_right_of_sll = NULL;

  /* Now sll is filled with exactly 2 large sorted blocks.   */
  /* It has at least 3 elements. It is terminated with NULL. */
  /* ptr_to_first_el_of_sll->e.ptr_to_left     */
  /* is guaranteed non-NULL.                 */

  left_block_end = right_rh =
    (left_rh = ptr_to_first_el_of_sll)->e.ptr_to_left;
  /* "right_block_end" is not used because it is for sure NULL. */

  /* Now, normally the next command would be:                   */
  /* ptr_to_last_ptr_to_right_of_sll = &ptr_to_first_el_of_sll; */
  /* But instead we start the final sll with global variable    */
  /* `temp_for_internal_use_TOKEN' to make it accessible for  */
  /* the recursive function `build_2balanced_tree_from_sll'.    */
  ptr_to_last_ptr_to_right_of_sll = &temp_for_internal_use_TOKEN;

  do {
    if ( TOKEN_arg1_GREATER_THAN_arg2(
           (left_rh->e.body), (right_rh->e.body)) ) {
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, right_rh)
      if ((right_rh = right_rh->e.ptr_to_right) == NULL) {
        *ptr_to_last_ptr_to_right_of_sll = left_rh;
        while (left_rh->e.ptr_to_right != left_block_end) {
          left_rh = left_rh->e.ptr_to_right;
        }
        ptr_to_last_ptr_to_right_of_sll = &(left_rh->e.ptr_to_right);
        break;
      }
    } else {
      append_to_sll_dont_change_content(
        ptr_to_last_ptr_to_right_of_sll, left_rh)
      if ((left_rh = left_rh->e.ptr_to_right) == left_block_end) {
        *ptr_to_last_ptr_to_right_of_sll = right_rh;
        while (right_rh->e.ptr_to_right != NULL) {
          right_rh = right_rh->e.ptr_to_right;
        }
        ptr_to_last_ptr_to_right_of_sll = &(right_rh->e.ptr_to_right);
        break;
      }
    }
  } while (1);

  /* sll is still not terminated with NULL.                  */
  /* We do not need this because the # of elements is known. */
  /* sll starts at `temp_for_internal_use_TOKEN'           */

  /* This is a recursive function. The argument is always >= 2. */
  sgt->ptr_to_root = build_2balanced_tree_from_sll(
    sgt->max_size_since_full_rebuild = sgt->num_entries = num_entries);
}







#define \
set_iter_to_the_smallest_element_of_the_tree_being_rebuilt(\
root_of_the_tree_being_rebuilt)\
{\
  ptr_to_last_el_of_stack_for_subtree_rebuild = stack_for_subtree_rebuild;\
  stack_for_subtree_rebuild[0] = root_of_the_tree_being_rebuilt;\
  while (\
    (\
      *(ptr_to_last_el_of_stack_for_subtree_rebuild + 1) =\
      (*ptr_to_last_el_of_stack_for_subtree_rebuild)->e.ptr_to_left\
    ) != NULL\
  ) ptr_to_last_el_of_stack_for_subtree_rebuild++;\
}



#define \
rm_the_smallest_el_of_the_tree_being_rebuilt_and_set_iter_to_next_smallest \
{\
  if (ptr_to_last_el_of_stack_for_subtree_rebuild\
      ==\
      stack_for_subtree_rebuild) {\
    stack_for_subtree_rebuild[0] =\
      stack_for_subtree_rebuild[0]->e.ptr_to_right;\
    if (*ptr_to_last_el_of_stack_for_subtree_rebuild != NULL) while (\
      (\
        *(ptr_to_last_el_of_stack_for_subtree_rebuild + 1) =\
        (*ptr_to_last_el_of_stack_for_subtree_rebuild)->e.ptr_to_left\
      ) != NULL\
    ) ptr_to_last_el_of_stack_for_subtree_rebuild++;\
  } else {\
    *ptr_to_last_el_of_stack_for_subtree_rebuild =\
      (*(ptr_to_last_el_of_stack_for_subtree_rebuild - 1))->e.ptr_to_left =\
        (*ptr_to_last_el_of_stack_for_subtree_rebuild)->e.ptr_to_right;\
    if (*ptr_to_last_el_of_stack_for_subtree_rebuild != NULL) while (\
      (\
        *(ptr_to_last_el_of_stack_for_subtree_rebuild + 1) =\
        (*ptr_to_last_el_of_stack_for_subtree_rebuild)->e.ptr_to_left\
      ) != NULL\
    ) ptr_to_last_el_of_stack_for_subtree_rebuild++;\
    else ptr_to_last_el_of_stack_for_subtree_rebuild--;\
  }\
}



int64_t
weight_of_TOKEN(
ptr_to_entry_of_sgt_TOKEN p) {
  if (p == NULL) return 0;
  else {
    return weight_of_TOKEN(p->e.ptr_to_left)
           +
           weight_of_TOKEN(p->e.ptr_to_right)
           +
           1;
  }
}



static ptr_to_node_of_sdll_sgt_TOKEN
rebuild_subtree_in_place(
int64_t n) {
  /* `smallest_remaining_el_of_subtree_being_rebuilt' is the read head. */
  /* The argument is always >= 2 (including all recursive calls).       */

  ptr_to_node_of_sdll_sgt_TOKEN rv;
  ptr_to_node_of_sdll_sgt_TOKEN t;
  int64_t left_branch_n;

  switch (n) {

    case 2:
    rv = *ptr_to_last_el_of_stack_for_subtree_rebuild;
    rm_the_smallest_el_of_the_tree_being_rebuilt_and_set_iter_to_next_smallest
    rv->e.ptr_to_right = t = *ptr_to_last_el_of_stack_for_subtree_rebuild;
    rv->e.ptr_to_left = NULL;
    rm_the_smallest_el_of_the_tree_being_rebuilt_and_set_iter_to_next_smallest
    t->e.ptr_to_left = t->e.ptr_to_right = NULL;
    return rv;

    case 3:
    t = *ptr_to_last_el_of_stack_for_subtree_rebuild;
    rm_the_smallest_el_of_the_tree_being_rebuilt_and_set_iter_to_next_smallest
    rv = *ptr_to_last_el_of_stack_for_subtree_rebuild;
    t->e.ptr_to_left = t->e.ptr_to_right = NULL;
    rm_the_smallest_el_of_the_tree_being_rebuilt_and_set_iter_to_next_smallest
    rv->e.ptr_to_left = t;
    t = rv->e.ptr_to_right = *ptr_to_last_el_of_stack_for_subtree_rebuild;;
    rm_the_smallest_el_of_the_tree_being_rebuilt_and_set_iter_to_next_smallest
    t->e.ptr_to_left = t->e.ptr_to_right = NULL;
    return rv;

    case 4:
    t = *ptr_to_last_el_of_stack_for_subtree_rebuild;
    rm_the_smallest_el_of_the_tree_being_rebuilt_and_set_iter_to_next_smallest
    rv = *ptr_to_last_el_of_stack_for_subtree_rebuild;
    t->e.ptr_to_left = t->e.ptr_to_right = NULL;
    rm_the_smallest_el_of_the_tree_being_rebuilt_and_set_iter_to_next_smallest
    rv->e.ptr_to_left = t;
    t = rv->e.ptr_to_right = *ptr_to_last_el_of_stack_for_subtree_rebuild;
    rm_the_smallest_el_of_the_tree_being_rebuilt_and_set_iter_to_next_smallest
    t->e.ptr_to_left = NULL;
    t = t->e.ptr_to_right = *ptr_to_last_el_of_stack_for_subtree_rebuild;
    rm_the_smallest_el_of_the_tree_being_rebuilt_and_set_iter_to_next_smallest
    t->e.ptr_to_left = t->e.ptr_to_right = NULL;
    return rv;

    default:
    t = rebuild_subtree_in_place(left_branch_n = (n >> 1));
    rv = *ptr_to_last_el_of_stack_for_subtree_rebuild;
    rm_the_smallest_el_of_the_tree_being_rebuilt_and_set_iter_to_next_smallest
    rv->e.ptr_to_left = t;
    rv->e.ptr_to_right = rebuild_subtree_in_place(n - left_branch_n - 1);
    return rv;

  }
}



void
insert__if_conflicts_treat_as_eps_smallest__sgt_TOKEN(
ptr_to_entry_of_sgt_TOKEN ptr_e, sgt_TOKEN * sgt) {
  int64_t w;
  int     h;
  path_element_for_sgt_insert
    path_to_new_node[MAX_NUM_LEVELS_IN_SGT_TOKEN + 7];
  path_element_for_sgt_insert * path_end;
  int depth_of_new_node = 0;
  ptr_to_entry_of_sgt_TOKEN pn = sgt->ptr_to_root;

  if (pn == NULL) {
    sgt->ptr_to_root = ptr_e;
  } else while (1) {
    depth_of_new_node++;
    if (TOKEN_arg1_GREATER_THAN_arg2(ptr_e->e.body, pn->e.body)) {
      if (pn->e.ptr_to_right != NULL) {
        pn = pn->e.ptr_to_right;
      } else {
        pn->e.ptr_to_right = ptr_e;
        break;
      }
    } else {
      if (pn->e.ptr_to_left != NULL) {
        pn = pn->e.ptr_to_left;
      } else {
        pn->e.ptr_to_left = ptr_e;
        break;
      }
    }
  }

  ptr_e->e.ptr_to_left = ptr_e->e.ptr_to_right = NULL;
  if ( (++(sgt->num_entries)) > sgt->max_size_since_full_rebuild )
    sgt->max_size_since_full_rebuild = sgt->num_entries;

  /* The element has just been inserted as into a regular binary search tree. */

  if (sgt->num_entries
      <
      min_num_elements_for_sgt_TOKEN_of_given_height[depth_of_new_node]) {

    /* Walk from root to the newly inserted node again to record the path. */
    path_end = path_to_new_node;
    pn = sgt->ptr_to_root;
    while (pn != NULL) {
      if (TOKEN_arg1_GREATER_THAN_arg2(ptr_e->e.body, pn->e.body)) {
        path_end->p = pn;
        path_end->descend_to_the_left = 0; /* i.e. = false */
        pn = pn->e.ptr_to_right;
      } else {
        path_end->p = pn;
        path_end->descend_to_the_left = 1; /* i.e. = true */
        pn = pn->e.ptr_to_left;
      }
      path_end++;
    }

    w = 1;
    h = 0;
    path_end--;
    /* Now `path_end' points to pointer to the new node                  */
    /* The new node makes the tree height unbalanced, so the new node    */
    /* can not be the root. So, there are at least two elements in path. */
    do {
      path_end--;
      h++;
      if (path_end->descend_to_the_left != 0) {
        w = w + 1 + weight_of_TOKEN(path_end->p->e.ptr_to_right);
      } else {
        w = w + 1 + weight_of_TOKEN(path_end->p->e.ptr_to_left);
      }
    } while (w >= min_num_elements_for_sgt_TOKEN_of_given_height[h]);

    /* Now `path_end->p' points to the scapegoat node. Rebuild! */

    set_iter_to_the_smallest_element_of_the_tree_being_rebuilt(path_end->p)
    if (path_end->p == sgt->ptr_to_root) {
      sgt->ptr_to_root = rebuild_subtree_in_place(w);
    } else {
      path_end--;
      if (path_end->descend_to_the_left != 0) {
        path_end->p->e.ptr_to_left = rebuild_subtree_in_place(w);
      } else {
        path_end->p->e.ptr_to_right = rebuild_subtree_in_place(w);
      }
    }

  }
}



void
insert__if_conflicts_treat_as_eps_greatest__sgt_TOKEN(
ptr_to_entry_of_sgt_TOKEN ptr_e, sgt_TOKEN * sgt) {
  int64_t w;
  int     h;
  path_element_for_sgt_insert
    path_to_new_node[MAX_NUM_LEVELS_IN_SGT_TOKEN + 7];
  path_element_for_sgt_insert * path_end;
  int depth_of_new_node = 0;
  ptr_to_entry_of_sgt_TOKEN pn = sgt->ptr_to_root;

  if (pn == NULL) {
    sgt->ptr_to_root = ptr_e;
  } else while (1) {
    depth_of_new_node++;
    if (TOKEN_arg1_GREATER_THAN_arg2(pn->e.body, ptr_e->e.body)) {
      if (pn->e.ptr_to_left != NULL) {
        pn = pn->e.ptr_to_left;
      } else {
        pn->e.ptr_to_left = ptr_e;
        break;
      }
    } else {
      if (pn->e.ptr_to_right != NULL) {
        pn = pn->e.ptr_to_right;
      } else {
        pn->e.ptr_to_right = ptr_e;
        break;
      }
    }
  }

  ptr_e->e.ptr_to_left = ptr_e->e.ptr_to_right = NULL;
  if ( (++(sgt->num_entries)) > sgt->max_size_since_full_rebuild )
    sgt->max_size_since_full_rebuild = sgt->num_entries;

  /* The element has just been inserted as into a regular binary search tree. */

  if (sgt->num_entries
      <
      min_num_elements_for_sgt_TOKEN_of_given_height[depth_of_new_node]) {

    /* Walk from root to the newly inserted node again to record the path. */
    path_end = path_to_new_node;
    pn = sgt->ptr_to_root;
    while (pn != NULL) {
      if (TOKEN_arg1_GREATER_THAN_arg2(pn->e.body, ptr_e->e.body)) {
        path_end->p = pn;
        path_end->descend_to_the_left = 1; /* i.e. = true */
        pn = pn->e.ptr_to_left;
      } else {
        path_end->p = pn;
        path_end->descend_to_the_left = 0; /* i.e. = false */
        pn = pn->e.ptr_to_right;
      }
      path_end++;
    }

    w = 1;
    h = 0;
    path_end--;
    /* Now `path_end' points to pointer to the new node                  */
    /* The new node makes the tree height unbalanced, so the new node    */
    /* can not be the root. So, there are at least two elements in path. */
    do {
      path_end--;
      h++;
      if (path_end->descend_to_the_left != 0) {
        w = w + 1 + weight_of_TOKEN(path_end->p->e.ptr_to_right);
      } else {
        w = w + 1 + weight_of_TOKEN(path_end->p->e.ptr_to_left);
      }
    } while (w >= min_num_elements_for_sgt_TOKEN_of_given_height[h]);

    /* Now `path_end->p' points to the scapegoat node. Rebuild! */

    set_iter_to_the_smallest_element_of_the_tree_being_rebuilt(path_end->p)
    if (path_end->p == sgt->ptr_to_root) {
      sgt->ptr_to_root = rebuild_subtree_in_place(w);
    } else {
      path_end--;
      if (path_end->descend_to_the_left != 0) {
        path_end->p->e.ptr_to_left = rebuild_subtree_in_place(w);
      } else {
        path_end->p->e.ptr_to_right = rebuild_subtree_in_place(w);
      }
    }

  }
}



int
insert__if_conflicts_bounce__sgt_TOKEN(
ptr_to_entry_of_sgt_TOKEN ptr_e, sgt_TOKEN * sgt) {
  int64_t w;
  int     h;
  path_element_for_sgt_insert
    path_to_new_node[MAX_NUM_LEVELS_IN_SGT_TOKEN + 7];
  path_element_for_sgt_insert * path_end;
  int depth_of_new_node = 0;
  ptr_to_entry_of_sgt_TOKEN pn = sgt->ptr_to_root;

  if (pn == NULL) {
    sgt->ptr_to_root = ptr_e;
  } else while (1) {
    depth_of_new_node++;
    if (TOKEN_arg1_GREATER_THAN_arg2(pn->e.body, ptr_e->e.body)) {
      if (pn->e.ptr_to_left != NULL) {
        pn = pn->e.ptr_to_left;
      } else {
        pn->e.ptr_to_left = ptr_e;
        break;
      }
    } else
    if (TOKEN_arg1_GREATER_THAN_arg2(ptr_e->e.body, pn->e.body)) {
      if (pn->e.ptr_to_right != NULL) {
        pn = pn->e.ptr_to_right;
      } else {
        pn->e.ptr_to_right = ptr_e;
        break;
      }
    } else return 0;
  }

  ptr_e->e.ptr_to_left = ptr_e->e.ptr_to_right = NULL;
  if ( (++(sgt->num_entries)) > sgt->max_size_since_full_rebuild )
    sgt->max_size_since_full_rebuild = sgt->num_entries;

  /* The element has just been inserted as into a regular binary search tree. */

  if (sgt->num_entries
      <
      min_num_elements_for_sgt_TOKEN_of_given_height[depth_of_new_node]) {

    /* Walk from root to the newly inserted node again to record the path. */
    path_end = path_to_new_node;
    pn = sgt->ptr_to_root;
    while (pn != NULL) {
      if (TOKEN_arg1_GREATER_THAN_arg2(pn->e.body, ptr_e->e.body)) {
        path_end->p = pn;
        path_end->descend_to_the_left = 1; /* i.e. = true */
        pn = pn->e.ptr_to_left;
      } else {
        path_end->p = pn;
        path_end->descend_to_the_left = 0; /* i.e. = false */
        pn = pn->e.ptr_to_right;
      }
      path_end++;
    }

    w = 1;
    h = 0;
    path_end--;
    /* Now `path_end' points to pointer to the new node                  */
    /* The new node makes the tree height unbalanced, so the new node    */
    /* can not be the root. So, there are at least two elements in path. */
    do {
      path_end--;
      h++;
      if (path_end->descend_to_the_left != 0) {
        w = w + 1 + weight_of_TOKEN(path_end->p->e.ptr_to_right);
      } else {
        w = w + 1 + weight_of_TOKEN(path_end->p->e.ptr_to_left);
      }
    } while (w >= min_num_elements_for_sgt_TOKEN_of_given_height[h]);

    /* Now `path_end->p' points to the scapegoat node. Rebuild! */

    set_iter_to_the_smallest_element_of_the_tree_being_rebuilt(path_end->p)
    if (path_end->p == sgt->ptr_to_root) {
      sgt->ptr_to_root = rebuild_subtree_in_place(w);
    } else {
      path_end--;
      if (path_end->descend_to_the_left != 0) {
        path_end->p->e.ptr_to_left = rebuild_subtree_in_place(w);
      } else {
        path_end->p->e.ptr_to_right = rebuild_subtree_in_place(w);
      }
    }

  }
  return 1;
}



TOKEN*
find_prep_rm__if_many_give_eps_smallest__sgt_TOKEN(
TOKEN * key, sgt_TOKEN * sgt) {
  TOKEN *                     rv = NULL;
  ptr_to_entry_of_sgt_TOKEN   temp = sgt->ptr_to_root;
  ptr_to_entry_of_sgt_TOKEN * p = &(sgt->ptr_to_root);

  ptr_to_home_sgt_of_the_element_ready_for_rm = sgt;

  while (temp != NULL)
  if ( TOKEN_arg1_GREATER_THAN_arg2( (*key), (temp->e.body)) ) {
    p =  &(temp->e.ptr_to_right);
    temp = temp->e.ptr_to_right;
  } else if ( TOKEN_arg1_GREATER_THAN_arg2( (temp->e.body), (*key)) ) {
    p =  &(temp->e.ptr_to_left);
    temp = temp->e.ptr_to_left;
  } else {
    ptr_to_ptr_to_the_element_ready_for_rm = p;
    rv = &(temp->e.body);
    p =  &(temp->e.ptr_to_left);
    temp = temp->e.ptr_to_left;
  }

  if (rv == NULL) ptr_to_ptr_to_the_element_ready_for_rm =
    &NULL_ptr_to_entry_of_sgt_TOKEN_having_an_address;

  return rv;
}



TOKEN*
find_prep_rm__if_many_give_eps_greatest__sgt_TOKEN(
TOKEN * key, sgt_TOKEN * sgt) {
  TOKEN *                     rv = NULL;
  ptr_to_entry_of_sgt_TOKEN   temp = sgt->ptr_to_root;
  ptr_to_entry_of_sgt_TOKEN * p = &(sgt->ptr_to_root);

  ptr_to_home_sgt_of_the_element_ready_for_rm = sgt;

  while (temp != NULL)
  if ( TOKEN_arg1_GREATER_THAN_arg2((*key), (temp->e.body)) ) {
    p =  &(temp->e.ptr_to_right);
    temp = temp->e.ptr_to_right;
  } else if ( TOKEN_arg1_GREATER_THAN_arg2( (temp->e.body), (*key)) ) {
    p =  &(temp->e.ptr_to_left);
    temp = temp->e.ptr_to_left;
  } else {
    ptr_to_ptr_to_the_element_ready_for_rm = p;
    rv = &(temp->e.body);
    p =  &(temp->e.ptr_to_right);
    temp = temp->e.ptr_to_right;
  }

  if (rv == NULL) ptr_to_ptr_to_the_element_ready_for_rm =
    &NULL_ptr_to_entry_of_sgt_TOKEN_having_an_address;

  return rv;
}



TOKEN*
find_prep_rm__smallest__sgt_TOKEN(
sgt_TOKEN * sgt) {
  ptr_to_home_sgt_of_the_element_ready_for_rm = sgt;
  ptr_to_ptr_to_the_element_ready_for_rm = &(sgt->ptr_to_root);
  if (sgt->ptr_to_root != NULL) {
    while((*ptr_to_ptr_to_the_element_ready_for_rm)->e.ptr_to_left != NULL) {
      ptr_to_ptr_to_the_element_ready_for_rm =
        &((*ptr_to_ptr_to_the_element_ready_for_rm)->e.ptr_to_left);
    }
    return &((*ptr_to_ptr_to_the_element_ready_for_rm)->e.body);
  } else return NULL;
}



TOKEN*
find_prep_rm__greatest__sgt_TOKEN(
sgt_TOKEN * sgt)  {
  ptr_to_home_sgt_of_the_element_ready_for_rm = sgt;
  ptr_to_ptr_to_the_element_ready_for_rm = &(sgt->ptr_to_root);
  if (sgt->ptr_to_root != NULL) {
    while((*ptr_to_ptr_to_the_element_ready_for_rm)->e.ptr_to_right != NULL) {
      ptr_to_ptr_to_the_element_ready_for_rm =
        &((*ptr_to_ptr_to_the_element_ready_for_rm)->e.ptr_to_right);
    }
    return &((*ptr_to_ptr_to_the_element_ready_for_rm)->e.body);
  } else return NULL;
}



ptr_to_entry_of_sgt_TOKEN
remove_entry_from_sgt_TOKEN(
void) {
  ptr_to_entry_of_sgt_TOKEN * p;
  ptr_to_entry_of_sgt_TOKEN   temp;
  ptr_to_entry_of_sgt_TOKEN   rv;

  if ((rv = *ptr_to_ptr_to_the_element_ready_for_rm) == NULL) return NULL;

  if ((*ptr_to_ptr_to_the_element_ready_for_rm)->e.ptr_to_right == NULL) {
    *ptr_to_ptr_to_the_element_ready_for_rm =
      (*ptr_to_ptr_to_the_element_ready_for_rm)->e.ptr_to_left;
  } else
  if ((*ptr_to_ptr_to_the_element_ready_for_rm)->e.ptr_to_left == NULL) {
    *ptr_to_ptr_to_the_element_ready_for_rm =
      (*ptr_to_ptr_to_the_element_ready_for_rm)->e.ptr_to_right;
  } else {
    p = &((*ptr_to_ptr_to_the_element_ready_for_rm)->e.ptr_to_left);
    while((*p)->e.ptr_to_right != NULL) p = &((*p)->e.ptr_to_right);
    temp = *p;
    *p = (*p)->e.ptr_to_left;
    temp->e.ptr_to_left =
      (*ptr_to_ptr_to_the_element_ready_for_rm)->e.ptr_to_left;
    temp->e.ptr_to_right =
      (*ptr_to_ptr_to_the_element_ready_for_rm)->e.ptr_to_right;
    *ptr_to_ptr_to_the_element_ready_for_rm = temp;
  }

  ptr_to_home_sgt_of_the_element_ready_for_rm->num_entries--;
  /*
  Note: Before the delete it was:   max / num <= 3/2
  So after delete:                  max / (num+1) <= 3/2,
                                    num+1 >= (2/3) * max.
  So,  3 * (max-num) = 3 * (max - (num+1) + 1) <= 3 * (max/3 + 1) <= max + 3.
  Assuming max <= 2^63 - 4 there is no overflow.
  */
  if (3 * (ptr_to_home_sgt_of_the_element_ready_for_rm->
        max_size_since_full_rebuild
      - ptr_to_home_sgt_of_the_element_ready_for_rm->num_entries)
      >
      ptr_to_home_sgt_of_the_element_ready_for_rm->
        max_size_since_full_rebuild) {
    ptr_to_home_sgt_of_the_element_ready_for_rm->
      max_size_since_full_rebuild =
        ptr_to_home_sgt_of_the_element_ready_for_rm->num_entries;
    if (ptr_to_home_sgt_of_the_element_ready_for_rm->num_entries >= 3) {

      set_iter_to_the_smallest_element_of_the_tree_being_rebuilt(
        ptr_to_home_sgt_of_the_element_ready_for_rm->ptr_to_root);
      ptr_to_home_sgt_of_the_element_ready_for_rm->ptr_to_root =
        rebuild_subtree_in_place(
          ptr_to_home_sgt_of_the_element_ready_for_rm->num_entries);
    }
  }
  return rv;
}



ptr_to_entry_of_sgt_TOKEN
for_debug_only__get_el_prep_rm_TOKEN(
void) {
  return *ptr_to_ptr_to_the_element_ready_for_rm;
}



int64_t
for_debug_only__min_w_required_TOKEN(
int h) {
  return min_num_elements_for_sgt_TOKEN_of_given_height[h];
}
