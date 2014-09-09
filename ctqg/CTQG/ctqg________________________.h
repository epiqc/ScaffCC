#ifndef CTQG_________________________H
#define CTQG_________________________H



#include <ctqg________________sdll_sgt.h>
#include <ctqg__________memory_manager.h>
#include <ctqg_________________defines.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>



/*
typedef enum {
  ID____________TT,      _, A-Z, a-z     read while    _, A-Z, a-z, 0-9.
  DECIMAL_CONST_TT,      0-9             read while    0-9, u, l.
  SPACE_________TT,      <SPACE>         read while    <SPACE>
  PERIODS_______TT,      .               read while    .
  OPERATOR______TT,      !<=>/-*+%&|~^:  read while    OPERATOR SYMBOL
  END_OF_LINE___TT,      <EOL>           exactly one byte
  BRACKETS______TT,      []              exactly one byte, count and store depth
  STDALN_SYM____TT,      "'`;\?#$,@(){}  exactly one byte
  INVALID_______TT
} token_type_t;

typedef struct {
  str_on_heap_TOKEN value;
  uint32_t          line;
  uint32_t          brackets_depth;
  token_type_t      type;
} TOKEN;
*/

typedef void*(*STATE_OF_AUTOMATON_func_ptr_t)(void);

typedef struct {
  str_on_heap_TOKEN ptr_back_to_name; /* Does not own memory. */
  uint32_t          signal_type;
  uint32_t          num_indexes;
  str_on_heap_TOKEN indexes[MAX_NUM_INDEXES];
  str_on_heap_TOKEN subst_indexes[MAX_NUM_INDEXES];
} var_decl_value_t;
/* `signal_type' - not a enum because used is multiple executables.
  0 - regular int,
  1 - regular bit,
  2 - zero_to_garbage ancilla bit,
  3 - one_to_garbage ancilla bit
  4 - zero_to_zero ancilla bit,
  5 - one_to_one ancilla_bit.
*/



void
tokenize(
void);

void
assemble_tokens(
void);

void
fprint_tokens(
FILE * stream, sdll_TOKEN sdll);

void /* Excpt: a single "-" is not cons. an op. (bec. of unary minus issue). */
to_next_operator_or_closed_parenthesis_token_of_same_depth_trNULLlfmst(
iter_sdll_TOKEN * iter);

void /* Excpt: a single "-" is not cons. an op. (bec. of unary minus issue). */
to_next_operator_token_of_same_depth_before_semicolumn_trNULLlfmst(
iter_sdll_TOKEN * iter);

void
to_next_non_separ_token_treating_NULL_as_leftmost(
iter_sdll_TOKEN * iter);

void
to_previous_non_separ_token_treating_NULL_as_rightmost(
iter_sdll_TOKEN * iter);

void
move_iter_and_delete_to_next_non_separ_token(
iter_sdll_TOKEN * iter);

void
delete_TOKEN_and_move_iter_to_right(
iter_sdll_TOKEN * iter, sdll_TOKEN * sdll);

void
copy_token_to_the_left_of_iter(
iter_sdll_TOKEN iter, TOKEN * source);

void
insert__space__to_the_left_of_iter(
iter_sdll_TOKEN iter);

void
insert__semicolumn_space__to_the_left_of_iter(
iter_sdll_TOKEN iter);

void
insert__stroper__to_the_left_of_iter(
iter_sdll_TOKEN iter, char * strid);

void
insert__strstdaln__to_the_left_of_iter(
iter_sdll_TOKEN iter, char * strid);

void
insert__strid__to_the_left_of_iter(
iter_sdll_TOKEN iter, char * strid);

void
insert__bracket_uint32_bracket__to_the_left_of_iter(
iter_sdll_TOKEN iter, uint32_t n);

void
insert__str_uint32_str__to_the_left_of_iter(
iter_sdll_TOKEN iter, char * str1, uint32_t n, char * str2);

