#include <stdio.h>

#include "utils.h"

void dump_bytes(void* v, int size, int as_chars){
	int i = 0;
	unsigned char *array = (unsigned char*) v;

	for (i = 0; i<size; ++i){
		printf(as_chars?"%c":"%02x ", array[i]);
		if( i%16 == 15 && !as_chars) {
			printf("\n");
		}
	}
	printf("\n");
}
