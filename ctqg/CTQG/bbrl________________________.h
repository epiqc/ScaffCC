#ifndef BBRL_________________________H
#define BBRL_________________________H



#include <bbrl________________sdll_sgt.h>
#include <bbrl__________memory_manager.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>



typedef enum {
  OK_____________________________________________________SOSMUTMRV,
  STACK_IS_EMPTY_________________________________________SOSMUTMRV,
  CANT_MAKE_COMPOSITION_WITH_THE_PREV_CALLEE_TO_MAIN_MAP_SOSMUTMRV,
  DUPLICATE_CALLEE_SIGNAL_NAME___________________________SOSMUTMRV,
  DUPLICATE_CALLER_SIGNAL_NAME___________________________SOSMUTMRV,
  NAME_IS_ALREADY_MAPPED__CANT_BE_USED_FOR_ANCILLA_SIG___SOSMUTMRV,
  INVALID________________________________________________SOSMUTMRV
} sosm_update_top_map__rv_t;



extern FILE *   out_file;
extern FILE *   out_signals_file;
extern FILE *   ancilla_file;
extern FILE *   sim_in_file;
extern FILE *   sim_out_file;
extern uint64_t bbrl_ncs;
extern uint64_t ni;
extern uint64_t num_zero_to_garbage_ancillas;
extern uint64_t num_one_to_garbage_ancillas;
extern uint64_t num_zero_to_zero_ancillas;
extern uint64_t num_one_to_one_ancillas;



#define MANDATORY_ARGS char * file, uint64_t line, uint64_t num_control_signals
#define LOCATION_INFO __FILE__, __LINE__, bbrl_ncs



#define BEGIN_DEFINITION_OF_MODULE \
uint64_t save_n_ztz_anc = num_zero_to_zero_ancillas;\
uint64_t save_n_oto_anc = num_one_to_one_ancillas;\
sdll_ATOM stack_of_control_signals = {NULL, NULL};\
int yQnoBegDef = \
begin_definition_of_module(\
__FILE__, __LINE__, file, line, &stack_of_control_signals);

int
begin_definition_of_module(
char * def_file, uint64_t def_line, char * file, uint64_t line,
sdll_ATOM * ptr_to_local_stack_of_cs);



#define ZERO_TO_GARBAGE_ANCILLA(name, index) \
{if (yQnoBegDef) \
zero_to_garbage_ancilla(__FILE__, __LINE__, (name), (index));}

void
zero_to_garbage_ancilla(
char * file, uint64_t line, char * sig_name, uint64_t sig_index);



#define ONE_TO_GARBAGE_ANCILLA(name, index) \
{if (yQnoBegDef) \
one_to_garbage_ancilla(__FILE__, __LINE__, (name), (index));}

void
one_to_garbage_ancilla(
char * file, uint64_t line, char * sig_name, uint64_t sig_index);



#define ZERO_TO_ZERO_ANCILLA(name, index) \
{if (yQnoBegDef) \
zero_to_zero_ancilla(__FILE__, __LINE__, (name), (index));}

void
zero_to_zero_ancilla(
char * file, uint64_t line, char * sig_name, uint64_t sig_index);



#define ONE_TO_ONE_ANCILLA(name, index) \
{if (yQnoBegDef) \
one_to_one_ancilla(__FILE__, __LINE__, (name), (index));}

void
one_to_one_ancilla(
char * file, uint64_t line, char * sig_name, uint64_t sig_index);



#define INSTANTIATE_MODULE \
{bbrl_ncs = num_control_signals; \
if (yQnoBegDef) \
instantiate_module(__FILE__, __LINE__, stack_of_control_signals);}

void
instantiate_module(
char * file, uint64_t line, sdll_ATOM stack_of_cs);



#define PUSH_CONTROL_SIGNAL(control_s, control_i) \
{if (yQnoBegDef) \
push_control_signal( \
__FILE__, __LINE__, (control_s), (control_i), &stack_of_control_signals);}

void
push_control_signal(
char * file, uint64_t line, char * cs, uint64_t ci, sdll_ATOM * stack_of_cs);



#define POP_AND_DELETE_CONTROL_SIGNAL \
{if (yQnoBegDef) \
pop_and_delete_control_signal( \
__FILE__, __LINE__, &stack_of_control_signals);}

