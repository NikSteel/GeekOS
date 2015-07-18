#include <stdbool.h>
#include <geekos/int.h>
#include <geekos/types.h>
#include <geekos/cons.h>
#include <geekos/thread.h>
#include <geekos/timer.h>
#include <60-330/job.h>
#include <60-330/tictactoe.h>
#include <60-330/rand.h>

//a tic tac toe game for use in demostrating Peterson's solution to the 
//critical sections problem

static struct tictactoe_t s_tictactoe = {.status = GAME_OPEN};

tictactoe_state_t *g_tictactoe_status = &(s_tictactoe.status);

static int moves[9][2] = {
	{2,0}, {1,1}, {0,2},
	{1,2}, {1,0}, {0,0},
	{2,2}, {2,1}, {0,1}
};

//join an open game, otherwise, start a new one, erasing any existing games
//in the process
char join_tictactoe_game(void) {
	
	if (s_tictactoe.status == GAME_READY) {
		bool iflag = int_begin_atomic();
		s_tictactoe.player_o = g_current->process->pid;
		s_tictactoe.status = GAME_PLAYING;
		cons_printf("PID %d: Joining process %d\'s tic tac toe game...\n", g_current->process->pid, s_tictactoe.player_x);
		int_end_atomic(iflag);
		return 'O';
	} else
		start_tictactoe_game();
		return 'X';
}

//initialize a tic tac toe game
void start_tictactoe_game(void) {
	bool iflag = int_begin_atomic();
	int i, j;
	for (i = 0; i < 3; ++i)
		for (j = 0; j < 3; ++j)
			s_tictactoe.grid[i][j] = ' ';
	s_tictactoe.player_x = g_current->process->pid;
	s_tictactoe.player_o = 0;
	s_tictactoe.winner = 0;
	s_tictactoe.status = GAME_READY;
	cons_printf("PID %d: Starting a new tic tac toe game...\n", g_current->process->pid);
	int_end_atomic(iflag);
}

//play the game (critical section)
void play_tictactoe(void) {
	//block if a game is not in progress
	if (s_tictactoe.status != GAME_PLAYING)
		return;
	//block unregistered players
	if ((g_current->process->pid != s_tictactoe.player_x) && (g_current->process->pid != s_tictactoe.player_o))
		return;

	//think
	plan_move();
	//make a move
	choose_move();
	//display game board
	print_board();
	//check for a winner
	check_for_winner();
}

//wait for a turn (remainder section)
void wait_for_turn(void) {
	//block if a game is not in progress
	if (s_tictactoe.status != GAME_PLAYING)
		return;
	char c = (g_current->process->pid == s_tictactoe.player_x) ? 'X' : 'O';
	cons_printf("PID %d: Player \'%c\' is waiting...\n", g_current->process->pid, c);
	busy_wait(JOB_DELAY);
}

//spend some time thinking about turn
void plan_move(void) {
	char c = (g_current->process->pid == s_tictactoe.player_x) ? 'X' : 'O';
	cons_printf("PID %d: Player \'%c\' is thinking.\n", g_current->process->pid, c);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d: Player \'%c\' is thinking..\n", g_current->process->pid, c);
	busy_wait(JOB_DELAY);
	cons_printf("PID %d: Player \'%c\' is thinking...\n", g_current->process->pid, c);
	busy_wait(JOB_DELAY);
}

//choose a move
void choose_move(void) {
	//block if a game is not in progress
	if (s_tictactoe.status != GAME_PLAYING)
		return;
	
	char c = (g_current->process->pid == s_tictactoe.player_x) ? 'X' : 'O';
	int i, x, y;
	shuffle_moves(5);
	for (i = 0; i < 9; ++i) {
		x = moves[i][0];
		y = moves[i][1];
		if (s_tictactoe.grid[x][y] == ' ') {
			s_tictactoe.grid[x][y] = c;
			break;
		}
	}
}

void shuffle_moves(int num_shuffle){
	int temp[2], a, b, i;
	for (i = 0; i < num_shuffle; ++i) {
		a = next_rand(9);
		b = next_rand(9);
		temp[0] = moves[a][0];
		temp[1] = moves[a][1];
		moves[a][0] = moves[b][0];
		moves[a][1] = moves[b][1];
		moves[b][0] = temp[0];
		moves[b][1] = temp[1];
	}
}

//print the tic tac toe game board
void print_board(void) {
	//block if a game is not in progress
	if (s_tictactoe.status != GAME_PLAYING)
		return;

	cons_printf("\n\t %c | %c | %c \n"
				"\t-----------\n"
				"\t %c | %c | %c \n"
				"\t-----------\n"
				"\t %c | %c | %c \n\n",
				s_tictactoe.grid[0][0],s_tictactoe.grid[0][1],s_tictactoe.grid[0][2],
				s_tictactoe.grid[1][0],s_tictactoe.grid[1][1],s_tictactoe.grid[1][2],
				s_tictactoe.grid[2][0],s_tictactoe.grid[2][1],s_tictactoe.grid[2][2]);
}

//check for a winner or draw condition, end the game if so 
void check_for_winner(void) {
	//block if a game is not in progress
	if (s_tictactoe.status != GAME_PLAYING)
		return;
		
	int i, j, val;
	//check diagonals
	val = s_tictactoe.grid[0][0];
	
	if ((val != ' ') && (val == s_tictactoe.grid[1][1]) && (val == s_tictactoe.grid[2][2]))
		return assign_winner(val);
	val = s_tictactoe.grid[2][0];
	if ((val != ' ') && (val == s_tictactoe.grid[1][1]) && (val == s_tictactoe.grid[0][2]))
		return assign_winner(val);

	//check rows and columns
	for (i = 0; i < 3; ++i) {
		//check rows
		val = s_tictactoe.grid[i][0];
		if ((val != ' ') && (val == s_tictactoe.grid[i][1]) && (val == s_tictactoe.grid[i][2]))
			return assign_winner(val);
		//check columns
		val = s_tictactoe.grid[0][i];
		if ((val != ' ') && (val == s_tictactoe.grid[1][i]) && (val == s_tictactoe.grid[2][i]))
			return assign_winner(val);
	}	

	//check for a draw (no blank spaces remain)
	bool draw = true;
	for (i = 0; i < 3; ++i)
		for (j = 0; j < 3; ++j)
			draw = draw && (s_tictactoe.grid[i][j] == ' ');
	if (draw)
		assign_draw();
		
	//otherwise, the game is still ongoing
}

//assign and announce a winner
void assign_winner(char c) {
	//block if a game is not in progress
	if (s_tictactoe.status != GAME_PLAYING)
		return;
	bool iflag = int_begin_atomic();
	s_tictactoe.status = GAME_DONE;
	s_tictactoe.winner = (c == 'X') ? s_tictactoe.player_x : s_tictactoe.player_o;
	cons_printf("PID %d: Player \'%c\' won the game!\n", g_current->process->pid, c);
	int_end_atomic(iflag);
}

//assign and announce a draw
void assign_draw(void) {
	//block if a game is not in progress
	if (s_tictactoe.status != GAME_PLAYING)
		return;
	bool iflag = int_begin_atomic();
	s_tictactoe.status = GAME_DONE;
	cons_printf("PID %d: It\'s a DRAW!\n", g_current->process->pid);
	int_end_atomic(iflag);
}