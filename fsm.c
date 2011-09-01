#include <stdio.h>
#include <stdlib.h>

#include "fsm.h"

struct fsm *fsm_create(void *data)
{
	struct fsm *fsm = malloc(sizeof(struct fsm));
	if (!fsm)
		return NULL;
	fsm->data = data;
	fsm->cstate = NULL;
	return fsm;
}

void fsm_destroy(struct fsm *fsm)
{
	struct state *s, *del;
	if (fsm) {
		s = fsm->states;
		while (s) {
			del = s;
			s = s->next;
			free(del);
		}
		free(fsm);
	}
}

int fsm_add_state(struct fsm *fsm, int id, state_func_t func, int flags)
{
	struct state *s;

	if (!fsm || id <= 0)
		return -1;
	for (s = fsm->states; s; s = s->next)
		if (s->id == id)
			return -1;

	s = malloc(sizeof(struct state));
	if (!s)
		return -1;
	s->id = id;
	s->func = func;
	s->flags = flags;
	if (s->flags & FSM_START_STATE && !fsm->cstate)
		fsm->cstate = s;
	s->next = fsm->states;
	fsm->states = s;

	return 0;
}

int fsm_update(struct fsm *fsm)
{
	int ret;
	struct state *s;

	if (!fsm->cstate)
		return -1;
	ret = fsm->cstate->func(fsm->data);
	if (ret < 0)
		return -1;
	if (ret == 0)
		return -2;
	for (s = fsm->states; s; s = s->next)
		if (s->id == ret)
			break;
	if (!s)
		return -1;
	fsm->cstate = s;
	return s->id;
}

#define STATE_NONE	1
#define STATE_A_FOUND	2
#define STATE_AAC_FOUND	3
#define STATE_ERR	4

struct aac {
	int nfound;
	int current;
};
struct aac s = {
	.nfound = 0,
	.current = 0,
};

int state_none(void *data)
{
	struct aac *n = (struct aac *)data;
	if (n->current == 'a') {
		n->nfound++;
		return STATE_A_FOUND;
	}
	return STATE_ERR;
}

int state_a_found(void *data)
{
	struct aac *n = (struct aac *)data;
	if (n->current == 'a') {
		if (n->nfound == 1) {
			n->nfound++;
			return 0;
		}
		return STATE_ERR;
	}
	if (n->current == 'c') {
		n->nfound = 0;
		return STATE_AAC_FOUND;
	}
	return STATE_ERR;
}

int state_aac_found(void *data)
{
	struct aac *n = (struct aac *)data;
	return 0;
}

int state_err(void *data)
{
	struct aac *n = (struct aac *)data;
	return 0;
}

int main()
{
	int i;
	char *buf = "aad";
	struct fsm *fsm = fsm_create(&s);
	fsm_add_state(fsm, STATE_NONE, state_none, FSM_START_STATE);
	fsm_add_state(fsm, STATE_A_FOUND, state_a_found, 0);
	fsm_add_state(fsm, STATE_AAC_FOUND, state_aac_found, FSM_END_STATE);
	fsm_add_state(fsm, STATE_ERR, state_err, FSM_END_STATE);
	for (i = 0; i < 3; i++) {
		s.current = buf[i];
		fsm_update(fsm);
	}
	if (!fsm->cstate->flags & FSM_END_STATE || fsm->cstate->id == STATE_ERR) {
		printf("didn't find\n");
	} else {
		printf("found!\n");
	}
	fsm_destroy(fsm);
	return 0;
}
