#ifndef BBRL________________SDLL_SGT_H
#define BBRL________________SDLL_SGT_H



#include <stdio.h>  /* `NULL' pointer */
#include <stdint.h> /* int64_t type   */



union abstract_el_of_heap_ATOM;

typedef union
abstract_el_of_heap_ATOM * ptr_to_node_of_sdll_sgt_ATOM;
typedef union
abstract_el_of_heap_ATOM * str_on_heap_ATOM;
typedef union
abstract_el_of_heap_ATOM * ptr_to_el_of_sll_of_unused_el_of_heap_ATOM;

typedef union abstract_el_of_heap_ATOM * iter_sdll_ATOM;
typedef union abstract_el_of_heap_ATOM * ptr_to_entry_of_sgt_ATOM;

typedef struct {
  str_on_heap_ATOM key_s;
  uint64_t         key_i;
  str_on_heap_ATOM value_s;
  uint64_t         value_i;
} ATOM;

void
print_to_file_ATOM(
FILE * f, ATOM * arg);

#define \
ATOM_arg1_GREATER_THAN_arg2(\
arg1, arg2) \
(\
((arg1.key_i > arg2.key_i) && (h_ATOM_strcmp_hh(arg1.key_s, arg2.key_s) >= 0))\
||\
((arg1.key_i <= arg2.key_i) && (h_ATOM_strcmp_hh(arg1.key_s, arg2.key_s) > 0)))



struct node_of_sdll_sgt_ATOM {
  ATOM body;
  ptr_to_node_of_sdll_sgt_ATOM ptr_to_left;
  ptr_to_node_of_sdll_sgt_ATOM ptr_to_right;
};

struct s_block_ATOM {
  char body[sizeof(struct node_of_sdll_sgt_ATOM)
            -
            sizeof(str_on_heap_ATOM)];
  str_on_heap_ATOM ptr_to_next_s_block;
};

struct el_of_sll_of_unused_el_of_heap_ATOM {
  ptr_to_el_of_sll_of_unused_el_of_heap_ATOM ptr_to_next_el_of_sll;
};

union abstract_el_of_heap_ATOM {
  struct node_of_sdll_sgt_ATOM               e;
  struct s_block_ATOM                        s;
  struct el_of_sll_of_unused_el_of_heap_ATOM u;
};



typedef struct {
  ptr_to_node_of_sdll_sgt_ATOM ptr_to_leftmost;
  ptr_to_node_of_sdll_sgt_ATOM ptr_to_rightmost;
} sdll_ATOM;

typedef struct {
  ptr_to_node_of_sdll_sgt_ATOM ptr_to_root;
  int64_t                      num_entries;
  int64_t                      max_size_since_full_rebuild;
} sgt_ATOM;

struct elem_of_stack_of_allocated_blocks_ATOM {
  union abstract_el_of_heap_ATOM                  body[99];
  struct elem_of_stack_of_allocated_blocks_ATOM * ptr_to_next_elem_of_stack;
};



extern ptr_to_el_of_sll_of_unused_el_of_heap_ATOM
  sll_of_unused_el_of_sdll_ATOM;
extern union abstract_el_of_heap_ATOM *
  temp_for_internal_use_ATOM;



void
print_ATOM(
ATOM * arg);

void
new_block_of_99_elem_of_sdll_ATOM(
void);

int
is_heap_ATOM_empty(
void);

void
reset_heap_ATOM(
void);

void
print_to_file_sdll_ATOM(
FILE * f, sdll_ATOM sdll);

void
print_sdll_ATOM(
sdll_ATOM sdll);

int64_t
count_elements_sdll_ATOM(
sdll_ATOM sdll);



#define \
set_iter_to_leftmost_el_sdll_ATOM(\
target_iter, sdll) {\
(target_iter) = (sdll).ptr_to_leftmost;}

#define \
set_iter_to_rightmost_el_sdll_ATOM(\
target_iter, sdll) {\
(target_iter) = (sdll).ptr_to_rightmost;}

