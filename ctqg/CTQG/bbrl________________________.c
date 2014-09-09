#include <bbrl________________________.h>



/* #define BBRL_DEBUG */

#define MAX_STACK_SIZE 16384



FILE *                        out_file = NULL;
FILE *                        out_signals_file = NULL;
FILE *                        ancilla_file = NULL;
FILE *                        sim_in_file = NULL;
FILE *                        sim_out_file = NULL;



struct {
  sgt_ATOM callee_to_main;
  sgt_ATOM caller_to_callee;
}                             stack_of_signal_maps[MAX_STACK_SIZE];
uint64_t                      size_of_stack_of_signal_maps = 0;

ATOM                          call_stack[MAX_STACK_SIZE];
uint64_t                      size_of_call_stack = 0;

sdll_ATOM *                   stack_for_mem_cleaning_only[MAX_STACK_SIZE];
uint64_t                      size_of_stack_for_mem_cleaning_only = 0;

sgt_ATOM                      set_of_signals_used_in_main = {NULL, 0, 0};
uint64_t                      num_zero_to_garbage_ancillas = 0;
uint64_t                      num_one_to_garbage_ancillas = 0;
uint64_t                      num_zero_to_zero_ancillas = 0;
uint64_t                      num_one_to_one_ancillas = 0;
uint64_t                      peak_num_zero_to_zero_ancillas = 0;
uint64_t                      peak_num_one_to_one_ancillas = 0;

int                           is_inside_instantiate_module_block = (1 != 0);
uint64_t                      bbrl_ncs;



uint64_t ni = (uint64_t) -1;

struct {
  uint64_t total_num_gates;
  uint64_t n_control_toffoli[64];
  uint64_t toffoli_with_64_or_more_controls;
} statistics =
{
  0,
  {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  },
  0
};



sgt_ATOM name_to_index_in_main_var_decl_table = {NULL, 0, 0};



int
begin_definition_of_module(
char * def_file, uint64_t def_line, char * file, uint64_t line,
sdll_ATOM * ptr_to_local_stack_of_cs) {
  #ifdef BBRL_DEBUG
  printf("Before BEGIN_DEFINITION_OF_MODULE on line ");
  s_file_and_line_fprint(stdout, def_file, def_line);
  printf(" c_st_size = ");
  uint64_fprint(stdout, size_of_call_stack);
  printf(" sm_st_size = ");
  uint64_fprint(stdout, size_of_stack_of_signal_maps);
  printf(".\n");
  #endif

  stack_for_mem_cleaning_only[size_of_stack_for_mem_cleaning_only++] =
    ptr_to_local_stack_of_cs;

  if (!is_inside_instantiate_module_block) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`INSTANTIATE_MODULE' keyword and signal map description "
            "are missing before (failed) instantiation on line ");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr, ".\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  } else {
    is_inside_instantiate_module_block = (0 != 0);
  }

  call_stack_push(file, line, def_file, def_line);

  #ifdef BBRL_DEBUG
  printf("After  BEGIN_DEFINITION_OF_MODULE on line ");
  s_file_and_line_fprint(stdout, def_file, def_line);
  printf(" c_st_size = ");
  uint64_fprint(stdout, size_of_call_stack);
  printf(" sm_st_size = ");
  uint64_fprint(stdout, size_of_stack_of_signal_maps);
  printf(".\n");
  #endif

  return (1 != 0);
}



void
zero_to_garbage_ancilla(
char * file, uint64_t line, char * sig_name, uint64_t sig_index) {
  sosm_update_top_map__rv_t r =
    INVALID________________________________________________SOSMUTMRV;

  if (is_inside_instantiate_module_block) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : ZERO_TO_GARBAGE_ANCILLA can not be requested inside of "
            "INSTANTIATE_MODULE block.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  r = stack_of_signal_maps__update_top_map__ztgarb(sig_name, sig_index);

  switch (r) {
    case OK_____________________________________________________SOSMUTMRV:
      /* Do nothing. */
    break;

    case STACK_IS_EMPTY_________________________________________SOSMUTMRV:
      fprintf(stderr, "Error.\n");
      s_file_and_line_fprint(stderr, file, line);
      fprintf(stderr,
              " : ZERO_TO_GARBAGE_ANCILLA is requested in main_module.\n");
      clean_exit(-1);
    break;

    case NAME_IS_ALREADY_MAPPED__CANT_BE_USED_FOR_ANCILLA_SIG___SOSMUTMRV:
      fprintf(stderr, "Error.\n");
      s_file_and_line_fprint(stderr, file, line);
      fprintf(stderr,
              " (in ZERO_TO_GARBAGE_ANCILLA request) : "
              "Signal with name `");
      s_signal_name_fprint(stderr, sig_name, sig_index);
      fprintf(stderr,
              "' is already mapped "
              "or this name is already used for another "
              "ANCILLA.\n");
      call_stack_fprint(stderr);
      clean_exit(-1);
    break;

    default:
      fprintf(stderr, "Error.\n");
      fprintf(stderr, "An internal BBRL error occurred on line ");
      s_file_and_line_fprint(stderr, __FILE__, __LINE__);
      fprintf(stderr, ".\n");
      call_stack_fprint(stderr);
      clean_exit(-2);
  }
}



void
one_to_garbage_ancilla(
char * file, uint64_t line, char * sig_name, uint64_t sig_index) {
  sosm_update_top_map__rv_t r =
    INVALID________________________________________________SOSMUTMRV;

  if (is_inside_instantiate_module_block) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : ONE_TO_GARBAGE_ANCILLA can not be requested inside of "
            "INSTANTIATE_MODULE block.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  r = stack_of_signal_maps__update_top_map__otgarb(sig_name, sig_index);

  switch (r) {
    case OK_____________________________________________________SOSMUTMRV:
      /* Do nothing. */
    break;

    case STACK_IS_EMPTY_________________________________________SOSMUTMRV:
      fprintf(stderr, "Error.\n");
      s_file_and_line_fprint(stderr, file, line);
      fprintf(stderr,
              " : ONE_TO_GARBAGE_ANCILLA is requested in main_module.\n");
      clean_exit(-1);
    break;

    case NAME_IS_ALREADY_MAPPED__CANT_BE_USED_FOR_ANCILLA_SIG___SOSMUTMRV:
      fprintf(stderr, "Error.\n");
      s_file_and_line_fprint(stderr, file, line);
      fprintf(stderr,
              " (in ONE_TO_GARBAGE_ANCILLA request) : "
              "Signal with name `");
      s_signal_name_fprint(stderr, sig_name, sig_index);
      fprintf(stderr,
              "' is already mapped "
              "or this name is already used for another "
              "ANCILLA.\n");
      call_stack_fprint(stderr);
      clean_exit(-1);
    break;

    default:
      fprintf(stderr, "Error.\n");
      fprintf(stderr, "An internal BBRL error occurred on line ");
      s_file_and_line_fprint(stderr, __FILE__, __LINE__);
      fprintf(stderr, ".\n");
      call_stack_fprint(stderr);
      clean_exit(-2);
  }
}



void
zero_to_zero_ancilla(
char * file, uint64_t line, char * sig_name, uint64_t sig_index) {
  sosm_update_top_map__rv_t r =
    INVALID________________________________________________SOSMUTMRV;

  if (is_inside_instantiate_module_block) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : ZERO_TO_ZERO_ANCILLA can not be requested inside of "
            "INSTANTIATE_MODULE block.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  r = stack_of_signal_maps__update_top_map__ztzero(sig_name, sig_index);

  switch (r) {
    case OK_____________________________________________________SOSMUTMRV:
      /* Do nothing. */
    break;

    case STACK_IS_EMPTY_________________________________________SOSMUTMRV:
      fprintf(stderr, "Error.\n");
      s_file_and_line_fprint(stderr, file, line);
      fprintf(stderr,
              " : ZERO_TO_ZERO_ANCILLA is requested in main_module.\n");
      clean_exit(-1);
    break;

    case NAME_IS_ALREADY_MAPPED__CANT_BE_USED_FOR_ANCILLA_SIG___SOSMUTMRV:
      fprintf(stderr, "Error.\n");
      s_file_and_line_fprint(stderr, file, line);
      fprintf(stderr,
              " (in ZERO_TO_ZERO_ANCILLA request) : "
              "Signal with name `");
      s_signal_name_fprint(stderr, sig_name, sig_index);
      fprintf(stderr,
              "' is already mapped "
              "or this name is already used for another "
              "ANCILLA.\n");
      call_stack_fprint(stderr);
      clean_exit(-1);
    break;

    default:
      fprintf(stderr, "Error.\n");
      fprintf(stderr, "An internal BBRL error occurred on line ");
      s_file_and_line_fprint(stderr, __FILE__, __LINE__);
      fprintf(stderr, ".\n");
      call_stack_fprint(stderr);
      clean_exit(-2);
  }
}



void
one_to_one_ancilla(
char * file, uint64_t line, char * sig_name, uint64_t sig_index) {
  sosm_update_top_map__rv_t r =
    INVALID________________________________________________SOSMUTMRV;

  if (is_inside_instantiate_module_block) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : ONE_TO_ONE_ANCILLA can not be requested inside of "
            "INSTANTIATE_MODULE block.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  r = stack_of_signal_maps__update_top_map__otone(sig_name, sig_index);

  switch (r) {
    case OK_____________________________________________________SOSMUTMRV:
      /* Do nothing. */
    break;

    case STACK_IS_EMPTY_________________________________________SOSMUTMRV:
      fprintf(stderr, "Error.\n");
      s_file_and_line_fprint(stderr, file, line);
      fprintf(stderr,
              " : ONE_TO_ONE_ANCILLA is requested in main_module.\n");
      clean_exit(-1);
    break;

    case NAME_IS_ALREADY_MAPPED__CANT_BE_USED_FOR_ANCILLA_SIG___SOSMUTMRV:
      fprintf(stderr, "Error.\n");
      s_file_and_line_fprint(stderr, file, line);
      fprintf(stderr,
              " (in ONE_TO_ONE_ANCILLA request) : "
              "Signal with name `");
      s_signal_name_fprint(stderr, sig_name, sig_index);
      fprintf(stderr,
              "' is already mapped "
              "or this name is already used for another "
              "ANCILLA.\n");
      call_stack_fprint(stderr);
      clean_exit(-1);
    break;

    default:
      fprintf(stderr, "Error.\n");
      fprintf(stderr, "An internal BBRL error occurred on line ");
      s_file_and_line_fprint(stderr, __FILE__, __LINE__);
      fprintf(stderr, ".\n");
      call_stack_fprint(stderr);
      clean_exit(-2);
  }
}



