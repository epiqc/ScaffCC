#ifndef CTQG________________SDLL_SGT_H
#define CTQG________________SDLL_SGT_H



#include <stdio.h>  /* `NULL' pointer */
#include <stdint.h> /* int64_t type   */



union abstract_el_of_heap_TOKEN;

typedef union
abstract_el_of_heap_TOKEN * ptr_to_node_of_sdll_sgt_TOKEN;
typedef union
abstract_el_of_heap_TOKEN * str_on_heap_TOKEN;
typedef union
abstract_el_of_heap_TOKEN * ptr_to_el_of_sll_of_unused_el_of_heap_TOKEN;

typedef union abstract_el_of_heap_TOKEN * iter_sdll_TOKEN;
typedef union abstract_el_of_heap_TOKEN * ptr_to_entry_of_sgt_TOKEN;

typedef enum {
  ID____________TT,
  DECIMAL_CONST_TT,
  SPACE_________TT,
  PERIODS_______TT,
  OPERATOR______TT,
  END_OF_LINE___TT,
  BRACKETS______TT,
  STDALN_SYM____TT,
  INVALID_______TT
} token_type_t;

typedef struct {
  str_on_heap_TOKEN value;
  uint32_t          line;
  uint32_t          brackets_depth;
  token_type_t      type;
} TOKEN;

void
print_to_file_TOKEN(
FILE * f, TOKEN * arg);

#define \
TOKEN_arg1_GREATER_THAN_arg2(\
arg1, arg2) \
(h_TOKEN_strcmp_hh((arg1).value, (arg2).value) > 0)



struct node_of_sdll_sgt_TOKEN {
  TOKEN body;
  ptr_to_node_of_sdll_sgt_TOKEN ptr_to_left;
  ptr_to_node_of_sdll_sgt_TOKEN ptr_to_right;
};

struct s_block_TOKEN {
  char body[sizeof(struct node_of_sdll_sgt_TOKEN)
            -
            sizeof(str_on_heap_TOKEN)];
  str_on_heap_TOKEN ptr_to_next_s_block;
};

struct el_of_sll_of_unused_el_of_heap_TOKEN {
  ptr_to_el_of_sll_of_unused_el_of_heap_TOKEN ptr_to_next_el_of_sll;
};

union abstract_el_of_heap_TOKEN {
  struct node_of_sdll_sgt_TOKEN               e;
  struct s_block_TOKEN                        s;
  struct el_of_sll_of_unused_el_of_heap_TOKEN u;
};



typedef struct {
  ptr_to_node_of_sdll_sgt_TOKEN ptr_to_leftmost;
  ptr_to_node_of_sdll_sgt_TOKEN ptr_to_rightmost;
} sdll_TOKEN;

typedef struct {
  ptr_to_node_of_sdll_sgt_TOKEN ptr_to_root;
  int64_t                      num_entries;
  int64_t                      max_size_since_full_rebuild;
} sgt_TOKEN;

struct elem_of_stack_of_allocated_blocks_TOKEN {
  union abstract_el_of_heap_TOKEN                  body[99];
  struct elem_of_stack_of_allocated_blocks_TOKEN * ptr_to_next_elem_of_stack;
};



extern ptr_to_el_of_sll_of_unused_el_of_heap_TOKEN
  sll_of_unused_el_of_sdll_TOKEN;
extern union abstract_el_of_heap_TOKEN *
  temp_for_internal_use_TOKEN;



void
print_TOKEN(
TOKEN * arg);

void
new_block_of_99_elem_of_sdll_TOKEN(
void);

int
is_heap_TOKEN_empty(
void);

void
reset_heap_TOKEN(
void);

void
print_to_file_sdll_TOKEN(
FILE * f, sdll_TOKEN sdll);

void
print_sdll_TOKEN(
sdll_TOKEN sdll);

int64_t
count_elements_sdll_TOKEN(
sdll_TOKEN sdll);