void
insert__str_h_str__to_the_left_of_iter(
iter_sdll_TOKEN iter, char * str1, str_on_heap_TOKEN h, char * str2);

void
insert__str_uint32_str_h_str_to_the_left_of_iter(
iter_sdll_TOKEN iter, char * str1, uint32_t n, char * str2,
str_on_heap_TOKEN h, char * str3);

void
insert__s_ui32_s_hpostfix_s_ui32_s_to_the_left_of_iter(
iter_sdll_TOKEN iter, char * str1, uint32_t n1, char * str2,
str_on_heap_TOKEN h1, char * str3, uint32_t n2, char * str4);

void
insert__s_ui32_s_hpostfix_s_to_the_left_of_iter(
iter_sdll_TOKEN iter, char * str1, uint32_t n1, char * str2,
str_on_heap_TOKEN h1, char * str3);

void
insert__str_h_str_hpostfix_str_to_the_left_of_iter(
iter_sdll_TOKEN iter, char * str1, str_on_heap_TOKEN h1, char * str2,
str_on_heap_TOKEN h2, char * str3);

void
delete_sdll_TOKEN(
sdll_TOKEN * sdll);

void
clean_exit(
int status);



var_decl_value_t*
search_var_decl(
str_on_heap_TOKEN name_of_module, str_on_heap_TOKEN name_of_var);

var_decl_value_t*
search_var_decl_by_number(
str_on_heap_TOKEN name_of_module, uint32_t n);

var_decl_value_t*   /* Checks for storage overflow internally. */
new_var_decl(       /* Returns NULL only for duplicate id.     */
str_on_heap_TOKEN name_of_module, str_on_heap_TOKEN name_of_var);

void
delete_st_var_decl_sgt(
void);

void /* Exits with error message on overflow. */
add_index_to_var_decl__no_alloc(
var_decl_value_t * var_decl_val, str_on_heap_TOKEN str);

void
replicate_var_decl_with_numeric_name(
str_on_heap_TOKEN module_name, uint32_t arg_number, var_decl_value_t * source);

str_on_heap_TOKEN /* t_iter1 ->`[' (otherwise fails). Ret. NULL on failure. */
read_expression_in_brackets__alloc(        /* Changes `[', `]' to `(', `)'. */
void);

str_on_heap_TOKEN /* t_iter1 ->`[' (otherwise fails). Ret. NULL on failure. */
read_expression_in_brackets_subst_param__alloc(
void);            /* Changes `[', `]' to `(', `)'. */

str_on_heap_TOKEN /* t_iter1 ->`[' (otherwise fails). Ret. NULL on failure. */
read_and_del_expression_in_brackets__alloc(/* Changes `[', `]' to `(', `)'. */
void);

void /* t_iter1 ->`['. PTR,PTR - range, PTR,NULL - value, NULL,NULL - fail */
read_range_in_brackets__alloc( /* If fails, t_iter1 does not mv. Rmvs [ ]. */
str_on_heap_TOKEN * target_lo, str_on_heap_TOKEN * target_hi);

void
delete_st_param_array(
void);

void
debug_print_st_param_array(
void);



void /* l(0) := 0; l(1) := 1 l(2)=l(3):= 2 */
binary_unsigned_a_eq_a_plus_b(
uint32_t * la, char * a, uint32_t lb, char * b);

void
binary_mult_by_10(
uint32_t * l, char * b);

void
h_to_binary_s__as_signed(
str_on_heap_TOKEN source, char * target);



void
ctqg_to_nc(
void);

void
nc_to_lf(
void);

void
lf_to_le(
void);

void
le_to_ld(
void);

void
ld_to_bbrl(
void);



void
run_automaton(
STATE_OF_AUTOMATON_func_ptr_t initial_state);

void*
termination_state(
void);



void*
d_st_inside_body_not_inside_ancilla_decl(
void);

void*
d_st_read_param_list_and_module_name(
void);