void
instantiate_module(
char * file, uint64_t line, sdll_ATOM stack_of_cs) {
  sosm_update_top_map__rv_t r =
    INVALID________________________________________________SOSMUTMRV;
  uint64_t                  i;
  iter_sdll_ATOM iter;

  if (is_inside_instantiate_module_block) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : A (duplicate) `INSTANTIATE_MODULE' keyword encountered "
            "inside of INSTANTIATE_MODULE block.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  } else {
    is_inside_instantiate_module_block = (1 != 0);
  }

  stack_of_signal_maps__push_a_pair_of_empty_maps();

  for (i = 0; i < bbrl_ncs; i++) {
    r = stack_of_signal_maps__update_top_map__regular("$", i, "$", i);
    switch (r) {
      case OK_____________________________________________________SOSMUTMRV:
        /* Do nothnig. */
      break;

      case CANT_MAKE_COMPOSITION_WITH_THE_PREV_CALLEE_TO_MAIN_MAP_SOSMUTMRV:
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "Before instantiation on line ");
        h_file_and_line_fprint(stderr,
                               call_stack[size_of_call_stack - 1].value_s,
                               call_stack[size_of_call_stack - 1].value_i);
        fprintf(stderr, " : Control signal `");
        s_signal_name_fprint(stderr, "$", i);
        fprintf(stderr,
                "' of the callee is not mapped.\n");
        call_stack_fprint(stderr);
        clean_exit(-1);
      break;

      default:
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "An internal BBRL error occurred on line ");
        s_file_and_line_fprint(stderr, __FILE__, __LINE__);
        fprintf(stderr, ".\n");
        call_stack_fprint(stderr);
        clean_exit(-2);
    }
  }

  set_iter_to_leftmost_el_sdll_ATOM(iter, stack_of_cs)
  while (iter != NULL) {
    r = stack_of_signal_maps__update_top_map__regular(
          "$", bbrl_ncs, (char*) iter->e.body.key_s, iter->e.body.key_i);
    if (size_of_call_stack == 1) {
      set_of_signals_used_in_main__update(
        (char*) iter->e.body.key_s, iter->e.body.key_i);
    }
    switch (r) {
      case OK_____________________________________________________SOSMUTMRV:
        /* Do nothnig. */
      break;

      case CANT_MAKE_COMPOSITION_WITH_THE_PREV_CALLEE_TO_MAIN_MAP_SOSMUTMRV:
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "Before instantiation on line ");
        h_file_and_line_fprint(stderr,
                               call_stack[size_of_call_stack - 1].value_s,
                               call_stack[size_of_call_stack - 1].value_i);
        fprintf(stderr, " : Signal `");
        s_signal_name_fprint(stderr,
          (char*) iter->e.body.key_s, iter->e.body.key_i);
        fprintf(stderr,
                "' of the callee is not mapped. In the body of the "
                "callee this signal is used for the first time "
                "as a control signal on line ");
        s_file_and_line_fprint(stderr,
                               (char*) iter->e.body.value_s,
                               iter->e.body.value_i);
        fprintf(stderr,
                ". Either map this signal before the instantiation "
                "or declare it ANCILLA in the body "
                "of the callee.\n");
        call_stack_fprint(stderr);
        clean_exit(-1);
      break;

      default:
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "An internal BBRL error occurred on line ");
        s_file_and_line_fprint(stderr, __FILE__, __LINE__);
        fprintf(stderr, ".\n");
        call_stack_fprint(stderr);
        clean_exit(-2);
    }
    bbrl_ncs++;
    move_iter_right_sdll_ATOM(iter)
  }
}



void
push_control_signal(
char * file, uint64_t line, char * cs, uint64_t ci, sdll_ATOM * stack_of_cs) {
  ATOM * a;

  if (is_inside_instantiate_module_block) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : `PUSH_CONTROL_SIGNAL' can not be used inside of "
            "INSTANTIATE_MODULE block.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  new_at_the_right_end_sdll_ATOM(a, *stack_of_cs);
  a->key_s = (void*) cs;
  a->key_i = ci;
  a->value_s = (void*) file;
  a->value_i = line;
}



void
pop_and_delete_control_signal(
char * file, uint64_t line, sdll_ATOM * stack_of_cs) {
  iter_sdll_ATOM iter;

  if (is_inside_instantiate_module_block) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : `POP_AND_DELETE_CONTROL_SIGNAL' can not be used inside of "
            "INSTANTIATE_MODULE block.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  if (is_nonempty_sdll_ATOM(*stack_of_cs)) {
    set_iter_to_rightmost_el_sdll_ATOM(iter, *stack_of_cs)
    /* `iter->e.body.key_s' and `iter->e.body.value_s'           */
    /* are pointers to constant strings, they do not own memory. */
    delete_and_iter_becomes_invalid_sdll_ATOM(iter, *stack_of_cs)
  } else {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : `POP_AND_DELETE_CONTROL_SIGNAL' : "
            "The control signal stack is empty. Check that "
            "the `push' and `pop' operations for control signals "
            "are balanced in this module.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }
}



uint64_t
pop_n_del_cs_for_else(
char * file, uint64_t line, sdll_ATOM * stack_of_cs) {
  iter_sdll_ATOM iter;
  uint64_t       rv = 888888;

  if (is_inside_instantiate_module_block) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : `pop_n_del_cs_for_else' can not be used inside of "
            "INSTANTIATE_MODULE block.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  if (is_nonempty_sdll_ATOM(*stack_of_cs)) {
    set_iter_to_rightmost_el_sdll_ATOM(iter, *stack_of_cs)
    rv = iter->e.body.key_i;
    /* `iter->e.body.key_s' and `iter->e.body.value_s'           */
    /* are pointers to constant strings, they do not own memory. */
    delete_and_iter_becomes_invalid_sdll_ATOM(iter, *stack_of_cs)
  } else {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : `pop_n_del_cs_for_else' : "
            "The control signal stack is empty. Check that "
            "the `push' and `pop' operations for control signals "
            "are balanced in this module.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  return rv;
}



void
map_signal(
char * file, uint64_t line,
char * key_s, uint64_t key_i, char * value_s, uint64_t value_i) {
  sosm_update_top_map__rv_t r =
    INVALID________________________________________________SOSMUTMRV;

  if (!is_inside_instantiate_module_block) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : `MAP_SIGNAL' keyword encountered "
            "not inside of INSTANTIATE_MODULE block.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  if (size_of_call_stack == 1) {
    set_of_signals_used_in_main__update(value_s, value_i);
  }

  r = stack_of_signal_maps__update_top_map__regular(
        key_s, key_i, value_s, value_i);

  switch (r) {
  case OK_____________________________________________________SOSMUTMRV:
    /* Do nothing. */
  break;

  case STACK_IS_EMPTY_________________________________________SOSMUTMRV:
    fprintf(stderr, "Error.\n");
    fprintf(stderr, "An internal BBRL error occurred on line ");
    s_file_and_line_fprint(stderr, __FILE__, __LINE__);
    fprintf(stderr, ".\n");
    call_stack_fprint(stderr);
    clean_exit(-2);
  break;

  case CANT_MAKE_COMPOSITION_WITH_THE_PREV_CALLEE_TO_MAIN_MAP_SOSMUTMRV:
    fprintf(stderr, "Error.\n");
    fprintf(stderr, "Before instantiation on line ");
    h_file_and_line_fprint(stderr,
                           call_stack[size_of_call_stack - 1].value_s,
                           call_stack[size_of_call_stack - 1].value_i);
    fprintf(stderr, " : Signal `");
    s_signal_name_fprint(stderr, value_s, value_i);
    fprintf(stderr,
            "' of the callee is not mapped. In the body of the "
            "callee this signal is used for the first time on line ");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            ". Either map this signal before the instantiation "
            "or declare it ANCILLA in the body "
            "of the callee.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  break;

  case DUPLICATE_CALLEE_SIGNAL_NAME___________________________SOSMUTMRV:
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr, " Duplicate mapping: Signal `");
    s_signal_name_fprint(stderr, key_s, key_i);
    fprintf(stderr,
            "' has already been mapped.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  break;

  case DUPLICATE_CALLER_SIGNAL_NAME___________________________SOSMUTMRV:
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr, " : Duplicate mapping to signal `");
    s_signal_name_fprint(stderr, value_s, value_i);
    fprintf(stderr,
            "' of the caller.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  break;

  default:
    fprintf(stderr, "Error.\n");
    fprintf(stderr, "An internal BBRL error occurred on line ");
    s_file_and_line_fprint(stderr, __FILE__, __LINE__);
    fprintf(stderr, ".\n");
    call_stack_fprint(stderr);
    clean_exit(-2);
  }
}


