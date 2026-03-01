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

This is a close reimplementation of Adam Milazzo's roguelike FOV algorithm.
His original code is in C# and is dedicated to the public domain (he makes
this assignment in the comments.) The original can be found here:

https://www.adammil.net/blog/v125_Roguelike_Vision_Algorithms.html#mycode

### Building

Standard single-header:
#define WB_MILAZZO_FOV_IMPLEMENTATION
and include this header to add the source to your project. Do this in a 
single C file only.

### Usage

// set up your state
wbfov_state state = {
	// your map, encoded such that (map[index] & OPAQUE_BIT) is nonzero if
	// the tile blocks light, and (vis[index] & VISIBLE_BIT) will be set to
	// 1 if it's visible. so long as the bits are different, it should be 
	// fine to overlap them
	.map = map,
	.vis = map,
	.opaqueMask = OPAQUE_BIT,
	.visibleMask = VISIBLE_BIT,
	.size = {mapw, maph},
};

// call this with the position, range, and state
wbfov_compute_visibility(player->pos, player->visRange, &state);
// the output will be set in the state.vis map


Status:
Completely untested! 

*/

#ifndef _WB_MILAZZO_FOV_H_
#define _WB_MILAZZO_FOV_H_
#include <stdint.h>

#ifndef _INT2_DECLARED_
#define _INT2_DECLARED_
typedef struct int2
{
	int x, y; ;
} int2;
#endif


#define POS_X 0
#define POS_Y 1
#define NEG_X 2
#define NEG_Y 3
const int2 wbfov_octant_lookup[] = {
	{POS_X, NEG_Y},
	{POS_Y, NEG_X},
	{NEG_Y, NEG_X},
	{NEG_X, NEG_Y},
	{NEG_X, POS_Y},
	{NEG_Y, POS_X},
	{POS_Y, POS_X},
	{POS_X, POS_Y}
};
#undef POS_X
#undef POS_Y
#undef NEG_X
#undef NEG_Y

typedef struct wbfov_state
{
	int* map;
	int* vis;
	int2 size;

	int opaqueMask;
	int visibleMask;
} wbfov_state;

static inline
int2 wbfov_calc_real_pos(int2 pos, int2 origin, int octant)
{
	int offset[] = {pos.x, pos.y, -pos.x, -pos.y};
	int2 diff = wbfov_octant_lookup[octant];
	int2 npos = {origin.x + offset[diff.x], origin.y + offset[diff.y]};
	return npos;
}

static inline 
bool wbfov_check_opaque(int2 pos, int2 origin, int octant, wbfov_state* state)
{
	int2 npos = wbfov_calc_real_pos(pos, origin, octant);
	int2 size = state->size;
	if(npos.x < 0 || npos.x >= size.y || npos.y < 0 || npos.y >= size.y) {
		return false;
	}
	return state->map[npos.y * size.x + npos.x] & state->opaqueMask;
}

static inline
void wbfov_set_visible(int2 pos, int2 origin, int octant, wbfov_state* state)
{
	int2 npos = wbfov_calc_real_pos(pos, origin, octant);
	int2 size = state->size;
	if(npos.x < 0 || npos.x >= size.y || npos.y < 0 || npos.y >= size.y) {
		return;
	}
	state->vis[npos.y * size.x + npos.x] |= state->visibleMask;
}

static inline 
int wbfov_within_distance(int2 pos, int2 origin, int limit)
{
	int2 diff = {pos.x - origin.x, pos.y - origin.y};
	diff.x *= diff.x;
	diff.y *= diff.y;
	return (diff.x + diff.y) <= (limit * limit);
}

void wbfov_compute_octant(
		int octant, 
		int2 origin, 
		int rangeLimit, 
		int x, 
		int2 hislope, 
		int2 loslope, 
		wbfov_state* state);
void wbfov_compute_visibility(int2 origin, int rangeLimit, wbfov_state* state);

#endif

