#include <ctqg________________________.h>



char       ctqg_file_name[4096];
char       bbrl_file_name[4096];
FILE *     ctqg_file = NULL;
FILE *     bbrl_file = NULL;

sdll_TOKEN        tokens = {NULL, NULL};

iter_sdll_TOKEN   t_iter1 = NULL;
iter_sdll_TOKEN   t_iter2 = NULL;

str_on_heap_TOKEN st_callee_name = NULL; /* Does not own memory. */
str_on_heap_TOKEN st_module_name = NULL; /* Does not own memory. */
int               e_st_does_module_use_constants = 888888;
int               e_st_does_dollar_block_have_constants = 888888;
uint64_t          e_st_num_of_constants_in_dollar_block = 888888;
uint64_t          e_st_max_n_of_c_in_a_single_d_b_in_module = 888888;
sdll_TOKEN        d_st_param_list = {NULL, NULL};
iter_sdll_TOKEN   e_st_iter__insert_ancilla_decl_to_the_right_of_me = NULL;
iter_sdll_TOKEN   e_st_iter__i_point_to_instantiate_module = NULL;
iter_sdll_TOKEN   e_st_iter__i_point_to_end_oper_parm = NULL;
uint64_t          e_st_position_of_param_defining_cs_of_oper = 888888;
iter_sdll_TOKEN   f_st_iter__insert_if_else_vars_to_the_left_of_me = NULL;
uint64_t          f_st_peak_number_of_oto_ancillas = 888888;
uint64_t          f_st_number_of_oto_ancillas = 888888;

sgt_TOKEN         st_var_decl_sgt = {NULL, 0, 0};
var_decl_value_t  st_var_decl_value_storage[MAX_NUM_VAR_DECL];
uint64_t          st_var_decl_num_entries = 0;
str_on_heap_TOKEN st_param_array[MAX_NUM_PARAMETERS];
uint64_t          st_param_array_num_entries = 0;



static void
er1(
uint32_t line) {
  fprintf(stderr, "Error.\n");
  fprintf(stderr,
          "A token (an ID, a constant, etc.) is too long on line %s:%d."
          "\n",
          ctqg_file_name, line);
  clean_exit(-1);
}



void
tokenize(
void) {
  int          rh;
  int          keep_iterating;
  char         ct[4096];
  uint32_t     ct_length = 0;
  token_type_t current_state = INVALID_______TT;
  uint32_t     current_line = 1;
  uint32_t     brackets_depth = 0;
  TOKEN *      tk_ptr;

  if ( (ctqg_file = fopen(ctqg_file_name, "r")) == NULL ) {
    fprintf(stderr,
            "Error.\n"
            "Can not open file `%s' for reading.\n", ctqg_file_name);
    clean_exit(-1);
  }

  rh = fgetc(ctqg_file);
  keep_iterating = (1 != 0);
  while (keep_iterating) switch (current_state) {

    case INVALID_______TT: /* Not reading any token. */
      if (rh == EOF) {
        keep_iterating = (0 != 0);
      } else {
        if ( (rh == '_') ||
             ((rh >= 'A') && (rh <= 'Z')) ||
             ((rh >= 'a') && (rh <= 'z')) ) {
          ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
          rh = fgetc(ctqg_file);
          current_state = ID____________TT;
        } else
        if ( (rh >= '0') && (rh <= '9') ) {
          ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
          rh = fgetc(ctqg_file);
          current_state = DECIMAL_CONST_TT;
        } else
        if ( rh == ' ' ) {
          ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
          rh = fgetc(ctqg_file);
          current_state = SPACE_________TT;
        } else
        if ( rh == '.' ) {
          ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
          rh = fgetc(ctqg_file);
          current_state = PERIODS_______TT;
        } else
        if ( (rh == '!') || (rh == '<') || (rh == '=') || (rh == '>') ||
             (rh == '/') || (rh == '-') || (rh == '*') || (rh == '+') ||
             (rh == '%') || (rh == '&') || (rh == '|') || (rh == '~') ||
             (rh == '^') || (rh == ':') ) {
          ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
          rh = fgetc(ctqg_file);
          current_state = OPERATOR______TT;
        } else
        if ( rh == '\012' ) {
          ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
          rh = fgetc(ctqg_file);
          current_state = END_OF_LINE___TT;
          ct[ct_length] = '\0';
          new_at_the_right_end_sdll_TOKEN(tk_ptr, tokens)
          h_TOKEN_alloc_strcpy_hs(&(tk_ptr->value), ct);
          tk_ptr->line = current_line;
          tk_ptr->brackets_depth = brackets_depth;
          tk_ptr->type = current_state;
          ct_length = 0; current_state = INVALID_______TT;
          current_line++;
        } else
        if ( rh == '[' ) {
          ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
          rh = fgetc(ctqg_file);
          current_state = BRACKETS______TT;
          ct[ct_length] = '\0';
          new_at_the_right_end_sdll_TOKEN(tk_ptr, tokens)
          h_TOKEN_alloc_strcpy_hs(&(tk_ptr->value), ct);
          tk_ptr->line = current_line;
          tk_ptr->brackets_depth = brackets_depth;
          tk_ptr->type = current_state;
          ct_length = 0; current_state = INVALID_______TT;
          brackets_depth++;
        } else
        if (rh == ']') {
          if (brackets_depth == 0) {
            fprintf(stderr, "Error.\n");
            fprintf(stderr,
                    "']' without matching '[' on line %s:%d.\n",
                    ctqg_file_name, current_line);
            clean_exit(-1);
          }
          brackets_depth--;
          ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
          rh = fgetc(ctqg_file);
          current_state = BRACKETS______TT;
          ct[ct_length] = '\0';
          new_at_the_right_end_sdll_TOKEN(tk_ptr, tokens)
          h_TOKEN_alloc_strcpy_hs(&(tk_ptr->value), ct);
          tk_ptr->line = current_line;
          tk_ptr->brackets_depth = brackets_depth;
          tk_ptr->type = current_state;
          ct_length = 0; current_state = INVALID_______TT;
        } else
        if ( (rh == '"') || (rh == '\047') || (rh == '`') ||
             (rh == ';') || (rh == '\134') || (rh == '?') || (rh == '#') ||
             (rh == '(') || (rh == ')') || (rh == '{') || (rh == '}') ||
             (rh == '$') || (rh == ',') || (rh == '@') ) {
          ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
          rh = fgetc(ctqg_file);
          current_state = STDALN_SYM____TT;
          ct[ct_length] = '\0';
          new_at_the_right_end_sdll_TOKEN(tk_ptr, tokens)
          h_TOKEN_alloc_strcpy_hs(&(tk_ptr->value), ct);
          tk_ptr->line = current_line;
          tk_ptr->brackets_depth = brackets_depth;
          tk_ptr->type = current_state;
          ct_length = 0; current_state = INVALID_______TT;
        } else {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "An illegal character ASCII(%d) encountered while "
                  "tokenizing file `%s' line %d.\n",
                  rh, ctqg_file_name, current_line);
          clean_exit(-1);
        }
      }
    break;

    case ID____________TT:
      if ( (rh == '_') ||
           ((rh >= 'A') && (rh <= 'Z')) ||
           ((rh >= 'a') && (rh <= 'z')) ||
           ((rh >= '0') && (rh <= '9')) ) {
        ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
        rh = fgetc(ctqg_file);
      } else {
        ct[ct_length] = '\0';
        new_at_the_right_end_sdll_TOKEN(tk_ptr, tokens)
        h_TOKEN_alloc_strcpy_hs(&(tk_ptr->value), ct);
        tk_ptr->line = current_line;
        tk_ptr->brackets_depth = brackets_depth;
        tk_ptr->type = current_state;
        ct_length = 0; current_state = INVALID_______TT;
      }
    break;

    case DECIMAL_CONST_TT:
      if ( ((rh >= '0') && (rh <= '9')) || (rh == 'u') || (rh == 'l') ) {
        ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
        rh = fgetc(ctqg_file);
      } else {
        ct[ct_length] = '\0';
        new_at_the_right_end_sdll_TOKEN(tk_ptr, tokens)
        h_TOKEN_alloc_strcpy_hs(&(tk_ptr->value), ct);
        tk_ptr->line = current_line;
        tk_ptr->brackets_depth = brackets_depth;
        tk_ptr->type = current_state;
        ct_length = 0; current_state = INVALID_______TT;
      }
    break;

    case SPACE_________TT:
      if ( rh == ' ' ) {
        ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
        rh = fgetc(ctqg_file);
      } else {
        ct[ct_length] = '\0';
        new_at_the_right_end_sdll_TOKEN(tk_ptr, tokens)
        h_TOKEN_alloc_strcpy_hs(&(tk_ptr->value), ct);
        tk_ptr->line = current_line;
        tk_ptr->brackets_depth = brackets_depth;
        tk_ptr->type = current_state;
        ct_length = 0; current_state = INVALID_______TT;
      }
    break;

    case PERIODS_______TT:
      if ( rh == '.' ) {
        ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
        rh = fgetc(ctqg_file);
      } else {
        ct[ct_length] = '\0';
        new_at_the_right_end_sdll_TOKEN(tk_ptr, tokens)
        h_TOKEN_alloc_strcpy_hs(&(tk_ptr->value), ct);
        tk_ptr->line = current_line;
        tk_ptr->brackets_depth = brackets_depth;
        tk_ptr->type = current_state;
        ct_length = 0; current_state = INVALID_______TT;
      }
    break;

    case OPERATOR______TT:
      if ( (rh == '!') || (rh == '<') || (rh == '=') || (rh == '>') ||
           (rh == '/') || (rh == '-') || (rh == '*') || (rh == '+') ||
           (rh == '%') || (rh == '&') || (rh == '|') || (rh == '~') ||
           (rh == '^') ) {
        ct[ct_length++] = rh; if (ct_length >= 4095) er1(current_line);
        rh = fgetc(ctqg_file);
      } else {
        ct[ct_length] = '\0';
        new_at_the_right_end_sdll_TOKEN(tk_ptr, tokens)
        h_TOKEN_alloc_strcpy_hs(&(tk_ptr->value), ct);
        tk_ptr->line = current_line;
        tk_ptr->brackets_depth = brackets_depth;
        tk_ptr->type = current_state;
        ct_length = 0; current_state = INVALID_______TT;
      }
    break;

    /* END_OF_LINE___TT, BRACKETS______TT, STDALN_SYM____TT are unreachable. */

    default:
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "An internal CTQG error occurred while "
              "tokenizing file `%s' line %d.\n",
              ctqg_file_name, current_line);
      clean_exit(-2);
  }

  if (brackets_depth != 0) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "'[' and ']' are not balanced at the end of file `%s'.\n",
            ctqg_file_name);
    clean_exit(-1);
  }

  fclose(ctqg_file);
  ctqg_file = NULL;
}



void
assemble_tokens(
void) {
  iter_sdll_TOKEN iter;

  if ( (bbrl_file = fopen(bbrl_file_name, "w")) == NULL ) {
    fprintf(stderr,
            "Error.\n"
            "Can not open file `%s' for writing.\n", bbrl_file_name);
    clean_exit(-1);
  }

  set_iter_to_leftmost_el_sdll_TOKEN(iter, tokens)
  while (iter != NULL) {
    h_TOKEN_fprintf_str(bbrl_file, iter->e.body.value, 0, ' ', 'l');
    move_iter_right_sdll_TOKEN(iter)
  }

  fclose(bbrl_file);
  bbrl_file = NULL;
}



void
fprint_tokens(
FILE * stream, sdll_TOKEN sdll) {
  iter_sdll_TOKEN iter;
  set_iter_to_leftmost_el_sdll_TOKEN(iter, sdll)
  while (iter != NULL) {
    print_to_file_TOKEN(stream, &(iter->e.body));
    move_iter_right_sdll_TOKEN(iter)
  }
}



void /* Excpt: a single "-" is not cons. an op. (bec. of unary minus issue). */
to_next_operator_or_closed_parenthesis_token_of_same_depth_trNULLlfmst(
iter_sdll_TOKEN * iter) {
  uint32_t bracket_depth;

  if (*iter == NULL) {
    bracket_depth = 0;
    set_iter_to_leftmost_el_sdll_TOKEN(*iter, tokens)
  } else {
    bracket_depth = (*iter)->e.body.brackets_depth;
    move_iter_right_sdll_TOKEN(*iter)
  }

  while (*iter != NULL) {
    if ( ((*iter)->e.body.type == OPERATOR______TT)
         &&
         ((*iter)->e.body.brackets_depth == bracket_depth)
         &&
         (h_TOKEN_strcmp_hs((*iter)->e.body.value, "-") != 0) ) {
      break;
    }
    if ( ((*iter)->e.body.type == STDALN_SYM____TT)
         &&
         ((*iter)->e.body.brackets_depth == bracket_depth)
         &&
         (h_TOKEN_strcmp_hs((*iter)->e.body.value, ")") == 0) ) {
      break;
    }
    move_iter_right_sdll_TOKEN(*iter)
  }
}



void /* Excpt: a single "-" is not cons. an op. (bec. of unary minus issue). */
to_next_operator_token_of_same_depth_before_semicolumn_trNULLlfmst(
iter_sdll_TOKEN * iter) {
  uint32_t bracket_depth;

  if (*iter == NULL) {
    bracket_depth = 0;
    set_iter_to_leftmost_el_sdll_TOKEN(*iter, tokens)
  } else {
    bracket_depth = (*iter)->e.body.brackets_depth;
    move_iter_right_sdll_TOKEN(*iter)
  }

  while (*iter != NULL) {
    if ( ((*iter)->e.body.type == OPERATOR______TT)
         &&
         ((*iter)->e.body.brackets_depth == bracket_depth)
         &&
         (h_TOKEN_strcmp_hs((*iter)->e.body.value, "-") != 0) ) {
      break;
    }
    if ( ((*iter)->e.body.type == STDALN_SYM____TT)
         &&
         (h_TOKEN_strcmp_hs((*iter)->e.body.value, ";") == 0) ) {
      *iter = NULL;
      break;
    }
    move_iter_right_sdll_TOKEN(*iter)
  }

  if (0) {
    if (*iter != NULL) {
      printf("Moved to token `");
      h_TOKEN_printf_str( (*iter)->e.body.value, 0, ' ', 'l' );
      printf("'.\n");
    } else {
      printf("Moved to the end of token list.\n");
    }
  }
}



void
to_next_non_separ_token_treating_NULL_as_leftmost(
iter_sdll_TOKEN * iter) {
  if (*iter == NULL) {
    set_iter_to_leftmost_el_sdll_TOKEN(*iter, tokens)
  } else {
    move_iter_right_sdll_TOKEN(*iter)
  }

  while (*iter != NULL) {
    if ( ((*iter)->e.body.type != SPACE_________TT)
         &&
         ((*iter)->e.body.type != END_OF_LINE___TT) ) break;
    move_iter_right_sdll_TOKEN(*iter)
  }

  if (0) {
    if (*iter != NULL) {
      printf("Moved to token `");
      h_TOKEN_printf_str( (*iter)->e.body.value, 0, ' ', 'l' );
      printf("'.\n");
    } else {
      printf("Moved to the end of token list.\n");
    }
  }
}



void
to_previous_non_separ_token_treating_NULL_as_rightmost(
iter_sdll_TOKEN * iter) {
 if (*iter == NULL) {
    set_iter_to_rightmost_el_sdll_TOKEN(*iter, tokens)
  } else {
    move_iter_left_sdll_TOKEN(*iter)
  }

  while (*iter != NULL) {
    if ( ((*iter)->e.body.type != SPACE_________TT)
         &&
         ((*iter)->e.body.type != END_OF_LINE___TT) ) break;
    move_iter_left_sdll_TOKEN(*iter)
  }
}



void
move_iter_and_delete_to_next_non_separ_token(
iter_sdll_TOKEN * iter) {
  if (*iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  } else {
    if ( (*iter)->e.body.type != END_OF_LINE___TT ) {
      delete_TOKEN_and_move_iter_to_right(iter, &tokens);
    } else {
      move_iter_right_sdll_TOKEN(*iter)
    }
  }

  while (*iter != NULL) {
    if ( ((*iter)->e.body.type != SPACE_________TT)
         &&
         ((*iter)->e.body.type != END_OF_LINE___TT) ) break;
    if ( (*iter)->e.body.type != END_OF_LINE___TT ) {
      delete_TOKEN_and_move_iter_to_right(iter, &tokens);
    } else {
      move_iter_right_sdll_TOKEN(*iter)
    }
  }
}



void
delete_TOKEN_and_move_iter_to_right(
iter_sdll_TOKEN * iter, sdll_TOKEN * sdll) {
  if ((*iter)->e.body.value != NULL) {
    h_TOKEN_str_delete((*iter)->e.body.value);
    (*iter)->e.body.value = NULL;
  }
  delete_and_move_iter_to_right_sdll_TOKEN(*iter, *sdll)
}



void
copy_token_to_the_left_of_iter(
iter_sdll_TOKEN iter, TOKEN * source) {
  TOKEN * ptr_to_new_token;

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hh(&(ptr_to_new_token->value), source->value);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = source->brackets_depth;
  ptr_to_new_token->type = source->type;
}



void
insert__space__to_the_left_of_iter(
iter_sdll_TOKEN iter) {
  TOKEN * ptr_to_new_token;

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), " ");
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = SPACE_________TT;
}



void
insert__semicolumn_space__to_the_left_of_iter(
iter_sdll_TOKEN iter) {
  TOKEN * ptr_to_new_token;

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), ";");
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = STDALN_SYM____TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), " ");
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = SPACE_________TT;
}



void
insert__stroper__to_the_left_of_iter(
iter_sdll_TOKEN iter, char * strid) {
  TOKEN * ptr_to_new_token;

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), strid);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = OPERATOR______TT;
}



void
insert__strstdaln__to_the_left_of_iter(
iter_sdll_TOKEN iter, char * strid) {
  TOKEN * ptr_to_new_token;

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), strid);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = STDALN_SYM____TT;
}



void
insert__strid__to_the_left_of_iter(
iter_sdll_TOKEN iter, char * strid) {
  TOKEN * ptr_to_new_token;

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), strid);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;
}



void
insert__bracket_uint32_bracket__to_the_left_of_iter(
iter_sdll_TOKEN iter, uint32_t n) {
  TOKEN * ptr_to_new_token;
  char    s[64];

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "[");
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = BRACKETS______TT;

  sprintf(s, "%u", n);
  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), s);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth - 1;
  ptr_to_new_token->type = DECIMAL_CONST_TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "]");
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = BRACKETS______TT;
}



void
insert__str_uint32_str__to_the_left_of_iter(
iter_sdll_TOKEN iter, char * str1, uint32_t n, char * str2) {
  TOKEN * ptr_to_new_token;
  char    s[64];

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str1);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  sprintf(s, "%u", n);
  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), s);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = DECIMAL_CONST_TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str2);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;
}



void
insert__str_h_str__to_the_left_of_iter(
iter_sdll_TOKEN iter, char * str1, str_on_heap_TOKEN h, char * str2) {
  TOKEN * ptr_to_new_token;

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str1);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hh(&(ptr_to_new_token->value), h);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str2);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;
}



void
insert__str_uint32_str_h_str_to_the_left_of_iter(
iter_sdll_TOKEN iter, char * str1, uint32_t n, char * str2,
str_on_heap_TOKEN h, char * str3) {
  TOKEN * ptr_to_new_token;
  char    s[64];

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str1);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  sprintf(s, "%u", n);
  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), s);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = DECIMAL_CONST_TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str2);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hh(&(ptr_to_new_token->value), h);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str3);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;
}



void
insert__s_ui32_s_hpostfix_s_ui32_s_to_the_left_of_iter(
iter_sdll_TOKEN iter, char * str1, uint32_t n1, char * str2,
str_on_heap_TOKEN h1, char * str3, uint32_t n2, char * str4) {
  static char       s[4096];
  char *            cptr = s;
  TOKEN *           ptr_to_new_token;
  str_on_heap_TOKEN h;

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  if (h_TOKEN_strlen(h1) >= 4096) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Name of a signal of callee is too long in "
            "instantiation on line %s:%d.\n",
            ctqg_file_name, (iter) ? iter->e.body.line : 999999999);
    clean_exit(-1);
  }

  h_TOKEN_strcpy_to_prealloc_sh(s, h1);
  while ( (*cptr != '\0') && (*cptr != ':') ) {
    cptr++;
  }
  if (*cptr == '\0') {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  cptr++;
  if (*cptr != ':') {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  cptr++;

  h_TOKEN_alloc_strcpy_hs(&h, cptr);

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str1);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  sprintf(s, "%u", n1);
  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), s);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = DECIMAL_CONST_TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str2);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hh(&(ptr_to_new_token->value), h);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str3);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  sprintf(s, "%u", n2);
  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), s);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = DECIMAL_CONST_TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str4);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  h_TOKEN_str_delete(h);
}



void
insert__s_ui32_s_hpostfix_s_to_the_left_of_iter(
iter_sdll_TOKEN iter, char * str1, uint32_t n1, char * str2,
str_on_heap_TOKEN h1, char * str3) {
  static char       s[4096];
  char *            cptr = s;
  TOKEN *           ptr_to_new_token;
  str_on_heap_TOKEN h;

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  if (h_TOKEN_strlen(h1) >= 4096) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Name of a signal of callee is too long in "
            "instantiation on line %s:%d.\n",
            ctqg_file_name, (iter) ? iter->e.body.line : 999999999);
    clean_exit(-1);
  }

  h_TOKEN_strcpy_to_prealloc_sh(s, h1);
  while ( (*cptr != '\0') && (*cptr != ':') ) {
    cptr++;
  }
  if (*cptr == '\0') {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  cptr++;
  if (*cptr != ':') {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  cptr++;

  h_TOKEN_alloc_strcpy_hs(&h, cptr);

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str1);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  sprintf(s, "%u", n1);
  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), s);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = DECIMAL_CONST_TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str2);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hh(&(ptr_to_new_token->value), h);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str3);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  h_TOKEN_str_delete(h);
}