void
end_definition_of_module(
char * file, uint64_t line,
int stack_of_cs_nonempty, uint64_t ztz, uint64_t oto) {
  uint64_t i;
  /* For cheking and removing `ztz' and `oto' ancillas. */
  ATOM                     search_key;
  ATOM *                   ptr_to_search_result;
  ptr_to_entry_of_sgt_ATOM ptr_to_detached_entry;

  #ifdef BBRL_DEBUG
  printf("Before END_DEFINITION_OF_MODULE on line ");
  s_file_and_line_fprint(stdout, file, line);
  printf(" c_st_size = ");
  uint64_fprint(stdout, size_of_call_stack);
  printf(" sm_st_size = ");
  uint64_fprint(stdout, size_of_stack_of_signal_maps);
  printf(".\n");
  #endif

  if (is_inside_instantiate_module_block) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : `END_DEFINITION_OF_MODULE' keyword encountered "
            "inside of INSTANTIATE_MODULE block.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  if (stack_of_cs_nonempty) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : Stack of control signals is not "
            "empty at `END_DEFINITION_OF_MODULE'.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  size_of_stack_for_mem_cleaning_only--;

  /* --- BEGIN Check if all `ztz' and `oto' values are restored. --- */
  if (sim_in_file != NULL) {
    for (i = ztz; i < num_zero_to_zero_ancillas; i++) {
      h_ATOM_alloc_strcpy_hs(&(search_key.key_s), "@00");
      search_key.key_i = i;
      ptr_to_search_result = find_prep_rm__if_many_give_eps_smallest__sgt_ATOM(
        &search_key,
        &set_of_signals_used_in_main);
      h_ATOM_str_delete(search_key.key_s);

      if (ptr_to_search_result == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "An internal BBRL error occurred on line ");
        s_file_and_line_fprint(stderr, __FILE__, __LINE__);
        fprintf(stderr, ".\n");
        call_stack_fprint(stderr);
        clean_exit(-2);
      }

      if ( ptr_to_search_result->value_i != 0 ) {
        fprintf(stderr, "Error.\n");
        s_file_and_line_fprint(stderr, file, line);
        fprintf(stderr,
                " : Not all ZERO_TO_ZERO_ANCILLA are restored to zero "
                "at `END_DEFINITION_OF_MODULE'.\n");
        call_stack_fprint(stderr);
        clean_exit(-1);
      }

      ptr_to_detached_entry = remove_entry_from_sgt_ATOM();
      delete_ATOM(&(ptr_to_detached_entry->e.body));
      delete_unused_entry_for_sgt_ATOM(ptr_to_detached_entry);
    }

    for (i = oto; i < num_one_to_one_ancillas; i++) {
      h_ATOM_alloc_strcpy_hs(&(search_key.key_s), "@11");
      search_key.key_i = i;
      ptr_to_search_result = find_prep_rm__if_many_give_eps_smallest__sgt_ATOM(
        &search_key,
        &set_of_signals_used_in_main);
      h_ATOM_str_delete(search_key.key_s);

      if (ptr_to_search_result == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "An internal BBRL error occurred on line ");
        s_file_and_line_fprint(stderr, __FILE__, __LINE__);
        fprintf(stderr, ".\n");
        call_stack_fprint(stderr);
        clean_exit(-2);
      }

      if ( ptr_to_search_result->value_i != 1 ) {
        fprintf(stderr, "Error.\n");
        s_file_and_line_fprint(stderr, file, line);
        fprintf(stderr,
                " : Not all ONE_TO_ONE_ANCILLA are restored to one "
                "at `END_DEFINITION_OF_MODULE'.\n");
        call_stack_fprint(stderr);
        clean_exit(-1);
      }

      ptr_to_detached_entry = remove_entry_from_sgt_ATOM();
      delete_ATOM(&(ptr_to_detached_entry->e.body));
      delete_unused_entry_for_sgt_ATOM(ptr_to_detached_entry);
    }
  }
  /* ---  END  Check if all `ztz' and `oto' values are restored. --- */
  num_zero_to_zero_ancillas = ztz;
  num_one_to_one_ancillas = oto;

  if ( call_stack_pop_and_delete() ) {
    /* Do nothing. */
  } else {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " : Possibly duplicate use of `END_DEFINITION_OF_MODULE' "
            "keyword (on this line or in some other place).\n");
    clean_exit(-1);
  }

  if (size_of_stack_of_signal_maps > 0) {
    if ( stack_of_signal_maps__pop_and_delete() ) {
      /* Do nothing. */
    } else {
      fprintf(stderr, "Error.\n");
      s_file_and_line_fprint(stderr, file, line);
      fprintf(stderr,
              " : Possibly duplicate use of `END_DEFINITION_OF_MODULE' "
              " (on this line or in some other place of the code.\n");
      clean_exit(-1);
    }
  }

  #ifdef BBRL_DEBUG
  printf("After  END_DEFINITION_OF_MODULE on line ");
  s_file_and_line_fprint(stdout, file, line);
  printf(" c_st_size = ");
  uint64_fprint(stdout, size_of_call_stack);
  printf(" sm_st_size = ");
  uint64_fprint(stdout, size_of_stack_of_signal_maps);
  printf(".\n");
  #endif
}



void
stack_of_signal_maps__push_a_pair_of_empty_maps(
void) {
  if (size_of_stack_of_signal_maps >= MAX_STACK_SIZE) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "The maximum size of STACK_OF_SIGNAL_MAPS "
            "has been exceeded.\n");
    clean_exit(-1);
  }

  stack_of_signal_maps[size_of_stack_of_signal_maps].callee_to_main.
    ptr_to_root                 = NULL;
  stack_of_signal_maps[size_of_stack_of_signal_maps].callee_to_main.
    num_entries                 = 0;
  stack_of_signal_maps[size_of_stack_of_signal_maps].callee_to_main.
    max_size_since_full_rebuild = 0;

  stack_of_signal_maps[size_of_stack_of_signal_maps].caller_to_callee.
    ptr_to_root                 = NULL;
  stack_of_signal_maps[size_of_stack_of_signal_maps].caller_to_callee.
    num_entries                 = 0;
  stack_of_signal_maps[size_of_stack_of_signal_maps].caller_to_callee.
    max_size_since_full_rebuild = 0;

  size_of_stack_of_signal_maps++;
}



sosm_update_top_map__rv_t
stack_of_signal_maps__update_top_map__regular(
char * key_s, uint64_t key_i, char * value_s, uint64_t value_i) {
  ptr_to_entry_of_sgt_ATOM ptr_to_new_entry;
  ATOM                     search_key;
  ATOM *                   ptr_to_search_result;

  if (size_of_stack_of_signal_maps <= 0) {
    return STACK_IS_EMPTY_________________________________________SOSMUTMRV;
  } else

  if (size_of_stack_of_signal_maps == 1) {
    new_entry_for_sgt_ATOM(ptr_to_new_entry)
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.key_s)  ,   key_s);
    ptr_to_new_entry->e.body.key_i   = key_i;
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.value_s), value_s);
    ptr_to_new_entry->e.body.value_i = value_i;
    if (insert__if_conflicts_bounce__sgt_ATOM(
          ptr_to_new_entry,
          &(stack_of_signal_maps[size_of_stack_of_signal_maps - 1]
            .
            callee_to_main)
       )) {
      /* Do nothing. */
    } else {
      delete_ATOM(&(ptr_to_new_entry->e.body));
      delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
      return DUPLICATE_CALLEE_SIGNAL_NAME___________________________SOSMUTMRV;
    }

    new_entry_for_sgt_ATOM(ptr_to_new_entry)
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.key_s)  , value_s);
    ptr_to_new_entry->e.body.key_i   = value_i;
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.value_s),   key_s);
    ptr_to_new_entry->e.body.value_i = key_i;
    if (insert__if_conflicts_bounce__sgt_ATOM(
          ptr_to_new_entry,
          &(stack_of_signal_maps[size_of_stack_of_signal_maps - 1]
            .
            caller_to_callee)
       )) {
      /* Do nothing. */
    } else {
      delete_ATOM(&(ptr_to_new_entry->e.body));
      delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
      return DUPLICATE_CALLER_SIGNAL_NAME___________________________SOSMUTMRV;
    }
  } else

  {
    h_ATOM_alloc_strcpy_hs(&(search_key.key_s), value_s);
    search_key.key_i = value_i;
    search_key.value_s = NULL; /* For correct deletion. */
    find__if_many_give_eps_smallest__sgt_ATOM(
      ptr_to_search_result,
      search_key,
      stack_of_signal_maps[size_of_stack_of_signal_maps-2].callee_to_main)
    delete_ATOM(&search_key);
    if (ptr_to_search_result == NULL) {
      return CANT_MAKE_COMPOSITION_WITH_THE_PREV_CALLEE_TO_MAIN_MAP_SOSMUTMRV;
    }
    new_entry_for_sgt_ATOM(ptr_to_new_entry)
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.key_s)  ,   key_s);
    ptr_to_new_entry->e.body.key_i   = key_i;
    h_ATOM_alloc_strcpy_hh(
      &(ptr_to_new_entry->e.body.value_s),
      ptr_to_search_result->value_s);
    ptr_to_new_entry->e.body.value_i = ptr_to_search_result->value_i;
    if (insert__if_conflicts_bounce__sgt_ATOM(
          ptr_to_new_entry,
          &(stack_of_signal_maps[size_of_stack_of_signal_maps - 1]
            .
            callee_to_main)
       )) {
      /* Do nothing. */
    } else {
      delete_ATOM(&(ptr_to_new_entry->e.body));
      delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
      return DUPLICATE_CALLEE_SIGNAL_NAME___________________________SOSMUTMRV;
    }

    new_entry_for_sgt_ATOM(ptr_to_new_entry)
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.key_s)  , value_s);
    ptr_to_new_entry->e.body.key_i   = value_i;
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.value_s),   key_s);
    ptr_to_new_entry->e.body.value_i = key_i;
    if (insert__if_conflicts_bounce__sgt_ATOM(
          ptr_to_new_entry,
          &(stack_of_signal_maps[size_of_stack_of_signal_maps - 1]
            .
            caller_to_callee)
       )) {
      /* Do nothing. */
    } else {
      delete_ATOM(&(ptr_to_new_entry->e.body));
      delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
      return DUPLICATE_CALLER_SIGNAL_NAME___________________________SOSMUTMRV;
    }
  }

  return OK_____________________________________________________SOSMUTMRV;
}



sosm_update_top_map__rv_t
stack_of_signal_maps__update_top_map__ztgarb(
char * key_s, uint64_t key_i) {
  ptr_to_entry_of_sgt_ATOM ptr_to_new_entry;

  if (size_of_stack_of_signal_maps <= 0) {
    return STACK_IS_EMPTY_________________________________________SOSMUTMRV;
  } else {
    new_entry_for_sgt_ATOM(ptr_to_new_entry)
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.key_s)  , key_s);
    ptr_to_new_entry->e.body.key_i   = key_i;
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.value_s), "@0g");
    ptr_to_new_entry->e.body.value_i = num_zero_to_garbage_ancillas++;
    if (insert__if_conflicts_bounce__sgt_ATOM(
          ptr_to_new_entry,
          &(stack_of_signal_maps[size_of_stack_of_signal_maps - 1]
            .
            callee_to_main)
       )) {
      /* Do nothing. */
    } else {
      delete_ATOM(&(ptr_to_new_entry->e.body));
      delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
      return NAME_IS_ALREADY_MAPPED__CANT_BE_USED_FOR_ANCILLA_SIG___SOSMUTMRV;
    }
    if (sim_in_file != NULL) {
      new_entry_for_sgt_ATOM(ptr_to_new_entry)
      h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.key_s)  , "@0g");
      ptr_to_new_entry->e.body.key_i   = num_zero_to_garbage_ancillas - 1;
      ptr_to_new_entry->e.body.value_s = NULL;
      ptr_to_new_entry->e.body.value_i = 0;
      if ( insert__if_conflicts_bounce__sgt_ATOM(
             ptr_to_new_entry, &set_of_signals_used_in_main) ) {
        /* Do nothing. */
      } else {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "An internal BBRL error occurred on line ");
        s_file_and_line_fprint(stderr, __FILE__, __LINE__);
        fprintf(stderr, ".\n");
        delete_ATOM(&(ptr_to_new_entry->e.body));
        delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
        clean_exit(-2);
      }
    }
  }

  return OK_____________________________________________________SOSMUTMRV;
}