#define POP_CONTROL_SIGNAL \
{if (yQnoBegDef) \
pop_and_delete_control_signal( \
__FILE__, __LINE__, &stack_of_control_signals);}

void
pop_and_delete_control_signal(
char * file, uint64_t line, sdll_ATOM * stack_of_cs);

uint64_t
pop_n_del_cs_for_else(
char * file, uint64_t line, sdll_ATOM * stack_of_cs);



#define MAP_SIGNAL(ks, ki, vs, vi) \
{if (yQnoBegDef) \
map_signal(__FILE__, __LINE__, (ks), (ki), (vs), (vi));}

void
map_signal(
char * file, uint64_t line,
char * key_s, uint64_t key_i, char * value_s, uint64_t value_i);



#define END_DEFINITION_OF_MODULE \
{if (yQnoBegDef) \
end_definition_of_module(__FILE__, __LINE__,\
is_nonempty_sdll_ATOM(stack_of_control_signals),\
save_n_ztz_anc, save_n_oto_anc);}

void
end_definition_of_module(
char * file, uint64_t line,
int stack_of_cs_nonempty, uint64_t ztz, uint64_t oto);



void
stack_of_signal_maps__push_a_pair_of_empty_maps(
void);

sosm_update_top_map__rv_t
stack_of_signal_maps__update_top_map__regular(
char * key_s, uint64_t key_i, char * value_s, uint64_t value_i);

sosm_update_top_map__rv_t
stack_of_signal_maps__update_top_map__ztgarb(
char * key_s, uint64_t key_i);

sosm_update_top_map__rv_t
stack_of_signal_maps__update_top_map__otgarb(
char * key_s, uint64_t key_i);

sosm_update_top_map__rv_t
stack_of_signal_maps__update_top_map__ztzero(
char * key_s, uint64_t key_i);

sosm_update_top_map__rv_t
stack_of_signal_maps__update_top_map__otone(
char * key_s, uint64_t key_i);

int /* R.v.: (1 != 0) on success, (0 != 0) - stack is empty. */
stack_of_signal_maps__pop_and_delete(
void);

int /* R.v.: (1 != 0) - success, (0 != 0) - failure. */
stack_of_signal_maps__fprint_global_name(
FILE * f, char * local_name_s, uint64_t local_name_i);



void
call_stack_push(
char *   instantiation_file,
uint64_t instantiation_line,
char *   body_file,
uint64_t body_line);

int /* R.v.: (1 != 0) on success, (0 != 0) - stack is empty. */
call_stack_pop_and_delete(
void);

void
call_stack_fprint(
FILE * f);



void
set_of_signals_used_in_main__update(
char * sig_name, uint64_t sig_index);

uint64_t *
get_ptr_to_value__in_set_of_signals_used_in_main(
char * local_name_s, uint64_t local_name_i);

void
create_signals_file(
void);

void
create_sim_out_file(
void);

void
read_sim_in_file__to__set_of_signals_used_in_main(
void);



void
uint64_fprint(
FILE * f, uint64_t i);

void
h_file_and_line_fprint(
FILE * f, str_on_heap_ATOM file, uint64_t line);

void
s_file_and_line_fprint(
FILE * f, char * file, uint64_t line);

void
h_signal_name_fprint(
FILE * f, str_on_heap_ATOM s, uint64_t i);

void
h_signal_name_fprint_with_spaces_tilda_and_eol(
FILE * f, str_on_heap_ATOM s, uint64_t i);

void
s_signal_name_fprint(
FILE * f, char * s, uint64_t i);

void
statistics_fprint(
FILE * f);

void
create_ancilla_file_for_qasm(
void);



void
delete_ATOM(
ATOM * arg);

void
delete_sgt_ATOM(
sgt_ATOM * arg);

void
clean_exit(
int status);



void
not(
char * file, uint64_t line, uint64_t num_control_signals);

#define YqNOT(xs, xi) \
INSTANTIATE_MODULE \
MAP_SIGNAL("x", 0,  (xs), (xi)) \
not(LOCATION_INFO);



void
cnot(
char * file, uint64_t line, uint64_t num_control_signals);

#define YqCNOT(cs, ci,  xs, xi) \
INSTANTIATE_MODULE \
MAP_SIGNAL("c", 0,  (cs), (ci)) \
MAP_SIGNAL("x", 0,  (xs), (xi)) \
cnot(LOCATION_INFO);