#define \
move_iter_left_sdll_ATOM(\
iter) {\
(iter) = (iter)->e.ptr_to_left;}

#define \
move_iter_right_sdll_ATOM(\
iter) {\
(iter) = (iter)->e.ptr_to_right;}

#define \
new_at_the_left_end_sdll_ATOM(\
target, sdll) {\
if (sll_of_unused_el_of_sdll_ATOM == NULL)\
new_block_of_99_elem_of_sdll_ATOM();\
temp_for_internal_use_ATOM = sll_of_unused_el_of_sdll_ATOM;\
sll_of_unused_el_of_sdll_ATOM =\
sll_of_unused_el_of_sdll_ATOM->u.ptr_to_next_el_of_sll;\
temp_for_internal_use_ATOM->e.ptr_to_left = NULL;\
if ((temp_for_internal_use_ATOM->e.ptr_to_right =\
(sdll).ptr_to_leftmost) == NULL) {\
(sdll).ptr_to_rightmost = temp_for_internal_use_ATOM;\
} else {\
(sdll).ptr_to_leftmost->e.ptr_to_left =\
temp_for_internal_use_ATOM;}\
(sdll).ptr_to_leftmost =\
temp_for_internal_use_ATOM;\
(target) = &(temp_for_internal_use_ATOM->e.body);}

#define \
new_at_the_right_end_sdll_ATOM(\
target, sdll) {\
if (sll_of_unused_el_of_sdll_ATOM == NULL)\
new_block_of_99_elem_of_sdll_ATOM();\
temp_for_internal_use_ATOM = sll_of_unused_el_of_sdll_ATOM;\
sll_of_unused_el_of_sdll_ATOM =\
sll_of_unused_el_of_sdll_ATOM->u.ptr_to_next_el_of_sll;\
temp_for_internal_use_ATOM->e.ptr_to_right = NULL;\
if ((temp_for_internal_use_ATOM->e.ptr_to_left =\
(sdll).ptr_to_rightmost) == NULL) {\
(sdll).ptr_to_leftmost = temp_for_internal_use_ATOM;\
} else {\
(sdll).ptr_to_rightmost->e.ptr_to_right =\
temp_for_internal_use_ATOM;}\
(sdll).ptr_to_rightmost =\
temp_for_internal_use_ATOM;\
(target) = &(temp_for_internal_use_ATOM->e.body);}

#define \
new_to_the_left_of_nonNULL_iter_sdll_ATOM(\
target, iter, sdll) {\
if (sll_of_unused_el_of_sdll_ATOM == NULL)\
new_block_of_99_elem_of_sdll_ATOM();\
temp_for_internal_use_ATOM = sll_of_unused_el_of_sdll_ATOM;\
sll_of_unused_el_of_sdll_ATOM =\
sll_of_unused_el_of_sdll_ATOM->u.ptr_to_next_el_of_sll;\
temp_for_internal_use_ATOM->e.ptr_to_right = (iter);\
if ((temp_for_internal_use_ATOM->e.ptr_to_left =\
(iter)->e.ptr_to_left) == NULL) {\
(sdll).ptr_to_leftmost = temp_for_internal_use_ATOM;\
} else {\
(iter)->e.ptr_to_left->e.ptr_to_right =\
temp_for_internal_use_ATOM;}\
(iter)->e.ptr_to_left =\
temp_for_internal_use_ATOM;\
(target) = &(temp_for_internal_use_ATOM->e.body);}

