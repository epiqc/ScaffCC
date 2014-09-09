#include <ctqg__________memory_manager.h>

#include <ctqg________________sdll_sgt.h>
/* #include more "sdll_sgt_ANOTHER_TYPE.h" */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



int memory_manager_message_level = -1;

uint64_t heap_size_TOKEN = 0;
/* uint64_t heap_size_ANOTHER_TYPE = 0; */

uint64_t total_size_of_all_heaps = 0;



void
update_memory_usage_info(
uint64_t * ptr_to_counter, uint64_t new_block_size) {
  static uint64_t next_threshold = (uint64_t) 1000;
  static int      first_digit_of_next_threshold = 1;
  uint64_t temp;

  *ptr_to_counter         += new_block_size;
  total_size_of_all_heaps += new_block_size;
  while (total_size_of_all_heaps >= next_threshold) {
    if (memory_manager_message_level < 0) {
      printf("Error.\n");
      printf("Memory manager message level is negative.\n");
      getchar();
      exit(-1);
    }

    temp = next_threshold / ((uint64_t) 1000);

    if (memory_manager_message_level >= 1) {
      printf("          "
             "          "
             "          "
             "          "
             "          "
             "          "
             "          "
             "         \r");
      if (temp < 1000) {
        printf("Memory usage has reached %u thousand bytes",
                (unsigned int) temp);
      } else {
        printf("Memory usage has reached %u million bytes",
               (unsigned int) (temp / ((uint64_t) 1000)));
      }
    }
    switch (first_digit_of_next_threshold) {
      case 1:
        first_digit_of_next_threshold = 2;
        next_threshold *= 2;
      break;

      case 2:
        first_digit_of_next_threshold = 5;
        next_threshold *= 2;
        next_threshold += next_threshold >> 2;
      break;

      case 5:
        first_digit_of_next_threshold = 1;
        next_threshold *= 2;
      break;

      default:
        printf("Error.\n");
        printf("Memory manager internal error.\n");
        getchar();
        exit(-1);
    }

    if (memory_manager_message_level >= 2) {
      printf(" (");

      /* Repeat this block with different names if needed. */
      printf("TOKEN:%u%%, ",
        (unsigned int) (
          (heap_size_TOKEN * ((uint64_t) 100) + ((uint64_t) 50))
          /
          total_size_of_all_heaps
        ));
    }
    if (memory_manager_message_level >= 1) printf(".\n");
  }
}



void
release_memory(
void) {
  int          memory_leak_occured = (0 != 0);
  unsigned int rm_in_hundreds_thousand =
    (unsigned int) (
      (total_size_of_all_heaps + ((uint64_t) 50000)) / ((uint64_t) 100000)
    );



  /* Repeat this block with different names if needed. */
  if (!is_heap_TOKEN_empty()) {
    printf("Error: heap_TOKEN "
           "is not empty at the time of memory release.\n");
    memory_leak_occured = (1 != 0);
  }
  reset_heap_TOKEN();
  heap_size_TOKEN = 0;



  if (memory_leak_occured) {
    getchar();
    exit(-1);
  }

  if (memory_manager_message_level < 0) {
    printf("Error.\n");
    printf("Memory manager internal error.\n");
    getchar();
    exit(-1);
  }

  if (memory_manager_message_level >= 1) {
    printf("%u.%u million bytes of memory have been released.\n",
           rm_in_hundreds_thousand / 10, rm_in_hundreds_thousand % 10);
  }
  if (memory_manager_message_level >= 2) {
    printf("There are no memory leaks.\n");
  }
  total_size_of_all_heaps = 0;
}
