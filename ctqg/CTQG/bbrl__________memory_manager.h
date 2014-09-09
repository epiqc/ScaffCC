#ifndef BBRL__________MEMORY_MANAGER_H
#define BBRL__________MEMORY_MANAGER_H



#include <stdint.h>



extern int memory_manager_message_level;

extern uint64_t heap_size_ATOM;
/* Add more lines for more types. */

extern uint64_t total_size_of_all_heaps;



void
update_memory_usage_info(
uint64_t * ptr_to_counter, uint64_t new_block_size);

void
release_memory(
void);



#endif
