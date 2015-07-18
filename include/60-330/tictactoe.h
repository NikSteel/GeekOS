#ifndef TICTACTOE_H
#define TICTACTOE_H

typedef enum {
	GAME_OPEN, GAME_READY, GAME_PLAYING, GAME_DONE 
} tictactoe_state_t;

struct tictactoe_t {
	char grid[3][3];
	int player_x;
	int player_o;
	tictactoe_state_t status;
	int winner;
};

extern tictactoe_state_t *g_tictactoe_status;

char join_tictactoe_game(void);
void start_tictactoe_game(void);
void play_tictactoe(void);
void wait_for_turn(void);
void plan_move(void);
void choose_move(void);
void print_board(void);
void check_for_winner(void);
void assign_winner(char);
void assign_draw(void);
void shuffle_moves(int);

#endif