void
insert__str_h_str_hpostfix_str_to_the_left_of_iter(
iter_sdll_TOKEN iter, char * str1, str_on_heap_TOKEN h1, char * str2,
str_on_heap_TOKEN h2, char * str3) {
  static char       s[4096];
  char *            cptr = s;
  TOKEN *           ptr_to_new_token;
  str_on_heap_TOKEN h;

  if (iter == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  if (h_TOKEN_strlen(h2) >= 4096) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Name of a signal of callee is too long in "
            "instantiation on line %s:%d.\n",
            ctqg_file_name, (iter) ? iter->e.body.line : 999999999);
    clean_exit(-1);
  }

  h_TOKEN_strcpy_to_prealloc_sh(s, h2);
  while ( (*cptr != '\0') && (*cptr != ':') ) {
    cptr++;
  }
  if (*cptr == '\0') {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  cptr++;
  if (*cptr != ':') {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  cptr++;

  h_TOKEN_alloc_strcpy_hs(&h, cptr);

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str1);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hh(&(ptr_to_new_token->value), h1);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str2);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hh(&(ptr_to_new_token->value), h);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, iter, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), str3);
  ptr_to_new_token->line = iter->e.body.line;
  ptr_to_new_token->brackets_depth = iter->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  h_TOKEN_str_delete(h);
}



void
delete_sdll_TOKEN(
sdll_TOKEN * sdll) {
  iter_sdll_TOKEN iter;
  set_iter_to_leftmost_el_sdll_TOKEN(iter, *sdll)
  while (iter != NULL) {
    delete_TOKEN_and_move_iter_to_right(&iter, sdll);
  }
}



void
clean_exit(
int status) {
  if (ctqg_file != NULL) {
    fclose(ctqg_file);
    ctqg_file = NULL;
  }

  if (bbrl_file != NULL) {
    fclose(bbrl_file);
    bbrl_file = NULL;
  }

  delete_st_param_array();
  delete_st_var_decl_sgt();
  delete_sdll_TOKEN(&d_st_param_list);
  delete_sdll_TOKEN(&tokens);

  if (status != 0) memory_manager_message_level = 0;
  release_memory();

  exit(status);
}



var_decl_value_t*
search_var_decl(
str_on_heap_TOKEN name_of_module, str_on_heap_TOKEN name_of_var) {
  var_decl_value_t *        rv;
  TOKEN                     search_key;
  TOKEN *                   ptr_to_search_result;
  str_on_heap_TOKEN         key;
  str_on_heap_TOKEN         tmp_separator;

  h_TOKEN_alloc_strcpy_hs(&tmp_separator, "::");
  h_TOKEN_alloc_strcpy_hh(&key, name_of_module);
  h_TOKEN_strcat(key, tmp_separator);
  h_TOKEN_strcat(key, name_of_var);
  h_TOKEN_str_delete(tmp_separator);

  search_key.value = key;
  find__if_many_give_eps_smallest__sgt_TOKEN(
    ptr_to_search_result, search_key, st_var_decl_sgt)
  if (ptr_to_search_result == NULL) {
    rv = NULL;
  } else {
    rv = &(st_var_decl_value_storage[ptr_to_search_result->line]);
  }

  h_TOKEN_str_delete(key);
  return rv;
}



var_decl_value_t*
search_var_decl_by_number(
str_on_heap_TOKEN name_of_module, uint32_t n) {
  var_decl_value_t *        rv;
  TOKEN                     search_key;
  TOKEN *                   ptr_to_search_result;
  str_on_heap_TOKEN         key;
  str_on_heap_TOKEN         tmp;
  char                      s[64];

  sprintf(s, "::%u", n);
  h_TOKEN_alloc_strcpy_hs(&tmp, s);
  h_TOKEN_alloc_strcpy_hh(&key, name_of_module);
  h_TOKEN_strcat(key, tmp);
  h_TOKEN_str_delete(tmp);

  search_key.value = key;
  find__if_many_give_eps_smallest__sgt_TOKEN(
    ptr_to_search_result, search_key, st_var_decl_sgt)
  if (ptr_to_search_result == NULL) {
    rv = NULL;
  } else {
    rv = &(st_var_decl_value_storage[ptr_to_search_result->line]);
  }

  h_TOKEN_str_delete(key);
  return rv;
}



var_decl_value_t*
new_var_decl(
str_on_heap_TOKEN name_of_module, str_on_heap_TOKEN name_of_var) {
  var_decl_value_t *        rv;
  ptr_to_entry_of_sgt_TOKEN ptr_to_new_entry;
  str_on_heap_TOKEN         key;
  str_on_heap_TOKEN         tmp_separator;
  uint32_t                  i;

  if (name_of_module == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Declaration not inside of a module body on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  if (st_var_decl_num_entries >= MAX_NUM_VAR_DECL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "The total number of declared variables (identifiers) "
            "exceeded the maximum value of %d on line %s:%d.\n",
            MAX_NUM_VAR_DECL,
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  h_TOKEN_alloc_strcpy_hs(&tmp_separator, "::");
  h_TOKEN_alloc_strcpy_hh(&key, name_of_module);
  h_TOKEN_strcat(key, tmp_separator);
  h_TOKEN_strcat(key, name_of_var);
  h_TOKEN_str_delete(tmp_separator);

  new_entry_for_sgt_TOKEN(ptr_to_new_entry);
  ptr_to_new_entry->e.body.value = key;

  if ( insert__if_conflicts_bounce__sgt_TOKEN(
         ptr_to_new_entry, &st_var_decl_sgt) ) {
    ptr_to_new_entry->e.body.line = st_var_decl_num_entries++;
    rv = &(st_var_decl_value_storage[ptr_to_new_entry->e.body.line]);
    rv->num_indexes = 0;
    for (i = 0; i < MAX_NUM_INDEXES; i++) {
      rv->indexes[i] = NULL;
      rv->subst_indexes[i] = NULL;
    }
    rv->ptr_back_to_name = ptr_to_new_entry->e.body.value;
  } else {
    delete_unused_entry_for_sgt_TOKEN(ptr_to_new_entry)
    h_TOKEN_str_delete(key);
    rv = NULL;
  }

  return rv;
}



void
delete_st_var_decl_sgt(
void) {
  ptr_to_entry_of_sgt_TOKEN ptr_to_detached_entry;
  var_decl_value_t *        temp;
  uint32_t                  i;

  while (is_nonempty_sgt_TOKEN(st_var_decl_sgt)) {
    find_prep_rm__smallest__sgt_TOKEN(&st_var_decl_sgt);
    ptr_to_detached_entry = remove_entry_from_sgt_TOKEN();
    if (ptr_to_detached_entry == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
        "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
      clean_exit(-2);
    }
    temp = &(st_var_decl_value_storage[ptr_to_detached_entry->e.body.line]);

              #ifdef void
              /* FOR DUBUGGING */
              printf("About to delete `");
              h_TOKEN_printf_str(
                ptr_to_detached_entry->e.body.value, 0, ' ', 'l');
              printf("'.\n");
              for (i = 0; i < MAX_NUM_INDEXES; i++) {
                if (temp->indexes[i] == NULL) {
                  printf("-----NULL-----");
                } else {
                  h_TOKEN_printf_str(
                    temp->indexes[i], 14, ' ', 'c');
                }
                printf("  ");
                if (temp->subst_indexes[i] == NULL) {
                  printf("-----NULL-----");
                } else {
                  h_TOKEN_printf_str(
                    temp->subst_indexes[i], 14, ' ', 'c');
                }
                printf("\n");
              }
              getchar();
              #endif

    h_TOKEN_str_delete(ptr_to_detached_entry->e.body.value);
    delete_unused_entry_for_sgt_TOKEN(ptr_to_detached_entry);
    for (i = 0; i < temp->num_indexes; i++) {
      if (temp->indexes[i] == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
          "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
        clean_exit(-2);
      }
      h_TOKEN_str_delete(temp->indexes[i]);
      temp->indexes[i] = NULL;
      if (temp->subst_indexes[i] == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
          "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
        clean_exit(-2);
      }
      h_TOKEN_str_delete(temp->subst_indexes[i]);
      temp->subst_indexes[i] = NULL;
    }
  }
}



void
add_index_to_var_decl__no_alloc(
var_decl_value_t * var_decl_val, str_on_heap_TOKEN str) {
  if (var_decl_val->num_indexes >= MAX_NUM_INDEXES) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Array has too many indexes on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    h_TOKEN_str_delete(str);
    clean_exit(-1);
  } else {
    var_decl_val->indexes[var_decl_val->num_indexes++] = str;
  }
}



void
replicate_var_decl_with_numeric_name(
str_on_heap_TOKEN module_name, uint32_t arg_number, var_decl_value_t * source) {
  ptr_to_entry_of_sgt_TOKEN ptr_to_new_entry;
  str_on_heap_TOKEN         key;
  str_on_heap_TOKEN         key_postfix;
  uint32_t                  i;
  char                      str[64];
  var_decl_value_t *        ptr_to_replica;

  if (module_name == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }
  if (st_var_decl_num_entries >= MAX_NUM_VAR_DECL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "The total number of declared variables (identifiers) "
            "is too large.\n");
    clean_exit(-1);
  }

  sprintf(str, "::%u", arg_number);
  h_TOKEN_alloc_strcpy_hs(&key_postfix, str);
  h_TOKEN_alloc_strcpy_hh(&key, module_name);
  h_TOKEN_strcat(key, key_postfix);
  h_TOKEN_str_delete(key_postfix);

  new_entry_for_sgt_TOKEN(ptr_to_new_entry);
  ptr_to_new_entry->e.body.value = key;

  if ( insert__if_conflicts_bounce__sgt_TOKEN(
         ptr_to_new_entry, &st_var_decl_sgt) ) {
    ptr_to_new_entry->e.body.line = st_var_decl_num_entries++;
    ptr_to_replica =
      &(st_var_decl_value_storage[ptr_to_new_entry->e.body.line]);
    ptr_to_replica->ptr_back_to_name = source->ptr_back_to_name;
    ptr_to_replica->signal_type = source->signal_type;
    ptr_to_replica->num_indexes = source->num_indexes;
    for (i = 0; i < MAX_NUM_INDEXES; i++) {
      if (i < source->num_indexes) {
        h_TOKEN_alloc_strcpy_hh(
          &(ptr_to_replica->indexes[i]), source->indexes[i]);
        h_TOKEN_alloc_strcpy_hh(
          &(ptr_to_replica->subst_indexes[i]), source->subst_indexes[i]);
      } else {
        ptr_to_replica->indexes[i] = NULL;
        ptr_to_replica->subst_indexes[i] = NULL;
      }
    }
  } else {
    delete_unused_entry_for_sgt_TOKEN(ptr_to_new_entry)
    h_TOKEN_str_delete(key);
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Duplicate name of module.\n");
    clean_exit(-1);
  }
}



str_on_heap_TOKEN
read_expression_in_brackets__alloc(
void) {
  static char       tmp[MAX_NUM_TOKENS_IN_EXPR][MAX_TOKEN_SIZE_IN_EXPR];
  static char       str[MAX_NUM_TOKENS_IN_EXPR * MAX_TOKEN_SIZE_IN_EXPR];
  uint64_t          num_tokens = 0;
  uint64_t          pos_in_str = 0;
  str_on_heap_TOKEN rv = NULL;
  iter_sdll_TOKEN   save_iter = t_iter1;
  uint32_t          initial_brackets_depth;
  uint32_t          i;
  uint32_t          j;

  if (t_iter1 == NULL) {
    t_iter1 = save_iter;
    rv = NULL;
  } else
  if ( (t_iter1->e.body.type == BRACKETS______TT)
       &&
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "[") == 0) ) {
    initial_brackets_depth = t_iter1->e.body.brackets_depth;
    if (num_tokens >= MAX_NUM_TOKENS_IN_EXPR) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Expression in brackets is too long on line %s:%d.\n",
              ctqg_file_name, t_iter1->e.body.line);
      clean_exit(-1);
    }
    strcpy(tmp[num_tokens++], "(");
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

    while (1 != 0) {
      if (t_iter1 == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Unterminated expression in brackets starts on line %s:%d.\n",
                ctqg_file_name, t_iter1->e.body.line);
        clean_exit(-1);
      }
      if (t_iter1->e.body.brackets_depth == initial_brackets_depth) {
        if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "]") != 0) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
            "An internal CTQG error occurred: "
            "a symbol other than `]' decreases brackets depth (1).\n");
          clean_exit(-2);
        }
        if (num_tokens >= MAX_NUM_TOKENS_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Expression in brackets has too many tokens on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          clean_exit(-1);
        }
        strcpy(tmp[num_tokens++], ")");
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
        break;
      } else {
        if (h_TOKEN_strlen(t_iter1->e.body.value) >= MAX_TOKEN_SIZE_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Token in brackets is too long on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          clean_exit(-1);
        }
        if (num_tokens >= MAX_NUM_TOKENS_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Expression in brackets has too many tokens on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          clean_exit(-1);
        }
        h_TOKEN_strcpy_to_prealloc_sh(tmp[num_tokens++], t_iter1->e.body.value);
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      }
    }

    for (i = 0; i < num_tokens; i++) {
      j = 0;
      while (tmp[i][j] != '\0') {
        str[pos_in_str++] = tmp[i][j];
        j++;
      }
    }
    str[pos_in_str++] = '\0';
    h_TOKEN_alloc_strcpy_hs(&rv, str);
  } else {
    t_iter1 = save_iter;
    rv = NULL;
  }

  return rv;
}



str_on_heap_TOKEN /* t_iter1 ->`[' (otherwise fails). Ret. NULL on failure. */
read_expression_in_brackets_subst_param__alloc(
void) {
  static char       tmp[MAX_NUM_TOKENS_IN_EXPR][MAX_TOKEN_SIZE_IN_EXPR];
  static char       str[MAX_NUM_TOKENS_IN_EXPR * MAX_TOKEN_SIZE_IN_EXPR];
  uint64_t          num_tokens = 0;
  uint64_t          pos_in_str = 0;
  str_on_heap_TOKEN rv = NULL;
  iter_sdll_TOKEN   save_iter = t_iter1;
  uint32_t          initial_brackets_depth;
  uint32_t          i;
  uint32_t          j;
  uint64_t          k;

  if (t_iter1 == NULL) {
    t_iter1 = save_iter;
    rv = NULL;
  } else
  if ( (t_iter1->e.body.type == BRACKETS______TT)
       &&
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "[") == 0) ) {
    initial_brackets_depth = t_iter1->e.body.brackets_depth;
    if (num_tokens >= MAX_NUM_TOKENS_IN_EXPR) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Expression in brackets is too long on line %s:%d.\n",
              ctqg_file_name, t_iter1->e.body.line);
      clean_exit(-1);
    }
    strcpy(tmp[num_tokens++], "(");
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

    while (1 != 0) {
      if (t_iter1 == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Unterminated expression in brackets starts on line %s:%d.\n",
                ctqg_file_name, t_iter1->e.body.line);
        clean_exit(-1);
      }
      if (t_iter1->e.body.brackets_depth == initial_brackets_depth) {
        if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "]") != 0) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
            "An internal CTQG error occurred: "
            "a symbol other than `]' decreases brackets depth (2).\n");
          clean_exit(-2);
        }
        if (num_tokens >= MAX_NUM_TOKENS_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Expression in brackets has too many tokens on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          clean_exit(-1);
        }
        strcpy(tmp[num_tokens++], ")");
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
        break;
      } else {
        if (h_TOKEN_strlen(t_iter1->e.body.value) >= MAX_TOKEN_SIZE_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Token in brackets is too long on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          clean_exit(-1);
        }
        if (num_tokens >= MAX_NUM_TOKENS_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Expression in brackets has too many tokens on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          clean_exit(-1);
        }
        h_TOKEN_strcpy_to_prealloc_sh(tmp[num_tokens], t_iter1->e.body.value);
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
        for (k = 0; k < st_param_array_num_entries; k++) {
          if ( h_TOKEN_strcmp_hs(
                 st_param_array[k],
                 tmp[num_tokens]) == 0 ) {
            sprintf(tmp[num_tokens], "yQp[%u]", (uint32_t) k);
            break;
          }
        }
        num_tokens++;
      }
    }

    for (i = 0; i < num_tokens; i++) {
      j = 0;
      while (tmp[i][j] != '\0') {
        str[pos_in_str++] = tmp[i][j];
        j++;
      }
    }
    str[pos_in_str++] = '\0';
    h_TOKEN_alloc_strcpy_hs(&rv, str);
  } else {
    t_iter1 = save_iter;
    rv = NULL;
  }

  return rv;
}



str_on_heap_TOKEN /* t_iter1 ->`[' (otherwise fails). Ret. NULL on failure. */
read_and_del_expression_in_brackets__alloc(/* Changes `[', `]' to `(', `)'. */
void) {
  static char       tmp[MAX_NUM_TOKENS_IN_EXPR][MAX_TOKEN_SIZE_IN_EXPR];
  static char       str[MAX_NUM_TOKENS_IN_EXPR * MAX_TOKEN_SIZE_IN_EXPR];
  uint64_t          num_tokens = 0;
  uint64_t          pos_in_str = 0;
  str_on_heap_TOKEN rv = NULL;
  iter_sdll_TOKEN   save_iter = t_iter1;
  uint32_t          initial_brackets_depth;
  uint32_t          i;
  uint32_t          j;

  if (t_iter1 == NULL) {
    t_iter1 = save_iter;
    rv = NULL;
  } else
  if ( (t_iter1->e.body.type == BRACKETS______TT)
       &&
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "[") == 0) ) {
    initial_brackets_depth = t_iter1->e.body.brackets_depth;
    if (num_tokens >= MAX_NUM_TOKENS_IN_EXPR) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Expression in brackets is too long on line %s:%d.\n",
              ctqg_file_name, t_iter1->e.body.line);
      clean_exit(-1);
    }
    strcpy(tmp[num_tokens++], "(");
    move_iter_and_delete_to_next_non_separ_token(&t_iter1);

    while (1 != 0) {
      if (t_iter1 == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Unterminated expression in brackets starts on line %s:%d.\n",
                ctqg_file_name, t_iter1->e.body.line);
        clean_exit(-1);
      }
      if (t_iter1->e.body.brackets_depth == initial_brackets_depth) {
        if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "]") != 0) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
            "An internal CTQG error occurred: "
            "a symbol other than `]' decreases brackets depth (3).\n");
          clean_exit(-2);
        }
        if (num_tokens >= MAX_NUM_TOKENS_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Expression in brackets has too many tokens on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          clean_exit(-1);
        }
        strcpy(tmp[num_tokens++], ")");
        move_iter_and_delete_to_next_non_separ_token(&t_iter1);
        break;
      } else {
        if (h_TOKEN_strlen(t_iter1->e.body.value) >= MAX_TOKEN_SIZE_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Token in brackets is too long on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          clean_exit(-1);
        }
        if (num_tokens >= MAX_NUM_TOKENS_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Expression in brackets has too many tokens on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          clean_exit(-1);
        }
        h_TOKEN_strcpy_to_prealloc_sh(tmp[num_tokens++], t_iter1->e.body.value);
        move_iter_and_delete_to_next_non_separ_token(&t_iter1);
      }
    }

    for (i = 0; i < num_tokens; i++) {
      j = 0;
      while (tmp[i][j] != '\0') {
        str[pos_in_str++] = tmp[i][j];
        j++;
      }
    }
    str[pos_in_str++] = '\0';
    h_TOKEN_alloc_strcpy_hs(&rv, str);
  } else {
    t_iter1 = save_iter;
    rv = NULL;
  }

  return rv;
}



void /* t_iter1 ->`['. PTR,PTR - range, PTR,NULL - value, NULL,NULL - fail */
read_range_in_brackets__alloc( /* If fails, t_iter1 does not mv. Rmvs [ ]. */
str_on_heap_TOKEN * target_lo, str_on_heap_TOKEN * target_hi) {
  static char       tmp[MAX_NUM_TOKENS_IN_EXPR][MAX_TOKEN_SIZE_IN_EXPR];
  static char       str[MAX_NUM_TOKENS_IN_EXPR * MAX_TOKEN_SIZE_IN_EXPR];
  uint64_t          num_tokens = 0;
  uint64_t          pos_in_str = 0;
  iter_sdll_TOKEN   save_iter = t_iter1;
  uint32_t          initial_brackets_depth;
  uint32_t          i;
  uint32_t          j;

  if (t_iter1 == NULL) {
    t_iter1 = save_iter;
    *target_lo = NULL;
    *target_hi = NULL;
  } else
  if ( (t_iter1->e.body.type == BRACKETS______TT)
       &&
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "[") == 0) ) {
    initial_brackets_depth = t_iter1->e.body.brackets_depth;
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

    while (1 != 0) {
      if (t_iter1 == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Unterminated expression in brackets starts on line %s:%d.\n",
                ctqg_file_name, t_iter1->e.body.line);
        clean_exit(-1);
      }
      if (t_iter1->e.body.brackets_depth == initial_brackets_depth) {
        if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "]") != 0) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
            "An internal CTQG error occurred: "
            "a symbol other than `]' decreases brackets depth (4).\n");
          h_TOKEN_printf_str(t_iter1->e.body.value, 0, ' ', 'l');
          printf("\n");
          clean_exit(-2);
        }
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
        for (i = 0; i < num_tokens; i++) {
          j = 0;
          while (tmp[i][j] != '\0') {
            str[pos_in_str++] = tmp[i][j];
            j++;
          }
        }
        str[pos_in_str++] = '\0';
        h_TOKEN_alloc_strcpy_hs(target_lo, str);
        *target_hi = NULL;
        return;
      } else
      if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "..") == 0) {
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
        for (i = 0; i < num_tokens; i++) {
          j = 0;
          while (tmp[i][j] != '\0') {
            str[pos_in_str++] = tmp[i][j];
            j++;
          }
        }
        str[pos_in_str++] = '\0';
        h_TOKEN_alloc_strcpy_hs(target_lo, str);
        break;
      } else
      {
        if (h_TOKEN_strlen(t_iter1->e.body.value) >= MAX_TOKEN_SIZE_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Token in brackets is too long on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          clean_exit(-1);
        }
        if (num_tokens >= MAX_NUM_TOKENS_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Expression in brackets has too many tokens on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          clean_exit(-1);
        }
        h_TOKEN_strcpy_to_prealloc_sh(tmp[num_tokens++], t_iter1->e.body.value);
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      }
    }
    /* If reached this line - `target_lo' is set, t_iter1 points to */
    /* the first token of upper bound.                              */
    num_tokens = 0;
    pos_in_str = 0;
    while (1 != 0) {
      if (t_iter1 == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Unterminated expression in brackets starts on line %s:%d.\n",
                ctqg_file_name, t_iter1->e.body.line);
        h_TOKEN_str_delete(*target_lo);
        clean_exit(-1);
      }
      if (t_iter1->e.body.brackets_depth == initial_brackets_depth) {
        if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "]") != 0) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
            "An internal CTQG error occurred: "
            "a symbol other than `]' decreases brackets depth (5).\n");
          h_TOKEN_str_delete(*target_lo);
          clean_exit(-2);
        }
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
        for (i = 0; i < num_tokens; i++) {
          j = 0;
          while (tmp[i][j] != '\0') {
            str[pos_in_str++] = tmp[i][j];
            j++;
          }
        }
        str[pos_in_str++] = '\0';
        h_TOKEN_alloc_strcpy_hs(target_hi, str);
        return;
      } else
      {
        if (h_TOKEN_strlen(t_iter1->e.body.value) >= MAX_TOKEN_SIZE_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Token in brackets is too long on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          h_TOKEN_str_delete(*target_lo);
          clean_exit(-1);
        }
        if (num_tokens >= MAX_NUM_TOKENS_IN_EXPR) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Expression in brackets has too many tokens on line %s:%d.\n",
                  ctqg_file_name, t_iter1->e.body.line);
          h_TOKEN_str_delete(*target_lo);
          clean_exit(-1);
        }
        h_TOKEN_strcpy_to_prealloc_sh(tmp[num_tokens++], t_iter1->e.body.value);
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      }
    }
  } else {
    t_iter1 = save_iter;
    *target_lo = NULL;
    *target_hi = NULL;
  }
}



