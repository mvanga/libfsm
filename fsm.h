#ifndef __FSM_H
#define __FSM_H

#define FSM_START_STATE	1
#define FSM_END_STATE	2

typedef int (*state_func_t)(void *);

struct state {
	int id;
	int flags;
	state_func_t func;
	struct state *next;
};

struct fsm {
	struct state *states;
	struct state *cstate;
	void *data;
};

#endif