sosm_update_top_map__rv_t
stack_of_signal_maps__update_top_map__otgarb(
char * key_s, uint64_t key_i) {
  ptr_to_entry_of_sgt_ATOM ptr_to_new_entry;

  if (size_of_stack_of_signal_maps <= 0) {
    return STACK_IS_EMPTY_________________________________________SOSMUTMRV;
  } else {
    new_entry_for_sgt_ATOM(ptr_to_new_entry)
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.key_s)  , key_s);
    ptr_to_new_entry->e.body.key_i   = key_i;
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.value_s), "@1g");
    ptr_to_new_entry->e.body.value_i = num_one_to_garbage_ancillas++;
    if (insert__if_conflicts_bounce__sgt_ATOM(
          ptr_to_new_entry,
          &(stack_of_signal_maps[size_of_stack_of_signal_maps - 1]
            .
            callee_to_main)
       )) {
      /* Do nothing. */
    } else {
      delete_ATOM(&(ptr_to_new_entry->e.body));
      delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
      return NAME_IS_ALREADY_MAPPED__CANT_BE_USED_FOR_ANCILLA_SIG___SOSMUTMRV;
    }
    if (sim_in_file != NULL) {
      new_entry_for_sgt_ATOM(ptr_to_new_entry)
      h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.key_s)  , "@1g");
      ptr_to_new_entry->e.body.key_i   = num_one_to_garbage_ancillas - 1;
      ptr_to_new_entry->e.body.value_s = NULL;
      ptr_to_new_entry->e.body.value_i = 1;
      if ( insert__if_conflicts_bounce__sgt_ATOM(
             ptr_to_new_entry, &set_of_signals_used_in_main) ) {
        /* Do nothing. */
      } else {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "An internal BBRL error occurred on line ");
        s_file_and_line_fprint(stderr, __FILE__, __LINE__);
        fprintf(stderr, ".\n");
        delete_ATOM(&(ptr_to_new_entry->e.body));
        delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
        clean_exit(-2);
      }
    }
  }

  return OK_____________________________________________________SOSMUTMRV;
}



sosm_update_top_map__rv_t
stack_of_signal_maps__update_top_map__ztzero(
char * key_s, uint64_t key_i) {
  ptr_to_entry_of_sgt_ATOM ptr_to_new_entry;

  if (size_of_stack_of_signal_maps <= 0) {
    return STACK_IS_EMPTY_________________________________________SOSMUTMRV;
  } else {
    new_entry_for_sgt_ATOM(ptr_to_new_entry)
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.key_s)  , key_s);
    ptr_to_new_entry->e.body.key_i   = key_i;
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.value_s), "@00");
    ptr_to_new_entry->e.body.value_i = num_zero_to_zero_ancillas++;
    peak_num_zero_to_zero_ancillas = num_zero_to_zero_ancillas;
    if (insert__if_conflicts_bounce__sgt_ATOM(
          ptr_to_new_entry,
          &(stack_of_signal_maps[size_of_stack_of_signal_maps - 1]
            .
            callee_to_main)
       )) {
      /* Do nothing. */
    } else {
      delete_ATOM(&(ptr_to_new_entry->e.body));
      delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
      return NAME_IS_ALREADY_MAPPED__CANT_BE_USED_FOR_ANCILLA_SIG___SOSMUTMRV;
    }
    if (sim_in_file != NULL) {
      new_entry_for_sgt_ATOM(ptr_to_new_entry)
      h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.key_s)  , "@00");
      ptr_to_new_entry->e.body.key_i   = num_zero_to_zero_ancillas - 1;
      ptr_to_new_entry->e.body.value_s = NULL;
      ptr_to_new_entry->e.body.value_i = 0;
      if ( insert__if_conflicts_bounce__sgt_ATOM(
             ptr_to_new_entry, &set_of_signals_used_in_main) ) {
        /* Do nothing. */
      } else {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "An internal BBRL error occurred on line ");
        s_file_and_line_fprint(stderr, __FILE__, __LINE__);
        fprintf(stderr, ".\n");
        delete_ATOM(&(ptr_to_new_entry->e.body));
        delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
        clean_exit(-2);
      }
    }
  }

  return OK_____________________________________________________SOSMUTMRV;
}



sosm_update_top_map__rv_t
stack_of_signal_maps__update_top_map__otone(
char * key_s, uint64_t key_i) {
  ptr_to_entry_of_sgt_ATOM ptr_to_new_entry;

  if (size_of_stack_of_signal_maps <= 0) {
    return STACK_IS_EMPTY_________________________________________SOSMUTMRV;
  } else {
    new_entry_for_sgt_ATOM(ptr_to_new_entry)
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.key_s)  , key_s);
    ptr_to_new_entry->e.body.key_i   = key_i;
    h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.value_s), "@11");
    ptr_to_new_entry->e.body.value_i = num_one_to_one_ancillas++;
    peak_num_one_to_one_ancillas = num_one_to_one_ancillas;
    if (insert__if_conflicts_bounce__sgt_ATOM(
          ptr_to_new_entry,
          &(stack_of_signal_maps[size_of_stack_of_signal_maps - 1]
            .
            callee_to_main)
       )) {
      /* Do nothing. */
    } else {
      delete_ATOM(&(ptr_to_new_entry->e.body));
      delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
      return NAME_IS_ALREADY_MAPPED__CANT_BE_USED_FOR_ANCILLA_SIG___SOSMUTMRV;
    }
    if (sim_in_file != NULL) {
      new_entry_for_sgt_ATOM(ptr_to_new_entry)
      h_ATOM_alloc_strcpy_hs(&(ptr_to_new_entry->e.body.key_s)  , "@11");
      ptr_to_new_entry->e.body.key_i   = num_one_to_one_ancillas - 1;
      ptr_to_new_entry->e.body.value_s = NULL;
      ptr_to_new_entry->e.body.value_i = 1;
      if ( insert__if_conflicts_bounce__sgt_ATOM(
             ptr_to_new_entry, &set_of_signals_used_in_main) ) {
        /* Do nothing. */
      } else {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "An internal BBRL error occurred on line ");
        s_file_and_line_fprint(stderr, __FILE__, __LINE__);
        fprintf(stderr, ".\n");
        delete_ATOM(&(ptr_to_new_entry->e.body));
        delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
        clean_exit(-2);
      }
    }
  }

  return OK_____________________________________________________SOSMUTMRV;
}



int /* R.v.: (1 != 0) on success, (0 != 0) - stack is empty. */
stack_of_signal_maps__pop_and_delete(
void) {
  if (size_of_stack_of_signal_maps <= 0) return (0 != 0);

  delete_sgt_ATOM(&(stack_of_signal_maps[size_of_stack_of_signal_maps - 1]
                    .
                    callee_to_main));
  delete_sgt_ATOM(&(stack_of_signal_maps[size_of_stack_of_signal_maps - 1]
                    .
                    caller_to_callee));
  size_of_stack_of_signal_maps--;
  return (1 != 0);
}



int /* R.v.: (1 != 0) - success, (0 != 0) - failure. */
stack_of_signal_maps__fprint_global_name(
FILE * f, char * local_name_s, uint64_t local_name_i) {
  ATOM   search_key;
  ATOM * ptr_to_search_result;

  h_ATOM_alloc_strcpy_hs(&(search_key.key_s), local_name_s);
  search_key.key_i = local_name_i;

  if (size_of_stack_of_signal_maps <= 0) return (0 != 0);

  find__if_many_give_eps_smallest__sgt_ATOM(
    ptr_to_search_result,
    search_key,
    stack_of_signal_maps[size_of_stack_of_signal_maps - 1].callee_to_main)
  h_ATOM_str_delete(search_key.key_s);

  if (ptr_to_search_result == NULL) return (0 != 0);

  if (f != NULL) {
    h_signal_name_fprint(f,
                         ptr_to_search_result->value_s,
                         ptr_to_search_result->value_i);
  }

  return (1 != 0);
}



void
call_stack_push(
char *   instantiation_file,
uint64_t instantiation_line,
char *   body_file,
uint64_t body_line) {
  if (size_of_call_stack >= MAX_STACK_SIZE) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "The maximum size of CALL_STACK "
            "has been exceeded.\n");
    clean_exit(-1);
  }

  h_ATOM_alloc_strcpy_hs(
    &(call_stack[size_of_call_stack].value_s), instantiation_file);
  call_stack[size_of_call_stack].value_i = instantiation_line;
  h_ATOM_alloc_strcpy_hs(
    &(call_stack[size_of_call_stack].key_s), body_file);
  call_stack[size_of_call_stack].key_i = body_line;

  size_of_call_stack++;
}



int /* R.v.: (1 != 0) on success, (0 != 0) - stack is empty. */
call_stack_pop_and_delete(
void) {
  if (size_of_call_stack <= 0) return (0 != 0);
  delete_ATOM(&(call_stack[size_of_call_stack - 1]));
  size_of_call_stack--;
  return (1 != 0);
}



void
call_stack_fprint(
FILE * f) {
  uint64_t i = size_of_call_stack;

  fprintf(f, "Call stack:\n");
  while (i > 0) {
    i--;
    fprintf(f, "The module defined on line ");
    h_file_and_line_fprint(f, call_stack[i].key_s, call_stack[i].key_i);
    fprintf(f, " is instantiated on line ");
    h_file_and_line_fprint(f, call_stack[i].value_s, call_stack[i].value_i);
    fprintf(f, ".\n");
  }
}