void
delete_st_param_array(
void) {
  uint64_t i;

  for (i = 0; i < st_param_array_num_entries; i++) {
    if (st_param_array[i] == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
        "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
      clean_exit(-2);
    }
    h_TOKEN_str_delete(st_param_array[i]);
    st_param_array[i] = NULL;
  }

  st_param_array_num_entries = 0;
}



void
debug_print_st_param_array(
void) {
  uint64_t i;

  for (i = 0; i < st_param_array_num_entries; i++) {
    if (st_param_array[i] == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
        "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
      clean_exit(-2);
    }
    h_TOKEN_printf_str(st_param_array[i], 0, ' ', 'l');
    printf("  ");
  }
}



void /* l(0) := 0; l(1) := 1 l(2)=l(3):= 2 */
binary_unsigned_a_eq_a_plus_b(
uint32_t * la, char * a, uint32_t lb, char * b) {
  uint32_t i;
  uint32_t new_la;
  char     carry = '0';
  char     temp;

  if (*la > lb) new_la = *la + 1; else new_la = lb + 1;
  if (new_la > MAX_NUM_BIN_DIGITS_IN_CONST) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "An integer or a binary string is too long "
            "on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  for (i = *la; i < new_la; i++) a[i] = '0';

  for (i = 0; i < lb; i++) {
    temp = a[i] + b[i] + carry;
    a[i] = '0' + (temp & 0x1);
    carry = ( (temp == (char) ('0' + '1' + '1'))
              ||
              (temp == (char) ('1' + '1' + '1')) ) ? '1' : '0';
  }

  for (     ; i < new_la; i++) {
    temp = a[i] + carry;
    a[i] = '0' + (temp & 0x1);
    carry = (  temp == (char) (       '1'+ '1')  ) ? '1' : '0';
  }

  while ( (new_la > 0) && (a[new_la - 1] == '0') ) new_la--;
  *la = new_la;
  a[*la] = '\0';
}



void
binary_mult_by_10(
uint32_t * l, char * b) {
  static char bc[MAX_NUM_BIN_DIGITS_IN_CONST + 1];
  uint32_t    bcl;
  uint32_t    i;

  bcl = *l + 3;
  if (bcl > MAX_NUM_BIN_DIGITS_IN_CONST) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "An integer or a binary string is too long "
            "on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  for (i = 3; i < bcl; i++) {
    bc[i] = b[i - 3];
  }
  bc[0] = '0';
  bc[1] = '0';
  bc[2] = '0';
  bc[bcl] = '\0';

  (*l)++;
  for (i = *l; i > 0; i--) {
    b[i] = b[i - 1];
  }
  b[0] = '0';
  binary_unsigned_a_eq_a_plus_b(l, b, bcl, bc);
}



void
h_to_binary_s__as_signed(
str_on_heap_TOKEN source, char * target) {
  static char dc[MAX_NUM_BIN_DIGITS_IN_CONST + 1];
  char *      unsgnd;
  int32_t     i;
  int32_t     l;
  uint32_t    tl;

  if (h_TOKEN_strlen(source) > MAX_NUM_BIN_DIGITS_IN_CONST) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "An integer or a binary string is too long "
            "on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  h_TOKEN_strcpy_to_prealloc_sh(dc, source);
  if (dc[0] == '-') unsgnd = &(dc[1]); else unsgnd = &(dc[0]);
  l = strlen(unsgnd);
  strcpy(target, ""); tl = 0;
  for (i = 0; i < l; i++) {
    binary_mult_by_10(&tl, target);
    switch (unsgnd[i]) {
      case '0':
        /* Do nothing. */
      break;
      case '1':
        binary_unsigned_a_eq_a_plus_b(&tl, target, 1, "1");
      break;
      case '2':
        binary_unsigned_a_eq_a_plus_b(&tl, target, 2, "01");
      break;
      case '3':
        binary_unsigned_a_eq_a_plus_b(&tl, target, 2, "11");
      break;
      case '4':
        binary_unsigned_a_eq_a_plus_b(&tl, target, 3, "001");
      break;
      case '5':
        binary_unsigned_a_eq_a_plus_b(&tl, target, 3, "101");
      break;
      case '6':
        binary_unsigned_a_eq_a_plus_b(&tl, target, 3, "011");
      break;
      case '7':
        binary_unsigned_a_eq_a_plus_b(&tl, target, 3, "111");
      break;
      case '8':
        binary_unsigned_a_eq_a_plus_b(&tl, target, 4, "0001");
      break;
      case '9':
        binary_unsigned_a_eq_a_plus_b(&tl, target, 4, "1001");
      break;
      default:
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Invalid character in decimal integer "
                "on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
    }
  }

  if (tl == 0) {
    strcpy(target, "0");
  } else
  if (dc[0] == '-') {
    for (i = 0; i < tl; i++) {
      if (target[i] == '0') target[i] = '1'; else target[i] = '0';
    }
    target[tl] = '1';
    tl++;
    if (tl > MAX_NUM_BIN_DIGITS_IN_CONST) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "An integer or a binary string is too long "
              "on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
    target[tl] = '\0';
    binary_unsigned_a_eq_a_plus_b(&tl, target, 1, "1");
  } else {
    target[tl] = '0';
    tl++;
    if (tl > MAX_NUM_BIN_DIGITS_IN_CONST) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "An integer or a binary string is too long "
              "on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
    target[tl] = '\0';
  }
}



void
ctqg_to_nc(
void) {
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  run_automaton(ctqg_to_nc_st_not_inside_comment);
  t_iter1 = NULL;
  t_iter2 = NULL;
}



void
nc_to_lf(
void) {
  TOKEN *  ptr_to_new_token;
  uint32_t bracket_depth;
  int      is_first_signal = (1 != 0);

  if ( (t_iter1 != NULL) || (t_iter2 != NULL) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  while (1 != 0) {
    t_iter2 = t_iter1;
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Main module is missing. `%s main_module' must be present "
              "somewhere in the program but was not found (in file \"%s\"). "
              "Note: nothing except for space and end-of-line can be "
              "placed between `%s' and `main_module'.\n",
              KEYWORD_MODULE, ctqg_file_name, KEYWORD_MODULE);
      clean_exit(-1);
    }
    if ( (h_TOKEN_strcmp_hs(t_iter2->e.body.value, KEYWORD_MODULE) == 0)
         &&
         (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "main_module") == 0) ) {
      break;
    }
  };

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "\n");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = END_OF_LINE___TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "/*yQdupmm*/");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = END_OF_LINE___TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "\n");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = END_OF_LINE___TT;

  while ( (t_iter2 != NULL)
          &&
          (h_TOKEN_strcmp_hs(t_iter2->e.body.value, "{") != 0)
          &&
          (h_TOKEN_strcmp_hs(t_iter2->e.body.value, "/*yQdupmm*/") != 0) ) {
    new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
    h_TOKEN_alloc_strcpy_hh(&(ptr_to_new_token->value), t_iter2->e.body.value);
    ptr_to_new_token->line = 999999999;
    ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
    ptr_to_new_token->type = t_iter2->e.body.type;
    move_iter_right_sdll_TOKEN(t_iter2);
  }

  if ( (t_iter2 == NULL)
       ||
       (h_TOKEN_strcmp_hs(t_iter2->e.body.value, "/*yQdupmm*/") == 0) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Syntax error in `%s main_module' in file \"%s\".\n",
            KEYWORD_MODULE, ctqg_file_name);
    clean_exit(-1);
  }

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hh(&(ptr_to_new_token->value), t_iter2->e.body.value);
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
  ptr_to_new_token->type = t_iter2->e.body.type;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "\n");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = END_OF_LINE___TT;

  /* Change original `main_module' to `ImainImoduleI' */
  h_TOKEN_str_delete(t_iter1->e.body.value);
  h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value), "ImainImoduleI");

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "  ");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = SPACE_________TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "$");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = STDALN_SYM____TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), " ");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = SPACE_________TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "ImainImoduleI");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "(");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = STDALN_SYM____TT;

  while (1 != 0) {
    bracket_depth = t_iter1->e.body.brackets_depth;
    move_iter_right_sdll_TOKEN(t_iter1)
    while (t_iter1 != NULL) {
      if ( (t_iter1->e.body.brackets_depth == bracket_depth)
           &&
           (
            (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_BIT) == 0)
            ||
            (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_INT) == 0)
           ) ) {
        break;
      }
      if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, "{") == 0 ) {
        t_iter1 = NULL;
        break;
      }
      move_iter_right_sdll_TOKEN(t_iter1)
    }
    if (t_iter1 == NULL) break;
    bracket_depth = t_iter1->e.body.brackets_depth;
    move_iter_right_sdll_TOKEN(t_iter1)
    while (t_iter1 != NULL) {
      if ( (t_iter1->e.body.brackets_depth == bracket_depth)
           &&
           (t_iter1->e.body.type == ID____________TT) ) {
        break;
      }
      if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, "{") == 0 ) {
        t_iter1 = NULL;
        break;
      }
      move_iter_right_sdll_TOKEN(t_iter1)
    }
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Syntax error in signals declaration "
              "of `%s main_module' in file \"%s\".\n",
              KEYWORD_MODULE, ctqg_file_name);
      clean_exit(-1);
    }
    if (!is_first_signal) {
      new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
      h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), ",");
      ptr_to_new_token->line = 999999999;
      ptr_to_new_token->brackets_depth = 0;
      ptr_to_new_token->type = STDALN_SYM____TT;
      new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
      h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), " ");
      ptr_to_new_token->line = 999999999;
      ptr_to_new_token->brackets_depth = 0;
      ptr_to_new_token->type = SPACE_________TT;
    }
    new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
    h_TOKEN_alloc_strcpy_hh(&(ptr_to_new_token->value), t_iter1->e.body.value);
    ptr_to_new_token->line = 999999999;
    ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
    ptr_to_new_token->type = t_iter1->e.body.type;
    is_first_signal = (0 != 0);
  }

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), ")");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = STDALN_SYM____TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), ";");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = STDALN_SYM____TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "\n");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = END_OF_LINE___TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "}");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = STDALN_SYM____TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "\n\n");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = END_OF_LINE___TT;

  t_iter1 = NULL;
  t_iter2 = NULL;
}



void
lf_to_le(
void) {
  if ( (t_iter1 != NULL) || (t_iter2 != NULL) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  run_automaton(f_st_outside_module_body);
}



void
le_to_ld(
void) {
  add_var_declarations_from_built_in_modules();

  if ( (t_iter1 != NULL) || (t_iter2 != NULL) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  run_automaton(e_st_create_var_decl_sgt);

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  run_automaton(e_st_inside_body);
}



void
ld_to_bbrl(
void) {
  add_var_declarations_from_built_in_modules();

  if ( (t_iter1 != NULL) || (t_iter2 != NULL) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  d_insert_add_def_mod();
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  run_automaton(d_st_inside_body_not_inside_ancilla_decl);
}



void
run_automaton(
STATE_OF_AUTOMATON_func_ptr_t initial_state) {
  STATE_OF_AUTOMATON_func_ptr_t current_state = initial_state;
  while (current_state != termination_state) {
    current_state = (STATE_OF_AUTOMATON_func_ptr_t) current_state();
  }
}



void*
termination_state(
void) {
  fprintf(stderr, "Error.\n");
  fprintf(stderr,
    "An internal CTQG error occurred: an attempt to execute "
    "function `termination_state'. The only role of this "
    "function is to provide a constant pointer to a void* of void function.\n");
  clean_exit(-2);
  return NULL;
}



void*
d_st_inside_body_not_inside_ancilla_decl(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;

  if (t_iter1 == NULL) {
    rv = d_st_pre_termination;
  } else
  if (t_iter1->e.body.type == ID____________TT) {
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_MODULE) == 0) {
      if (is_nonempty_sdll_TOKEN(d_st_param_list)) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "`%s' keyword encountered on line %s:%d but the previous "
                "module definition is not complete.\n",
                KEYWORD_MODULE, ctqg_file_name, t_iter1->e.body.line);
        clean_exit(-1);
      }
      h_TOKEN_str_delete(t_iter1->e.body.value);
      h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value), "void ");
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      delete_st_param_array();
      rv = d_st_read_param_list_and_module_name;
    } else
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_ZTG_ANC) == 0) {
      move_iter_and_delete_to_next_non_separ_token(&t_iter1);
      rv = d_st_read_ztg_declaration;
    } else
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_OTG_ANC) == 0) {
      move_iter_and_delete_to_next_non_separ_token(&t_iter1);
      rv = d_st_read_otg_declaration;
    } else
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_ZTZ_ANC) == 0) {
      move_iter_and_delete_to_next_non_separ_token(&t_iter1);
      rv = d_st_read_ztz_declaration;
    } else
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_OTO_ANC) == 0) {
      move_iter_and_delete_to_next_non_separ_token(&t_iter1);
      rv = d_st_read_oto_declaration;
    } else
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "INSTANTIATE_MODULE") == 0) {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = d_st_read_instantiate_module_keyword;
    } else
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "LOCATION_INFO") == 0) {
      st_callee_name = NULL;
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = d_st_inside_body_not_inside_ancilla_decl;
    } else
    {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = d_st_inside_body_not_inside_ancilla_decl;
    }
  } else
  if (t_iter1->e.body.type == DECIMAL_CONST_TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = d_st_inside_body_not_inside_ancilla_decl;
  } else
  if (t_iter1->e.body.type == SPACE_________TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  } else
  if (t_iter1->e.body.type == PERIODS_______TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = d_st_inside_body_not_inside_ancilla_decl;
  } else
  if (t_iter1->e.body.type == OPERATOR______TT) {
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "%%") == 0) {
      move_iter_and_delete_to_next_non_separ_token(&t_iter1);
      rv = d_st_convert_signal_of_callee;
    } else {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = d_st_inside_body_not_inside_ancilla_decl;
    }
  } else
  if (t_iter1->e.body.type == END_OF_LINE___TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  } else
  if (t_iter1->e.body.type == BRACKETS______TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = d_st_inside_body_not_inside_ancilla_decl;
  } else
  if (t_iter1->e.body.type == STDALN_SYM____TT) {
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "@") == 0) {
      move_iter_and_delete_to_next_non_separ_token(&t_iter1);
      rv = d_st_convert_local_signal;
    } else {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = d_st_inside_body_not_inside_ancilla_decl;
    }
  } else {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  return (void*) rv;
}



