#ifndef POETRY_H
#define POETRY_H

#include <stdbool.h>

#define POEM_WORD_LENGTH 15
#define POEM_CLAUSE_LENGTH 50
#define POEM_NUM_FLOWER 8
#define POEM_NUM_COLOUR 9
#define POEM_NUM_STATEMENT 6
#define POEM_NUM_CADENCE 7
#define POEM_NUM_RAND 18

struct poem_t {
	char flower_a[15];
	char colour_a[15];
	char flower_b[15];
	char colour_b[POEM_WORD_LENGTH];
	char statement[POEM_CLAUSE_LENGTH];
	char cadence[POEM_CLAUSE_LENGTH];
};

void print_poem(void);
void write_poem(void);
void wait_to_write_poem(void);
void plan_poem(void);
void savour_poem(void);

#endif