void
set_of_signals_used_in_main__update(
char * sig_name, uint64_t sig_index) {
  ptr_to_entry_of_sgt_ATOM ptr_to_new_entry;

  new_entry_for_sgt_ATOM(ptr_to_new_entry)
  h_ATOM_alloc_strcpy_hs(
      &(ptr_to_new_entry->e.body.key_s), sig_name);
  ptr_to_new_entry->e.body.key_i = sig_index;
  ptr_to_new_entry->e.body.value_s = NULL;
  ptr_to_new_entry->e.body.value_i = ni;
  if ( insert__if_conflicts_bounce__sgt_ATOM(
         ptr_to_new_entry, &set_of_signals_used_in_main) ) {
    /* If in simulation mode report error. */
    if (sim_in_file != NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Signal `");
      h_signal_name_fprint(stderr,
                           ptr_to_new_entry->e.body.key_s,
                           ptr_to_new_entry->e.body.key_i);
      fprintf(stderr,
              "' is missing in the input file for simulation.\n");
      clean_exit(-1);
    }
  } else {
    delete_ATOM(&(ptr_to_new_entry->e.body));
    delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
  }
}



uint64_t *
get_ptr_to_value__in_set_of_signals_used_in_main(
char * local_name_s, uint64_t local_name_i) {
  ATOM   search_key;
  ATOM * ptr_to_search_result;

  #ifdef BBRL_DEBUG
  printf("Searching for %s[%d].\n", local_name_s, (int) local_name_i);
  #endif

  h_ATOM_alloc_strcpy_hs(&(search_key.key_s), local_name_s);
  search_key.key_i = local_name_i;

  if (size_of_stack_of_signal_maps <= 0) return NULL;

  find__if_many_give_eps_smallest__sgt_ATOM(
    ptr_to_search_result,
    search_key,
    stack_of_signal_maps[size_of_stack_of_signal_maps - 1].callee_to_main)
  h_ATOM_str_delete(search_key.key_s);

  if (ptr_to_search_result == NULL) return NULL;

  #ifdef BBRL_DEBUG
  printf("Found : ");
  h_ATOM_printf_str(ptr_to_search_result->value_s, 0, ' ', 'l');
  printf("[%d]\n.", (int) ptr_to_search_result->value_i);
  #endif

  h_ATOM_alloc_strcpy_hh(
    &(search_key.key_s),
    ptr_to_search_result->value_s);
  search_key.key_i = ptr_to_search_result->value_i;
  #ifdef BBRL_DEBUG
  printf("Searching2 : ");
  h_ATOM_printf_str(search_key.key_s, 0, ' ', 'l');
  printf("[%d]\n.", (int) search_key.key_i);
  #endif
  find__if_many_give_eps_smallest__sgt_ATOM(
    ptr_to_search_result,
    search_key,
    set_of_signals_used_in_main)
  h_ATOM_str_delete(search_key.key_s);

  if (ptr_to_search_result == NULL) return NULL;

  #ifdef BBRL_DEBUG
  printf("Found2 : ");
  h_ATOM_printf_str(ptr_to_search_result->key_s, 0, ' ', 'l');
  printf("[%d]\n.", (int) ptr_to_search_result->key_i);
  #endif

  return &(ptr_to_search_result->value_i);
}



void
create_signals_file(
void) {
  ptr_to_entry_of_sgt_ATOM ptr_to_detached_entry;

  while (is_nonempty_sgt_ATOM(set_of_signals_used_in_main)) {
    find_prep_rm__smallest__sgt_ATOM(&set_of_signals_used_in_main);
    ptr_to_detached_entry = remove_entry_from_sgt_ATOM();
    if (out_signals_file != NULL) {
      fprintf(out_signals_file, ". ");
      h_signal_name_fprint_with_spaces_tilda_and_eol(
        out_signals_file,
        ptr_to_detached_entry->e.body.key_s,
        ptr_to_detached_entry->e.body.key_i);
    }
    delete_ATOM(&(ptr_to_detached_entry->e.body));
    delete_unused_entry_for_sgt_ATOM(ptr_to_detached_entry);
  }
}



void
create_sim_out_file(
void) {
  ptr_to_entry_of_sgt_ATOM ptr_to_detached_entry;

  while (is_nonempty_sgt_ATOM(set_of_signals_used_in_main)) {
    find_prep_rm__smallest__sgt_ATOM(&set_of_signals_used_in_main);
    ptr_to_detached_entry = remove_entry_from_sgt_ATOM();
    if (sim_out_file != NULL) {
      if ( (h_ATOM_strcmp_hs(
              ptr_to_detached_entry->e.body.key_s, "@0g") != 0)
           &&
           (h_ATOM_strcmp_hs(
              ptr_to_detached_entry->e.body.key_s, "@1g") != 0)
           &&
           (h_ATOM_strcmp_hs(
              ptr_to_detached_entry->e.body.key_s, "@00") != 0)
           &&
           (h_ATOM_strcmp_hs(
              ptr_to_detached_entry->e.body.key_s, "@11") != 0) ) {
        fprintf(sim_out_file,
                "%c ",
                (ptr_to_detached_entry->e.body.value_i == 0) ? '0' : '1');
        h_signal_name_fprint_with_spaces_tilda_and_eol(
          sim_out_file,
          ptr_to_detached_entry->e.body.key_s,
          ptr_to_detached_entry->e.body.key_i);
      }
    }
    delete_ATOM(&(ptr_to_detached_entry->e.body));
    delete_unused_entry_for_sgt_ATOM(ptr_to_detached_entry);
  }
}



void
read_sim_in_file__to__set_of_signals_used_in_main(
void) {
  ptr_to_entry_of_sgt_ATOM ptr_to_new_entry;
  char                     value;
  char                     aux_char;
  int                      signal_type;
  uint64_t                 num_indexes;
  uint64_t *               ranges;
  uint64_t                 i;
  uint64_t                 j;
  uint64_t                 k;
  uint64_t                 l;

  while (fscanf(sim_in_file, "%c", &value) == 1) {
    while ( (value == ' ') || (value == '\n') ) {
      if (fscanf(sim_in_file, "%c", &value) != 1) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Input file for simulation is corrupted. (10)\n");
        clean_exit(-1);
      }
    }
    new_entry_for_sgt_ATOM(ptr_to_new_entry)

    if (h_ATOM_alloc_fscan_str(
          sim_in_file,
          &(ptr_to_new_entry->e.body.key_s))) {
      /* Do nothing. */
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Input file for simulation is corrupted: "
              "a variable name is missing.\n");
      delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
      clean_exit(-1);
    }

    if ( search_main_var_decl_table(
           &signal_type,
           &num_indexes,
           &ranges,
           ptr_to_new_entry->e.body.key_s) ) {
      k = 1;
      for (j = 0; j < num_indexes; j++) {
        k *= ranges[j];
      }
      i = 0;
      for (j = 0; j < num_indexes; j++) {
        k = k / ranges[j];
        if ( (fscanf(sim_in_file, "%c", &aux_char) != 1)
             ||
             (aux_char != '[') ) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Input file for simulation is corrupted (6).\n");
          h_ATOM_str_delete(ptr_to_new_entry->e.body.key_s);
          delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
          clean_exit(-1);
        }
        l = 0;
        aux_char = '0';
        while ( (aux_char <= '9') && (aux_char >= '0') ) {
          l = l * 10 + ((uint64_t) (aux_char - '0'));
          if (fscanf(sim_in_file, "%c", &aux_char) != 1) {
            fprintf(stderr, "Error.\n");
            fprintf(stderr,
                    "Input file for simulation is corrupted. (7)\n");
            h_ATOM_str_delete(ptr_to_new_entry->e.body.key_s);
            delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
            clean_exit(-1);
          }
        }
        i = i + k * l;
        if ( aux_char != ']' ) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Input file for simulation is corrupted (8).\n");
          h_ATOM_str_delete(ptr_to_new_entry->e.body.key_s);
          delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
          clean_exit(-1);
        }
        if ( (fscanf(sim_in_file, "%c", &aux_char) != 1)
             ||
             (aux_char != ' ') ) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Input file for simulation is corrupted (9).\n");
          h_ATOM_str_delete(ptr_to_new_entry->e.body.key_s);
          delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
          clean_exit(-1);
        }
      }
      fscanf(sim_in_file, "%c", &aux_char);
      fscanf(sim_in_file, "%c", &aux_char);
      ptr_to_new_entry->e.body.key_i = i;
    } else {
      if (fscanf(sim_in_file, "%c", &aux_char) != 1) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Input file for simulation is corrupted.\n");
        h_ATOM_str_delete(ptr_to_new_entry->e.body.key_s);
        delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
        clean_exit(-1);
      }
      if (aux_char == '[') {
        fscanf(sim_in_file, "%c", &aux_char);
        ptr_to_new_entry->e.body.key_i = 0;
        aux_char = '0';
        while ( (aux_char <= '9') && (aux_char >= '0') ) {
          ptr_to_new_entry->e.body.key_i =
            ptr_to_new_entry->e.body.key_i * 10 + ((uint64_t) (aux_char - '0'));
          if (fscanf(sim_in_file, "%c", &aux_char) != 1) {
            fprintf(stderr, "Error.\n");
            fprintf(stderr,
                    "Input file for simulation is corrupted. (3)\n");
            h_ATOM_str_delete(ptr_to_new_entry->e.body.key_s);
            delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
            clean_exit(-1);
          }
        }
        fscanf(sim_in_file, "%c", &aux_char);
        fscanf(sim_in_file, "%c", &aux_char);
        fscanf(sim_in_file, "%c", &aux_char);
        fscanf(sim_in_file, "%c", &aux_char);
      } else if (aux_char == '~') {
        ptr_to_new_entry->e.body.key_i = ni;
        fscanf(sim_in_file, "%c", &aux_char);
      } else {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Input file for simulation is corrupted. (4)\n");
        h_ATOM_str_delete(ptr_to_new_entry->e.body.key_s);
        delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
        clean_exit(-1);
      }
    }

    ptr_to_new_entry->e.body.value_s = NULL;
    if (value == '0') {
      ptr_to_new_entry->e.body.value_i = 0;
    } else if (value == '1') {
      ptr_to_new_entry->e.body.value_i = 1;
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Input file for simulation : "
              "Each line must start from either `0' or `1'.\n");
      h_ATOM_str_delete(ptr_to_new_entry->e.body.key_s);
      delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
      clean_exit(-1);
    }

    if ( insert__if_conflicts_bounce__sgt_ATOM(
           ptr_to_new_entry, &set_of_signals_used_in_main) ) {
      /* The insertion is successful. Do nothing. */
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Input file for simulation contains a duplicate signal name.\n");
      h_ATOM_str_delete(ptr_to_new_entry->e.body.key_s);
      delete_unused_entry_for_sgt_ATOM(ptr_to_new_entry)
      clean_exit(-1);
    }
  }
}