#define \
set_iter_to_leftmost_el_sdll_TOKEN(\
target_iter, sdll) {\
(target_iter) = (sdll).ptr_to_leftmost;}

#define \
set_iter_to_rightmost_el_sdll_TOKEN(\
target_iter, sdll) {\
(target_iter) = (sdll).ptr_to_rightmost;}

#define \
move_iter_left_sdll_TOKEN(\
iter) {\
(iter) = (iter)->e.ptr_to_left;}

#define \
move_iter_right_sdll_TOKEN(\
iter) {\
(iter) = (iter)->e.ptr_to_right;}

#define \
new_at_the_left_end_sdll_TOKEN(\
target, sdll) {\
if (sll_of_unused_el_of_sdll_TOKEN == NULL)\
new_block_of_99_elem_of_sdll_TOKEN();\
temp_for_internal_use_TOKEN = sll_of_unused_el_of_sdll_TOKEN;\
sll_of_unused_el_of_sdll_TOKEN =\
sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;\
temp_for_internal_use_TOKEN->e.ptr_to_left = NULL;\
if ((temp_for_internal_use_TOKEN->e.ptr_to_right =\
(sdll).ptr_to_leftmost) == NULL) {\
(sdll).ptr_to_rightmost = temp_for_internal_use_TOKEN;\
} else {\
(sdll).ptr_to_leftmost->e.ptr_to_left =\
temp_for_internal_use_TOKEN;}\
(sdll).ptr_to_leftmost =\
temp_for_internal_use_TOKEN;\
(target) = &(temp_for_internal_use_TOKEN->e.body);}

#define \
new_at_the_right_end_sdll_TOKEN(\
target, sdll) {\
if (sll_of_unused_el_of_sdll_TOKEN == NULL)\
new_block_of_99_elem_of_sdll_TOKEN();\
temp_for_internal_use_TOKEN = sll_of_unused_el_of_sdll_TOKEN;\
sll_of_unused_el_of_sdll_TOKEN =\
sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;\
temp_for_internal_use_TOKEN->e.ptr_to_right = NULL;\
if ((temp_for_internal_use_TOKEN->e.ptr_to_left =\
(sdll).ptr_to_rightmost) == NULL) {\
(sdll).ptr_to_leftmost = temp_for_internal_use_TOKEN;\
} else {\
(sdll).ptr_to_rightmost->e.ptr_to_right =\
temp_for_internal_use_TOKEN;}\
(sdll).ptr_to_rightmost =\
temp_for_internal_use_TOKEN;\
(target) = &(temp_for_internal_use_TOKEN->e.body);}

#define \
new_to_the_left_of_nonNULL_iter_sdll_TOKEN(\
target, iter, sdll) {\
if (sll_of_unused_el_of_sdll_TOKEN == NULL)\
new_block_of_99_elem_of_sdll_TOKEN();\
temp_for_internal_use_TOKEN = sll_of_unused_el_of_sdll_TOKEN;\
sll_of_unused_el_of_sdll_TOKEN =\
sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;\
temp_for_internal_use_TOKEN->e.ptr_to_right = (iter);\
if ((temp_for_internal_use_TOKEN->e.ptr_to_left =\
(iter)->e.ptr_to_left) == NULL) {\
(sdll).ptr_to_leftmost = temp_for_internal_use_TOKEN;\
} else {\
(iter)->e.ptr_to_left->e.ptr_to_right =\
temp_for_internal_use_TOKEN;}\
(iter)->e.ptr_to_left =\
temp_for_internal_use_TOKEN;\
(target) = &(temp_for_internal_use_TOKEN->e.body);}