#ifdef WB_MILAZZO_FOV_IMPLEMENTATION
#ifndef _WB_MILAZZO_FOV_C_
#define _WB_MILAZZO_FOV_C_

void wbfov_compute_visibility(int2 origin, int rangeLimit, wbfov_state* state)
{
	state->vis[origin.y * state->size.x + origin.x] |= state->visibleMask;
	for(int octant = 0; octant < 8; ++octant) {
		wbfov_compute_octant(
			octant, 
			origin, 
			rangeLimit, 
			1, 
			(int2){1, 1}, 
			(int2){0, 1}, 
			state);
	}
}

void wbfov_compute_octant(
		int octant, 
		int2 origin, 
		int rangeLimit, 
		int x, 
		int2 hislope, 
		int2 loslope, 
		wbfov_state* state)
{
	for(; x <= rangeLimit; ++x) {
		int topY, bottomY;
		if(hislope.x == 1) {
			topY = x;
		} else {
			topY = ((x*2-1) * hislope.y + hislope.x) / (hislope.x*2);
			if(wbfov_check_opaque((int2){x, topY}, origin, octant, state)) {
				if(hislope.y * (topY*2+1) >= hislope.x * (x*2)) {
					if(wbfov_check_opaque((int2){x, topY+1}, origin, octant, state)) {
						topY++;
					}
				}
			} else {
				int ax = x * 2;
				if(wbfov_check_opaque((int2){x+1, topY+1}, origin, octant, state)) {
					ax++;
				}
				if(hislope.y * (topY*2+1) > hislope.x * ax) {
					topY++;
				}
			}
		}

		if(loslope.y == 0) {
			bottomY = 0;
		} else {
			bottomY = ((x*2-1) * loslope.y + loslope.x) / (loslope.x * 2);
			if(loslope.y * (bottomY*2+1) >= loslope.x * (x*2)) {
				if(wbfov_check_opaque((int2){x, bottomY}, origin, octant, state)) {
					if(!wbfov_check_opaque((int2){x, bottomY+1}, origin, octant, state)) {
						bottomY++;
					}
				}
			}
		}

		int wasOpaque = -1;
		for(int y = topY; y >= bottomY; y--) {
			int2 pos = {x, y};
			if(rangeLimit >= 0) {
				if(!wbfov_within_distance(pos, origin, rangeLimit)) {
					continue;
				}
			}

			bool isOpaque = wbfov_check_opaque(pos, origin, octant, state);
			if(!isOpaque) {
				if((y != topY) || (hislope.y * (y*4+1) > hislope.x * (x*4+1))) {
					if((y != bottomY) || (loslope.y * (y*4+1) < loslope.x * (x*4+1))) {
						wbfov_set_visible(pos, origin, octant, state);
					}
				}
			}

			if(x == rangeLimit) continue;

			if(isOpaque) {
				if(wasOpaque == 0) {
					int2 nslope = {y*2+1, x*2};
					if(wbfov_check_opaque((int2){x, y+1}, origin, octant, state)) {
						nslope.y--;
					}
					if(hislope.y * nslope.x > hislope.x * nslope.y) {
						if(y == bottomY) {
							loslope = nslope;
							break;
						} else {
							wbfov_compute_octant(
								octant, 
								origin, 
								rangeLimit, 
								x+1, 
								hislope, 
								nslope, 
								state);
						}
					} else {
						if(y == bottomY) {
							return;
						}
					}
				}
				wasOpaque = 1;
			} else {
				if(wasOpaque > 0) {
					int2 nslope = {y*2+1, x*2};
					if(wbfov_check_opaque((int2){x+1, y+1}, origin, octant, state)) {
						nslope.y++;
					}
					if(loslope.y * nslope.x >= loslope.x * nslope.y) {
						return;
					}
					hislope = nslope;
				}
				wasOpaque = 0;
			}
		}

		if(wasOpaque != 0) break;
	}
}

#endif
#endif