void*
d_st_read_param_list_and_module_name(
void) {
  TOKEN * ptr_to_new_token;
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;

  /* `d_st_param_list' is empty at this moment. */

  while (1 != 0) {
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file `%s'.\n", ctqg_file_name);
      clean_exit(-1);
    } else
    if (t_iter1->e.body.type == ID____________TT) {
      /* The list of parameters has been read to `d_st_param_list' */
      /* and the corresponing tokens have been erased or moved.    */
      /* `t_iter1' points to the name of module token.*/
      st_module_name = t_iter1->e.body.value;
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = d_st_read_declarations_of_signals;
      break;
    } else
    if ( (t_iter1->e.body.type == OPERATOR______TT)
         &&
         (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "<") == 0) ) {
      t_iter2 = t_iter1;
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      delete_TOKEN_and_move_iter_to_right(&t_iter2, &tokens);
      if ( (t_iter1 == NULL) || (t_iter1->e.body.type != ID____________TT) ) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Name of parameter expected after `<' on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, d_st_param_list)
      h_TOKEN_alloc_strcpy_hh(
        &(ptr_to_new_token->value), t_iter1->e.body.value);
      ptr_to_new_token->line = 0;
      ptr_to_new_token->brackets_depth = 0;
      ptr_to_new_token->type = ID____________TT;

      if (st_param_array_num_entries >= MAX_NUM_PARAMETERS) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Module has too many parameters "
                "(MAX_NUM_PARAMETERS = %u) on line %s:%d.\n",
                MAX_NUM_PARAMETERS,
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      h_TOKEN_alloc_strcpy_hh(
              &(st_param_array[st_param_array_num_entries]),
              t_iter1->e.body.value);
      st_param_array_num_entries++;

      t_iter2 = t_iter1;
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      delete_TOKEN_and_move_iter_to_right(&t_iter2, &tokens);
      if ( (t_iter1 == NULL)
           ||
           (t_iter1->e.body.type != OPERATOR______TT)
           ||
           (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ">") != 0) ) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "`>' expected after name of parameter on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      t_iter2 = t_iter1;
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      delete_TOKEN_and_move_iter_to_right(&t_iter2, &tokens);
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Name of module or `<name_of_param>' expected on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
  }

  return (void*) rv;
}



void*
d_st_read_declarations_of_signals(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  TOKEN *                       ptr_to_new_token;
  iter_sdll_TOKEN               iter;
  var_decl_value_t *            ptr_to_new_var_decl;
  str_on_heap_TOKEN             str;
  str_on_heap_TOKEN             subst_param_str;
  char                          s[64]; /* Large enough to hold any dec. int. */
  uint32_t                      i;

  if ( (t_iter1 == NULL)
       ||
       (t_iter1->e.body.type != STDALN_SYM____TT)
       ||
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "(") != 0) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`(' expected after name of module on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value), "MANDATORY_ARGS");
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;
  move_iter_right_sdll_TOKEN(t_iter1)

  while (is_nonempty_sdll_TOKEN(d_st_param_list)) {
    set_iter_to_leftmost_el_sdll_TOKEN(iter, d_st_param_list)

    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter1, tokens)
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value), ",");
    ptr_to_new_token->line = t_iter1->e.body.line;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = STDALN_SYM____TT;
    move_iter_right_sdll_TOKEN(t_iter1)

    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter1, tokens)
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value), " ");
    ptr_to_new_token->line = t_iter1->e.body.line;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = SPACE_________TT;
    move_iter_right_sdll_TOKEN(t_iter1)

    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter1, tokens)
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value), "int64_t");
    ptr_to_new_token->line = t_iter1->e.body.line;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = ID____________TT;
    move_iter_right_sdll_TOKEN(t_iter1)

    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter1, tokens)
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value), " ");
    ptr_to_new_token->line = t_iter1->e.body.line;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = SPACE_________TT;
    move_iter_right_sdll_TOKEN(t_iter1)

    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter1, tokens)
    ptr_to_new_token->value = iter->e.body.value;
    ptr_to_new_token->line = t_iter1->e.body.line;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = ID____________TT;
    move_iter_right_sdll_TOKEN(t_iter1)

    delete_leftmost_sdll_TOKEN(d_st_param_list)
  }
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  /* t_iter1 -> 1st symb of 1st decl. Process declarations of signals. */
  while (1 != 0) {
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file `%s'.\n", ctqg_file_name);
      clean_exit(-1);
    } else
    if ( (t_iter1->e.body.type == ID____________TT)
         &&
         (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_BIT) == 0) ) {
      new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
        ptr_to_new_token, t_iter1, tokens)
      h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "/* ");
      ptr_to_new_token->line = t_iter1->e.body.line;
      ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
      ptr_to_new_token->type = OPERATOR______TT;
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      if ( (t_iter1 == NULL)
           ||
           (t_iter1->e.body.type != ID____________TT) ) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Identifier expected after `%s' on line %s:%d.\n",
                KEYWORD_BIT,
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      ptr_to_new_var_decl = new_var_decl(
                              st_module_name, t_iter1->e.body.value);
      if (ptr_to_new_var_decl == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "Duplicate identifier `");
        h_TOKEN_fprintf_str(stderr, t_iter1->e.body.value, 0, ' ', 'l');
        fprintf(stderr,
                "' on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      ptr_to_new_var_decl->signal_type = 1 /* regular bit */;
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      while (1 != 0) {
        if (t_iter1 == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Unexpected end of file after declaration on line %s:%d.\n",
                  ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
          clean_exit(-1);
        }
        if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0 ) {
          new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
            ptr_to_new_token, t_iter1, tokens)
          h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), " */ ");
          ptr_to_new_token->line = t_iter1->e.body.line;
          ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
          ptr_to_new_token->type = OPERATOR______TT;
          break;
        } else
        if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ",") == 0 ) {
          new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
            ptr_to_new_token, t_iter1, tokens)
          h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), " */ ");
          ptr_to_new_token->line = t_iter1->e.body.line;
          ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
          ptr_to_new_token->type = OPERATOR______TT;
          to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
          break;
        }
        iter = t_iter1;
        str = read_expression_in_brackets__alloc();
        if (str == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Expression in brackets expected on line %s:%d.\n",
                  ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
          clean_exit(-1);
        }
        add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, str);
        t_iter1 = iter;
        str = read_expression_in_brackets_subst_param__alloc();
        if (str == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
            "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
          clean_exit(-2);
        }
        ptr_to_new_var_decl->subst_indexes[
          ptr_to_new_var_decl->num_indexes - 1
        ] = str;
      }
      if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0 ) {
        break; /* Finished reading declarations. */
      }
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    } else
    if ( (t_iter1->e.body.type == ID____________TT)
         &&
         (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_INT) == 0) ) {
      new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
        ptr_to_new_token, t_iter1, tokens)
      h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "/* ");
      ptr_to_new_token->line = t_iter1->e.body.line;
      ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
      ptr_to_new_token->type = OPERATOR______TT;
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

      iter = t_iter1;
      str = read_expression_in_brackets__alloc();
      if (str == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Expression in brackets expected after `%s' on line %s:%d.\n",
                KEYWORD_INT,
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      t_iter1 = iter;
      subst_param_str = read_expression_in_brackets_subst_param__alloc();

      if ( (t_iter1 == NULL)
           ||
           (t_iter1->e.body.type != ID____________TT) ) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Identifier expected after `%s[...]' on line %s:%d.\n",
                KEYWORD_INT,
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        h_TOKEN_str_delete(str);
        h_TOKEN_str_delete(subst_param_str);
        clean_exit(-1);
      }

      ptr_to_new_var_decl = new_var_decl(
                              st_module_name, t_iter1->e.body.value);
      if (ptr_to_new_var_decl == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "Duplicate identifier `");
        h_TOKEN_fprintf_str(stderr, t_iter1->e.body.value, 0, ' ', 'l');
        fprintf(stderr,
                "' on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        h_TOKEN_str_delete(str);
        h_TOKEN_str_delete(subst_param_str);
        clean_exit(-1);
      }
      ptr_to_new_var_decl->signal_type = 0 /* regular int */;
      /* The value of `str' was set approx 35 lines above. Now use it. */
      add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, str);
      if (subst_param_str == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
          "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
        clean_exit(-2);
      }
      ptr_to_new_var_decl->subst_indexes[
        ptr_to_new_var_decl->num_indexes - 1
      ] = subst_param_str;
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

      while (1 != 0) {
        if (t_iter1 == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Unexpected end of file after declaration on line %s:%d.\n",
                  ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
          clean_exit(-1);
        }
        if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0 ) {
          new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
            ptr_to_new_token, t_iter1, tokens)
          h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), " */ ");
          ptr_to_new_token->line = t_iter1->e.body.line;
          ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
          ptr_to_new_token->type = OPERATOR______TT;
          break;
        } else
        if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ",") == 0 ) {
          new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
            ptr_to_new_token, t_iter1, tokens)
          h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), " */ ");
          ptr_to_new_token->line = t_iter1->e.body.line;
          ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
          ptr_to_new_token->type = OPERATOR______TT;
          to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
          break;
        }
        iter = t_iter1;
        str = read_expression_in_brackets__alloc();
        if (str == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Expression in brackets expected on line %s:%d.\n",
                  ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
          clean_exit(-1);
        }
        add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, str);
        t_iter1 = iter;
        str = read_expression_in_brackets_subst_param__alloc();
        if (str == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
            "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
          clean_exit(-2);
        }
        ptr_to_new_var_decl->subst_indexes[
          ptr_to_new_var_decl->num_indexes - 1
        ] = str;
      }
      /* Rotate indexes to the left by 1, e.g [0][1][2] -> [1][2][0] */
      if (ptr_to_new_var_decl->num_indexes >= 2) {
        str = ptr_to_new_var_decl->indexes[0];
        subst_param_str = ptr_to_new_var_decl->subst_indexes[0];
        for (i = 0; i < ptr_to_new_var_decl->num_indexes - 1; i++) {
          ptr_to_new_var_decl->indexes[i] = ptr_to_new_var_decl->indexes[i + 1];
          ptr_to_new_var_decl->subst_indexes[i] =
            ptr_to_new_var_decl->subst_indexes[i + 1];
        }
        ptr_to_new_var_decl->indexes[
          ptr_to_new_var_decl->num_indexes - 1
        ] = str;
        ptr_to_new_var_decl->subst_indexes[
          ptr_to_new_var_decl->num_indexes - 1
        ] = subst_param_str;
      }

      if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0 ) {
        break; /* Finished reading declarations. */
      }
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "`%s' or `%s' expected on line %s:%d.\n",
              KEYWORD_BIT, KEYWORD_INT,
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
  }

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  if ( (t_iter1 == NULL)
       ||
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "{") != 0) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Beginning of module body starting with `{' "
            "expected on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value),
    " BEGIN_DEFINITION_OF_MODULE uint64_t yQj; int64_t yQp[");
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  sprintf(s, "%u", MAX_NUM_PARAMETERS);
  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value), s);
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
  ptr_to_new_token->type = DECIMAL_CONST_TT;
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value),
    "]; ");
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  rv = d_st_inside_body_not_inside_ancilla_decl;

  return (void*) rv;
}



void*
d_st_pre_termination(
void) {
  d_create__main_var_decl_table_and_finalize_token_list();

  delete_st_param_array();
  delete_st_var_decl_sgt();
  st_var_decl_num_entries = 0;
  delete_sdll_TOKEN(&d_st_param_list);
  st_callee_name = NULL; /* Does not own memory. */
  st_module_name = NULL; /* Does not own memory. */
  t_iter1 = NULL;
  t_iter2 = NULL;

  return (void*) termination_state;
}



void*
d_st_read_ztg_declaration(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  d_read_ancilla_declaration(2);
  rv = d_st_inside_body_not_inside_ancilla_decl;
  return (void*) rv;
}



void*
d_st_read_otg_declaration(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  d_read_ancilla_declaration(3);
  rv = d_st_inside_body_not_inside_ancilla_decl;
  return (void*) rv;
}



void*
d_st_read_ztz_declaration(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  d_read_ancilla_declaration(4);
  rv = d_st_inside_body_not_inside_ancilla_decl;
  return (void*) rv;
}



void*
d_st_read_oto_declaration(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  d_read_ancilla_declaration(5);
  rv = d_st_inside_body_not_inside_ancilla_decl;
  return (void*) rv;
}



void*
d_st_read_instantiate_module_keyword(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  int32_t                       i;
  TOKEN *                       ptr_to_new_token;
  iter_sdll_TOKEN               temp_t_iter;
  char                          s[64]; /* Large enough to hold any dec. int. */

  t_iter2 = t_iter1;
  while (1 != 0) {
    if (t_iter2 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unterminated `INSTANTIATE_MODULE' block on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
    if  ( h_TOKEN_strcmp_hs(t_iter2->e.body.value, "LOCATION_INFO") == 0 ) {
      break;
    }
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter2);
  }

  to_previous_non_separ_token_treating_NULL_as_rightmost(&t_iter2);
  if ( (t_iter2 == NULL)
       ||
       (h_TOKEN_strcmp_hs(t_iter2->e.body.value, "(" ) != 0) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`(' expected before `LOCATION_INFO' on line %s:%d.\n",
            ctqg_file_name, (t_iter2) ? t_iter2->e.body.line : 999999999);
    clean_exit(-1);
  }

  to_previous_non_separ_token_treating_NULL_as_rightmost(&t_iter2);
  if ( (t_iter2 == NULL)
       ||
       (t_iter2->e.body.type != ID____________TT) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "A name of module expected before `(LOCATION_INFO' "
            "on line %s:%d.\n",
            ctqg_file_name, (t_iter2) ? t_iter2->e.body.line : 999999999);
    clean_exit(-1);
  }

  st_callee_name = t_iter2->e.body.value;

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter2);
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter2);
  /* Now `t_iter2' again points to `LOCATION_INFO'. */
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter2);
  if ( (t_iter2 == NULL)
       ||
       ((h_TOKEN_strcmp_hs(t_iter2->e.body.value, ",") != 0)
        &&
        (h_TOKEN_strcmp_hs(t_iter2->e.body.value, ")") != 0)
       ) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`,' or `(' expected after `LOCATION_INFO' on line %s:%d.\n",
            ctqg_file_name, t_iter2->e.body.line);
    clean_exit(-1);
  }
  temp_t_iter = t_iter1;
  to_previous_non_separ_token_treating_NULL_as_rightmost(&temp_t_iter);
  move_iter_right_sdll_TOKEN(temp_t_iter);
  i = 0;
  while ( (t_iter2 != NULL)
          &&
          (h_TOKEN_strcmp_hs(t_iter2->e.body.value, ",") == 0) ) {
    if (i >= MAX_NUM_PARAMETERS) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Number of parameters exceeds "
              "the limit (MAX_NUM_PARAMETERS = %u) on line %s:%d.\n",
              MAX_NUM_PARAMETERS,
              ctqg_file_name, t_iter2->e.body.line);
      clean_exit(-1);
    }
    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, temp_t_iter, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), " yQp[");
    ptr_to_new_token->line = temp_t_iter->e.body.line;
    ptr_to_new_token->brackets_depth = temp_t_iter->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;

    sprintf(s, "%u", i);
    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, temp_t_iter, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), s);
    ptr_to_new_token->line = temp_t_iter->e.body.line;
    ptr_to_new_token->brackets_depth = temp_t_iter->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;

    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, temp_t_iter, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "] = ");
    ptr_to_new_token->line = temp_t_iter->e.body.line;
    ptr_to_new_token->brackets_depth = temp_t_iter->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;

    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter2);
    while (1 != 0) {
      if (t_iter2 == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Unexpected end of file "
                "on line %s:%d.\n",
                ctqg_file_name, (t_iter2) ? t_iter2->e.body.line : 999999999);
        clean_exit(-1);
      }
      if ( (h_TOKEN_strcmp_hs(t_iter2->e.body.value, ",") == 0)
           ||
           (h_TOKEN_strcmp_hs(t_iter2->e.body.value, ")") == 0) ) {
        break;
      }
      new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
        ptr_to_new_token, temp_t_iter, tokens)
      h_TOKEN_alloc_strcpy_hh(
        &(ptr_to_new_token->value),
        t_iter2->e.body.value);
      ptr_to_new_token->line = temp_t_iter->e.body.line;
      ptr_to_new_token->brackets_depth = temp_t_iter->e.body.brackets_depth;
      ptr_to_new_token->type = ID____________TT;
      move_iter_and_delete_to_next_non_separ_token(&t_iter2);
    }

    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, temp_t_iter, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), ";");
    ptr_to_new_token->line = temp_t_iter->e.body.line;
    ptr_to_new_token->brackets_depth = temp_t_iter->e.body.brackets_depth;
    ptr_to_new_token->type = STDALN_SYM____TT;

    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter2, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "yQp[");
    ptr_to_new_token->line = t_iter2->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;

    /* sprintf(s, "%u", i); `i' is already printed to `s' */
    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter2, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), s);
    ptr_to_new_token->line = t_iter2->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;

    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter2, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "]");
    ptr_to_new_token->line = t_iter2->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;

    i++;
  }

  rv = d_st_inside_body_not_inside_ancilla_decl;
  return (void*) rv;
}



void*
d_st_convert_local_signal(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  str_on_heap_TOKEN             var_name; /* Does not own memory. */
  TOKEN *                       ptr_to_new_token;

  if ( (t_iter1 == NULL)
       ||
       (t_iter1->e.body.type != ID____________TT) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Identifier expected after `@' on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "\"");
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
  ptr_to_new_token->type = STDALN_SYM____TT;
  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "\"");
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
  ptr_to_new_token->type = STDALN_SYM____TT;
  var_name = t_iter1->e.body.value;
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), ", ");
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
  ptr_to_new_token->type = STDALN_SYM____TT;
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  if (st_module_name == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`@' encountered not inside module body on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  d_convert_id(st_module_name, var_name, 1 != 0 /* Native ID */);

  rv = d_st_inside_body_not_inside_ancilla_decl;
  return (void*) rv;
}



void*
d_st_convert_signal_of_callee(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  str_on_heap_TOKEN             var_name; /* Does not own memory. */
  TOKEN *                       ptr_to_new_token;

  if ( (t_iter1 == NULL)
       ||
       (t_iter1->e.body.type != ID____________TT) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Identifier expected after `%%' on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "\"");
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
  ptr_to_new_token->type = STDALN_SYM____TT;
  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "\"");
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
  ptr_to_new_token->type = STDALN_SYM____TT;
  var_name = t_iter1->e.body.value;
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), ", ");
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
  ptr_to_new_token->type = STDALN_SYM____TT;
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  if (st_callee_name == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`%%%%' encountered not inside INSTANTIATE_MODULE block "
            "on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  d_convert_id(st_callee_name, var_name, 0 != 0 /* Non-native ID */);

  rv = d_st_inside_body_not_inside_ancilla_decl;
  return (void*) rv;
}



void
d_convert_id(
str_on_heap_TOKEN name_of_module, str_on_heap_TOKEN name_of_var,
int is_native_id) {
  var_decl_value_t * ptr_to_var_decl;
  str_on_heap_TOKEN  expr_in_br;
  TOKEN *            ptr_to_new_token;
  int32_t            i;

  if (name_of_module == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  ptr_to_var_decl  = search_var_decl(name_of_module, name_of_var);

  if (ptr_to_var_decl == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,  "Undeclared variable `");
    h_TOKEN_fprintf_str(stderr, name_of_module, 0, ' ', 'l');
    fprintf(stderr,  "::");
    h_TOKEN_fprintf_str(stderr, name_of_var, 0, ' ', 'l');
    fprintf(stderr,
            "' on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  if ( (ptr_to_var_decl->signal_type >= 2)
       &&
       (!is_native_id) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,  "Ancilla signals of callee can not be mapped. Signal `");
    h_TOKEN_fprintf_str(stderr, name_of_module, 0, ' ', 'l');
    fprintf(stderr,  "::");
    h_TOKEN_fprintf_str(stderr, name_of_var, 0, ' ', 'l');
    fprintf(stderr,
            "' on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  expr_in_br = read_and_del_expression_in_brackets__alloc();

  if (expr_in_br == NULL) {
    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter1, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "0");
    ptr_to_new_token->line = t_iter1->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
    ptr_to_new_token->type = DECIMAL_CONST_TT;
    if (ptr_to_var_decl->num_indexes != 0) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,  "Variable `");
      h_TOKEN_fprintf_str(stderr, name_of_module, 0, ' ', 'l');
      fprintf(stderr,  "::");
      h_TOKEN_fprintf_str(stderr, name_of_var, 0, ' ', 'l');
      fprintf(stderr,
              "' requires %u indexes on line %s:%d.\n",
              ptr_to_var_decl->num_indexes,
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
  } else {
    for (i = ptr_to_var_decl->num_indexes - 1; i > 0; i--) {
      new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
        ptr_to_new_token, t_iter1, tokens)
      if (is_native_id) {
        h_TOKEN_alloc_strcpy_hh(
          &(ptr_to_new_token->value),
          ptr_to_var_decl->indexes[i]);
      } else {
        h_TOKEN_alloc_strcpy_hh(
          &(ptr_to_new_token->value),
          ptr_to_var_decl->subst_indexes[i]);
      }
      ptr_to_new_token->line = t_iter1->e.body.line;
      ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
      ptr_to_new_token->type = ID____________TT;
      new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
        ptr_to_new_token, t_iter1, tokens)
      h_TOKEN_alloc_strcpy_hs(
        &(ptr_to_new_token->value), " * (");
      ptr_to_new_token->line = t_iter1->e.body.line;
      ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
      ptr_to_new_token->type = OPERATOR______TT;
    }
    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter1, tokens)
    h_TOKEN_alloc_strcpy_hh(
      &(ptr_to_new_token->value),
      expr_in_br);
    ptr_to_new_token->line = t_iter1->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;

    i = ptr_to_var_decl->num_indexes - 1;
    while (1 != 0) {
      if (i == 0) break;
      h_TOKEN_str_delete(expr_in_br);
      expr_in_br = read_and_del_expression_in_brackets__alloc();
      if (expr_in_br == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,  "Variable `");
        h_TOKEN_fprintf_str(stderr, name_of_module, 0, ' ', 'l');
        fprintf(stderr,  "::");
        h_TOKEN_fprintf_str(stderr, name_of_var, 0, ' ', 'l');
        fprintf(stderr,
                "' requires %u indexes on line %s:%d.\n",
                ptr_to_var_decl->num_indexes,
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
        ptr_to_new_token, t_iter1, tokens)
      h_TOKEN_alloc_strcpy_hs(
        &(ptr_to_new_token->value), ") + ");
      ptr_to_new_token->line = t_iter1->e.body.line;
      ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
      ptr_to_new_token->type = OPERATOR______TT;
      new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
        ptr_to_new_token, t_iter1, tokens)
      h_TOKEN_alloc_strcpy_hh(
        &(ptr_to_new_token->value),
        expr_in_br);
      ptr_to_new_token->line = t_iter1->e.body.line;
      ptr_to_new_token->brackets_depth = t_iter1->e.body.brackets_depth;
      ptr_to_new_token->type = ID____________TT;
      i--;
    }
    h_TOKEN_str_delete(expr_in_br);
  }
}



void
d_read_ancilla_declaration(
uint32_t signal_type) {
  char *             keyword;
  str_on_heap_TOKEN  var_name;
  str_on_heap_TOKEN  expr_in_br;
  str_on_heap_TOKEN  expr_in_br2;
  uint32_t           i;
  var_decl_value_t * ptr_to_new_var_decl;
  TOKEN *            ptr_to_new_token;
  iter_sdll_TOKEN    iter;

  switch (signal_type) {
    case 2:
      keyword = KEYWORD_ZTG_ANC;
    break;
    case 3:
      keyword = KEYWORD_OTG_ANC;
    break;
    case 4:
      keyword = KEYWORD_ZTZ_ANC;
    break;
    case 5:
      keyword = KEYWORD_OTO_ANC;
    break;
    default:
      keyword = KEYWORD_OTO_ANC; /* Make `used uninitialized' warning  happy. */
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
        "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
      clean_exit(-2);
  }

  if ( (t_iter1 == NULL)
       ||
       (t_iter1->e.body.type != ID____________TT) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Identifier expected after `%s' on line %s:%d.\n",
            keyword,
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  h_TOKEN_alloc_strcpy_hh(&var_name, t_iter1->e.body.value);
  ptr_to_new_var_decl = new_var_decl(st_module_name, var_name);
  if (ptr_to_new_var_decl == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr, "Duplicate identifier `");
    h_TOKEN_fprintf_str(stderr, t_iter1->e.body.value, 0, ' ', 'l');
    fprintf(stderr,
            "' on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    h_TOKEN_str_delete(var_name);
    clean_exit(-1);
  }
  ptr_to_new_var_decl->signal_type = signal_type;
  move_iter_and_delete_to_next_non_separ_token(&t_iter1);
  while (1 != 0) {
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file after `%s' declaration on line %s:%d.\n",
              keyword,
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      h_TOKEN_str_delete(var_name);
      clean_exit(-1);
    }
    if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ";") == 0 ) {
      break;
    }
    iter = t_iter1;
    expr_in_br2 = read_expression_in_brackets_subst_param__alloc();
    if (expr_in_br2 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "In `%s' ancilla declaration "
              "an expression in brackets expected on line %s:%d.\n",
              keyword,
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      h_TOKEN_str_delete(var_name);
      clean_exit(-1);
    }
    t_iter1 = iter;
    expr_in_br = read_and_del_expression_in_brackets__alloc();
    if (expr_in_br == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
        "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
      clean_exit(-2);
    }
    add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, expr_in_br);
    ptr_to_new_var_decl->subst_indexes[
      ptr_to_new_var_decl->num_indexes - 1
    ] = expr_in_br2;
  }

  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value),
    "for (yQj = 0; yQj < 1ull");
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;
  delete_TOKEN_and_move_iter_to_right(&t_iter1, &tokens);

  for (i = 0; i < ptr_to_new_var_decl->num_indexes; i++) {
    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter1, tokens)
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value), " * ");
    ptr_to_new_token->line = t_iter1->e.body.line;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = OPERATOR______TT;
    move_iter_right_sdll_TOKEN(t_iter1)

    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter1, tokens)
    h_TOKEN_alloc_strcpy_hh(
      &(ptr_to_new_token->value), ptr_to_new_var_decl->indexes[i]);
    ptr_to_new_token->line = t_iter1->e.body.line;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = ID____________TT;
    move_iter_right_sdll_TOKEN(t_iter1)
  }

  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value), "; yQj++) ");
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;
  move_iter_right_sdll_TOKEN(t_iter1)

  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  switch (signal_type) {
    case 2:
        h_TOKEN_alloc_strcpy_hs(
          &(ptr_to_new_token->value), "ZERO_TO_GARBAGE_ANCILLA(\"");
    break;
    case 3:
        h_TOKEN_alloc_strcpy_hs(
          &(ptr_to_new_token->value), "ONE_TO_GARBAGE_ANCILLA(\"");
    break;
    case 4:
        h_TOKEN_alloc_strcpy_hs(
          &(ptr_to_new_token->value), "ZERO_TO_ZERO_ANCILLA(\"");
    break;
    case 5:
        h_TOKEN_alloc_strcpy_hs(
          &(ptr_to_new_token->value), "ONE_TO_ONE_ANCILLA(\"");
    break;
    default:
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
        "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
      clean_exit(-2);
  }
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;
  move_iter_right_sdll_TOKEN(t_iter1)

  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hh(&(ptr_to_new_token->value), var_name);
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;
  move_iter_right_sdll_TOKEN(t_iter1)

  new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter1, tokens)
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value), "\", yQj)");
  ptr_to_new_token->line = t_iter1->e.body.line;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;
  move_iter_right_sdll_TOKEN(t_iter1)

  h_TOKEN_str_delete(var_name);
}



void
d_insert_add_def_mod(
void) {
  int32_t  par_depth = 0;
  int      add_def_mod_already_inserted_for_this_module = (1 != 0);
  TOKEN *  ptr_to_new_token;
  while (1 != 0) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    if (t_iter1 == NULL) break;
    if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, "{") == 0 ) {
      par_depth++;
    } else
    if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, "}") == 0 ) {
      par_depth--;
      if (par_depth < 0) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Unbalanced `{', `}' "
                "on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      } else
      if (par_depth == 0) {
        if (!add_def_mod_already_inserted_for_this_module) {
          add_def_mod_already_inserted_for_this_module = (1 != 0);
          new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
            ptr_to_new_token, t_iter1, tokens)
          h_TOKEN_alloc_strcpy_hs(
            &(ptr_to_new_token->value), "END_DEFINITION_OF_MODULE");
          ptr_to_new_token->line = t_iter1->e.body.line;
          ptr_to_new_token->brackets_depth = 0;
          ptr_to_new_token->type = ID____________TT;
        }
      }
    } else
    if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_MODULE) == 0 ) {
      if (par_depth != 0) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "`%s' keyword encountered at non-zero "
                "depth in respect to `{', `}' "
                "on line %s:%d.\n",
                KEYWORD_MODULE,
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      add_def_mod_already_inserted_for_this_module = (0 != 0);
    }
  }

  t_iter1 = NULL;
  t_iter2 = NULL;
}