#define \
new_to_the_right_of_nonNULL_iter_sdll_TOKEN(\
target, iter, sdll) {\
if (sll_of_unused_el_of_sdll_TOKEN == NULL)\
new_block_of_99_elem_of_sdll_TOKEN();\
temp_for_internal_use_TOKEN = sll_of_unused_el_of_sdll_TOKEN;\
sll_of_unused_el_of_sdll_TOKEN =\
sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;\
temp_for_internal_use_TOKEN->e.ptr_to_left = (iter);\
if ((temp_for_internal_use_TOKEN->e.ptr_to_right =\
(iter)->e.ptr_to_right) == NULL) {\
(sdll).ptr_to_rightmost = temp_for_internal_use_TOKEN;\
} else {\
(iter)->e.ptr_to_right->e.ptr_to_left =\
temp_for_internal_use_TOKEN;}\
(iter)->e.ptr_to_right =\
temp_for_internal_use_TOKEN;\
(target) = &(temp_for_internal_use_TOKEN->e.body);}

#define \
delete_leftmost_sdll_TOKEN(\
sdll) {\
temp_for_internal_use_TOKEN = (sdll).ptr_to_leftmost;\
if (temp_for_internal_use_TOKEN ==\
(sdll).ptr_to_rightmost) {\
(sdll).ptr_to_rightmost = NULL;\
} else {\
temp_for_internal_use_TOKEN->e.ptr_to_right->e.ptr_to_left\
= NULL;} (sdll).ptr_to_leftmost =\
temp_for_internal_use_TOKEN->e.ptr_to_right;\
temp_for_internal_use_TOKEN->u.ptr_to_next_el_of_sll =\
sll_of_unused_el_of_sdll_TOKEN;\
sll_of_unused_el_of_sdll_TOKEN =\
temp_for_internal_use_TOKEN;}

#define \
delete_rightmost_sdll_TOKEN(\
sdll) {\
temp_for_internal_use_TOKEN = (sdll).ptr_to_rightmost;\
if (temp_for_internal_use_TOKEN ==\
(sdll).ptr_to_leftmost) {\
(sdll).ptr_to_leftmost = NULL;\
} else {\
temp_for_internal_use_TOKEN->e.ptr_to_left->e.ptr_to_right\
= NULL;} (sdll).ptr_to_rightmost =\
temp_for_internal_use_TOKEN->e.ptr_to_left;\
temp_for_internal_use_TOKEN->u.ptr_to_next_el_of_sll =\
sll_of_unused_el_of_sdll_TOKEN;\
sll_of_unused_el_of_sdll_TOKEN =\
temp_for_internal_use_TOKEN;}

#define \
delete_and_move_iter_to_left_sdll_TOKEN(\
iter, sdll) {\
temp_for_internal_use_TOKEN = (iter);\
if ((iter)->e.ptr_to_left == NULL) {\
(sdll).ptr_to_leftmost = (iter)->e.ptr_to_right;\
} else {\
(iter)->e.ptr_to_left->e.ptr_to_right = (iter)->e.ptr_to_right;}\
if ((iter)->e.ptr_to_right == NULL) {\
(sdll).ptr_to_rightmost = (iter)->e.ptr_to_left;\
} else {\
(iter)->e.ptr_to_right->e.ptr_to_left = (iter)->e.ptr_to_left;}\
(iter) = (iter)->e.ptr_to_left;\
temp_for_internal_use_TOKEN->u.ptr_to_next_el_of_sll =\
sll_of_unused_el_of_sdll_TOKEN;\
sll_of_unused_el_of_sdll_TOKEN =\
temp_for_internal_use_TOKEN;}

#define \
delete_and_move_iter_to_right_sdll_TOKEN(\
iter, sdll) {\
temp_for_internal_use_TOKEN = (iter);\
if ((iter)->e.ptr_to_left == NULL) {\
(sdll).ptr_to_leftmost = (iter)->e.ptr_to_right;\
} else {\
(iter)->e.ptr_to_left->e.ptr_to_right = (iter)->e.ptr_to_right;}\
if ((iter)->e.ptr_to_right == NULL) {\
(sdll).ptr_to_rightmost = (iter)->e.ptr_to_left;\
} else {\
(iter)->e.ptr_to_right->e.ptr_to_left = (iter)->e.ptr_to_left;}\
(iter) = (iter)->e.ptr_to_right;\
temp_for_internal_use_TOKEN->u.ptr_to_next_el_of_sll =\
sll_of_unused_el_of_sdll_TOKEN;\
sll_of_unused_el_of_sdll_TOKEN =\
temp_for_internal_use_TOKEN;}