#define \
new_to_the_right_of_nonNULL_iter_sdll_ATOM(\
target, iter, sdll) {\
if (sll_of_unused_el_of_sdll_ATOM == NULL)\
new_block_of_99_elem_of_sdll_ATOM();\
temp_for_internal_use_ATOM = sll_of_unused_el_of_sdll_ATOM;\
sll_of_unused_el_of_sdll_ATOM =\
sll_of_unused_el_of_sdll_ATOM->u.ptr_to_next_el_of_sll;\
temp_for_internal_use_ATOM->e.ptr_to_left = (iter);\
if ((temp_for_internal_use_ATOM->e.ptr_to_right =\
(iter)->e.ptr_to_right) == NULL) {\
(sdll).ptr_to_rightmost = temp_for_internal_use_ATOM;\
} else {\
(iter)->e.ptr_to_right->e.ptr_to_left =\
temp_for_internal_use_ATOM;}\
(iter)->e.ptr_to_right =\
temp_for_internal_use_ATOM;\
(target) = &(temp_for_internal_use_ATOM->e.body);}

#define \
delete_leftmost_sdll_ATOM(\
sdll) {\
temp_for_internal_use_ATOM = (sdll).ptr_to_leftmost;\
if (temp_for_internal_use_ATOM ==\
(sdll).ptr_to_rightmost) {\
(sdll).ptr_to_rightmost = NULL;\
} else {\
temp_for_internal_use_ATOM->e.ptr_to_right->e.ptr_to_left\
= NULL;} (sdll).ptr_to_leftmost =\
temp_for_internal_use_ATOM->e.ptr_to_right;\
temp_for_internal_use_ATOM->u.ptr_to_next_el_of_sll =\
sll_of_unused_el_of_sdll_ATOM;\
sll_of_unused_el_of_sdll_ATOM =\
temp_for_internal_use_ATOM;}

#define \
delete_rightmost_sdll_ATOM(\
sdll) {\
temp_for_internal_use_ATOM = (sdll).ptr_to_rightmost;\
if (temp_for_internal_use_ATOM ==\
(sdll).ptr_to_leftmost) {\
(sdll).ptr_to_leftmost = NULL;\
} else {\
temp_for_internal_use_ATOM->e.ptr_to_left->e.ptr_to_right\
= NULL;} (sdll).ptr_to_rightmost =\
temp_for_internal_use_ATOM->e.ptr_to_left;\
temp_for_internal_use_ATOM->u.ptr_to_next_el_of_sll =\
sll_of_unused_el_of_sdll_ATOM;\
sll_of_unused_el_of_sdll_ATOM =\
temp_for_internal_use_ATOM;}

#define \
delete_and_move_iter_to_left_sdll_ATOM(\
iter, sdll) {\
temp_for_internal_use_ATOM = (iter);\
if ((iter)->e.ptr_to_left == NULL) {\
(sdll).ptr_to_leftmost = (iter)->e.ptr_to_right;\
} else {\
(iter)->e.ptr_to_left->e.ptr_to_right = (iter)->e.ptr_to_right;}\
if ((iter)->e.ptr_to_right == NULL) {\
(sdll).ptr_to_rightmost = (iter)->e.ptr_to_left;\
} else {\
(iter)->e.ptr_to_right->e.ptr_to_left = (iter)->e.ptr_to_left;}\
(iter) = (iter)->e.ptr_to_left;\
temp_for_internal_use_ATOM->u.ptr_to_next_el_of_sll =\
sll_of_unused_el_of_sdll_ATOM;\
sll_of_unused_el_of_sdll_ATOM =\
temp_for_internal_use_ATOM;}

#define \
delete_and_move_iter_to_right_sdll_ATOM(\
iter, sdll) {\
temp_for_internal_use_ATOM = (iter);\
if ((iter)->e.ptr_to_left == NULL) {\
(sdll).ptr_to_leftmost = (iter)->e.ptr_to_right;\
} else {\
(iter)->e.ptr_to_left->e.ptr_to_right = (iter)->e.ptr_to_right;}\
if ((iter)->e.ptr_to_right == NULL) {\
(sdll).ptr_to_rightmost = (iter)->e.ptr_to_left;\
} else {\
(iter)->e.ptr_to_right->e.ptr_to_left = (iter)->e.ptr_to_left;}\
(iter) = (iter)->e.ptr_to_right;\
temp_for_internal_use_ATOM->u.ptr_to_next_el_of_sll =\
sll_of_unused_el_of_sdll_ATOM;\
sll_of_unused_el_of_sdll_ATOM =\
temp_for_internal_use_ATOM;}