void
d_create__main_var_decl_table_and_finalize_token_list(
void) {
  TOKEN *             ptr_to_new_token;
  char                s[64]; /* Large enough to hold any decimal integer. */
  uint64_t            i;
  uint64_t            j;
  uint32_t            num_main_var_decl = 0;
  str_on_heap_TOKEN * ptr_to_num_main_var_decl_string;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value),
    "\n"
    "#define MAIN_VAR_DECL_TABLE_IS_PRESENT\n"
    "\n"
    "struct {\n"
    "  char *   name;\n"
    "  int      signal_type;\n"
    "  uint64_t num_indexes;\n"
    "  uint64_t ranges[");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;

  sprintf(s, "%d", MAX_NUM_INDEXES);
  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value), s);
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = DECIMAL_CONST_TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value),
    "];\n"
    "} main_var_decl_table[");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
  /* Postpone setting the string value till the end of func. when */
  /* `num_main_var_decl' is known.                                */
  /* sprintf(s, "%u", ...num_main_var_decl...);                 */
  /* h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), s);    */
  ptr_to_num_main_var_decl_string = &(ptr_to_new_token->value);
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = DECIMAL_CONST_TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value),
    "] = {\n");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;

  for (i = 0; i < st_var_decl_num_entries; i++) {
    /* Ignore declarations which do not start with `main_module::'. */
    if ( (h_TOKEN_strcmp_hs(
            st_var_decl_value_storage[i].ptr_back_to_name,
            "main_module::") < 0)
         ||
         (h_TOKEN_strcmp_hs(
            st_var_decl_value_storage[i].ptr_back_to_name,
            "main_module:;") >= 0)
         ) {
      continue;
    }

    num_main_var_decl++;

    new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value),
      "  {\"");
    ptr_to_new_token->line = 999999999;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = ID____________TT;

    new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
    h_TOKEN_alloc_strcpy_hh(
      &(ptr_to_new_token->value),
      st_var_decl_value_storage[i].ptr_back_to_name);
    ptr_to_new_token->line = 999999999;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = ID____________TT;

    new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value),
      "\", ");
    ptr_to_new_token->line = 999999999;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = ID____________TT;

    sprintf(s, "%u", st_var_decl_value_storage[i].signal_type);
    new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value), s);
    ptr_to_new_token->line = 999999999;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = DECIMAL_CONST_TT;

    new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value),
      ", ");
    ptr_to_new_token->line = 999999999;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = ID____________TT;

    sprintf(s, "%u", (uint32_t) st_var_decl_value_storage[i].num_indexes);
    new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value), s);
    ptr_to_new_token->line = 999999999;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = DECIMAL_CONST_TT;

    new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value),
      ", {");
    ptr_to_new_token->line = 999999999;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = ID____________TT;

    for (j = 0; j < st_var_decl_value_storage[i].num_indexes; j++) {
      new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
      h_TOKEN_alloc_strcpy_hh(
        &(ptr_to_new_token->value),
        st_var_decl_value_storage[i].indexes[j]);
      ptr_to_new_token->line = 999999999;
      ptr_to_new_token->brackets_depth = 0;
      ptr_to_new_token->type = ID____________TT;

      new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
      h_TOKEN_alloc_strcpy_hs(
        &(ptr_to_new_token->value),
        ", ");
      ptr_to_new_token->line = 999999999;
      ptr_to_new_token->brackets_depth = 0;
      ptr_to_new_token->type = ID____________TT;
    }

    new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value),
      "}},\n");
    ptr_to_new_token->line = 999999999;
    ptr_to_new_token->brackets_depth = 0;
    ptr_to_new_token->type = ID____________TT;
  }

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value),
    "};\n\n#define NUM_MAIN_VAR_DECL ");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;

  sprintf(s, "%u", num_main_var_decl);
  h_TOKEN_alloc_strcpy_hs(ptr_to_num_main_var_decl_string, s);

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
  h_TOKEN_alloc_strcpy_hh(
    &(ptr_to_new_token->value),
    *ptr_to_num_main_var_decl_string);
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;

  new_at_the_right_end_sdll_TOKEN(ptr_to_new_token, tokens);
  h_TOKEN_alloc_strcpy_hs(
    &(ptr_to_new_token->value),
    "\n\n#include \"bbrl_________c_main_function.c\"\n");
  ptr_to_new_token->line = 999999999;
  ptr_to_new_token->brackets_depth = 0;
  ptr_to_new_token->type = ID____________TT;
}



void*
e_st_create_var_decl_sgt(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;

  if (t_iter1 == NULL) {
    rv = e_st_create_var_decl_sgt_pre_termination;
  } else
  if (t_iter1->e.body.type == ID____________TT) {
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_MODULE) == 0) {
      if (is_nonempty_sdll_TOKEN(d_st_param_list)) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "`%s' keyword encountered on line %s:%d but the previous "
                "module definition is not complete.\n",
                KEYWORD_MODULE, ctqg_file_name, t_iter1->e.body.line);
        clean_exit(-1);
      }
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      delete_st_param_array();
      rv = e_st_read_param_array_and_module_name;
    } else
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_ZTG_ANC) == 0) {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = e_st_read_ztg_declaration;
    } else
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_OTG_ANC) == 0) {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = e_st_read_otg_declaration;
    } else
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_ZTZ_ANC) == 0) {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = e_st_read_ztz_declaration;
    } else
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_OTO_ANC) == 0) {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = e_st_read_oto_declaration;
    } else
    {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = e_st_create_var_decl_sgt;
    }
  } else
  if (t_iter1->e.body.type == DECIMAL_CONST_TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = e_st_create_var_decl_sgt;
  } else
  if (t_iter1->e.body.type == SPACE_________TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  } else
  if (t_iter1->e.body.type == PERIODS_______TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = e_st_create_var_decl_sgt;
  } else
  if (t_iter1->e.body.type == OPERATOR______TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = e_st_create_var_decl_sgt;
  } else
  if (t_iter1->e.body.type == END_OF_LINE___TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  } else
  if (t_iter1->e.body.type == BRACKETS______TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = e_st_create_var_decl_sgt;
  } else
  if (t_iter1->e.body.type == STDALN_SYM____TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = e_st_create_var_decl_sgt;
  } else {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  return (void*) rv;
}



void*
e_st_create_var_decl_sgt_pre_termination(
void) {
  if (0) e_debug_print_decl_table();

  delete_st_param_array();
  /* Do not do! delete_st_var_decl_sgt(); */
  /* Do not do! st_var_decl_num_entries = 0; */
  /* Do not do! delete_sdll_TOKEN(&d_st_param_list); */
  /* Do not do! st_callee_name = NULL; */ /* Does not own memory. */
  st_module_name = NULL; /* Does not own memory. */
  /* Do not do! e_st_does_module_use_constants = 888888; */
  /* Do not do! e_st_max_n_of_c_in_a_single_d_b_in_module = 888888; */
  /* Do not do! e_st_does_dollar_block_have_constants = 888888; */
  /* Do not do! e_st_num_of_constants_in_dollar_block = 888888; */
  t_iter1 = NULL;
  t_iter2 = NULL;

  return (void*) termination_state;
}



void*
e_st_read_param_array_and_module_name(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;

  /* `d_st_param_list' is empty and not used. */

  while (1 != 0) {
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file `%s'.\n", ctqg_file_name);
      clean_exit(-1);
    } else
    if (t_iter1->e.body.type == ID____________TT) {
      /* The list of parameters has been read to `st_param_array'. */
      /* `t_iter1' points to the name of module token.*/
      st_module_name = t_iter1->e.body.value;
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = e_st_read_declarations_of_signals;
      break;
    } else
    if ( (t_iter1->e.body.type == OPERATOR______TT)
         &&
         (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "<") == 0) ) {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      if ( (t_iter1 == NULL) || (t_iter1->e.body.type != ID____________TT) ) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Name of parameter expected after `<' on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      if (st_param_array_num_entries >= MAX_NUM_PARAMETERS) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Module has too many parameters "
                "(MAX_NUM_PARAMETERS = %u) on line %s:%d.\n",
                MAX_NUM_PARAMETERS,
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      h_TOKEN_alloc_strcpy_hh(
              &(st_param_array[st_param_array_num_entries]),
              t_iter1->e.body.value);
      st_param_array_num_entries++;

      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      if ( (t_iter1 == NULL)
           ||
           (t_iter1->e.body.type != OPERATOR______TT)
           ||
           (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ">") != 0) ) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "`>' expected after name of parameter on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Name of module or `<name_of_param>' expected on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
  }

  return (void*) rv;
}



void*
e_st_skip_param_array__read_module_name__skip_decl_of_sig(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;

  /* `d_st_param_list' is empty and not used. */
  /* `st_param_array'  is empty and not used. */

  while (1 != 0) {
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file `%s'.\n", ctqg_file_name);
      clean_exit(-1);
    } else
    if (t_iter1->e.body.type == ID____________TT) {
      /* The list of parameters has been read and discarded. */
      /* `t_iter1' points to the name of module token.*/
      st_module_name = t_iter1->e.body.value;
      if ( e_st_does_module_use_constants != 888888 ) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "An internal CTQG error occurred: %s:%d.\n",
                __FILE__, __LINE__);
        clean_exit(-2);
      }
      e_st_does_module_use_constants = (0 != 0);
      e_st_max_n_of_c_in_a_single_d_b_in_module = 0;
      while (1 != 0) {
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
        if (t_iter1 == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Unexpected end of file `%s'.\n", ctqg_file_name);
          clean_exit(-1);
        }
        if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, "{") == 0 ) break;
      }
      e_st_iter__insert_ancilla_decl_to_the_right_of_me = t_iter1;
      rv = e_st_inside_body;
      break;
    } else
    if ( (t_iter1->e.body.type == OPERATOR______TT)
         &&
         (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "<") == 0) ) {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      if ( (t_iter1 == NULL) || (t_iter1->e.body.type != ID____________TT) ) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Name of parameter expected after `<' on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      if ( (t_iter1 == NULL)
           ||
           (t_iter1->e.body.type != OPERATOR______TT)
           ||
           (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ">") != 0) ) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "`>' expected after name of parameter on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Name of module or `<name_of_param>' expected on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
  }

  return (void*) rv;
}



void*
e_st_read_ztg_declaration(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  e_read_ancilla_declaration(2);
  rv = e_st_create_var_decl_sgt;
  return (void*) rv;
}



void*
e_st_read_otg_declaration(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  e_read_ancilla_declaration(3);
  rv = e_st_create_var_decl_sgt;
  return (void*) rv;
}



void*
e_st_read_ztz_declaration(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  e_read_ancilla_declaration(4);
  rv = e_st_create_var_decl_sgt;
  return (void*) rv;
}



void*
e_st_read_oto_declaration(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  e_read_ancilla_declaration(5);
  rv = e_st_create_var_decl_sgt;
  return (void*) rv;
}



void*
e_st_read_declarations_of_signals(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  iter_sdll_TOKEN               iter;
  var_decl_value_t *            ptr_to_new_var_decl;
  str_on_heap_TOKEN             str;
  str_on_heap_TOKEN             subst_param_str;
  uint32_t                      i;
  int32_t                       sig_decl_number;

  if ( (t_iter1 == NULL)
       ||
       (t_iter1->e.body.type != STDALN_SYM____TT)
       ||
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "(") != 0) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`(' expected after name of module on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  /* t_iter1 -> 1st symb of 1st decl. Process declarations of signals. */
  sig_decl_number = -1;
  while (1 != 0) {
    sig_decl_number++;
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file `%s'.\n", ctqg_file_name);
      clean_exit(-1);
    } else
    if ( (t_iter1->e.body.type == ID____________TT)
         &&
         (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_BIT) == 0) ) {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      if ( (t_iter1 == NULL)
           ||
           (t_iter1->e.body.type != ID____________TT) ) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Identifier expected after `%s' on line %s:%d.\n",
                KEYWORD_BIT,
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      ptr_to_new_var_decl = new_var_decl(
                              st_module_name, t_iter1->e.body.value);
      if (ptr_to_new_var_decl == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "Duplicate identifier `");
        h_TOKEN_fprintf_str(stderr, t_iter1->e.body.value, 0, ' ', 'l');
        fprintf(stderr,
                "' on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      ptr_to_new_var_decl->signal_type = 1 /* regular bit */;
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      while (1 != 0) {
        if (t_iter1 == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Unexpected end of file after declaration on line %s:%d.\n",
                  ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
          clean_exit(-1);
        }
        if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0 ) {
          break;
        } else
        if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ",") == 0 ) {
          break;
        }
        iter = t_iter1;
        str = read_expression_in_brackets__alloc();
        if (str == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Expression in brackets expected on line %s:%d.\n",
                  ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
          clean_exit(-1);
        }
        add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, str);
        t_iter1 = iter;
        str = read_expression_in_brackets_subst_param__alloc();
        if (str == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
            "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
          clean_exit(-2);
        }
        ptr_to_new_var_decl->subst_indexes[
          ptr_to_new_var_decl->num_indexes - 1
        ] = str;
      }
      replicate_var_decl_with_numeric_name(
        st_module_name, sig_decl_number, ptr_to_new_var_decl);
      if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0 ) {
        break; /* Finished reading declarations. */
      }
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    } else
    if ( (t_iter1->e.body.type == ID____________TT)
         &&
         (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_INT) == 0) ) {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

      iter = t_iter1;
      str = read_expression_in_brackets__alloc();
      if (str == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Expression in brackets expected after `%s' on line %s:%d.\n",
                KEYWORD_INT,
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      t_iter1 = iter;
      subst_param_str = read_expression_in_brackets_subst_param__alloc();

      if ( (t_iter1 == NULL)
           ||
           (t_iter1->e.body.type != ID____________TT) ) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Identifier expected after `%s[...]' on line %s:%d.\n",
                KEYWORD_INT,
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        h_TOKEN_str_delete(str);
        h_TOKEN_str_delete(subst_param_str);
        clean_exit(-1);
      }

      ptr_to_new_var_decl = new_var_decl(
                              st_module_name, t_iter1->e.body.value);
      if (ptr_to_new_var_decl == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr, "Duplicate identifier `");
        h_TOKEN_fprintf_str(stderr, t_iter1->e.body.value, 0, ' ', 'l');
        fprintf(stderr,
                "' on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        h_TOKEN_str_delete(str);
        h_TOKEN_str_delete(subst_param_str);
        clean_exit(-1);
      }
      ptr_to_new_var_decl->signal_type = 0 /* regular int */;
      /* The value of `str' was set approx 35 lines above. Now use it. */
      add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, str);
      if (subst_param_str == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
          "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
        clean_exit(-2);
      }
      ptr_to_new_var_decl->subst_indexes[
        ptr_to_new_var_decl->num_indexes - 1
      ] = subst_param_str;
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

      while (1 != 0) {
        if (t_iter1 == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Unexpected end of file after declaration on line %s:%d.\n",
                  ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
          clean_exit(-1);
        }
        if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0 ) {
          break;
        } else
        if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ",") == 0 ) {
          break;
        }
        iter = t_iter1;
        str = read_expression_in_brackets__alloc();
        if (str == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Expression in brackets expected on line %s:%d.\n",
                  ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
          clean_exit(-1);
        }
        add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, str);
        t_iter1 = iter;
        str = read_expression_in_brackets_subst_param__alloc();
        if (str == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
            "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
          clean_exit(-2);
        }
        ptr_to_new_var_decl->subst_indexes[
          ptr_to_new_var_decl->num_indexes - 1
        ] = str;
      }
      /* Rotate indexes to the left by 1, e.g [0][1][2] -> [1][2][0] */
      if (ptr_to_new_var_decl->num_indexes >= 2) {
        str = ptr_to_new_var_decl->indexes[0];
        subst_param_str = ptr_to_new_var_decl->subst_indexes[0];
        for (i = 0; i < ptr_to_new_var_decl->num_indexes - 1; i++) {
          ptr_to_new_var_decl->indexes[i] = ptr_to_new_var_decl->indexes[i + 1];
          ptr_to_new_var_decl->subst_indexes[i] =
            ptr_to_new_var_decl->subst_indexes[i + 1];
        }
        ptr_to_new_var_decl->indexes[
          ptr_to_new_var_decl->num_indexes - 1
        ] = str;
        ptr_to_new_var_decl->subst_indexes[
          ptr_to_new_var_decl->num_indexes - 1
        ] = subst_param_str;
      }
      replicate_var_decl_with_numeric_name(
        st_module_name, sig_decl_number, ptr_to_new_var_decl);

      if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0 ) {
        break; /* Finished reading declarations. */
      }
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "`%s' or `%s' expected on line %s:%d.\n",
              KEYWORD_BIT, KEYWORD_INT,
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
  }

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  if ( (t_iter1 == NULL)
       ||
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "{") != 0) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Beginning of module body starting with `{' "
            "expected on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  rv = e_st_create_var_decl_sgt;

  return (void*) rv;
}



void*
e_st_inside_body(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;

  if (t_iter1 == NULL) {
    e_leaving_a_module(); /* Used exactly in 2 pl. : here and 13 lines below. */
    rv = e_st_pre_termination;
  } else
  if (t_iter1->e.body.type == ID____________TT) {
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_MODULE) == 0) {
      if (is_nonempty_sdll_TOKEN(d_st_param_list)) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "`%s' keyword encountered on line %s:%d but the previous "
                "module definition is not complete.\n",
                KEYWORD_MODULE, ctqg_file_name, t_iter1->e.body.line);
        clean_exit(-1);
      }
      if (st_module_name != NULL) e_leaving_a_module();
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      delete_st_param_array();
      rv = e_st_skip_param_array__read_module_name__skip_decl_of_sig;
    } else {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = e_st_inside_body;
    }
  } else
  if (t_iter1->e.body.type == DECIMAL_CONST_TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = e_st_inside_body;
  } else
  if (t_iter1->e.body.type == SPACE_________TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  } else
  if (t_iter1->e.body.type == PERIODS_______TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = e_st_inside_body;
  } else
  if (t_iter1->e.body.type == OPERATOR______TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = e_st_inside_body;
  } else
  if (t_iter1->e.body.type == END_OF_LINE___TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  } else
  if (t_iter1->e.body.type == BRACKETS______TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = e_st_inside_body;
  } else
  if (t_iter1->e.body.type == STDALN_SYM____TT) {
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "$") == 0) {
      t_iter2 = t_iter1; /* t_iter2 points to `$'. */
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = e_st_inside_dollar_block;
    } else {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      rv = e_st_inside_body;
    }
  } else {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  return (void*) rv;
}



