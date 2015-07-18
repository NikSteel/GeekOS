#include <stdbool.h>
#include <60-330/peterson.h>

//Peterson's solution to the critical sections problem
void peterson(void (*critical_section)(void), void (*remainder_section)(void), struct peterson_t* p, peterson_index_t index) {
	int i = (index == P_I) ? 0 : 1;
	int j = (index == P_I) ? 1 : 0;
	p->flag[i] = true;
	p->turn = j;
	while (p->flag[j] && p->turn == j);
	critical_section();
	p->flag[i] = false;
	remainder_section();
}
