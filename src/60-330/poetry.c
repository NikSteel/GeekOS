#include <stdbool.h>
#include <geekos/string.h>
#include <geekos/thread.h>
#include <geekos/cons.h>
#include <geekos/timer.h>
#include <60-330/poetry.h>
#include <60-330/job.h>
#include <60-330/rand.h>

static struct poem_t s_poem = {.flower_a = "", .flower_b = "", .colour_a = "", .colour_b = "", .statement = "", .cadence = ""};

static const char s_flowers[POEM_NUM_FLOWER][POEM_WORD_LENGTH] = {
	"Roses", "Daisies", "Sunflowers", "Daffodils", "Lilies", "Irises", "Tulips", "Poppies"
};

static const char s_colours[POEM_NUM_COLOUR][POEM_WORD_LENGTH] = {
	"red", "yellow", "blue", "orange", "purple", "green", "brown", "black", "white"
};

static const char s_statement[POEM_NUM_STATEMENT][POEM_CLAUSE_LENGTH] = {
	"Bicycles rock",
	"Sugar is sweet",
	"Trucks drink gas",
	"Words suck",
	"Sheep are fun",
	"Rhyming is hard"
};

static const char s_cadence[POEM_NUM_CADENCE][POEM_CLAUSE_LENGTH] = {
	"I like ponies.",
	"And so are you.",
	"Where's my car?",
	"SEGFAULT...",
	"Who ate my chips?",
	"I am so smrt.",
	"Supercalifragilisticexpialidocious"
};

static const int s_rand[POEM_NUM_RAND] = {
	3, 5, 7, 11, 13, 17, 23, 31, 53, 107, 43, 63, 197, 8, 24, 1024, 555, 999
};	

void write_poem(void) {
	plan_poem();
	strncpy(s_poem.flower_a, s_flowers[next_rand(POEM_NUM_FLOWER)], POEM_WORD_LENGTH);
	strncpy(s_poem.colour_a, s_colours[next_rand(POEM_NUM_COLOUR)], POEM_WORD_LENGTH);
	strncpy(s_poem.flower_b, s_flowers[next_rand(POEM_NUM_FLOWER)], POEM_WORD_LENGTH);
	strncpy(s_poem.colour_b, s_colours[next_rand(POEM_NUM_COLOUR)], POEM_WORD_LENGTH);
	strncpy(s_poem.statement, s_statement[next_rand(POEM_NUM_STATEMENT)], POEM_CLAUSE_LENGTH);
	strncpy(s_poem.cadence, s_cadence[next_rand(POEM_NUM_CADENCE)], POEM_CLAUSE_LENGTH);
	print_poem();
	savour_poem();
}

void wait_to_write_poem(void) {
	cons_printf("PID %d: A poet is waiting...\n", g_current->process->pid);
	busy_wait(JOB_DELAY);
}

void plan_poem(void) {
	cons_printf("PID %d: The poet is thinking.\n", g_current->process->pid);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d: The poet is thinking..\n", g_current->process->pid);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d: The poet is thinking...\n", g_current->process->pid);
	busy_wait(JOB_DELAY);
}

void savour_poem(void) {
	cons_printf("PID %d: The poet is savouring the poem.\n", g_current->process->pid);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d: I have written an amazing poem!\n", g_current->process->pid);
	busy_wait(JOB_DELAY);
}

void print_poem(void) {
	cons_printf("\n\t~~~~~~~~~~~~~~~\n"
				"\t%s are %s,\n"
				"\t%s are %s,\n"
				"\t%s,\n"
				"\t%s\n"
				"\t~~~~~~~~~~~~~~~\n\n",
				s_poem.flower_a, s_poem.colour_a, s_poem.flower_b, s_poem.colour_b,
				s_poem.statement, s_poem.cadence);
}




	