void
uint64_fprint(
FILE * f, uint64_t i) {
  static uint64_t t[58] = {
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)  100000) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)   50000) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)   20000) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)   10000) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)    5000) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)    2000) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)    1000) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)     500) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)     200) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)     100) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)      50) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)      20) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)      10) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       5) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       2) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       1) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)5000000) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)2000000) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)1000000) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t) 500000) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t) 200000) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t) 100000) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)  50000) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)  20000) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)  10000) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)   5000) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)   2000) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)   1000) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)    500) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)    200) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)    100) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)     50) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)     20) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)     10) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      5) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      2) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      1) + ((uint64_t)      0),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)5000000),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)2000000),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)1000000),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t) 500000),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t) 200000),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t) 100000),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)  50000),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)  20000),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)  10000),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)   5000),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)   2000),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)   1000),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)    500),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)    200),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)    100),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)     50),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)     20),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)     10),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      5),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      2),
    ((uint64_t)10000000)*((uint64_t)10000000)*((uint64_t)       0) +
    ((uint64_t)10000000)*((uint64_t)      0) + ((uint64_t)      1)
  };

  char   s[21];
  char * p;
  strcpy(s, "00000000000000000000");

  if (i >= t[ 0]) {i -= t[ 0]; s[ 0] += 1;}

  if (i >= t[ 1]) {i -= t[ 1]; s[ 1] += 5;}
  if (i >= t[ 2]) {i -= t[ 2]; s[ 1] += 2;}
  if (i >= t[ 2]) {i -= t[ 2]; s[ 1] += 2;}
  if (i >= t[ 3]) {i -= t[ 3]; s[ 1] += 1;}

  if (i >= t[ 4]) {i -= t[ 4]; s[ 2] += 5;}
  if (i >= t[ 5]) {i -= t[ 5]; s[ 2] += 2;}
  if (i >= t[ 5]) {i -= t[ 5]; s[ 2] += 2;}
  if (i >= t[ 6]) {i -= t[ 6]; s[ 2] += 1;}

  if (i >= t[ 7]) {i -= t[ 7]; s[ 3] += 5;}
  if (i >= t[ 8]) {i -= t[ 8]; s[ 3] += 2;}
  if (i >= t[ 8]) {i -= t[ 8]; s[ 3] += 2;}
  if (i >= t[ 9]) {i -= t[ 9]; s[ 3] += 1;}

  if (i >= t[10]) {i -= t[10]; s[ 4] += 5;}
  if (i >= t[11]) {i -= t[11]; s[ 4] += 2;}
  if (i >= t[11]) {i -= t[11]; s[ 4] += 2;}
  if (i >= t[12]) {i -= t[12]; s[ 4] += 1;}

  if (i >= t[13]) {i -= t[13]; s[ 5] += 5;}
  if (i >= t[14]) {i -= t[14]; s[ 5] += 2;}
  if (i >= t[14]) {i -= t[14]; s[ 5] += 2;}
  if (i >= t[15]) {i -= t[15]; s[ 5] += 1;}

  if (i >= t[16]) {i -= t[16]; s[ 6] += 5;}
  if (i >= t[17]) {i -= t[17]; s[ 6] += 2;}
  if (i >= t[17]) {i -= t[17]; s[ 6] += 2;}
  if (i >= t[18]) {i -= t[18]; s[ 6] += 1;}

  if (i >= t[19]) {i -= t[19]; s[ 7] += 5;}
  if (i >= t[20]) {i -= t[20]; s[ 7] += 2;}
  if (i >= t[20]) {i -= t[20]; s[ 7] += 2;}
  if (i >= t[21]) {i -= t[21]; s[ 7] += 1;}

  if (i >= t[22]) {i -= t[22]; s[ 8] += 5;}
  if (i >= t[23]) {i -= t[23]; s[ 8] += 2;}
  if (i >= t[23]) {i -= t[23]; s[ 8] += 2;}
  if (i >= t[24]) {i -= t[24]; s[ 8] += 1;}

  if (i >= t[25]) {i -= t[25]; s[ 9] += 5;}
  if (i >= t[26]) {i -= t[26]; s[ 9] += 2;}
  if (i >= t[26]) {i -= t[26]; s[ 9] += 2;}
  if (i >= t[27]) {i -= t[27]; s[ 9] += 1;}

  if (i >= t[28]) {i -= t[28]; s[10] += 5;}
  if (i >= t[29]) {i -= t[29]; s[10] += 2;}
  if (i >= t[29]) {i -= t[29]; s[10] += 2;}
  if (i >= t[30]) {i -= t[30]; s[10] += 1;}

  if (i >= t[31]) {i -= t[31]; s[11] += 5;}
  if (i >= t[32]) {i -= t[32]; s[11] += 2;}
  if (i >= t[32]) {i -= t[32]; s[11] += 2;}
  if (i >= t[33]) {i -= t[33]; s[11] += 1;}

  if (i >= t[34]) {i -= t[34]; s[12] += 5;}
  if (i >= t[35]) {i -= t[35]; s[12] += 2;}
  if (i >= t[35]) {i -= t[35]; s[12] += 2;}
  if (i >= t[36]) {i -= t[36]; s[12] += 1;}

  if (i >= t[37]) {i -= t[37]; s[13] += 5;}
  if (i >= t[38]) {i -= t[38]; s[13] += 2;}
  if (i >= t[38]) {i -= t[38]; s[13] += 2;}
  if (i >= t[39]) {i -= t[39]; s[13] += 1;}

  if (i >= t[40]) {i -= t[40]; s[14] += 5;}
  if (i >= t[41]) {i -= t[41]; s[14] += 2;}
  if (i >= t[41]) {i -= t[41]; s[14] += 2;}
  if (i >= t[42]) {i -= t[42]; s[14] += 1;}

  if (i >= t[43]) {i -= t[43]; s[15] += 5;}
  if (i >= t[44]) {i -= t[44]; s[15] += 2;}
  if (i >= t[44]) {i -= t[44]; s[15] += 2;}
  if (i >= t[45]) {i -= t[45]; s[15] += 1;}

  if (i >= t[46]) {i -= t[46]; s[16] += 5;}
  if (i >= t[47]) {i -= t[47]; s[16] += 2;}
  if (i >= t[47]) {i -= t[47]; s[16] += 2;}
  if (i >= t[48]) {i -= t[48]; s[16] += 1;}

  if (i >= t[49]) {i -= t[49]; s[17] += 5;}
  if (i >= t[50]) {i -= t[50]; s[17] += 2;}
  if (i >= t[50]) {i -= t[50]; s[17] += 2;}
  if (i >= t[51]) {i -= t[51]; s[17] += 1;}

  if (i >= t[52]) {i -= t[52]; s[18] += 5;}
  if (i >= t[53]) {i -= t[53]; s[18] += 2;}
  if (i >= t[53]) {i -= t[53]; s[18] += 2;}
  if (i >= t[54]) {i -= t[54]; s[18] += 1;}

  if (i >= t[55]) {i -= t[55]; s[19] += 5;}
  if (i >= t[56]) {i -= t[56]; s[19] += 2;}
  if (i >= t[56]) {i -= t[56]; s[19] += 2;}
  if (i >= t[57]) {i -= t[57]; s[19] += 1;}

  for (p = s; *p == '0'; p++) /* Do nothing just move the pointer */ ;
  if (*p == '\0') p--;

  fprintf(f, "%s", p);
}



void
h_file_and_line_fprint(
FILE * f, str_on_heap_ATOM file, uint64_t line) {
  fprintf(f, "<");
  h_ATOM_fprintf_str(f, file, 0, ' ', 'l');
  fprintf(f, ":");
  uint64_fprint(f, line);
  fprintf(f, ">");
}



void
s_file_and_line_fprint(
FILE * f, char * file, uint64_t line) {
  fprintf(f, "<%s:", file);
  uint64_fprint(f, line);
  fprintf(f, ">");
}



void
h_signal_name_fprint(
FILE * f, str_on_heap_ATOM s, uint64_t i) {
  int        signal_type;
  uint64_t   num_indexes;
  uint64_t * ranges;
  uint64_t   j;
  uint64_t   k;
  uint64_t   l;

  h_ATOM_fprintf_str(f, s, 0, ' ', 'l');
  if (i != ni) {
    if ( search_main_var_decl_table(
           &signal_type, &num_indexes, &ranges, s) ) {
      k = 1;
      for (j = 0; j < num_indexes; j++) {
        k *= ranges[j];
      }
      for (j = 0; j < num_indexes; j++) {
        k = k / ranges[j];
        fprintf(f, "[");
        l = i / k;
        uint64_fprint(f, l);
        fprintf(f, "]");
        i = i - l * k;
      }
    } else {
      fprintf(f, "[");
      uint64_fprint(f, i);
      fprintf(f, "]");
    }
  }
}



void
h_signal_name_fprint_with_spaces_tilda_and_eol(
FILE * f, str_on_heap_ATOM s, uint64_t i) {
  int        signal_type;
  uint64_t   num_indexes;
  uint64_t * ranges;
  uint64_t   j;
  uint64_t   k;
  uint64_t   l;

  h_ATOM_fprintf_str(f, s, 0, ' ', 'l');
  if (i != ni) {
    if ( search_main_var_decl_table(
           &signal_type, &num_indexes, &ranges, s) ) {
      k = 1;
      for (j = 0; j < num_indexes; j++) {
        k *= ranges[j];
      }
      for (j = 0; j < num_indexes; j++) {
        k = k / ranges[j];
        fprintf(f, " [");
        l = i / k;
        uint64_fprint(f, l);
        fprintf(f, "]");
        i = i - l * k;
      }
    } else {
      fprintf(f, " [ ");
      uint64_fprint(f, i);
      fprintf(f, " ]");
    }
  }
  fprintf(f, " ~\n");
}



void
s_signal_name_fprint(
FILE * f, char * s, uint64_t i) {
  if (i != ni) {
    fprintf(f, "%s[[", s);
    uint64_fprint(f, i);
    fprintf(f, "]]");
  } else {
    fprintf(f, "%s", s);
  }
}



void
statistics_fprint(
FILE * f) {
  uint64_t i;

  fprintf(f, "Gate count:\n");

  for (i = 0; i < 64; i++) {
    if (statistics.n_control_toffoli[i] != 0) {
      if (i == 0) fprintf(f, "         NOT = "); else
      if (i == 1) fprintf(f, "        CNOT = "); else
      if (i == 2) fprintf(f, "     TOFFOLI = "); else
      fprintf(f, "%2d-c TOFFOLI = ", (int) i);

      uint64_fprint(f, statistics.n_control_toffoli[i]);
      fprintf(f, "\n");
    }
  }

  if (statistics.toffoli_with_64_or_more_controls != 0) {
    fprintf(f, "64 or more control TOFFOLI : ");
    uint64_fprint(f, statistics.toffoli_with_64_or_more_controls);
    fprintf(f, ",\n");
  }

  fprintf(f, "Number of const-to-garbage ancilla signals : ");
  uint64_fprint(f,
    num_zero_to_garbage_ancillas +
    num_one_to_garbage_ancillas);
  fprintf(f, ".\n");
  fprintf(f, "Number of reusable ancilla signals : ");
  uint64_fprint(f,
    peak_num_zero_to_zero_ancillas +
    peak_num_one_to_one_ancillas);
  fprintf(f, ".\n");
}



void
create_ancilla_file_for_qasm(
void) {
  fprintf(ancilla_file, "#_of_00_ancilla ");
  uint64_fprint(ancilla_file, peak_num_zero_to_zero_ancillas);
  fprintf(ancilla_file, "\n");
  fprintf(ancilla_file, "#_of_11_ancilla ");
  uint64_fprint(ancilla_file, peak_num_one_to_one_ancillas);
  fprintf(ancilla_file, "\n");
  fprintf(ancilla_file, "#_of_0g_ancilla ");
  uint64_fprint(ancilla_file, num_zero_to_garbage_ancillas);
  fprintf(ancilla_file, "\n");
  fprintf(ancilla_file, "#_of_1g_ancilla ");
  uint64_fprint(ancilla_file, num_one_to_garbage_ancillas);
  fprintf(ancilla_file, "\n");
}