#define \
delete_and_iter_becomes_invalid_sdll_ATOM(\
iter, sdll) {\
if ((iter)->e.ptr_to_left == NULL) {\
(sdll).ptr_to_leftmost = (iter)->e.ptr_to_right;\
} else {\
(iter)->e.ptr_to_left->e.ptr_to_right = (iter)->e.ptr_to_right;}\
if ((iter)->e.ptr_to_right == NULL) {\
(sdll).ptr_to_rightmost = (iter)->e.ptr_to_left;\
} else {\
(iter)->e.ptr_to_right->e.ptr_to_left = (iter)->e.ptr_to_left;}\
(iter)->u.ptr_to_next_el_of_sll = sll_of_unused_el_of_sdll_ATOM;\
sll_of_unused_el_of_sdll_ATOM = (iter);}

#define \
move_elem_to_the_left_end_sdll_ATOM(\
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
move_elem_to_the_right_end_sdll_ATOM(\
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
merge_sdll_ATOM(\
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
is_nonempty_sdll_ATOM(\
sdll) (\
(sdll).ptr_to_leftmost != NULL)



void
h_ATOM_alloc_strcpy_hs(
str_on_heap_ATOM * target, char const * source);

void
h_ATOM_alloc_strcpy_hh(
str_on_heap_ATOM * target, str_on_heap_ATOM source);

void
h_ATOM_strcpy_to_prealloc_sh(
char * target, str_on_heap_ATOM source);

void
h_ATOM_str_delete(
str_on_heap_ATOM arg);

int64_t
h_ATOM_strlen(
str_on_heap_ATOM arg);

void
h_ATOM_strcat(
str_on_heap_ATOM arg1, str_on_heap_ATOM arg2);

int
h_ATOM_strcmp_hs(
str_on_heap_ATOM arg1, char const * arg2);

int
h_ATOM_strcmp_hh(
str_on_heap_ATOM arg1, str_on_heap_ATOM arg2);

int
h_ATOM_alloc_fscan_str(
FILE * stream, str_on_heap_ATOM * target);

int
h_ATOM_alloc_scan_str(
str_on_heap_ATOM * target);

int64_t
h_ATOM_fprintf_str(
FILE * stream, str_on_heap_ATOM s, int64_t ml, char pad, char just);

int64_t
h_ATOM_printf_str(
str_on_heap_ATOM s, int64_t ml, char pad, char just);



void
sort_smaller_to_the_left_sdll_ATOM(
sdll_ATOM * sdll);

void
sort_smaller_to_the_right_sdll_ATOM(
sdll_ATOM * sdll);

void
move_all_elements_to_empty_sgt_ATOM(
sgt_ATOM * sgt, sdll_ATOM * sdll);

void
insert__if_conflicts_treat_as_eps_smallest__sgt_ATOM(
ptr_to_entry_of_sgt_ATOM ptr_e, sgt_ATOM * sgt);

void
insert__if_conflicts_treat_as_eps_greatest__sgt_ATOM(
ptr_to_entry_of_sgt_ATOM ptr_e, sgt_ATOM * sgt);

int
insert__if_conflicts_bounce__sgt_ATOM(
ptr_to_entry_of_sgt_ATOM ptr_e, sgt_ATOM * sgt);

ATOM*
find_prep_rm__if_many_give_eps_smallest__sgt_ATOM(
ATOM * key, sgt_ATOM * sgt);

ATOM*
find_prep_rm__if_many_give_eps_greatest__sgt_ATOM(
ATOM * key, sgt_ATOM * sgt);

ATOM*
find_prep_rm__smallest__sgt_ATOM(
sgt_ATOM * sgt);

ATOM*
find_prep_rm__greatest__sgt_ATOM(
sgt_ATOM * sgt);

ptr_to_entry_of_sgt_ATOM
remove_entry_from_sgt_ATOM(
void);



#define \
new_entry_for_sgt_ATOM(\
target) {\
if (sll_of_unused_el_of_sdll_ATOM == NULL)\
new_block_of_99_elem_of_sdll_ATOM();\
(target) = sll_of_unused_el_of_sdll_ATOM;\
sll_of_unused_el_of_sdll_ATOM =\
sll_of_unused_el_of_sdll_ATOM->u.ptr_to_next_el_of_sll;}

#define \
delete_unused_entry_for_sgt_ATOM(\
p) {\
(p)->u.ptr_to_next_el_of_sll = sll_of_unused_el_of_sdll_ATOM;\
sll_of_unused_el_of_sdll_ATOM = (p);}

#define \
find__if_many_give_eps_smallest__sgt_ATOM(\
target, key, sgt) {\
(target) = NULL;\
temp_for_internal_use_ATOM = (sgt).ptr_to_root;\
while (temp_for_internal_use_ATOM != NULL)\
if ( ATOM_arg1_GREATER_THAN_arg2(\
(key), (temp_for_internal_use_ATOM->e.body)) ) {\
temp_for_internal_use_ATOM =\
temp_for_internal_use_ATOM->e.ptr_to_right;\
} else \
if ( ATOM_arg1_GREATER_THAN_arg2(\
(temp_for_internal_use_ATOM->e.body), (key)) ) {\
temp_for_internal_use_ATOM =\
temp_for_internal_use_ATOM->e.ptr_to_left;\
} else {(target) = &(temp_for_internal_use_ATOM->e.body);\
temp_for_internal_use_ATOM =\
temp_for_internal_use_ATOM->e.ptr_to_left;}}

#define \
find__if_many_give_eps_greatest__sgt_ATOM(\
target, key, sgt) {\
(target) = NULL;\
temp_for_internal_use_ATOM = (sgt).ptr_to_root;\
while (temp_for_internal_use_ATOM != NULL)\
if ( ATOM_arg1_GREATER_THAN_arg2(\
(key), (temp_for_internal_use_ATOM->e.body)) ) {\
temp_for_internal_use_ATOM =\
temp_for_internal_use_ATOM->e.ptr_to_right;\
} else \
if ( ATOM_arg1_GREATER_THAN_arg2(\
(temp_for_internal_use_ATOM->e.body), (key)) ) {\
temp_for_internal_use_ATOM =\
temp_for_internal_use_ATOM->e.ptr_to_left;\
} else {(target) = &(temp_for_internal_use_ATOM->e.body);\
temp_for_internal_use_ATOM =\
temp_for_internal_use_ATOM->e.ptr_to_right;}}

#define \
find__smallest__sgt_ATOM(\
target, sgt) {\
if ( (temp_for_internal_use_ATOM = (sgt).ptr_to_root) != NULL) {\
while( temp_for_internal_use_ATOM->e.ptr_to_left != NULL )\
temp_for_internal_use_ATOM = temp_for_internal_use_ATOM->e.ptr_to_left;\
(target) = &(temp_for_internal_use_ATOM->e.body);}\
else (target) = NULL;}

#define \
find__greatest__sgt_ATOM(\
target, sgt) {\
if ( (temp_for_internal_use_ATOM = (sgt).ptr_to_root) != NULL) {\
while( temp_for_internal_use_ATOM->e.ptr_to_right != NULL )\
temp_for_internal_use_ATOM = temp_for_internal_use_ATOM->e.ptr_to_right;\
(target) = &(temp_for_internal_use_ATOM->e.body);}\
else (target) = NULL;}

#define \
is_nonempty_sgt_ATOM( \
sgt) (\
(sgt).num_entries != 0)



#endif