void*
e_st_inside_dollar_block(
void) {
  /* `t_iter1' points to the next non-space token after the `$'. */
  /* `t_iter2' points to the `$'.                                */
  iter_sdll_TOKEN               save_t_iter1 = t_iter1;
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;

  if ( e_st_does_dollar_block_have_constants != 888888 ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "An internal CTQG error occurred: %s:%d.\n",
            __FILE__, __LINE__);
    clean_exit(-2);
  }
  e_st_does_dollar_block_have_constants = (0 != 0);
  e_st_num_of_constants_in_dollar_block = 0;


  if (t_iter1 == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Unexpected end of file after `$' on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  } else
  if (t_iter1->e.body.type == ID____________TT) {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file after `$ <ID>' on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    } else
    if ( (t_iter1->e.body.type == STDALN_SYM____TT)
         &&
         (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "(") == 0) ) {
      t_iter1 = save_t_iter1;
      rv = e_st_read_instantiation; /* $ ID( ...   */
    } else
    {
      t_iter1 = save_t_iter1;
      rv = e_st_read_operator;      /* $ ID ... OP ... */
    }
  } else
  if (t_iter1->e.body.type == DECIMAL_CONST_TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "A decimal constant encountered after `$' on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  } else
  if (t_iter1->e.body.type == SPACE_________TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  } else
  if (t_iter1->e.body.type == PERIODS_______TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`.' encountered after `$' on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  } else
  if (t_iter1->e.body.type == OPERATOR______TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Unexpected symbol after `$' on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  } else
  if (t_iter1->e.body.type == END_OF_LINE___TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  } else
  if (t_iter1->e.body.type == BRACKETS______TT) {
    t_iter1 = save_t_iter1;
    rv = e_st_read_instantiation; /* $ [ ...   */
  } else
  if (t_iter1->e.body.type == STDALN_SYM____TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Unexpected symbol after `$' on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  } else {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  return (void*) rv;
}



void*
e_st_read_instantiation(
void) {
  /* `t_iter1' points to the next non-space token after the `$'. */
  /* `t_iter2' points to the `$'.                                */
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  str_on_heap_TOKEN             temp_str = NULL;
  uint32_t                      ee_arg_number;

  /* --- BEGIN Read parameters of instantiation. */
  if (st_param_array_num_entries != 0) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }

  while (1 != 0) {
    temp_str = read_expression_in_brackets__alloc();
    if (temp_str == NULL) break;

    if (st_param_array_num_entries >= MAX_NUM_PARAMETERS) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Too many parameters "
              "(MAX_NUM_PARAMETERS = %u) are given on line %s:%d.\n",
              MAX_NUM_PARAMETERS,
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      h_TOKEN_str_delete(temp_str);
      clean_exit(-1);
    }
    st_param_array[st_param_array_num_entries] = temp_str;
    st_param_array_num_entries++;
  }
  /* ---  END  Read parameters of instantiation. */

  e_write_parameters_of_instantiation();

  /* Caller name is in `st_module_name'. */
  /* Get callee name to `st_callee_name'. */
  if ( (t_iter1 == NULL)
       ||
       (t_iter1->e.body.type != ID____________TT) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "The name of callee is expected in "
            "instantiation on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  st_callee_name = t_iter1->e.body.value; /* Does not own memory */

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  if ( (t_iter1 == NULL)
       ||
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "(") != 0) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`(' expected after name of callee in "
            "instantiation on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  ee_arg_number = 0;
  while (1 != 0) {
    e_process_compound_signal_or_constant(ee_arg_number);
    ee_arg_number++;
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file in "
              "instantiation on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0) {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      break;
    }
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ",") == 0) {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      continue;
    }
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`,' or `)' expected after argument in "
            "instantiation on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  /* Now t_iter1 must point to `;' */
  if ( (t_iter1 == NULL)
       ||
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ";") != 0) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`;' expected after "
            "instantiation on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  e_write_closing_of_instantiation();

  while (t_iter2 != t_iter1) {
    move_iter_and_delete_to_next_non_separ_token(&t_iter2);
  }
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  delete_TOKEN_and_move_iter_to_right(&t_iter2, &tokens);

  /* printf("Instantiation:  "); debug_print_st_param_array(); printf("\n"); */

  st_callee_name = NULL;
  delete_st_param_array();

  e_st_does_dollar_block_have_constants = 888888;
  e_st_iter__i_point_to_instantiate_module = NULL;
  /* Not needed: e_st_iter__i_point_to_end_oper_parm = NULL; */

  rv = e_st_inside_body;
  return (void*) rv;
}



void*
e_st_read_operator(
void) {
  /* `t_iter1' points to the next non-space token after the `$'. */
  /* `t_iter2' points to the `$'.                                */
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  uint32_t                      ee_arg_number;
  TOKEN *                       ptr_to_new_token = NULL;

  /* --- BEGIN Place 'end_oper_parm' marker. */
  if (st_param_array_num_entries != 0) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }
  st_param_array_num_entries = 1; /* Operators take exactly 1 parameter. */
  h_TOKEN_alloc_strcpy_hs(&(st_param_array[0]), "yQ_BAD_ST_PARAM_VALUE");

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter2, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "{");
  ptr_to_new_token->line = t_iter2->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
  ptr_to_new_token->type = STDALN_SYM____TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter2, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "yQp[0] = ");
  ptr_to_new_token->line = t_iter2->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter2, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value),
    "/*end_oper_parm*/ ");
  ptr_to_new_token->line = t_iter2->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  if (e_st_iter__i_point_to_end_oper_parm != NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
      "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }
  e_st_iter__i_point_to_end_oper_parm = t_iter2;
  move_iter_left_sdll_TOKEN(e_st_iter__i_point_to_end_oper_parm);

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter2, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "INSTANTIATE_MODULE ");
  ptr_to_new_token->line = t_iter2->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  e_st_iter__i_point_to_instantiate_module = t_iter2;
  move_iter_left_sdll_TOKEN(e_st_iter__i_point_to_instantiate_module);
  /* ---  END  Place 'end_oper_parm' marker. */

  /* An analog of `e_write_parameters_of_instantiation' */
  /* is embedded in the previous block.                 */

  /* {yQp[0] = *end_oper_parm* INSTANTIATE_MODULE   $      a   += b * c; */
  /*                                              iter2  iter1           */

  /* Caller name is in `st_module_name'.           */
  /* Get callee name to `st_callee_name' and get   */
  /* `e_st_position_of_param_defining_cs_of_oper'. */
  e_get_name_and_position_of_param_defining_cs__of_operator();

  /* {yQp[0] = *end_oper_parm* INSTANTIATE_MODULE   $      a  oname b * c; */
  /*                                              iter2  iter1  |          */
  /*                                                       st_callee_name  */

  ee_arg_number = 0;
  while (1 != 0) {
    if (ee_arg_number
        ==
        e_st_position_of_param_defining_cs_of_oper) {
      e_process_param_defining_cs_of_oper(ee_arg_number);
    } else {
      e_process_compound_signal_or_constant(ee_arg_number);
    }
    ee_arg_number++;
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file in "
              "instantiation on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ";") == 0) {
      break;
    }
    if (t_iter1->e.body.type == OPERATOR______TT) {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      continue;
    }
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`;' or <OPERATOR> token expected after argument in "
            "instantiation on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  /* Now t_iter1 must point to `;' */

  e_write_closing_of_instantiation();

  while (t_iter2 != t_iter1) {
    move_iter_and_delete_to_next_non_separ_token(&t_iter2);
  }
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  delete_TOKEN_and_move_iter_to_right(&t_iter2, &tokens);

  /* printf("Operator:  "); debug_print_st_param_array(); printf("\n"); */

  st_callee_name = NULL;
  delete_st_param_array();

  e_st_does_dollar_block_have_constants = 888888;
  e_st_iter__i_point_to_instantiate_module = NULL;
  e_st_iter__i_point_to_end_oper_parm = NULL;

  rv = e_st_inside_body;
  return (void*) rv;
}



void*
e_st_pre_termination(
void) {
  /* Do not do! delete_st_param_array(); */
  delete_st_var_decl_sgt();
  st_var_decl_num_entries = 0;
  e_st_iter__insert_ancilla_decl_to_the_right_of_me = NULL;
  e_st_iter__i_point_to_instantiate_module = NULL;
  e_st_iter__i_point_to_end_oper_parm = NULL;
  /* Do not do! delete_sdll_TOKEN(&d_st_param_list); */
  st_callee_name = NULL; /* Does not own memory. */
  st_module_name = NULL; /* Does not own memory. */
  e_st_does_module_use_constants = 888888;
  e_st_max_n_of_c_in_a_single_d_b_in_module = 888888;
  e_st_does_dollar_block_have_constants = 888888;
  e_st_num_of_constants_in_dollar_block = 888888;
  t_iter1 = NULL;
  t_iter2 = NULL;

  return (void*) termination_state;
}



void
add_var_declarations_from_built_in_modules(
void) {
  var_decl_value_t * ptr_to_new_var_decl;
  str_on_heap_TOKEN  h1;
  str_on_heap_TOKEN  h2;
  char *             str;

  /**/
  /**/
  str =                                           "not";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "x"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              1           /* regular bit */;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "cnot";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "c"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              1           /* regular bit */;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "cnot";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "x"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              1           /* regular bit */;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "toffoli";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "c1"                        );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              1           /* regular bit */;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "toffoli";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "c2"                        );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              1           /* regular bit */;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "toffoli";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "x"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              1           /* regular bit */;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           2,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "a_swap_b";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a_swap_b";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "assign_value_of_b_to_a";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "assign_value_of_b_to_a";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "a__eq__a_plus_b";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a__eq__a_plus_b";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "a__eq__a_minus_b";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a__eq__a_minus_b";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "a_less_than_b__as_signed";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "x"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              1           /* regular bit */;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a_less_than_b__as_signed";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a_less_than_b__as_signed";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           2,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "a_less_than_or_eq_to"
                                                  "_b__as_signed";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "x"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              1           /* regular bit */;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a_less_than_or_eq_to"
                                                  "_b__as_signed";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a_less_than_or_eq_to"
                                                  "_b__as_signed";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           2,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "a_greater_than_b__as_signed";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "x"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              1           /* regular bit */;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a_greater_than_b__as_signed";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a_greater_than_b__as_signed";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           2,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "a_greater_than_or_eq_to"
                                                  "_b__as_signed";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "x"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              1           /* regular bit */;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a_greater_than_or_eq_to"
                                                  "_b__as_signed";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a_greater_than_or_eq_to"
                                                  "_b__as_signed";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           2,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "is_a_eq_to_b";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "x"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              1           /* regular bit */;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "is_a_eq_to_b";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "is_a_eq_to_b";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           2,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "is_a_not_eq_to_b";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "x"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              1           /* regular bit */;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "is_a_not_eq_to_b";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "is_a_not_eq_to_b";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           2,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "a__eq__a_plus_b_times_c";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a__eq__a_plus_b_times_c";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a__eq__a_plus_b_times_c";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "c"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           2,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "a__eq__a_minus_b_times_c";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a__eq__a_minus_b_times_c";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "a__eq__a_minus_b_times_c";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "c"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(n)"                       );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0])"                  );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           2,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "fxp_invert_sign";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "fxp_abs";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "fxp_floor";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "fxp_mult";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "fxp_mult";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "fxp_mult";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "c"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           2,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "fxp_inverse";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "fxp_inverse";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "fxp_exp";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "fxp_exp";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "fxp_ln";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "fxp_ln";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "fxp_cos";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "fxp_cos";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);

  /**/
  /**/
  str =                                           "fxp_sin";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "a"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           0,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
  str =                                           "fxp_sin";
  h_TOKEN_alloc_strcpy_hs(&h2,                    "b"                         );
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  ptr_to_new_var_decl = new_var_decl(h1, h2);
  h_TOKEN_str_delete(h1);
  h_TOKEN_str_delete(h2);
  if (ptr_to_new_var_decl == NULL) { fprintf(stderr, "Error.\nAn internal "
    "CTQG error occurred: %s:%d.\n", __FILE__, __LINE__); clean_exit(-2); }
  ptr_to_new_var_decl->signal_type =              0           /* regular int */;
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(m+k)"                     );
  add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, h1);
  h_TOKEN_alloc_strcpy_hs(&h1,                    "(yQp[0]+yQp[1])"           );
  ptr_to_new_var_decl->subst_indexes[ptr_to_new_var_decl->num_indexes - 1] = h1;
  h_TOKEN_alloc_strcpy_hs(&h1, str);
  replicate_var_decl_with_numeric_name(
    h1,                                           1,       ptr_to_new_var_decl);
  h_TOKEN_str_delete(h1);
}



void
e_leaving_a_module(
void) {
  TOKEN * ptr_to_new_token;
  char    s[64]; /* Enough to hold any size decimal integer. */

  if (st_module_name == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "A program (quantum circuit) must contain at least one module.\n");
    clean_exit(-1);
  }

  if (e_st_does_module_use_constants) {
    /* Insert at the beginning of the module the neccassary declarations */
    /* for dynamic restorable ancilla requests.                          */
    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token,
      e_st_iter__insert_ancilla_decl_to_the_right_of_me,
      tokens)
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value),
      "uint64_t yQcurrz, yQcurro, yQnztz = 0, yQnoto = 0; "
      "char * yQc[");
    ptr_to_new_token->line =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.body.line;
    ptr_to_new_token->brackets_depth =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;
    e_st_iter__insert_ancilla_decl_to_the_right_of_me =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.ptr_to_right;

    sprintf(s, "%u",
      (uint32_t) e_st_max_n_of_c_in_a_single_d_b_in_module);
    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token,
      e_st_iter__insert_ancilla_decl_to_the_right_of_me,
      tokens)
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value),
      s);
    ptr_to_new_token->line =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.body.line;
    ptr_to_new_token->brackets_depth =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.body.brackets_depth
      + 1;
    ptr_to_new_token->type = DECIMAL_CONST_TT;
    e_st_iter__insert_ancilla_decl_to_the_right_of_me =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.ptr_to_right;

    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token,
      e_st_iter__insert_ancilla_decl_to_the_right_of_me,
      tokens)
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value),
      "]; uint64_t yQc_ee[");
    ptr_to_new_token->line =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.body.line;
    ptr_to_new_token->brackets_depth =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;
    e_st_iter__insert_ancilla_decl_to_the_right_of_me =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.ptr_to_right;

    sprintf(s, "%u",
      (uint32_t) e_st_max_n_of_c_in_a_single_d_b_in_module);
    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token,
      e_st_iter__insert_ancilla_decl_to_the_right_of_me,
      tokens)
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value),
      s);
    ptr_to_new_token->line =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.body.line;
    ptr_to_new_token->brackets_depth =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.body.brackets_depth
      + 1;
    ptr_to_new_token->type = DECIMAL_CONST_TT;
    e_st_iter__insert_ancilla_decl_to_the_right_of_me =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.ptr_to_right;

    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token,
      e_st_iter__insert_ancilla_decl_to_the_right_of_me,
      tokens)
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value),
      "]; ");
    ptr_to_new_token->line =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.body.line;
    ptr_to_new_token->brackets_depth =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;
    e_st_iter__insert_ancilla_decl_to_the_right_of_me =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.ptr_to_right;
  } else {
    new_to_the_right_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token,
      e_st_iter__insert_ancilla_decl_to_the_right_of_me,
      tokens)
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value),
      "/*no_const*/ ");
    ptr_to_new_token->line =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.body.line;
    ptr_to_new_token->brackets_depth =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;
    e_st_iter__insert_ancilla_decl_to_the_right_of_me =
      e_st_iter__insert_ancilla_decl_to_the_right_of_me->e.ptr_to_right;
  }

  e_st_iter__insert_ancilla_decl_to_the_right_of_me = NULL;

  if ( e_st_does_dollar_block_have_constants != 888888 ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
    clean_exit(-2);
  }
  e_st_does_module_use_constants = 888888;
  e_st_max_n_of_c_in_a_single_d_b_in_module = 888888;
}



void
e_read_ancilla_declaration(
uint32_t signal_type) {
  char *             keyword;
  str_on_heap_TOKEN  var_name;
  var_decl_value_t * ptr_to_new_var_decl;
  iter_sdll_TOKEN    iter;
  str_on_heap_TOKEN  expr_in_br;
  str_on_heap_TOKEN  expr_in_br2;

  switch (signal_type) {
    case 2:
      keyword = KEYWORD_ZTG_ANC;
    break;
    case 3:
      keyword = KEYWORD_OTG_ANC;
    break;
    case 4:
      keyword = KEYWORD_ZTZ_ANC;
    break;
    case 5:
      keyword = KEYWORD_OTO_ANC;
    break;
    default:
      keyword = KEYWORD_OTO_ANC; /* Make `used uninitialized' warning  happy. */
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
        "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
      clean_exit(-2);
  }

  if ( (t_iter1 == NULL)
       ||
       (t_iter1->e.body.type != ID____________TT) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Identifier expected after `%s' on line %s:%d.\n",
            keyword,
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  h_TOKEN_alloc_strcpy_hh(&var_name, t_iter1->e.body.value);
  ptr_to_new_var_decl = new_var_decl(st_module_name, var_name);
  if (ptr_to_new_var_decl == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr, "Duplicate identifier `");
    h_TOKEN_fprintf_str(stderr, t_iter1->e.body.value, 0, ' ', 'l');
    fprintf(stderr,
            "' on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    h_TOKEN_str_delete(var_name);
    clean_exit(-1);
  }
  ptr_to_new_var_decl->signal_type = signal_type;
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  while (1 != 0) {
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file after `%s' declaration on line %s:%d.\n",
              keyword,
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      h_TOKEN_str_delete(var_name);
      clean_exit(-1);
    }
    if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ";") == 0 ) {
      break;
    }
    iter = t_iter1;
    expr_in_br2 = read_expression_in_brackets_subst_param__alloc();
    if (expr_in_br2 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "In `%s' ancilla declaration "
              "an expression in brackets expected on line %s:%d.\n",
              keyword,
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      h_TOKEN_str_delete(var_name);
      clean_exit(-1);
    }
    t_iter1 = iter;
    expr_in_br = read_expression_in_brackets__alloc();
    if (expr_in_br == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
        "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
      clean_exit(-2);
    }
    add_index_to_var_decl__no_alloc(ptr_to_new_var_decl, expr_in_br);
    ptr_to_new_var_decl->subst_indexes[
      ptr_to_new_var_decl->num_indexes - 1
    ] = expr_in_br2;
  }

  h_TOKEN_str_delete(var_name);
}



void /* Leaves `t_iter1' pointing to the delimiter `,', `)' or smth. else */
e_process_compound_signal_or_constant(
uint32_t ee_argument_number) {
  var_decl_value_t * ptr_to_ee_var_decl =
    search_var_decl_by_number(st_callee_name, ee_argument_number);

  if (t_iter1 == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Constant or name of signal expected but end of file "
            "found on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  if (ptr_to_ee_var_decl == NULL) {
    ptr_to_ee_var_decl =
      search_var_decl_by_number(st_callee_name, 0);
    if (ptr_to_ee_var_decl == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "In instantiation on line %s:%d "
              "module with name `",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      h_TOKEN_fprintf_str(stderr, st_callee_name, 0, ' ', 'l');
      fprintf(stderr,
              "' is not defined.\n");
      clean_exit(-1);
    }
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "In instantiation on line %s:%d "
            "module `",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    h_TOKEN_fprintf_str(stderr, st_callee_name, 0, ' ', 'l');
    fprintf(stderr,
            "' does not require that many arguments.\n");
    clean_exit(-1);
  }

  if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "'") == 0) {
    e_process_bit_constant(ptr_to_ee_var_decl);
  } else
  if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "\"") == 0) {
    e_process_1d_bit_array_constant(ptr_to_ee_var_decl);
  } else
  if ( (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "-") == 0)
       ||
       (t_iter1->e.body.type == DECIMAL_CONST_TT) ) {
    e_process_integer_constant(ptr_to_ee_var_decl);
  } else
  {
    e_process_compound_signal(ptr_to_ee_var_decl);
  }
}



void /* Leaves `t_iter1' pointing to <operator> or `;' */
e_process_param_defining_cs_of_oper(
uint32_t ee_argument_number) {
  var_decl_value_t * ptr_to_ee_var_decl =
    search_var_decl_by_number(st_callee_name, ee_argument_number);

  /* --- BEGIN e_process_compound_signal<pdef> variables --- */
  uint32_t           i;
  str_on_heap_TOKEN  lo;
  str_on_heap_TOKEN  hi;
  uint32_t           num_r_indexes;
  str_on_heap_TOKEN  r_sig_name; /* Does not own m. */
  var_decl_value_t * ptr_to_r_var_decl;
  /* ---  END  e_process_compound_signal<pdef> variables --- */

  if (t_iter1 == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Constant or name of signal expected but end of file "
            "found on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  if (ptr_to_ee_var_decl == NULL) {
    ptr_to_ee_var_decl =
      search_var_decl_by_number(st_callee_name, 0);
    if (ptr_to_ee_var_decl == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "In instantiation on line %s:%d "
              "module with name `",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      h_TOKEN_fprintf_str(stderr, st_callee_name, 0, ' ', 'l');
      fprintf(stderr,
              "' is not defined.\n");
      clean_exit(-1);
    }
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "In instantiation on line %s:%d "
            "module `",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    h_TOKEN_fprintf_str(stderr, st_callee_name, 0, ' ', 'l');
    fprintf(stderr,
            "' does not require that many arguments.\n");
    clean_exit(-1);
  }

  if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "'") == 0) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "In operator on line %s:%d : "
            "bit constant can not be used as an argument here.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  } else
  if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "\"") == 0) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "In operator on line %s:%d : "
            "constant bit array can not be used as an argument here.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  } else
  if ( (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "-") == 0)
       ||
       (t_iter1->e.body.type == DECIMAL_CONST_TT) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "In operator on line %s:%d : "
            "integer constant can not be used as an argument here.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  } else
  {
    /* --- BEGIN e_process_compound_signal<pdef>(ptr_to_ee_var_decl) --- */
    r_sig_name = t_iter1->e.body.value; /* Does not own m. */
    ptr_to_r_var_decl = search_var_decl(st_module_name, t_iter1->e.body.value);
    if (ptr_to_r_var_decl == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "In instantiation on line %s:%d : "
              "signal `",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      h_TOKEN_fprintf_str(stderr, t_iter1->e.body.value, 0, ' ', 'l');
      fprintf(stderr, "' is not declared in module `");
      h_TOKEN_fprintf_str(stderr, st_module_name, 0, ' ', 'l');
      fprintf(stderr, "'.\n");
      clean_exit(-1);
    }

    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

    insert__str_uint32_str__to_the_left_of_iter(
      t_iter2, "yQni = ", ptr_to_r_var_decl->num_indexes, "; ");

    num_r_indexes = 0;
    for (i = 0; i < ptr_to_r_var_decl->num_indexes; i++) {
      read_range_in_brackets__alloc(&lo, &hi);
      if (hi != NULL) {
        num_r_indexes++;
        if (num_r_indexes == 1) {
          insert__str_h_str__to_the_left_of_iter(
            e_st_iter__i_point_to_end_oper_parm,
            "((", hi, ")+1)-");
          insert__str_h_str__to_the_left_of_iter(
            e_st_iter__i_point_to_end_oper_parm,
            "(", lo, "); ");
        }
        insert__str_uint32_str__to_the_left_of_iter(
          t_iter2, "yQir[", i, "] = 1; ");
        insert__str_uint32_str_h_str_to_the_left_of_iter(
          t_iter2, "yQil[", i, "] = ", lo, "; ");
        insert__str_uint32_str_h_str_to_the_left_of_iter(
          t_iter2, "yQih[", i, "] = ", hi, "; ");
        insert__str_uint32_str_h_str_to_the_left_of_iter(
          t_iter2, "yQim[", i, "] = ", ptr_to_r_var_decl->indexes[i], "; ");
          h_TOKEN_str_delete(lo);
          h_TOKEN_str_delete(hi);
      } else
      if (lo != NULL) {
        insert__str_uint32_str__to_the_left_of_iter(
          t_iter2, "yQir[", i, "] = 0; ");
        insert__str_uint32_str_h_str_to_the_left_of_iter(
          t_iter2, "yQil[", i, "] = ", lo, "; ");
        insert__str_uint32_str__to_the_left_of_iter(
          t_iter2, "yQih[", i, "] = 0; ");
        insert__str_uint32_str_h_str_to_the_left_of_iter(
          t_iter2, "yQim[", i, "] = ", ptr_to_r_var_decl->indexes[i], "; ");
          h_TOKEN_str_delete(lo);
      } else
      {
        num_r_indexes++;
        if (num_r_indexes == 1) {
          insert__str_h_str__to_the_left_of_iter(
            e_st_iter__i_point_to_end_oper_parm,
            "(", ptr_to_r_var_decl->indexes[i], "); "
          );
        }
        insert__str_uint32_str__to_the_left_of_iter(
          t_iter2, "yQir[", i, "] = 1; ");
        insert__str_uint32_str__to_the_left_of_iter(
          t_iter2, "yQil[", i, "] = 0; ");
        insert__str_uint32_str_h_str_to_the_left_of_iter(
          t_iter2, "yQih[", i, "] = ", ptr_to_r_var_decl->indexes[i], "-1; ");
        insert__str_uint32_str_h_str_to_the_left_of_iter(
          t_iter2, "yQim[", i, "] = ", ptr_to_r_var_decl->indexes[i], "; ");
      }
    }

    if (num_r_indexes != ptr_to_ee_var_decl->num_indexes) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Type mismatch in instantiation on line %s:%d. "
              "Signal `",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      h_TOKEN_fprintf_str(
        stderr, ptr_to_ee_var_decl->ptr_back_to_name, 0, ' ', 'l');
      fprintf(stderr,
              "' of the callee is an array of dimension %u. "
              "It can not be mapped to an array of dimension %u.\n",
              ptr_to_ee_var_decl->num_indexes,
              num_r_indexes);
      clean_exit(-1);
    }

    for (i = 0; i < ptr_to_ee_var_decl->num_indexes; i++) {
      insert__str_uint32_str_h_str_to_the_left_of_iter(
        t_iter2,
        "yQiee[",
        i,
        "] = ",
        ptr_to_ee_var_decl->subst_indexes[i],
        "; ");
    }

    insert__str_h_str_hpostfix_str_to_the_left_of_iter(
      t_iter2,
      "map_cs(__FILE__, __LINE__, \"",
      r_sig_name,
      "\", \"",
      ptr_to_ee_var_decl->ptr_back_to_name,
      "\"); ");

    /* ---  END  e_process_compound_signal<pdef>(ptr_to_ee_var_decl) --- */
  }
}