#define \
delete_and_iter_becomes_invalid_sdll_TOKEN(\
iter, sdll) {\
if ((iter)->e.ptr_to_left == NULL) {\
(sdll).ptr_to_leftmost = (iter)->e.ptr_to_right;\
} else {\
(iter)->e.ptr_to_left->e.ptr_to_right = (iter)->e.ptr_to_right;}\
if ((iter)->e.ptr_to_right == NULL) {\
(sdll).ptr_to_rightmost = (iter)->e.ptr_to_left;\
} else {\
(iter)->e.ptr_to_right->e.ptr_to_left = (iter)->e.ptr_to_left;}\
(iter)->u.ptr_to_next_el_of_sll = sll_of_unused_el_of_sdll_TOKEN;\
sll_of_unused_el_of_sdll_TOKEN = (iter);}

#define \
move_elem_to_the_left_end_sdll_TOKEN(\
target_sdll, iter_elem, source_sdll) {\
if ((iter_elem)->e.ptr_to_left == NULL) {\
(source_sdll).ptr_to_leftmost = (iter_elem)->e.ptr_to_right;\
} else {\
(iter_elem)->e.ptr_to_left->e.ptr_to_right = (iter_elem)->e.ptr_to_right;}\
if ((iter_elem)->e.ptr_to_right == NULL) {\
(source_sdll).ptr_to_rightmost = (iter_elem)->e.ptr_to_left;\
} else {\
(iter_elem)->e.ptr_to_right->e.ptr_to_left = (iter_elem)->e.ptr_to_left;}\
iter_elem->e.ptr_to_left = NULL;\
if ((iter_elem->e.ptr_to_right =\
(target_sdll).ptr_to_leftmost) == NULL) {\
(target_sdll).ptr_to_rightmost = iter_elem;\
} else {\
(target_sdll).ptr_to_leftmost->e.ptr_to_left =\
iter_elem;}\
(target_sdll).ptr_to_leftmost =\
iter_elem;}

#define \
move_elem_to_the_right_end_sdll_TOKEN(\
target_sdll, iter_elem, source_sdll) {\
if ((iter_elem)->e.ptr_to_left == NULL) {\
(source_sdll).ptr_to_leftmost = (iter_elem)->e.ptr_to_right;\
} else {\
(iter_elem)->e.ptr_to_left->e.ptr_to_right = (iter_elem)->e.ptr_to_right;}\
if ((iter_elem)->e.ptr_to_right == NULL) {\
(source_sdll).ptr_to_rightmost = (iter_elem)->e.ptr_to_left;\
} else {\
(iter_elem)->e.ptr_to_right->e.ptr_to_left = (iter_elem)->e.ptr_to_left;}\
iter_elem->e.ptr_to_right = NULL;\
if ((iter_elem->e.ptr_to_left =\
(target_sdll).ptr_to_rightmost) == NULL) {\
(target_sdll).ptr_to_leftmost = iter_elem;\
} else {\
(target_sdll).ptr_to_rightmost->e.ptr_to_right =\
iter_elem;}\
(target_sdll).ptr_to_rightmost =\
iter_elem;}

