// STORAGE defines the integer type (in KB) to be used througout
#ifndef _STORAGE_H_
#define _STORAGE_H_

typedef struct sto {
	char *unit; // can be 'kb', 'mb', 'gb'.
	int size;
} *STORAGE;

#endif // _STORAGE_H_