void
toffoli(
char * file, uint64_t line, uint64_t num_control_signals);

#define YqTOFFOLI(c1s, c1i, c2s, c2i, xs, xi) \
INSTANTIATE_MODULE \
MAP_SIGNAL("c1", 0,  (c1s), (c1i)) \
MAP_SIGNAL("c2", 0,  (c2s), (c2i)) \
MAP_SIGNAL("x", 0,   (xs), (xi)) \
toffoli(LOCATION_INFO);



/* ----------------- BEGIN `bbrl_lib_integer_arithmetic_.h' ----------------- */



/* Swap two integers.                                   */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
void
a_swap_b(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* Assign integer value: a := b.                        */
/*                                                      */
/* Zero-to-garbage bits : n                             */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
void
assign_value_of_b_to_a(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* In place adder for two n-bit integers: a += b .      */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* TOFFOLI : 2n - 2                                     */
/* CNOT    : 6n - 3                                     */
/* NOT     : 0                                          */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
void
a__eq__a_plus_b(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* In place subtractor for two n-bit integers: a -= b . */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* TOFFOLI : 2n - 2                                     */
/* CNOT    : 6n - 3                                     */
/* NOT     : 0                                          */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
void
a__eq__a_minus_b(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* In place subtractor for two n-bit integers: a -= b   */
/* Inverses unsigned_underflow bit if an unsigned       */
/* overflow occures.                                    */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* TOFFOLI : 2n - 1                                     */
/* CNOT    : 6n - 2                                     */
/* NOT     : 0                                          */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
/*          unsigned_underflow[0]                       */
void
a__eq__a_minus_b__w_unsigned_underflow_bit(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* Comparator: Inverses x if a < b as                   */
/* signed. Does not change a and b.                     */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* TOFFOLI : 4n - 3                                     */
/* CNOT    : 12n - 5                                    */
/* NOT     : 4                                          */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
/*          x[0]                                        */
void
a_less_than_b__as_signed(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* Comparator: Inverses x if a <= b as                  */
/* signed. Does not change a and b.                     */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* TOFFOLI : 4n - 3                                     */
/* CNOT    : 12n - 5                                    */
/* NOT     : 5                                          */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
/*          x[0]                                        */
void
a_less_than_or_eq_to_b__as_signed(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* Comparator: Inverses x if a > b as                   */
/* signed. Does not change a and b.                     */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* TOFFOLI : 4n - 3                                     */
/* CNOT    : 12n - 5                                    */
/* NOT     : 4                                          */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
/*          x[0]                                        */
void
a_greater_than_b__as_signed(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* Comparator: Inverses x if a >= b as                  */
/* signed. Does not change a and b.                     */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* TOFFOLI : 4n - 3                                     */
/* CNOT    : 12n - 5                                    */
/* NOT     : 5                                          */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
/*          x[0]                                        */
void
a_greater_than_or_eq_to_b__as_signed(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* Comparator: Inverses x if a == b.                    */
/* Does not change a and b.                             */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
/*          x[0]                                        */
void
is_a_eq_to_b(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* Comparator: Inverses x if a != b.                    */
/* Does not change a and b.                             */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
/*          x[0]                                        */
void
is_a_not_eq_to_b(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* a += b * c;  a, b and c are n-bit integers.          */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* Weight (NOT=1, CNOT=2, TOFFOLI=3, 3cTOFFOLI=4 ... ): */
/*   13*n^2 - 4*n                                       */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
/*          c[n-1 ... 0]                                */
void
a__eq__a_plus_b_times_c(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* a -= b * c;  a, b and c are n-bit integers.          */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* Weight (NOT=1, CNOT=2, TOFFOLI=3, 3cTOFFOLI=4 ... ): */
/*   13*n^2 - 4*n                                       */
/*                                                      */
/* Signals:                                             */
/*          a[n-1 ... 0]                                */
/*          b[n-1 ... 0]                                */
/*          c[n-1 ... 0]                                */
void
a__eq__a_minus_b_times_c(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* -----------------  END  `bbrl_lib_integer_arithmetic_.h' ----------------- */



int /* Ret. (1 != 0) on success, (0 != 0) if not found. */
search_main_var_decl_table(
int * signal_type, uint64_t * num_indexes, uint64_t ** ranges,
str_on_heap_ATOM key);



#endif
