/*
Copyright 2026 William Bundy, all rights reserved.

Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

This is a possibly diabolical implementation of A* in C. 

### Building

Standard single-header:
#define WB_PATHFIND_IMPLEMENTATION
and include this header to add the source to your project. Do this in a 
single C file only.

### Usage:

wbpath_state state = {
	// your tilemap data, such that:
	//		map[index] & SOLID_MASK == 1
	// if a tile is impassable
	.map = map,
	.solidMask = SOLID_MASK,
	.size = {mapw, maph},

	// temp/scratch data used by the algorithm
	// can be re-used, must be zeroed
	.cells = calloc(mapw * maph, sizeof(wbpath_cell)),

	// used to make wbpath_trace work correctly
	// can be left uninitialized
	.start = {0, 0},
	.numSteps = 0
};

// this performs the A* search, populating the path in the 
// ".open" field of the cells grid
int ret = wbpath_search(start, end, &state);
if(ret == WBPATH_NOT_FOUND) {
	printf("can't reach target!\n");
	return
}

// this extracts the path as a series of deltas
// numSteps in the state struct will be correct, so you can use
// that to heap allocate the list if necessary.
int2 steps[32];
int numSteps = wbpath_trace(steps, 32, &state);

for(int i = 0; i < numSteps; ++i) {
	entity->x += steps[i].x;
	entity->y += steps[i].y;
	simulate();
}

Status:
Completely untested! Probably has astoundingly awful bugs
*/
#ifndef _WB_PATHFIND_H_
#define _WB_PATHFIND_H_

#include <stdint.h>

#ifndef _INT2_DECLARED_
#define _INT2_DECLARED_
typedef struct int2
{
	int x, y;
} int2;

#endif

#define CELL_CLOSED (1<<31)
typedef  struct wbpath_cell
{
	uint32_t parent;
	uint32_t open;
	uint32_t h;
	uint32_t g;
} wbpath_cell;

typedef struct wbpath_state
{
	int* map;
	wbpath_cell* cells;
	int2 size;
	int2 start;

	int solidMask;
	int numSteps;
} wbpath_state;

int wbpath_search(int2 start, int2 end, wbpath_state* state);

static inline
int wbpath_pos_to_index(int2 pos, int2 size)
{
	return pos.y * size.x + pos.x;
}

static inline
int2 wbpath_index_to_pos(int index, int2 size)
{
	return {index % size.x, index / size.x};
}

#define WBPATH_FOUND 0
#define WBPATH_NOT_FOUND -1

#endif

#ifdef WB_PATHFIND_IMPLEMENTATION
#ifndef _WB_PATHFIND_C_
#define _WB_PATHFIND_C_

static inline
int wbpath_abs(int a)
{
	return a < 0 ? -a : a;
}

static inline int wbpath_f(wbpath_cell* cell)
{
	return (cell->g & ~CELL_CLOSED) + cell->h;
}

int wbpath_trace(int2* steps, int maxSteps, wbpath_state* state)
{
	int2 size = state->size;
	int2 start = state->start;
	wbpath_cell* cells = state->cells;
	int numSteps = state->numSteps;
	int index = start.y * size.x + start.x;
	int2 pos = start;
	for(int i = 0; i < numSteps; ++i) {
		if(i >= maxSteps) break;
		int next = cells[index].open;
		if(next == -1) {
			steps[i] = (int2){0, 0};
		} else {
			int2 nextpos = wbpath_index_to_pos(next, size);
			steps[i] = (int2){nextpos.x - pos.x, nextpos.y - pos.y};
			pos = nextpos;
		}
		index = next;
	}

	return numSteps;
}

int wbpath_search(int2 start, int2 end, wbpath_state* state)
{
	int2 size = state->size;
	int* map = state->map;
	wbpath_cell* cells = state->cells;
	int open = start.y * size.x + start.x;
	int numOpen = 1;

	int startIndex = open;
	int endIndex = end.y * size.x + end.x;

	while(numOpen > 0) {
		int index = open;
		open = cells[index].open;
		numOpen--;
		cells[index].g |= CELL_CLOSED;
		int x = index % size.x;
		int y = index / size.x;
		for(int i = -1; i <= 1; ++i) {
			if(y + i < 0 || y + i >= size.y) continue;
			for(int j = -1; j <= 1; ++j) {
				if(i == 0 && j == 0) continue;
				if(x + j < 0 && x + j >= size.x) continue;
				int child = (y + i) * size.x + (x + j);

				if(child == endIndex) {
					cells[child].parent = index;
					cells[child].open = -1;
					int lastParent = child;
					int nextParent = index;
					int steps = 0;
					while(lastParent != startIndex) {
						cells[nextParent].open = lastParent;
						lastParent = nextParent;
						nextParent = cells[nextParent].parent;
						steps++;
					}
					state->numSteps = steps;
					state->start = start;
					return WBPATH_FOUND;
				}

				wbpath_cell* cell = &cells[child];
				int closed = cell->g & CELL_CLOSED;
				if(closed || (state->map[child] & state->solidMask)) {
					continue;
				}

				int g = (cells[index].g & ~CELL_CLOSED) + 1;
				// taxicab heuristic
				int h = wbpath_abs(x + j - end.x) + wbpath_abs(y + i - end.y);

				if(cells[child].h == 0 || wbpath_f(cell) > (g + h)) {
					cell->g = g;
					cell->h = h;
					cell->parent = index;
					int f = (g + h);
					if(numOpen == 0) {
						numOpen = 1;
						open = child;
					} else {
						int lastOpen = -1;
						int nextOpen = open;
						while(wbpath_f(&cells[nextOpen]) < f) {
							lastOpen = nextOpen;
							nextOpen = cells[nextOpen].open;
						}

						if(lastOpen != -1) {
							cells[lastOpen].open = child;
						}
						cells[child].open = nextOpen;
						numOpen++;
					}
				}
			}
		}
	}
	return WBPATH_NOT_FOUND;
}


#endif
#endif