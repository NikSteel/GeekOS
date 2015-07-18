#ifndef PETERSON_H
#define PETERSON_H

#include <stdbool.h>

struct peterson_t {
	bool flag[2];
	int turn;
};

typedef enum { P_I = 0, P_J = 1 } peterson_index_t;

void peterson(void (*)(void), void (*)(void), struct peterson_t*, peterson_index_t);

#endif