void /* Dumps info from `st_param_array' to the left of t_iter2. */
e_write_parameters_of_instantiation(
void) {
  uint64_t i;
  TOKEN *  ptr_to_new_token = NULL;
  char     s[64]; /* Large enough for any decimal const. */

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter2, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "{");
  ptr_to_new_token->line = t_iter2->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
  ptr_to_new_token->type = STDALN_SYM____TT;

  for (i = 0; i < st_param_array_num_entries; i++) {
    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter2, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "yQp[");
    ptr_to_new_token->line = t_iter2->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;

    sprintf(s, "%u", (uint32_t) i);
    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter2, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), s);
    ptr_to_new_token->line = t_iter2->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
    ptr_to_new_token->type = DECIMAL_CONST_TT;

    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter2, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "] = ");
    ptr_to_new_token->line = t_iter2->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;

    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter2, tokens)
    h_TOKEN_alloc_strcpy_hh(
      &(ptr_to_new_token->value), st_param_array[i]);
    ptr_to_new_token->line = t_iter2->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;

    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter2, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "; ");
    ptr_to_new_token->line = t_iter2->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
    ptr_to_new_token->type = STDALN_SYM____TT;
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter2, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "INSTANTIATE_MODULE ");
  ptr_to_new_token->line = t_iter2->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  e_st_iter__i_point_to_instantiate_module = t_iter2;
  move_iter_left_sdll_TOKEN(e_st_iter__i_point_to_instantiate_module);
}



void /* Uses `st_callee_name' and `st_param_array_num_entries'. */
e_write_closing_of_instantiation(
void) {
  uint64_t i;
  TOKEN *  ptr_to_new_token = NULL;
  char     s[64]; /* Large enough for any decimal const. */

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter2, tokens)
  h_TOKEN_alloc_strcpy_hh(&(ptr_to_new_token->value), st_callee_name);
  ptr_to_new_token->line = t_iter2->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter2, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "(LOCATION_INFO");
  ptr_to_new_token->line = t_iter2->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  for (i = 0; i < st_param_array_num_entries; i++) {
    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter2, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), ", yQp[");
    ptr_to_new_token->line = t_iter2->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;

    sprintf(s, "%u", (uint32_t) i);
    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter2, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), s);
    ptr_to_new_token->line = t_iter2->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
    ptr_to_new_token->type = DECIMAL_CONST_TT;

    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token, t_iter2, tokens)
    h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "]");
    ptr_to_new_token->line = t_iter2->e.body.line;
    ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;
  }

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter2, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), ");");
  ptr_to_new_token->line = t_iter2->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
  ptr_to_new_token->type = ID____________TT;

  new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
    ptr_to_new_token, t_iter2, tokens)
  h_TOKEN_alloc_strcpy_hs(&(ptr_to_new_token->value), "}");
  ptr_to_new_token->line = t_iter2->e.body.line;
  ptr_to_new_token->brackets_depth = t_iter2->e.body.brackets_depth;
  ptr_to_new_token->type = STDALN_SYM____TT;

  if (e_st_does_dollar_block_have_constants) {
    insert__str_uint32_str__to_the_left_of_iter(
      e_st_iter__i_point_to_instantiate_module,
      "updt_anc_set(&yQnztz, &yQnoto, yQc, yQc_ee, ",
      e_st_num_of_constants_in_dollar_block,
      "); yQcurrz = 0; yQcurro = 0; "
    );
  }
}



void
e_get_name_and_position_of_param_defining_cs__of_operator(
void) {
  /* {yQp[0] = *end_oper_parm* INSTANTIATE_MODULE   $      a   += b * c; */
  /*                                              iter2  iter1           */

  iter_sdll_TOKEN save_t_iter1 = t_iter1;
  iter_sdll_TOKEN look_ahead_iter = NULL;

  if ( (t_iter1 == NULL)
       ||
       (t_iter1->e.body.type != ID____________TT) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "an identifier is expected after `$' in "
            "instantiation on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  to_next_operator_token_of_same_depth_before_semicolumn_trNULLlfmst(&t_iter1);
  if ( (t_iter1 == NULL)
       ||
       (t_iter1->e.body.type != OPERATOR______TT) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "An operator token is expected after `$ <Argument>' in "
            "operator on line %s:%d.\n",
            ctqg_file_name, (t_iter2) ? t_iter2->e.body.line : 999999999);
    clean_exit(-1);
  }
  if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, "<=>") == 0 ) {
    h_TOKEN_str_delete(t_iter1->e.body.value);
    h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value), "a_swap_b");
    e_st_position_of_param_defining_cs_of_oper = 0;
  } else
  if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, ":=") == 0 ) {
    h_TOKEN_str_delete(t_iter1->e.body.value);
    h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value), "assign_value_of_b_to_a");
    e_st_position_of_param_defining_cs_of_oper = 0;
  } else
  if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, "+=") == 0 ) {
    look_ahead_iter = t_iter1;
    to_next_operator_token_of_same_depth_before_semicolumn_trNULLlfmst(
      &look_ahead_iter);
    if (look_ahead_iter == NULL) {
      h_TOKEN_str_delete(t_iter1->e.body.value);
      h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value), "a__eq__a_plus_b");
      e_st_position_of_param_defining_cs_of_oper = 0;
    } else
    if ( h_TOKEN_strcmp_hs(look_ahead_iter->e.body.value, "*") == 0 ) {
      h_TOKEN_str_delete(t_iter1->e.body.value);
      h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value),
        "a__eq__a_plus_b_times_c");
      e_st_position_of_param_defining_cs_of_oper = 0;
    } else
    {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unrecognized operator token after `$ <Argument> +=' in "
              "operator on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
  } else
  if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, "-=") == 0 ) {
    look_ahead_iter = t_iter1;
    to_next_operator_token_of_same_depth_before_semicolumn_trNULLlfmst(
      &look_ahead_iter);
    if (look_ahead_iter == NULL) {
      h_TOKEN_str_delete(t_iter1->e.body.value);
      h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value), "a__eq__a_minus_b");
      e_st_position_of_param_defining_cs_of_oper = 0;
    } else
    if ( h_TOKEN_strcmp_hs(look_ahead_iter->e.body.value, "*") == 0 ) {
      h_TOKEN_str_delete(t_iter1->e.body.value);
      h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value),
        "a__eq__a_minus_b_times_c");
      e_st_position_of_param_defining_cs_of_oper = 0;
    } else
    {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unrecognized operator token after `$ <Argument> -=' in "
              "operator on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
  } else
  if ( h_TOKEN_strcmp_hs(t_iter1->e.body.value, "^=") == 0 ) {
    look_ahead_iter = t_iter1;
    to_next_operator_token_of_same_depth_before_semicolumn_trNULLlfmst(
      &look_ahead_iter);
    if (look_ahead_iter == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "An integer comparison operator "
              "(`>', `>=', `<', `<=', `==' or `!=') is expected "
              "after `<Argument> ^= <Argument>' "
              "on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    } else
    if ( h_TOKEN_strcmp_hs(look_ahead_iter->e.body.value, ">") == 0 ) {
      h_TOKEN_str_delete(t_iter1->e.body.value);
      h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value),
        "a_greater_than_b__as_signed");
      e_st_position_of_param_defining_cs_of_oper = 1;
    } else
    if ( h_TOKEN_strcmp_hs(look_ahead_iter->e.body.value, ">=") == 0 ) {
      h_TOKEN_str_delete(t_iter1->e.body.value);
      h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value),
        "a_greater_than_or_eq_to_b__as_signed");
      e_st_position_of_param_defining_cs_of_oper = 1;
    } else
    if ( h_TOKEN_strcmp_hs(look_ahead_iter->e.body.value, "<") == 0 ) {
      h_TOKEN_str_delete(t_iter1->e.body.value);
      h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value),
        "a_less_than_b__as_signed");
      e_st_position_of_param_defining_cs_of_oper = 1;
    } else
    if ( h_TOKEN_strcmp_hs(look_ahead_iter->e.body.value, "<=") == 0 ) {
      h_TOKEN_str_delete(t_iter1->e.body.value);
      h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value),
        "a_less_than_or_eq_to_b__as_signed");
      e_st_position_of_param_defining_cs_of_oper = 1;
    } else
    if ( h_TOKEN_strcmp_hs(look_ahead_iter->e.body.value, "==") == 0 ) {
      h_TOKEN_str_delete(t_iter1->e.body.value);
      h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value),
        "is_a_eq_to_b");
      e_st_position_of_param_defining_cs_of_oper = 1;
    } else
    if ( h_TOKEN_strcmp_hs(look_ahead_iter->e.body.value, "!=") == 0 ) {
      h_TOKEN_str_delete(t_iter1->e.body.value);
      h_TOKEN_alloc_strcpy_hs(&(t_iter1->e.body.value),
        "is_a_not_eq_to_b");
      e_st_position_of_param_defining_cs_of_oper = 1;
    } else
    {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "An integer comparison operator "
              "(`>', `>=', `<', `<=', `==' or `!=') is expected "
              "after `<Argument> ^= <Argument>' "
              "on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
  }

  st_callee_name = t_iter1->e.body.value; /* Does not own memory */
  t_iter1 = save_t_iter1;
}



void /* Leaves `t_iter1' pointing to the delimiter `,', `)' or smth. else */
e_process_bit_constant(
var_decl_value_t * ptr_to_ee_var_decl) {
  str_on_heap_TOKEN bit_string; /* Does not own m. */
  uint32_t          value_of_bit_constant;

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  if ( (t_iter1 == NULL)
       ||
       (t_iter1->e.body.type != DECIMAL_CONST_TT) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "In instantiation on line %s:%d "
            "`0' or `1' expected after \"'\".\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  bit_string = t_iter1->e.body.value; /* Pointer assign., no string copy. */

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  if ( (t_iter1 == NULL)
       ||
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "'") != 0) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "In instantiation on line %s:%d \"'\" expected after "
            "`0' or `1' bit constant character.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  if (ptr_to_ee_var_decl->num_indexes != 0) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Type mismatch in instantiation on line %s:%d. "
            "Signal `",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    h_TOKEN_fprintf_str(
      stderr, ptr_to_ee_var_decl->ptr_back_to_name, 0, ' ', 'l');
    fprintf(stderr,
            "' of the callee is an array of dimension %u. "
            "It can not be mapped to a bit constant.\n",
            ptr_to_ee_var_decl->num_indexes);
    clean_exit(-1);
  }

  value_of_bit_constant = (bit_string->s.body[0] == '0') ? 0 : 1;

  insert__str_uint32_str__to_the_left_of_iter(
    e_st_iter__i_point_to_instantiate_module,
    "yQc_ee[",
    e_st_num_of_constants_in_dollar_block,
    "] = 1; ");

  insert__str_uint32_str_h_str_to_the_left_of_iter(
    e_st_iter__i_point_to_instantiate_module,
    "yQc[",
    (uint32_t) e_st_num_of_constants_in_dollar_block,
    "] = \"",
    bit_string,
    "\"; ");

  insert__s_ui32_s_hpostfix_s_to_the_left_of_iter(
    t_iter2,
    "map_bc(__FILE__, __LINE__, ",
    value_of_bit_constant,
    ", \"",
    ptr_to_ee_var_decl->ptr_back_to_name,
    "\", &yQcurrz, &yQcurro); "
  );

  e_st_does_dollar_block_have_constants = (1 != 0);
  e_st_does_module_use_constants = (1 != 0);
  e_st_num_of_constants_in_dollar_block++;
  if (e_st_num_of_constants_in_dollar_block
      >
      e_st_max_n_of_c_in_a_single_d_b_in_module) {
    e_st_max_n_of_c_in_a_single_d_b_in_module =
      e_st_num_of_constants_in_dollar_block;
  }
}



void /* Leaves `t_iter1' pointing to the delimiter `,', `)' or smth. else */
e_process_1d_bit_array_constant(
var_decl_value_t * ptr_to_ee_var_decl) {
  str_on_heap_TOKEN bit_string; /* Does not own m. */
  uint64_t          num_zeros;
  uint64_t          num_ones;

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  if ( (t_iter1 == NULL)
       ||
       (t_iter1->e.body.type != DECIMAL_CONST_TT) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "In instantiation on line %s:%d "
            "a string of `0's and `1's expected after `\"'.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  bit_string = t_iter1->e.body.value; /* Pointer assign., no string copy. */

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  if ( (t_iter1 == NULL)
       ||
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "\"") != 0) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "In instantiation on line %s:%d `\"' expected after "
            "the string of `0's and `1's.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  if (ptr_to_ee_var_decl->num_indexes != 1) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Type mismatch in instantiation on line %s:%d. "
            "Signal `",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    h_TOKEN_fprintf_str(
      stderr, ptr_to_ee_var_decl->ptr_back_to_name, 0, ' ', 'l');
    fprintf(stderr,
            "' of the callee is an array of dimension %u. "
            "It can not be mapped to a constant bit array of dimension 1.\n",
            ptr_to_ee_var_decl->num_indexes);
    clean_exit(-1);
  }

  insert__str_uint32_str_h_str_to_the_left_of_iter(
    e_st_iter__i_point_to_instantiate_module,
    "yQc_ee[",
    e_st_num_of_constants_in_dollar_block,
    "] = ",
    ptr_to_ee_var_decl->subst_indexes[0],
    "; ");

  insert__str_uint32_str_h_str_to_the_left_of_iter(
    e_st_iter__i_point_to_instantiate_module,
    "yQc[",
    (uint32_t) e_st_num_of_constants_in_dollar_block,
    "] = \"",
    bit_string,
    "\"; ");

  if (!h_TOKEN_count_zeros_and_ones_in_str(
         bit_string, &num_zeros, &num_ones)) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "In instantiation on line %s:%d : "
            "Constant bit array must consist of `0's and `1's only.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  insert__s_ui32_s_hpostfix_s_ui32_s_to_the_left_of_iter(
    t_iter2,
    "map_1dba(__FILE__, __LINE__, yQc[",
    (uint32_t) e_st_num_of_constants_in_dollar_block,
    "], \"",
    ptr_to_ee_var_decl->ptr_back_to_name,
    "\", &yQcurrz, &yQcurro, yQc_ee[",
    (uint32_t) (e_st_num_of_constants_in_dollar_block),
    "]); "
  );

  e_st_does_dollar_block_have_constants = (1 != 0);
  e_st_does_module_use_constants = (1 != 0);
  e_st_num_of_constants_in_dollar_block++;
  if (e_st_num_of_constants_in_dollar_block
      >
      e_st_max_n_of_c_in_a_single_d_b_in_module) {
    e_st_max_n_of_c_in_a_single_d_b_in_module =
      e_st_num_of_constants_in_dollar_block;
  }
}



void /* Leaves `t_iter1' pointing to the delimiter `,', `)' or smth. else */
e_process_integer_constant(
var_decl_value_t * ptr_to_ee_var_decl) {
  str_on_heap_TOKEN bit_string; /* Owns memory (unlike in e_process_1d_b...). */
  uint64_t          num_zeros;
  uint64_t          num_ones;
  static char       bc[MAX_NUM_BIN_DIGITS_IN_CONST + 1];

  if (t_iter1->e.body.type != DECIMAL_CONST_TT) { /* It is a "-". */
    h_TOKEN_alloc_strcpy_hs(&bit_string, "-");
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    if ( (t_iter1 == NULL)
         ||
         (t_iter1->e.body.type != DECIMAL_CONST_TT) ) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "In instantiation on line %s:%d "
              "a decimal constant expected after `-'.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
    h_TOKEN_strcat(bit_string, t_iter1->e.body.value);
  } else {
    h_TOKEN_alloc_strcpy_hh(&bit_string, t_iter1->e.body.value);
  }
  h_to_binary_s__as_signed(bit_string, bc);
  h_TOKEN_str_delete(bit_string);
  h_TOKEN_alloc_strcpy_hs(&bit_string, bc);

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  if (ptr_to_ee_var_decl->num_indexes != 1) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Type mismatch in instantiation on line %s:%d. "
            "Signal `",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    h_TOKEN_fprintf_str(
      stderr, ptr_to_ee_var_decl->ptr_back_to_name, 0, ' ', 'l');
    fprintf(stderr,
            "' of the callee is an array of dimension %u. "
            "It can not be mapped to a constant integer "
            "(that is to a bit array of dimension 1).\n",
            ptr_to_ee_var_decl->num_indexes);
    clean_exit(-1);
  }

  insert__str_uint32_str_h_str_to_the_left_of_iter(
    e_st_iter__i_point_to_instantiate_module,
    "yQc_ee[",
    e_st_num_of_constants_in_dollar_block,
    "] = ",
    ptr_to_ee_var_decl->subst_indexes[0],
    "; ");

  insert__str_uint32_str_h_str_to_the_left_of_iter(
    e_st_iter__i_point_to_instantiate_module,
    "yQc[",
    (uint32_t) e_st_num_of_constants_in_dollar_block,
    "] = \"",
    bit_string,
    "\"; ");

  if (!h_TOKEN_count_zeros_and_ones_in_str(
         bit_string, &num_zeros, &num_ones)) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "In instantiation on line %s:%d : "
            "Constant bit array must consist of `0's and `1's only.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  insert__s_ui32_s_hpostfix_s_ui32_s_to_the_left_of_iter(
    t_iter2,
    "map_1dba(__FILE__, __LINE__, yQc[",
    (uint32_t) e_st_num_of_constants_in_dollar_block,
    "], \"",
    ptr_to_ee_var_decl->ptr_back_to_name,
    "\", &yQcurrz, &yQcurro, yQc_ee[",
    (uint32_t) (e_st_num_of_constants_in_dollar_block),
    "]); "
  );

  e_st_does_dollar_block_have_constants = (1 != 0);
  e_st_does_module_use_constants = (1 != 0);
  e_st_num_of_constants_in_dollar_block++;
  if (e_st_num_of_constants_in_dollar_block
      >
      e_st_max_n_of_c_in_a_single_d_b_in_module) {
    e_st_max_n_of_c_in_a_single_d_b_in_module =
      e_st_num_of_constants_in_dollar_block;
  }

  h_TOKEN_str_delete(bit_string);
}



void /* Leaves `t_iter1' pointing to the delimiter `,', `)' or smth. else */
e_process_compound_signal(
var_decl_value_t * ptr_to_ee_var_decl) {
  uint32_t           i;
  str_on_heap_TOKEN  lo;
  str_on_heap_TOKEN  hi;
  uint32_t           num_r_indexes;
  str_on_heap_TOKEN  r_sig_name = t_iter1->e.body.value; /* Does not own m. */
  var_decl_value_t * ptr_to_r_var_decl =
    search_var_decl(st_module_name, t_iter1->e.body.value);

  if (ptr_to_r_var_decl == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "In instantiation on line %s:%d : "
            "signal `",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    h_TOKEN_fprintf_str(stderr, t_iter1->e.body.value, 0, ' ', 'l');
    fprintf(stderr, "' is not declared in module `");
    h_TOKEN_fprintf_str(stderr, st_module_name, 0, ' ', 'l');
    fprintf(stderr, "'.\n");
    clean_exit(-1);
  }

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  insert__str_uint32_str__to_the_left_of_iter(
    t_iter2, "yQni = ", ptr_to_r_var_decl->num_indexes, "; ");

  num_r_indexes = 0;
  for (i = 0; i < ptr_to_r_var_decl->num_indexes; i++) {
    read_range_in_brackets__alloc(&lo, &hi);
    if (hi != NULL) {
      num_r_indexes++;
      insert__str_uint32_str__to_the_left_of_iter(
        t_iter2, "yQir[", i, "] = 1; ");
      insert__str_uint32_str_h_str_to_the_left_of_iter(
        t_iter2, "yQil[", i, "] = ", lo, "; ");
      insert__str_uint32_str_h_str_to_the_left_of_iter(
        t_iter2, "yQih[", i, "] = ", hi, "; ");
      insert__str_uint32_str_h_str_to_the_left_of_iter(
        t_iter2, "yQim[", i, "] = ", ptr_to_r_var_decl->indexes[i], "; ");
        h_TOKEN_str_delete(lo);
        h_TOKEN_str_delete(hi);
    } else
    if (lo != NULL) {
      insert__str_uint32_str__to_the_left_of_iter(
        t_iter2, "yQir[", i, "] = 0; ");
      insert__str_uint32_str_h_str_to_the_left_of_iter(
        t_iter2, "yQil[", i, "] = ", lo, "; ");
      insert__str_uint32_str__to_the_left_of_iter(
        t_iter2, "yQih[", i, "] = 0; ");
      insert__str_uint32_str_h_str_to_the_left_of_iter(
        t_iter2, "yQim[", i, "] = ", ptr_to_r_var_decl->indexes[i], "; ");
        h_TOKEN_str_delete(lo);
    } else
    {
      num_r_indexes++;
      insert__str_uint32_str__to_the_left_of_iter(
        t_iter2, "yQir[", i, "] = 1; ");
      insert__str_uint32_str__to_the_left_of_iter(
        t_iter2, "yQil[", i, "] = 0; ");
      insert__str_uint32_str_h_str_to_the_left_of_iter(
        t_iter2, "yQih[", i, "] = ", ptr_to_r_var_decl->indexes[i], "-1; ");
      insert__str_uint32_str_h_str_to_the_left_of_iter(
        t_iter2, "yQim[", i, "] = ", ptr_to_r_var_decl->indexes[i], "; ");
    }
  }

  if (num_r_indexes != ptr_to_ee_var_decl->num_indexes) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Type mismatch in instantiation on line %s:%d. "
            "Signal `",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    h_TOKEN_fprintf_str(
      stderr, ptr_to_ee_var_decl->ptr_back_to_name, 0, ' ', 'l');
    fprintf(stderr,
            "' of the callee is an array of dimension %u. "
            "It can not be mapped to an array of dimension %u.\n",
            ptr_to_ee_var_decl->num_indexes,
            num_r_indexes);
    clean_exit(-1);
  }

  for (i = 0; i < ptr_to_ee_var_decl->num_indexes; i++) {
    insert__str_uint32_str_h_str_to_the_left_of_iter(
      t_iter2,
      "yQiee[",
      i,
      "] = ",
      ptr_to_ee_var_decl->subst_indexes[i],
      "; ");
  }

  insert__str_h_str_hpostfix_str_to_the_left_of_iter(
    t_iter2,
    "map_cs(__FILE__, __LINE__, \"",
    r_sig_name,
    "\", \"",
    ptr_to_ee_var_decl->ptr_back_to_name,
    "\"); ");
}



