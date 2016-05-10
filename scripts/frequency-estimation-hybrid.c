// Note: This isn't really memoization. Just a hash table is implemented to keep track 
// of invocation frequencies of each module in the program

#include <stdlib.h>    /* malloc    */
#include <stdio.h>     /* printf    */
#include <stddef.h>    /* offsetof  */
#include <string.h>    /* strcpy    */
#include <stdbool.h>   /* bool      */
#include <stdint.h>    /* int64_t   */
#include "uthash.h"    /* HASH_ADD  */
#include <math.h>      /* floorf    */

#define _MAX_FUNCTION_NAME 32
#define _MAX_INT_PARAMS 4
#define _MAX_DOUBLE_PARAMS 4
#define _MAX_CALL_DEPTH 16

// DEBUG switch
bool debugFreqEstimationHybrid = false;

/*****************
* Stack Definition  
******************/
// The stack is in fact a call stack, but keeps frequencies of all previous parents
// so that a childs frequency would be multiplied by that of all before it

// elements on the stack are of type:
typedef unsigned stackElement_t;

// defining a structure to act as stack for pointer values to resources that must be updated                    
typedef struct {
  stackElement_t *contents;
  int top;
  int maxSize;
} resourcesStack_t;

// declare global "resources" array address stack
resourcesStack_t *resourcesStack = NULL;

void stackInit (int maxSize) {
  resourcesStack = (resourcesStack_t*)malloc(sizeof(resourcesStack_t));
  if (resourcesStack == NULL) {
    fprintf(stderr, "Insufficient memory to initialize stack.\n");
    exit(1);
  }
  stackElement_t *newContents;
  newContents = (stackElement_t*)malloc( sizeof(stackElement_t)*maxSize );
  if (newContents == NULL) {
    fprintf(stderr, "Insufficient memory to initialize stack.\n");
    exit(1);
  }
  resourcesStack->contents = newContents;
  resourcesStack->maxSize = maxSize;
  resourcesStack->top = -1; /* i.e. empty */ 
}

void stackDestroy() {
  free(resourcesStack->contents);
  resourcesStack->contents = NULL;
  resourcesStack->maxSize = 0;
  resourcesStack->top = -1;
}

void stackPush (stackElement_t stackElement) {
  if (resourcesStack->top >= resourcesStack->maxSize - 1) {
    fprintf (stderr, "Can't push element on stack: Stack is full.\n");
    exit(1);
  }
  // insert element and update "top"
  resourcesStack->contents[++resourcesStack->top] = stackElement;
}

void stackPop () {
  if (debugFreqEstimationHybrid)
    printf("Popping from stack\n");  
  if (resourcesStack->top < 0) {
    fprintf (stderr, "Can't pop element from stack: Stack is empty.\n");
    exit(1);    
  }

  //update "top"
  resourcesStack->top--;

}

/**********************
* Hash Table Definition
***********************/

// defining a structure that can be hashed using "uthash.h"
typedef struct {
  
  char function_name[_MAX_FUNCTION_NAME];             /* these three fields */
  int int_params[_MAX_INT_PARAMS];                    /* comprise */
  double double_params[_MAX_DOUBLE_PARAMS];           /* the key */

  // resources[0] ---> Invocation count (frequency) of module 
  // resources[1] ---> Number of integer arguments
  // resources[2] ---> Number of double arguments
  unsigned long long resources[3];                    /* hash table value field */

  UT_hash_handle hh;                                  /* make this structure hashable  */

} hash_entry_t;

// defining multi-field key for hash table
typedef struct {
  char function_name[_MAX_FUNCTION_NAME];      
  int int_params[_MAX_INT_PARAMS];             
  double double_params[_MAX_DOUBLE_PARAMS];    
} lookup_key_t;

// how many bytes long is the key?
const size_t keylen = _MAX_FUNCTION_NAME * sizeof(char)
                    + _MAX_INT_PARAMS * sizeof(int)
                    + _MAX_DOUBLE_PARAMS * sizeof(double);

// declare global memoization hash table
hash_entry_t *memos = NULL;