#define \
merge_sdll_TOKEN(\
target_sdll, left_source_sdll, right_source_sdll) {\
if ( (right_source_sdll).ptr_to_rightmost == NULL ) {\
(target_sdll).ptr_to_leftmost = (left_source_sdll).ptr_to_leftmost;\
(target_sdll).ptr_to_rightmost = (left_source_sdll).ptr_to_rightmost;\
} else \
if ( (left_source_sdll).ptr_to_leftmost == NULL ) {\
(target_sdll).ptr_to_leftmost = (right_source_sdll).ptr_to_leftmost;\
(target_sdll).ptr_to_rightmost = (right_source_sdll).ptr_to_rightmost;\
} else {\
(target_sdll).ptr_to_rightmost = (right_source_sdll).ptr_to_rightmost;\
(right_source_sdll).ptr_to_leftmost->e.ptr_to_left =\
(left_source_sdll).ptr_to_rightmost;\
(left_source_sdll).ptr_to_rightmost->e.ptr_to_right =\
(right_source_sdll).ptr_to_leftmost;\
(target_sdll).ptr_to_leftmost = (left_source_sdll).ptr_to_leftmost;}}

#define \
is_nonempty_sdll_TOKEN(\
sdll) (\
(sdll).ptr_to_leftmost != NULL)



void
h_TOKEN_alloc_strcpy_hs(
str_on_heap_TOKEN * target, char const * source);

void
h_TOKEN_alloc_strcpy_hh(
str_on_heap_TOKEN * target, str_on_heap_TOKEN source);

void
h_TOKEN_strcpy_to_prealloc_sh(
char * target, str_on_heap_TOKEN source);

void
h_TOKEN_str_delete(
str_on_heap_TOKEN arg);

int64_t
h_TOKEN_strlen(
str_on_heap_TOKEN arg);

void
h_TOKEN_strcat(
str_on_heap_TOKEN arg1, str_on_heap_TOKEN arg2);

int
h_TOKEN_strcmp_hs(
str_on_heap_TOKEN arg1, char const * arg2);

int
h_TOKEN_strcmp_hh(
str_on_heap_TOKEN arg1, str_on_heap_TOKEN arg2);

int
h_TOKEN_alloc_fscan_str(
FILE * stream, str_on_heap_TOKEN * target);

int
h_TOKEN_alloc_scan_str(
str_on_heap_TOKEN * target);

int64_t
h_TOKEN_fprintf_str(
FILE * stream, str_on_heap_TOKEN s, int64_t ml, char pad, char just);

int64_t
h_TOKEN_printf_str(
str_on_heap_TOKEN s, int64_t ml, char pad, char just);

int /* Ret. 0 != 0 if a symbol other than `0' or `1' enc. Otherw. ret 1 != 0. */
h_TOKEN_count_zeros_and_ones_in_str(
str_on_heap_TOKEN s, uint64_t * n_zeros, uint64_t * n_ones);



void
sort_smaller_to_the_left_sdll_TOKEN(
sdll_TOKEN * sdll);

void
sort_smaller_to_the_right_sdll_TOKEN(
sdll_TOKEN * sdll);

void
move_all_elements_to_empty_sgt_TOKEN(
sgt_TOKEN * sgt, sdll_TOKEN * sdll);

void
insert__if_conflicts_treat_as_eps_smallest__sgt_TOKEN(
ptr_to_entry_of_sgt_TOKEN ptr_e, sgt_TOKEN * sgt);

void
insert__if_conflicts_treat_as_eps_greatest__sgt_TOKEN(
ptr_to_entry_of_sgt_TOKEN ptr_e, sgt_TOKEN * sgt);

int
insert__if_conflicts_bounce__sgt_TOKEN(
ptr_to_entry_of_sgt_TOKEN ptr_e, sgt_TOKEN * sgt);

TOKEN*
find_prep_rm__if_many_give_eps_smallest__sgt_TOKEN(
TOKEN * key, sgt_TOKEN * sgt);

TOKEN*
find_prep_rm__if_many_give_eps_greatest__sgt_TOKEN(
TOKEN * key, sgt_TOKEN * sgt);

TOKEN*
find_prep_rm__smallest__sgt_TOKEN(
sgt_TOKEN * sgt);

TOKEN*
find_prep_rm__greatest__sgt_TOKEN(
sgt_TOKEN * sgt);