void*
d_st_read_declarations_of_signals(
void);

void*
d_st_pre_termination(
void);

void*
d_st_read_ztg_declaration(
void);

void*
d_st_read_otg_declaration(
void);

void*
d_st_read_ztz_declaration(
void);

void*
d_st_read_oto_declaration(
void);

void*
d_st_read_instantiate_module_keyword(
void);

void*
d_st_convert_local_signal(
void);

void*
d_st_convert_signal_of_callee(
void);

void
d_convert_id(
str_on_heap_TOKEN name_of_module, str_on_heap_TOKEN name_of_var,
int is_native_id);

void
d_read_ancilla_declaration(
uint32_t signal_type);

void
d_insert_add_def_mod(
void);

void
d_create__main_var_decl_table_and_finalize_token_list(
void);



void*
e_st_create_var_decl_sgt(
void);

void*
e_st_create_var_decl_sgt_pre_termination(
void);

void*
e_st_read_param_array_and_module_name(
void);

void*
e_st_skip_param_array__read_module_name__skip_decl_of_sig(
void);

void*
e_st_read_ztg_declaration(
void);

void*
e_st_read_otg_declaration(
void);

void*
e_st_read_ztz_declaration(
void);

void*
e_st_read_oto_declaration(
void);

void*
e_st_read_declarations_of_signals(
void);

void*
e_st_inside_body(
void);

void*
e_st_inside_dollar_block(
void);

void*
e_st_read_instantiation(
void);

void*
e_st_read_operator(
void);

void*
e_st_pre_termination(
void);

void
add_var_declarations_from_built_in_modules(
void);

void
e_leaving_a_module(
void);

void
e_read_ancilla_declaration(
uint32_t signal_type);

void /* Leaves `t_iter1' pointing to the delimiter `,', `)' or smth. else */
e_process_compound_signal_or_constant(
uint32_t ee_argument_number);

void /* Leaves `t_iter1' pointing to <operator> or `;' */
e_process_param_defining_cs_of_oper(
uint32_t ee_argument_number);

void /* Dumps info from `st_param_array' to the left of t_iter2. */
e_write_parameters_of_instantiation(
void);

void /* Uses `st_callee_name' and `st_param_array_num_entries'. */
e_write_closing_of_instantiation(
void);

void
e_get_name_and_position_of_param_defining_cs__of_operator(
void);

void /* Leaves `t_iter1' pointing to the delimiter `,', `)' or smth. else */
e_process_bit_constant(
var_decl_value_t * ptr_to_ee_var_decl);

void /* Leaves `t_iter1' pointing to the delimiter `,', `)' or smth. else */
e_process_1d_bit_array_constant(
var_decl_value_t * ptr_to_ee_var_decl);

void /* Leaves `t_iter1' pointing to the delimiter `,', `)' or smth. else */
e_process_integer_constant(
var_decl_value_t * ptr_to_ee_var_decl);

void /* Leaves `t_iter1' pointing to the delimiter `,', `)' or smth. else */
e_process_compound_signal(
var_decl_value_t * ptr_to_ee_var_decl);

void
e_debug_print_decl_table(
void);



void*
f_st_outside_module_body(
void);

void*
f_st_inside_module_body(
void);

void*
f_st_pre_termination(
void);

void
f_if_to_layer_e(
iter_sdll_TOKEN iter__i_point_to_dollar_sign);

void
f_else_to_layer_e(
iter_sdll_TOKEN iter__i_point_to_dollar_sign);

void
f_endif_to_layer_e(
iter_sdll_TOKEN iter__i_point_to_dollar_sign);

int /* Returns 1 != 0 if this is the last term, 0 != 0 otherwise. */
f_process_one_term_of_if(
iter_sdll_TOKEN iter__i_point_to_dollar_sign);



void*
ctqg_to_nc_st_not_inside_comment(
void);

void*
ctqg_to_nc_st_inside_c_style_comment(
void);



#endif