void
e_debug_print_decl_table(
void) {
  uint64_t i;
  uint64_t j;

  printf("debug_print_decl_table:\n");
  for (i = 0; i < st_var_decl_num_entries; i++) {
    printf("%2d. \"", (int) i);
    h_TOKEN_printf_str(
      st_var_decl_value_storage[i].ptr_back_to_name, 0, ' ', 'l');
    printf("\" type=%u #ind=%u {",
      st_var_decl_value_storage[i].signal_type,
      st_var_decl_value_storage[i].num_indexes
    );
    for (j = 0; j < st_var_decl_value_storage[i].num_indexes; j++) {
      h_TOKEN_printf_str(
        st_var_decl_value_storage[i].indexes[j], 0, ' ', 'l');
      if (j != st_var_decl_value_storage[i].num_indexes - 1) {
        printf(", ");
      }
    }
    printf("}  {");
    for (j = 0; j < st_var_decl_value_storage[i].num_indexes; j++) {
      h_TOKEN_printf_str(
        st_var_decl_value_storage[i].subst_indexes[j], 0, ' ', 'l');
      if (j != st_var_decl_value_storage[i].num_indexes - 1) {
        printf(", ");
      }
    }
    printf("}\n");
  }
}



void*
f_st_outside_module_body(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  uint64_t parenthesis_depth;

  while (t_iter1 != NULL) {
    if (t_iter1->e.body.type == ID____________TT) {
      if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, KEYWORD_MODULE) == 0) {
        break;
      }
    }
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  }

  if (t_iter1 == NULL) {
    rv = f_st_pre_termination;
  } else {
    /* `t_iter1' points to KEYWORD_MODULE. */
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    while (t_iter1 != NULL) {
      if (t_iter1->e.body.type == STDALN_SYM____TT) {
        if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "(") == 0) {
          break;
        }
      }
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    }

    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file after the last `%s' keyword.\n",
              KEYWORD_MODULE);
      clean_exit(-1);
    }

    /* `t_iter1' points to `('. */
    parenthesis_depth = 1;
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    while (t_iter1 != NULL) {
      if (t_iter1->e.body.type == STDALN_SYM____TT) {
        if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "(") == 0) {
          parenthesis_depth++;
        }
        if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0) {
          parenthesis_depth--;
          if (parenthesis_depth == 0) {
            break;
          }
        }
      }
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    }

    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file after the last `%s' keyword. "
              "Possibly unbalanced `(' and `)'.\n",
              KEYWORD_MODULE);
      clean_exit(-1);
    }

    /* `t_iter1' points to `)'. */
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    if ( (t_iter1 == NULL)
         ||
         (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "{") != 0) ) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "`{' expected before the body of module "
              "on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }

    /* `t_iter1' points to `{'. */
    f_st_iter__insert_if_else_vars_to_the_left_of_me = t_iter1;
    move_iter_right_sdll_TOKEN(
      f_st_iter__insert_if_else_vars_to_the_left_of_me
    )
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = f_st_inside_module_body;
  }

  return (void*) rv;
}



void*
f_st_inside_module_body(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;
  uint64_t                      curly_parenthesis_depth = 1;
  iter_sdll_TOKEN               iter__i_point_to_dollar_sign = NULL;
  TOKEN *                       ptr_to_new_token;

  f_st_peak_number_of_oto_ancillas = 0;

  while (1 != 0) {
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file: unbalanced `{' and `}'.\n");
      clean_exit(-1);
    } else
    if (t_iter1->e.body.type == STDALN_SYM____TT) {
      if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "{") == 0) {
        curly_parenthesis_depth++;
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      } else
      if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "}") == 0) {
        curly_parenthesis_depth--;
        if (curly_parenthesis_depth == 0) {
          to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
          rv = f_st_outside_module_body;
          break;
        }
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      } else
      if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "$") == 0) {
        /* Function call replaces the whole $if, $else or $endif       */
        /* statement with proper lower level code and leaves `t_iter1' */
        /* pointing to the token located mmediately after this code.   */
        iter__i_point_to_dollar_sign = t_iter1;
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
        if (t_iter1 == NULL) {
          fprintf(stderr, "Error.\n");
          fprintf(stderr,
                  "Unexpected end of file after the last `$'.\n");
          clean_exit(-1);
        } else
        if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "if") == 0) {
          f_if_to_layer_e(iter__i_point_to_dollar_sign);
        } else
        if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "else") == 0) {
          f_else_to_layer_e(iter__i_point_to_dollar_sign);
        } else
        if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "endif") == 0) {
          f_endif_to_layer_e(iter__i_point_to_dollar_sign);
        }
        iter__i_point_to_dollar_sign = NULL;
      } else {
        to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
      }
    } else {
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    }
  }

  /* --- BEGIN Insert if-else related declarations of variables. --- */
  if (f_st_peak_number_of_oto_ancillas == 0) {
    new_to_the_left_of_nonNULL_iter_sdll_TOKEN(
      ptr_to_new_token,
      f_st_iter__insert_if_else_vars_to_the_left_of_me,
      tokens)
    h_TOKEN_alloc_strcpy_hs(
      &(ptr_to_new_token->value),
      "/*no_if_else*/ ");
    ptr_to_new_token->line =
      f_st_iter__insert_if_else_vars_to_the_left_of_me->e.body.line;
    ptr_to_new_token->brackets_depth =
      f_st_iter__insert_if_else_vars_to_the_left_of_me->e.body.brackets_depth;
    ptr_to_new_token->type = ID____________TT;
  } else {
    insert__strid__to_the_left_of_iter(
      f_st_iter__insert_if_else_vars_to_the_left_of_me,
      "uint64_t yQtif, yQnifa = 0; "
    );
    insert__strid__to_the_left_of_iter(
      f_st_iter__insert_if_else_vars_to_the_left_of_me,
      "one_to_one"
    );
    insert__space__to_the_left_of_iter(
      f_st_iter__insert_if_else_vars_to_the_left_of_me
    );
    insert__strid__to_the_left_of_iter(
      f_st_iter__insert_if_else_vars_to_the_left_of_me,
      "yQotoif"
    );
    insert__bracket_uint32_bracket__to_the_left_of_iter(
      f_st_iter__insert_if_else_vars_to_the_left_of_me,
      (uint32_t) f_st_peak_number_of_oto_ancillas
    );
    insert__semicolumn_space__to_the_left_of_iter(
      f_st_iter__insert_if_else_vars_to_the_left_of_me
    );
  }
  f_st_peak_number_of_oto_ancillas = 888888;
  f_st_iter__insert_if_else_vars_to_the_left_of_me = NULL;
  /* ---  END  Insert if-else related declarations of variables. --- */

  return (void*) rv;
}



void*
f_st_pre_termination(
void) {
  /* Do not do! delete_st_param_array(); */
  /* Do not do! delete_st_var_decl_sgt(); */
  /* Do not do! st_var_decl_num_entries = 0; */
  /* Do not do! st_callee_name = NULL; */ /* Does not own memory. */
  /* Do not do! st_module_name = NULL; */ /* Does not own memory. */
  t_iter1 = NULL;
  t_iter2 = NULL;

  return (void*) termination_state;
}



void
f_if_to_layer_e(
iter_sdll_TOKEN iter__i_point_to_dollar_sign) {
  iter_sdll_TOKEN first_token_of_duplicate_me_block = NULL;
  iter_sdll_TOKEN first_token_after_duplicate_me_block = NULL;
  iter_sdll_TOKEN iter = NULL;

  f_st_number_of_oto_ancillas = 0;

  insert__strid__to_the_left_of_iter(
    iter__i_point_to_dollar_sign,
    "one_to_garbage_ancilla(__FILE__, __LINE__, \"yQifa\", yQnifa); "
  );
  first_token_of_duplicate_me_block = iter__i_point_to_dollar_sign;
  move_iter_left_sdll_TOKEN(first_token_of_duplicate_me_block);

  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  if ( (t_iter1 == NULL)
       ||
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "(") != 0) ) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "`(' expected after `$if' "
            "on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);

  while (!f_process_one_term_of_if(iter__i_point_to_dollar_sign));
  move_iter_right_sdll_TOKEN(first_token_of_duplicate_me_block);

  insert__str_uint32_str__to_the_left_of_iter(
    iter__i_point_to_dollar_sign,
    "for (yQtif = 0; yQtif < ",
    (uint32_t) f_st_number_of_oto_ancillas,
    "; yQtif++) {push_control_signal"
    "(__FILE__,__LINE__,\"yQotoif\",yQtif,&stack_of_control_signals);} "
  );
  first_token_after_duplicate_me_block = iter__i_point_to_dollar_sign;
  move_iter_left_sdll_TOKEN(first_token_after_duplicate_me_block);
  move_iter_left_sdll_TOKEN(first_token_after_duplicate_me_block);
  move_iter_left_sdll_TOKEN(first_token_after_duplicate_me_block);
  insert__str_uint32_str__to_the_left_of_iter(
    iter__i_point_to_dollar_sign,
    "INSTANTIATE_MODULE "
    "MAP_SIGNAL(\"x\", 0, \"yQifa\", yQnifa) "
    "not(LOCATION_INFO); "
    "for (yQtif = 0; yQtif < ",
    (uint32_t) f_st_number_of_oto_ancillas,
    "; yQtif++) "
    "{pop_and_delete_control_signal("
    "__FILE__,__LINE__,&stack_of_control_signals);} "
  );

  iter = first_token_of_duplicate_me_block;
  while(iter != first_token_after_duplicate_me_block) {
    copy_token_to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      &(iter->e.body)
    );
    move_iter_right_sdll_TOKEN(iter)
  }

  insert__strid__to_the_left_of_iter(
    iter__i_point_to_dollar_sign,
    "push_control_signal("
    "__FILE__,__LINE__,\"yQifa\",yQnifa,&stack_of_control_signals); "
    "yQnifa++; "
  );

  while (iter__i_point_to_dollar_sign != t_iter1) {
    move_iter_and_delete_to_next_non_separ_token(
      &iter__i_point_to_dollar_sign
    );
  }
}



void
f_else_to_layer_e(
iter_sdll_TOKEN iter__i_point_to_dollar_sign) {
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  if (t_iter1 == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Unexpected end of file after `$else' "
            "on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  insert__strid__to_the_left_of_iter(
    iter__i_point_to_dollar_sign,
    "yQtif = pop_n_del_cs_for_else"
    "(__FILE__, __LINE__, &stack_of_control_signals); "
    "INSTANTIATE_MODULE MAP_SIGNAL(\"x\", 0, \"yQifa\", yQtif) "
    "not(LOCATION_INFO); "
    "push_control_signal"
    "(__FILE__,__LINE__,\"yQifa\",yQtif,&stack_of_control_signals); "
  );

  while (iter__i_point_to_dollar_sign != t_iter1) {
    move_iter_and_delete_to_next_non_separ_token(
      &iter__i_point_to_dollar_sign
    );
  }
}



void
f_endif_to_layer_e(
iter_sdll_TOKEN iter__i_point_to_dollar_sign) {
  to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  if (t_iter1 == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Unexpected end of file after `$endif' "
            "on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  insert__strid__to_the_left_of_iter(
    iter__i_point_to_dollar_sign,
    "pop_and_delete_control_signal"
    "(__FILE__, __LINE__, &stack_of_control_signals); "
  );

  while (iter__i_point_to_dollar_sign != t_iter1) {
    move_iter_and_delete_to_next_non_separ_token(
      &iter__i_point_to_dollar_sign
    );
  }
}



int /* Returns 1 != 0 if this is the last term, 0 != 0 otherwise. */
f_process_one_term_of_if(
iter_sdll_TOKEN iter__i_point_to_dollar_sign) {
  int             rv = (1 != 0);
  iter_sdll_TOKEN i1 = t_iter1;
  iter_sdll_TOKEN i2 = NULL;
  iter_sdll_TOKEN iter = NULL;

  if (t_iter1 == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Unexpected end of file after `$if (' "
            "on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  }

  if (h_TOKEN_strcmp_hs(i1->e.body.value, "!") == 0) {
    /* It is a `not(compound_signal[...])'. */
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file after `$if (' "
              "on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
    i2 = t_iter1;
    to_next_operator_or_closed_parenthesis_token_of_same_depth_trNULLlfmst(
      &t_iter1);
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Unexpected end of file after `$if (' "
              "on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "||") == 0) {
      rv = (0 != 0);
    } else
    if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0) {
      rv = (1 != 0);
    } else {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Syntax error after `$if (' "
              "on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
    insert__strstdaln__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      "$"
    );
    insert__space__to_the_left_of_iter(
      iter__i_point_to_dollar_sign
    );
    insert__strid__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      "cnot"
    );
    insert__strstdaln__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      "("
    );
    iter = i2;
    while(iter != t_iter1) {
      copy_token_to_the_left_of_iter(
        iter__i_point_to_dollar_sign,
        &(iter->e.body)
      );
      move_iter_right_sdll_TOKEN(iter)
    }
    insert__strstdaln__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      ","
    );
    insert__strid__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      "yQotoif"
    );
    insert__bracket_uint32_bracket__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      (uint32_t) f_st_number_of_oto_ancillas
    );
    insert__strstdaln__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      ")"
    );
    insert__semicolumn_space__to_the_left_of_iter(
      iter__i_point_to_dollar_sign
    );
    insert__strstdaln__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      "$"
    );
    insert__space__to_the_left_of_iter(
      iter__i_point_to_dollar_sign
    );
    insert__strid__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      "not"
    );
    insert__strstdaln__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      "("
    );
    insert__strid__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      "yQotoif"
    );
    insert__bracket_uint32_bracket__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      (uint32_t) f_st_number_of_oto_ancillas
    );
    insert__strstdaln__to_the_left_of_iter(
      iter__i_point_to_dollar_sign,
      ")"
    );
    insert__semicolumn_space__to_the_left_of_iter(
      iter__i_point_to_dollar_sign
    );
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
  } else
  if (i1->e.body.type != ID____________TT) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Syntax error after `$if (' "
            "on line %s:%d.\n",
            ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
    clean_exit(-1);
  } else
  {
    /* It is an other than `not(compound_signal[...])' term. */
    to_next_operator_or_closed_parenthesis_token_of_same_depth_trNULLlfmst(
      &t_iter1);
    if (t_iter1 == NULL) {
      fprintf(stderr, "Error.\n");
      fprintf(stderr,
              "Syntax error after after `$if (' "
              "on line %s:%d.\n",
              ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
      clean_exit(-1);
    }
    if ( (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "||") == 0)
         ||
         (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0) ) {
      /* It is a `compound_signal[...]' term. */
      if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "||") == 0) {
        rv = (0 != 0);
      } else
      if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0) {
        rv = (1 != 0);
      } else {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
          "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
        clean_exit(-2);
      }
      insert__strstdaln__to_the_left_of_iter(
        iter__i_point_to_dollar_sign,
        "$"
      );
      insert__space__to_the_left_of_iter(
        iter__i_point_to_dollar_sign
      );
      insert__strid__to_the_left_of_iter(
        iter__i_point_to_dollar_sign,
        "cnot"
      );
      insert__strstdaln__to_the_left_of_iter(
        iter__i_point_to_dollar_sign,
        "("
      );
      iter = i1;
      while(iter != t_iter1) {
        copy_token_to_the_left_of_iter(
          iter__i_point_to_dollar_sign,
          &(iter->e.body)
        );
        move_iter_right_sdll_TOKEN(iter)
      }
      insert__strstdaln__to_the_left_of_iter(
        iter__i_point_to_dollar_sign,
        ","
      );
      insert__strid__to_the_left_of_iter(
        iter__i_point_to_dollar_sign,
        "yQotoif"
      );
      insert__bracket_uint32_bracket__to_the_left_of_iter(
        iter__i_point_to_dollar_sign,
        (uint32_t) f_st_number_of_oto_ancillas
      );
      insert__strstdaln__to_the_left_of_iter(
        iter__i_point_to_dollar_sign,
        ")"
      );
      insert__semicolumn_space__to_the_left_of_iter(
        iter__i_point_to_dollar_sign
      );
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    } else {
      /* It is an integer comparison term. */
      to_next_operator_or_closed_parenthesis_token_of_same_depth_trNULLlfmst(
        &t_iter1);
      if (t_iter1 == NULL) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Syntax error after after `$if (' "
                "on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      if ( (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "||") != 0)
           &&
           (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") != 0) ) {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
                "Syntax error after after `$if (' "
                "on line %s:%d.\n",
                ctqg_file_name, (t_iter1) ? t_iter1->e.body.line : 999999999);
        clean_exit(-1);
      }
      if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "||") == 0) {
        rv = (0 != 0);
      } else
      if (h_TOKEN_strcmp_hs(t_iter1->e.body.value, ")") == 0) {
        rv = (1 != 0);
      } else {
        fprintf(stderr, "Error.\n");
        fprintf(stderr,
          "An internal CTQG error occurred: %s:%d.\n", __FILE__, __LINE__);
        clean_exit(-2);
      }
      insert__strstdaln__to_the_left_of_iter(
        iter__i_point_to_dollar_sign,
        "$"
      );
      insert__space__to_the_left_of_iter(
        iter__i_point_to_dollar_sign
      );
      insert__strid__to_the_left_of_iter(
        iter__i_point_to_dollar_sign,
        "yQotoif"
      );
      insert__bracket_uint32_bracket__to_the_left_of_iter(
        iter__i_point_to_dollar_sign,
        (uint32_t) f_st_number_of_oto_ancillas
      );
      insert__stroper__to_the_left_of_iter(
        iter__i_point_to_dollar_sign,
        "^="
      );
      insert__space__to_the_left_of_iter(
        iter__i_point_to_dollar_sign
      );
      iter = i1;
      while(iter != t_iter1) {
        copy_token_to_the_left_of_iter(
          iter__i_point_to_dollar_sign,
          &(iter->e.body)
        );
        move_iter_right_sdll_TOKEN(iter)
      }
      insert__semicolumn_space__to_the_left_of_iter(
        iter__i_point_to_dollar_sign
      );
      to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    }
  }

  f_st_number_of_oto_ancillas++;
  if (f_st_number_of_oto_ancillas > f_st_peak_number_of_oto_ancillas) {
    f_st_peak_number_of_oto_ancillas = f_st_number_of_oto_ancillas;
  }
  return rv;
}



void*
ctqg_to_nc_st_not_inside_comment(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;

  if (t_iter1 == NULL) {
    rv = termination_state;
  } else
  if ( (t_iter1->e.body.type == OPERATOR______TT)
       &&
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "/*") == 0) ) {
    /* Do not move `t_iter1'. */
    rv = ctqg_to_nc_st_inside_c_style_comment;
  } else {
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    rv = ctqg_to_nc_st_not_inside_comment;
  }

  return (void*) rv;
}



void*
ctqg_to_nc_st_inside_c_style_comment(
void) {
  STATE_OF_AUTOMATON_func_ptr_t rv = NULL;

  if (t_iter1 == NULL) {
    fprintf(stderr, "Error.\n");
    fprintf(stderr,
            "Unterminated `/* ... */' style comment in file `%s'.\n",
            ctqg_file_name);
    clean_exit(-1);
  } else
  if ( (t_iter1->e.body.type == OPERATOR______TT)
       &&
       (h_TOKEN_strcmp_hs(t_iter1->e.body.value, "*/") == 0) ) {
    t_iter2 = t_iter1;
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    delete_TOKEN_and_move_iter_to_right(&t_iter2, &tokens);
    rv = ctqg_to_nc_st_not_inside_comment;
  } else {
    t_iter2 = t_iter1;
    to_next_non_separ_token_treating_NULL_as_leftmost(&t_iter1);
    delete_TOKEN_and_move_iter_to_right(&t_iter2, &tokens);
    rv = ctqg_to_nc_st_inside_c_style_comment;
  }

  return (void*) rv;
}



int
main(
int argc, char ** argv) {
  uint64_t length_of_file_name;
  if (argc < 2) {
    fprintf(stderr,
            "Error.\n"
            "No input file.\n");
    clean_exit(-1);
  }

  length_of_file_name = strlen(argv[1]);
  if (length_of_file_name >= 4096) {
    fprintf(stderr,
            "Error.\n"
            "The name of the input file is too long.\n");
    clean_exit(-1);
  }
  strcpy(ctqg_file_name, argv[1]);
  strcpy(bbrl_file_name, argv[1]);
  if ( (length_of_file_name < 6) ||
       (bbrl_file_name[length_of_file_name - 1] != 'g') ||
       (bbrl_file_name[length_of_file_name - 2] != 'q') ||
       (bbrl_file_name[length_of_file_name - 3] != 't') ||
       (bbrl_file_name[length_of_file_name - 4] != 'c') ||
       (bbrl_file_name[length_of_file_name - 5] != '.') ) {
    fprintf(stderr,
            "Error.\n"
            "Input file must have extension `.ctqg'.\n");
    clean_exit(-1);
  }

  bbrl_file_name[length_of_file_name - 1] = '\0';
  bbrl_file_name[length_of_file_name - 2] = '\0';
  bbrl_file_name[length_of_file_name - 3] = '\0';
  bbrl_file_name[length_of_file_name - 4] = 'c';

  memory_manager_message_level = 0; /* 1 */



  tokenize(); /* fprint_tokens(stdout, tokens); */
  ctqg_to_nc(); /* Only remove C style comments.                  */
  nc_to_lf();   /* Only duplicate `main_module'.                  */
  lf_to_le();   /* Only unfold $if-$else-$endif statements.       */
  le_to_ld();
  ld_to_bbrl();
  assemble_tokens();



  clean_exit(0);
  return 0; /* Make compiler happy. */
}