ptr_to_entry_of_sgt_TOKEN
remove_entry_from_sgt_TOKEN(
void);



#define \
new_entry_for_sgt_TOKEN(\
target) {\
if (sll_of_unused_el_of_sdll_TOKEN == NULL)\
new_block_of_99_elem_of_sdll_TOKEN();\
(target) = sll_of_unused_el_of_sdll_TOKEN;\
sll_of_unused_el_of_sdll_TOKEN =\
sll_of_unused_el_of_sdll_TOKEN->u.ptr_to_next_el_of_sll;}

#define \
delete_unused_entry_for_sgt_TOKEN(\
p) {\
(p)->u.ptr_to_next_el_of_sll = sll_of_unused_el_of_sdll_TOKEN;\
sll_of_unused_el_of_sdll_TOKEN = (p);}

#define \
find__if_many_give_eps_smallest__sgt_TOKEN(\
target, key, sgt) {\
(target) = NULL;\
temp_for_internal_use_TOKEN = (sgt).ptr_to_root;\
while (temp_for_internal_use_TOKEN != NULL)\
if ( TOKEN_arg1_GREATER_THAN_arg2(\
(key), (temp_for_internal_use_TOKEN->e.body)) ) {\
temp_for_internal_use_TOKEN =\
temp_for_internal_use_TOKEN->e.ptr_to_right;\
} else \
if ( TOKEN_arg1_GREATER_THAN_arg2(\
(temp_for_internal_use_TOKEN->e.body), (key)) ) {\
temp_for_internal_use_TOKEN =\
temp_for_internal_use_TOKEN->e.ptr_to_left;\
} else {(target) = &(temp_for_internal_use_TOKEN->e.body);\
temp_for_internal_use_TOKEN =\
temp_for_internal_use_TOKEN->e.ptr_to_left;}}

#define \
find__if_many_give_eps_greatest__sgt_TOKEN(\
target, key, sgt) {\
(target) = NULL;\
temp_for_internal_use_TOKEN = (sgt).ptr_to_root;\
while (temp_for_internal_use_TOKEN != NULL)\
if ( TOKEN_arg1_GREATER_THAN_arg2(\
(key), (temp_for_internal_use_TOKEN->e.body)) ) {\
temp_for_internal_use_TOKEN =\
temp_for_internal_use_TOKEN->e.ptr_to_right;\
} else \
if ( TOKEN_arg1_GREATER_THAN_arg2(\
(temp_for_internal_use_TOKEN->e.body), (key)) ) {\
temp_for_internal_use_TOKEN =\
temp_for_internal_use_TOKEN->e.ptr_to_left;\
} else {(target) = &(temp_for_internal_use_TOKEN->e.body);\
temp_for_internal_use_TOKEN =\
temp_for_internal_use_TOKEN->e.ptr_to_right;}}

#define \
find__smallest__sgt_TOKEN(\
target, sgt) {\
if ( (temp_for_internal_use_TOKEN = (sgt).ptr_to_root) != NULL) {\
while( temp_for_internal_use_TOKEN->e.ptr_to_left != NULL )\
temp_for_internal_use_TOKEN = temp_for_internal_use_TOKEN->e.ptr_to_left;\
(target) = &(temp_for_internal_use_TOKEN->e.body);}\
else (target) = NULL;}

#define \
find__greatest__sgt_TOKEN(\
target, sgt) {\
if ( (temp_for_internal_use_TOKEN = (sgt).ptr_to_root) != NULL) {\
while( temp_for_internal_use_TOKEN->e.ptr_to_right != NULL )\
temp_for_internal_use_TOKEN = temp_for_internal_use_TOKEN->e.ptr_to_right;\
(target) = &(temp_for_internal_use_TOKEN->e.body);}\
else (target) = NULL;}

#define \
is_nonempty_sgt_TOKEN( \
sgt) (\
(sgt).num_entries != 0)



#endif