void print_hash_table() {
  printf("<<<---------------------------------------\n");  
  printf("current hash table:\n");
  int i;
  hash_entry_t *memo;
  for (memo=memos; memo != NULL; memo=memo->hh.next) {
    printf("%s -- ", memo->function_name);
    for (i=0; i<_MAX_INT_PARAMS; i++)
     printf("%d ", memo->int_params[i]); 
    printf("-- ");
    for (i=0; i<_MAX_DOUBLE_PARAMS; i++)
     printf("%f ", memo->double_params[i]);   
    printf("-- ");       
    for (i=0; i<3; i++)
     printf("%llu ", memo->resources[i]);   
    printf("\n"); 
  }
  printf("--------------------------------------->>>\n");  
}

/* add_memo: add entry in hash table */
void add_memo ( char *function_name,                          /* function name string */
                int *int_params, unsigned num_ints,           /* integer parameters */
                double *double_params, unsigned num_doubles,  /* double parameters */
                unsigned long long *resources                 /* resources for that function version */
                ) {             

  // allocate space for "memo" entry and
  // set "memo" space to zero to avoid random byte inconsistency in lookup 
  hash_entry_t *memo = calloc( 1, sizeof(hash_entry_t) );
  if (memo == NULL) {
    fprintf(stderr, "Insufficient memory to add memo.\n");
    exit(1);
  }  

  // copy into field of "memo"
  strcpy (memo->function_name, function_name);
  memcpy (memo->int_params, int_params, num_ints * sizeof(int));
  memcpy (memo->double_params, double_params, num_doubles * sizeof(double));
  memcpy (memo->resources, resources, 3 * sizeof(unsigned long long));
 
  // HASH_ADD (hh_name, head, keyfield_name, key_len, item_ptr)
  HASH_ADD(hh, memos, function_name, keylen, memo);
}

/* find_memo: find an entry in hash table */
hash_entry_t *find_memo ( char *function_name, 
                          int *int_params, unsigned num_ints,
                          double *double_params, unsigned num_doubles
                              ) {

  // returned entry -- NULL if not found
  hash_entry_t *memo = NULL;

  // build appropriate lookup_key object
  lookup_key_t *lookup_key = calloc(1, sizeof(lookup_key_t) );
  if (lookup_key == NULL) {
    fprintf(stderr, "Insufficient memory to create lookup key.\n");
    exit(1);
  }  
  memset (lookup_key, 0, sizeof(lookup_key_t));
  strcpy (lookup_key->function_name, function_name);
  memcpy (lookup_key->int_params, int_params, num_ints * sizeof(int));
  memcpy (lookup_key->double_params, double_params, num_doubles * sizeof(double));

  HASH_FIND (hh, memos, lookup_key, keylen, memo);
    

  //free(lookup_key->double_params);  
  //free(lookup_key->int_params);
  //free(lookup_key->function_name);
  free(lookup_key);
  lookup_key = NULL;
  return memo;
}

/* delete_memo: delete a hash table entry */
void delete_memo (hash_entry_t *memos, hash_entry_t *memo) {
  HASH_DEL (memos, memo);
  free(memo);
}

void delete_all_memos() {
  hash_entry_t *memo, *tmp;

  // deletion-safe iteration
  HASH_ITER(hh, memos, memo, tmp) {
    HASH_DEL(memos, memo);  
    free(memo);            
  }
}

/*****************************
* Functions to be instrumented
******************************/