void
delete_ATOM(
ATOM * arg) {
  if (arg->key_s   != NULL) {
    h_ATOM_str_delete(arg->key_s);
    arg->key_s   = NULL;
  }
  if (arg->value_s != NULL) {
    h_ATOM_str_delete(arg->value_s);
    arg->value_s = NULL;
  }
}



void
delete_sgt_ATOM(
sgt_ATOM * arg) {
  ptr_to_entry_of_sgt_ATOM ptr_to_detached_entry;

  while (is_nonempty_sgt_ATOM(*arg)) {
    find_prep_rm__smallest__sgt_ATOM(arg);
    ptr_to_detached_entry = remove_entry_from_sgt_ATOM();
    delete_ATOM(&(ptr_to_detached_entry->e.body));
    delete_unused_entry_for_sgt_ATOM(ptr_to_detached_entry);
  }
}



void
clean_exit(
int status) {
  uint64_t i;
  iter_sdll_ATOM iter;

  if (out_file != NULL) {
    fclose(out_file);
    out_file = NULL;
  }

  if (out_signals_file != NULL) {
    fclose(out_signals_file);
    out_signals_file = NULL;
  }

  if (ancilla_file != NULL) {
    fclose(ancilla_file);
    ancilla_file = NULL;
  }

  if (sim_in_file != NULL) {
    fclose(sim_in_file);
    sim_in_file = NULL;
  }

  if (sim_out_file != NULL) {
    fclose(sim_out_file);
    sim_out_file = NULL;
  }

  while (size_of_stack_of_signal_maps > 0) {
    stack_of_signal_maps__pop_and_delete();
  }

  while (size_of_call_stack > 0) {
    call_stack_pop_and_delete();
  }

  delete_sgt_ATOM(&set_of_signals_used_in_main);
  delete_sgt_ATOM(&name_to_index_in_main_var_decl_table);

  for (i = 0; i < size_of_stack_for_mem_cleaning_only; i++) {
    while (is_nonempty_sdll_ATOM(*(stack_for_mem_cleaning_only[i]))) {
      set_iter_to_rightmost_el_sdll_ATOM(
        iter, *(stack_for_mem_cleaning_only[i]) )
      /* `iter->e.body.key_s' and `iter->e.body.value_s'           */
      /* are pointers to constant strings, they do not own memory. */
      delete_and_iter_becomes_invalid_sdll_ATOM(
        iter, *(stack_for_mem_cleaning_only[i]) )
    }
  }

  if (status != 0) memory_manager_message_level = 0;
  release_memory();

  exit(status);
}



void
not(
char * file, uint64_t line, uint64_t num_control_signals) {
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t value;

  /* Signals:
       x[0];
  */

  statistics.total_num_gates++;
  if (num_control_signals < 64) {
    statistics.n_control_toffoli[num_control_signals]++;
  } else {
    statistics.toffoli_with_64_or_more_controls++;
  }

  /* --- BEGIN output gate --- */
  if (out_file != NULL) fprintf(out_file, "TOF ");
  if (out_file != NULL) fprintf(out_file, "%u   ",
                          (uint32_t) num_control_signals);
  for (i = 0; i < num_control_signals; i++) {
    /* Note: `stack_of_signal_maps__fprint_global_name' must always */
    /* be called, even if not producing any output.                 */
    if ( stack_of_signal_maps__fprint_global_name(
           out_file, "$", i) ) {
      if (out_file != NULL) fprintf(out_file, " ");
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr, "Before instantiation of `not' on line ");
      s_file_and_line_fprint(stderr, file, line);
      fprintf(stderr, " : Control signal `");
      s_signal_name_fprint(stderr, "$", i);
      fprintf(stderr, "' of `not' is not mapped.\n");
      call_stack_fprint(stderr);
      clean_exit(-1);
    }
  }
  if (out_file != NULL) fprintf(out_file, "  ");
  if ( stack_of_signal_maps__fprint_global_name(out_file, "x", 0) ) {
    /* Do nothing. */
  } else {
    fprintf(stderr, "Error.\n");
    fprintf(stderr, "Before instantiation of `not' on line ");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr, " : Signal `x' of `not' is not mapped.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }
  if (out_file != NULL) fprintf(out_file, "\n");
  /* ---  END  output gate --- */

  if (sim_in_file != NULL) {
    value = 1;
    for (i = 0; i < num_control_signals; i++) {
      value &= *(get_ptr_to_value__in_set_of_signals_used_in_main("$", i));
    }
    *(get_ptr_to_value__in_set_of_signals_used_in_main("x", 0)) ^= value;
  }

  END_DEFINITION_OF_MODULE
}



void
cnot(
char * file, uint64_t line, uint64_t num_control_signals) {
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t value;

  /* Signals:
       c;
       x;
  */

  statistics.total_num_gates++;
  if (num_control_signals + 1 < 64) {
    statistics.n_control_toffoli[num_control_signals + 1]++;
  } else {
    statistics.toffoli_with_64_or_more_controls++;
  }

  /* --- BEGIN output gate --- */
  if (out_file != NULL) fprintf(out_file, "TOF ");
  if (out_file != NULL) fprintf(out_file, "%u   ",
                          (uint32_t) num_control_signals + 1);
  for (i = 0; i < num_control_signals; i++) {
    /* Note: `stack_of_signal_maps__fprint_global_name' must always */
    /* be called, even if not producing any output.                 */
    if ( stack_of_signal_maps__fprint_global_name(
           out_file, "$", i) ) {
      if (out_file != NULL) fprintf(out_file, " ");
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr, "Before instantiation of `not' on line ");
      s_file_and_line_fprint(stderr, file, line);
      fprintf(stderr, " : Control signal `");
      s_signal_name_fprint(stderr, "$", i);
      fprintf(stderr, "' of `cnot' is not mapped.\n");
      call_stack_fprint(stderr);
      clean_exit(-1);
    }
  }
  if ( stack_of_signal_maps__fprint_global_name(out_file, "c", 0) ) {
    if (out_file != NULL) fprintf(out_file, " ");
  } else {
    fprintf(stderr, "Error.\n");
    fprintf(stderr, "Before instantiation of `cnot' on line ");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr, " : Signal `c' of `cnot' is not mapped.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }
  if (out_file != NULL) fprintf(out_file, "  ");
  if ( stack_of_signal_maps__fprint_global_name(out_file, "x", 0) ) {
    /* Do nothing. */
  } else {
    fprintf(stderr, "Error.\n");
    fprintf(stderr, "Before instantiation of `not' on line ");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr, " : Signal `x' of `cnot' is not mapped.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }
  if (out_file != NULL) fprintf(out_file, "\n");
  /* ---  END  output gate --- */

  if (sim_in_file != NULL) {
    value = 1;
    value &= *(get_ptr_to_value__in_set_of_signals_used_in_main("c", 0));
    for (i = 0; i < num_control_signals; i++) {
      value &= *(get_ptr_to_value__in_set_of_signals_used_in_main("$", i));
    }
    *(get_ptr_to_value__in_set_of_signals_used_in_main("x", 0)) ^= value;
  }

  END_DEFINITION_OF_MODULE
}



void
toffoli(
char * file, uint64_t line, uint64_t num_control_signals) {
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t value;

  /* Signals:
       c1;
       c2;
       x;
  */

  statistics.total_num_gates++;
  if (num_control_signals + 2 < 64) {
    statistics.n_control_toffoli[num_control_signals + 2]++;
  } else {
    statistics.toffoli_with_64_or_more_controls++;
  }

  /* --- BEGIN output gate --- */
  if (out_file != NULL) fprintf(out_file, "TOF ");
  if (out_file != NULL) fprintf(out_file, "%u   ",
                          (uint32_t) num_control_signals + 2);
  for (i = 0; i < num_control_signals; i++) {
    /* Note: `stack_of_signal_maps__fprint_global_name' must always */
    /* be called, even if not producing any output.                 */
    if ( stack_of_signal_maps__fprint_global_name(
           out_file, "$", i) ) {
      if (out_file != NULL) fprintf(out_file, " ");
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr, "Before instantiation of `not' on line ");
      s_file_and_line_fprint(stderr, file, line);
      fprintf(stderr, " : Control signal `");
      s_signal_name_fprint(stderr, "$", i);
      fprintf(stderr, "' of `toffoli' is not mapped.\n");
      call_stack_fprint(stderr);
      clean_exit(-1);
    }
  }
  if ( stack_of_signal_maps__fprint_global_name(out_file, "c1", 0) ) {
    if (out_file != NULL) fprintf(out_file, " ");
  } else {
    fprintf(stderr, "Error.\n");
    fprintf(stderr, "Before instantiation of `toffoli' on line ");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr, " : Signal `c1' of `toffoli' is not mapped.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }
  if ( stack_of_signal_maps__fprint_global_name(out_file, "c2", 0) ) {
    if (out_file != NULL) fprintf(out_file, " ");
  } else {
    fprintf(stderr, "Error.\n");
    fprintf(stderr, "Before instantiation of `toffoli' on line ");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr, " : Signal `c2' of `toffoli' is not mapped.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }
  if (out_file != NULL) fprintf(out_file, "  ");
  if ( stack_of_signal_maps__fprint_global_name(out_file, "x", 0) ) {
    /* Do nothing. */
  } else {
    fprintf(stderr, "Error.\n");
    fprintf(stderr, "Before instantiation of `toffoli' on line ");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr, " : Signal `x' of `toffoli' is not mapped.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }
  if (out_file != NULL) fprintf(out_file, "\n");
  /* ---  END  output gate --- */

  if (sim_in_file != NULL) {
    value = 1;
    value &= *(get_ptr_to_value__in_set_of_signals_used_in_main("c1", 0));
    value &= *(get_ptr_to_value__in_set_of_signals_used_in_main("c2", 0));
    for (i = 0; i < num_control_signals; i++) {
      value &= *(get_ptr_to_value__in_set_of_signals_used_in_main("$", i));
    }
    *(get_ptr_to_value__in_set_of_signals_used_in_main("x", 0)) ^= value;
  }

  END_DEFINITION_OF_MODULE
}



/* ----------------- BEGIN `bbrl_lib_integer_arithmetic_.c' ----------------- */

void
a_swap_b(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
  */

  /* Make sure n != 0. */
  if (n == 0) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module a_swap_b(...)' : "
            "The number of bits must be >= 1.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  for (i = 0; i < n; i++) {
    YqCNOT("a", i,   "b", i)
    YqCNOT("b", i,   "a", i)
    YqCNOT("a", i,   "b", i)
  }

  END_DEFINITION_OF_MODULE
}



void
assign_value_of_b_to_a(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
  */

  /* Make sure n != 0. */
  if (n == 0) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module assign_value_of_b_to_a(...)' : "
            "The number of bits must be >= 1.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  for (i = 0; i < n; i++) {
    ZERO_TO_GARBAGE_ANCILLA("yQassignztz", i)
    YqCNOT("a", i,   "yQassignztz", i)
    YqCNOT("yQassignztz", i,   "a", i)
    YqCNOT("b", i,   "a", i)
  }

  END_DEFINITION_OF_MODULE
}



void
a__eq__a_plus_b(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
  */

  /* Make sure n != 0. */
  if (n == 0) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module a__eq__a_plus_b(...)' : "
            "The number of bits must be >= 1.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  for (i = n - 1; i > 0; i--) {
    YqCNOT("a", i,       "b", i)
    YqCNOT("a", i - 1,   "a", i)
  }

  YqCNOT("a", 0,   "b", 0)

  for (i = 1; i <= n - 1; i++) {
    YqTOFFOLI("b", i - 1,   "a", i - 1,   "a", i)
  }

  for (i = n - 1; i > 0; i--) {
    YqTOFFOLI("b", i - 1,   "a", i - 1,   "b", i)
    YqCNOT("b", i - 1,   "b", i)
  }

  YqCNOT("a", 0,   "b", 0)
  YqCNOT("b", 0,   "a", 0)

  for (i = 1; i <= n - 1; i++) {
    YqCNOT("b", i - 1,   "b", i)
    YqCNOT("a", i,       "b", i)
    YqCNOT("b", i,       "a", i)
  }

  END_DEFINITION_OF_MODULE
}



void
a__eq__a_minus_b(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
  */

  /* Make sure n != 0. */
  if (n == 0) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module a__eq__a_minus_b(...)' : "
            "The number of bits must be >= 1.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  for (i = n - 1; i > 0; i--) {
    YqCNOT("b", i,       "a", i)
    YqCNOT("a", i,       "b", i)
    YqCNOT("b", i - 1,   "b", i)
  }

  YqCNOT("b", 0,   "a", 0)
  YqCNOT("a", 0,   "b", 0)

  for (i = 1; i <= n - 1; i++) {
    YqCNOT("b", i - 1,   "b", i)
    YqTOFFOLI("b", i - 1,
            "a", i - 1,
            "b", i)
  }

  for (i = n - 1; i > 0; i--) {
    YqTOFFOLI("b", i - 1,
            "a", i - 1,
            "a", i)
  }

  YqCNOT("a", 0,   "b", 0)

  for (i = 1; i <= n - 1; i++) {
    YqCNOT("a", i - 1,   "a", i)
    YqCNOT("a", i,       "b", i)
  }

  END_DEFINITION_OF_MODULE
}



void
a__eq__a_minus_b__w_unsigned_underflow_bit(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
              unsigned_underflow
  */

  /* Make sure n != 0. */
  if (n == 0) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module a__eq__a_minus_b(...)' : "
            "The number of bits must be >= 1.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  for (i = n - 1; i > 0; i--) {
    YqCNOT("b", i,       "a", i)
    YqCNOT("a", i,       "b", i)
    YqCNOT("b", i - 1,   "b", i)
  }

  YqCNOT("b", 0,   "a", 0)
  YqCNOT("a", 0,   "b", 0)

  for (i = 1; i <= n - 1; i++) {
    YqCNOT("b", i - 1,   "b", i)
    YqTOFFOLI("b", i - 1,
            "a", i - 1,
            "b", i)
  }

  YqTOFFOLI("a", n - 1,
          "b", n - 1,
          "unsigned_underflow", 0);

  for (i = n - 1; i > 0; i--) {
    YqTOFFOLI("b", i - 1,
            "a", i - 1,
            "a", i)
  }

  YqCNOT("a", 0,   "b", 0)

  for (i = 1; i <= n - 1; i++) {
    YqCNOT("a", i - 1,   "a", i)
    YqCNOT("a", i,       "b", i)
  }

  YqCNOT("a", n - 1,   "unsigned_underflow", 0);

  END_DEFINITION_OF_MODULE
}



void
a_less_than_b__as_signed(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
              x
  */

  YqNOT("a", n - 1);
  YqNOT("b", n - 1);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "a", i)
      MAP_SIGNAL("b", i,   "b", i)
    }
    MAP_SIGNAL("unsigned_underflow", 0,
               "x", 0);
  a__eq__a_minus_b__w_unsigned_underflow_bit(LOCATION_INFO
    , i);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "a", i)
      MAP_SIGNAL("b", i,   "b", i)
    }
  a__eq__a_plus_b(LOCATION_INFO
    , i);

  YqNOT("a", n - 1);
  YqNOT("b", n - 1);

  END_DEFINITION_OF_MODULE
}



void
a_less_than_or_eq_to_b__as_signed(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
              x
  */

  YqNOT("x", 0);

  YqNOT("a", n - 1);
  YqNOT("b", n - 1);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "b", i)
      MAP_SIGNAL("b", i,   "a", i)
    }
    MAP_SIGNAL("unsigned_underflow", 0,
               "x", 0);
  a__eq__a_minus_b__w_unsigned_underflow_bit(LOCATION_INFO
    , i);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "b", i)
      MAP_SIGNAL("b", i,   "a", i)
    }
  a__eq__a_plus_b(LOCATION_INFO
    , i);

  YqNOT("a", n - 1);
  YqNOT("b", n - 1);

  END_DEFINITION_OF_MODULE
}



void
a_greater_than_b__as_signed(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
              x
  */

  YqNOT("b", n - 1);
  YqNOT("a", n - 1);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "b", i)
      MAP_SIGNAL("b", i,   "a", i)
    }
    MAP_SIGNAL("unsigned_underflow", 0,
               "x", 0);
  a__eq__a_minus_b__w_unsigned_underflow_bit(LOCATION_INFO
    , i);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "b", i)
      MAP_SIGNAL("b", i,   "a", i)
    }
  a__eq__a_plus_b(LOCATION_INFO
    , i);

  YqNOT("b", n - 1);
  YqNOT("a", n - 1);

  END_DEFINITION_OF_MODULE
}



void
a_greater_than_or_eq_to_b__as_signed(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
              x
  */

  YqNOT("x", 0);

  YqNOT("b", n - 1);
  YqNOT("a", n - 1);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "a", i)
      MAP_SIGNAL("b", i,   "b", i)
    }
    MAP_SIGNAL("unsigned_underflow", 0,
               "x", 0);
  a__eq__a_minus_b__w_unsigned_underflow_bit(LOCATION_INFO
    , i);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "a", i)
      MAP_SIGNAL("b", i,   "b", i)
    }
  a__eq__a_plus_b(LOCATION_INFO
    , i);

  YqNOT("b", n - 1);
  YqNOT("a", n - 1);

  END_DEFINITION_OF_MODULE
}



void
is_a_eq_to_b(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
              x
  */

  YqNOT("a", n - 1);
  YqNOT("b", n - 1);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "a", i)
      MAP_SIGNAL("b", i,   "b", i)
    }
    MAP_SIGNAL("unsigned_underflow", 0,
               "x", 0);
  a__eq__a_minus_b__w_unsigned_underflow_bit(LOCATION_INFO
    , i);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "a", i)
      MAP_SIGNAL("b", i,   "b", i)
    }
  a__eq__a_plus_b(LOCATION_INFO
    , i);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "b", i)
      MAP_SIGNAL("b", i,   "a", i)
    }
    MAP_SIGNAL("unsigned_underflow", 0,
               "x", 0);
  a__eq__a_minus_b__w_unsigned_underflow_bit(LOCATION_INFO
    , i);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "b", i)
      MAP_SIGNAL("b", i,   "a", i)
    }
  a__eq__a_plus_b(LOCATION_INFO
    , i);

  YqNOT("b", n - 1);
  YqNOT("a", n - 1);
  YqNOT("x", 0);

  END_DEFINITION_OF_MODULE
}



void
is_a_not_eq_to_b(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
              x
  */

  YqNOT("a", n - 1);
  YqNOT("b", n - 1);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "a", i)
      MAP_SIGNAL("b", i,   "b", i)
    }
    MAP_SIGNAL("unsigned_underflow", 0,
               "x", 0);
  a__eq__a_minus_b__w_unsigned_underflow_bit(LOCATION_INFO
    , i);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "a", i)
      MAP_SIGNAL("b", i,   "b", i)
    }
  a__eq__a_plus_b(LOCATION_INFO
    , i);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "b", i)
      MAP_SIGNAL("b", i,   "a", i)
    }
    MAP_SIGNAL("unsigned_underflow", 0,
               "x", 0);
  a__eq__a_minus_b__w_unsigned_underflow_bit(LOCATION_INFO
    , i);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "b", i)
      MAP_SIGNAL("b", i,   "a", i)
    }
  a__eq__a_plus_b(LOCATION_INFO
    , i);

  YqNOT("b", n - 1);
  YqNOT("a", n - 1);

  END_DEFINITION_OF_MODULE
}



void
a__eq__a_plus_b_times_c(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t j;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
              c[n-1 ... 0]
  */

  /* Make sure n != 0. */
  if (n == 0) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module a__eq__a_plus_b_times_c(...)' : "
            "The number of bits must be >= 1.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  for (i = 0; i < n; i++) {
    PUSH_CONTROL_SIGNAL("b", i)
    INSTANTIATE_MODULE
      for (j = 0; j < n - i; j++) {
        MAP_SIGNAL("a", j,   "a", i + j)
        MAP_SIGNAL("b", j,   "c", j)
      }
    a__eq__a_plus_b(LOCATION_INFO
      , n - i);
    POP_AND_DELETE_CONTROL_SIGNAL
  }

  END_DEFINITION_OF_MODULE
}



void
a__eq__a_minus_b_times_c(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t j;

  /* Signals:
              a[n-1 ... 0]
              b[n-1 ... 0]
              c[n-1 ... 0]
  */

  /* Make sure n != 0. */
  if (n == 0) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module a__eq__a_minus_b_times_c(...)' : "
            "The number of bits must be >= 1.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  for (i = 0; i < n; i++) {
    PUSH_CONTROL_SIGNAL("b", i)
    INSTANTIATE_MODULE
      for (j = 0; j < n - i; j++) {
        MAP_SIGNAL("a", j,   "a", i + j)
        MAP_SIGNAL("b", j,   "c", j)
      }
    a__eq__a_minus_b(LOCATION_INFO
      , n - i);
    POP_AND_DELETE_CONTROL_SIGNAL
  }

  END_DEFINITION_OF_MODULE
}

/* -----------------  END  `bbrl_lib_integer_arithmetic_.c' ----------------- */