/* memoize: memoization function */
/* A call to this function ensures that the relevant entry */
/* in the hash table has been created */
int memoize ( char *function_name, 
               int *int_params, unsigned num_ints,
               double *double_params, unsigned num_doubles,
               unsigned repeat
                              ) {

  static unsigned long long total_call_count = 0;
  total_call_count++;
  //printf("Total Call Count = %d", total_call_count);

  if (debugFreqEstimationHybrid) {
    printf("memoize called on %s !\n", function_name);
    printf("repeat value = %d !\n", repeat);
  }

  hash_entry_t *memo;
  memo = find_memo(function_name, int_params, num_ints, double_params, num_doubles);

  if (memo == NULL) {
    if (debugFreqEstimationHybrid)
      printf("NOT memoized before :(\n");
    // create entry with zero resources
    // the function call in LLVM IR will be called and will populate
    unsigned long long *resources = calloc (3, sizeof(unsigned long long));
    resources[0] = repeat;  // invocation count
    resources[1] = num_ints; // number of int args
    resources[2] = num_doubles; // number of double args

    // multiply by the frequency of all parents before this module, then add to memos table
    int it;
    unsigned long long accumulative_mult = 1;
    for (it = resourcesStack->top; it > 0; it--)
      accumulative_mult *= resourcesStack->contents[it];

    resources[0] *= accumulative_mult;

    add_memo(function_name, int_params, num_ints, double_params, num_doubles, resources);

    // push this function versions resources to the top of stack
    // will use the frequency of it to multiply all children frequencies hereafter by that frequency
    if (debugFreqEstimationHybrid)
      printf("pushing to stack: %s\n", function_name);
  }

  else {
    if (debugFreqEstimationHybrid)
      printf("Memoized already! :)\n");
    // add to the frequency of execution (repeat times)
    int it;
    unsigned long long accumulative_mult = 1;
    for (it = resourcesStack->top; it > 0; it--)
      accumulative_mult *= resourcesStack->contents[it];

    memo->resources[0] += repeat*accumulative_mult;
  }

  // put on the stack the individual frequency of this module
  stackPush(repeat); 
  
  // print updated hash table
  if (debugFreqEstimationHybrid)
    print_hash_table();

  return 0;
}

/* exit_scope: resource transfer function*/
/* A call to this function occurs after the module has been entered into the hash table*/
/* It will pop the last frequency off of the stack so it won't multiply anymore */
void exit_scope () 
{
  if (debugFreqEstimationHybrid)
    printf("exiting scope...\n");
  
  stackPop();
}

//void qasm_gate () {
//}

void qasm_initialize ()
{
  if (debugFreqEstimationHybrid)
    printf("initializing stack....\n");

  // initialize with maximum possible levels of calling depth
  stackInit(_MAX_CALL_DEPTH);
  
  // put "main" in the first row of both the hash table and the stack
    //int main_int_params[_MAX_INT_PARAMS] = {0};
    int *main_int_params = calloc (_MAX_INT_PARAMS, sizeof(int));
    //double main_double_params[_MAX_DOUBLE_PARAMS] = {0.0};
    double *main_double_params = calloc (_MAX_DOUBLE_PARAMS, sizeof(double));
    //unsigned long long main_resources[3] = {0};
    unsigned long long *main_resources = calloc (3, sizeof(unsigned long long));
    // main is executed once
    main_resources[0] = 1;

  add_memo("main                           ", main_int_params, 0, main_double_params, 0, main_resources);

  stackPush(1); 
}

void qasm_resource_summary ()
{
  // Profiling info: Total Gates, Execution Frequency, Number of Int Params, Number of Double Params
 
  int i;
  hash_entry_t *memo;
  for (memo=memos; memo != NULL; memo=memo->hh.next) {
    printf("%s ", memo->function_name);
    for (i=0; i<_MAX_INT_PARAMS; i++) {
      printf("%2d ", memo->int_params[i]); 
    }
    for (i=0; i<_MAX_DOUBLE_PARAMS; i++) {
      printf("%12f ", floorf(memo->double_params[i] * 10000 + 0.5) / 10000);   
    }
    printf("%8llu %8llu %8llu \n", memo->resources[0], memo->resources[1], memo->resources[2]);   
     
  }

  // free allocated memory for the "stack"
  stackDestroy();

  // free allocated memory for the "memos" table
  delete_all_memos();
}

/*
int main () {
  
  qasm_initialize();


  // create an entry
  char *function_name = "random_func                    ";
  int int_params [3] = {1, 6, -2};
  double double_params [1] = {-3.14};
  unsigned long long resources [3] = {1,3,1};


  printf("*** main *** : memos->function_name = %s \n", memos->function_name);
  
  // insert entry
  memoize (function_name, int_params, 3, double_params, 1, 1);

  printf("*** main *** : memos->function_name = %s \n", memos->function_name);


  // find entry
  hash_entry_t *memo;
  memo = find_memo (function_name, int_params, 3, double_params, 1);
  
  if (memo == NULL)
  { printf("*** Entry not found *** \n"); return -1; }

  printf("Resources for this function: \n");
  int i;
  for (i=0; i<3; i++)
    printf("%llu, ", memo->resources[i]);
  printf("\n");

  memoize(function_name, int_params, 3, double_params, 1, 4);
  qasm_resource_summary();

  return 0;
}
*/
