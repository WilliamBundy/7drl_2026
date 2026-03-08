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

This is a single-header implementation of Eller's algorithm for generating mazes.

Status:
Tested and works, though I'm not sure how useful it is on its own.

*/



#ifdef WB_CLANGD_ENABLE
#define WB_MAZEGEN_IMPLEMENTATION
#endif

#ifndef _WB_MAZEGEN_H_
#define _WB_MAZEGEN_H_

#ifndef WB_MAZEGEN_memset
#include <string.h>
#define WB_MAZEGEN_memset memset
#endif

#include <stdint.h>

typedef struct wbMazegenCtx
{
	int width, height, nextRow, lastSet;
	int* cells;

	uint32_t rightWallChance;
	uint32_t bottomWallChance;

	int* scratchRow;
	int* scratchRwalls;
	int* scratchBwalls;
	int* scratchSets;
	uint64_t seed;
} wbMazegenCtx;

static inline
size_t wb_mazegen_GetCtxSize(int width, int height)
{
	return sizeof(wbMazegenCtx) + (4 * sizeof(int) * (width/2) + 1) + 256;
}

wbMazegenCtx* wb_mazegen_InitCtx(int* cells, int width, int height, void* memory, size_t size);
void wb_mazegen_GenerateRow(wbMazegenCtx* ctx, int y);
void wb_mazegen_Generate(wbMazegenCtx* ctx, uint64_t seed);

#endif

#ifdef WB_MAZEGEN_IMPLEMENTATION
#ifndef _WB_MAZEGEN_C_
#define _WB_MAZEGEN_C_

static inline
uint64_t wb_mazegen_rng(uint64_t* x)
{
	// this is splitmix64
	*x += UINT64_C(0x9E3779B97F4A7C15);
	uint64_t z = *x;
	z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	return z ^ (z >> 31);	
}

wbMazegenCtx* wb_mazegen_InitCtx(int* cells, int width, int height, void* memory, size_t size)
{
	size_t expectedSize = wb_mazegen_GetCtxSize(width, height);
	if(size < expectedSize) {
		return nullptr;
	}
	if(!memory) {
		return nullptr;
	}

	wbMazegenCtx* ctx = memory;
	ctx->cells = cells;
	ctx->scratchRwalls = (void*)(ctx + 1);
	ctx->scratchBwalls = (ctx->scratchRwalls + width/2 + 1);
	ctx->scratchRow = (ctx->scratchBwalls + width/2 + 1);
	ctx->scratchSets = (ctx->scratchRow + width/2 + 1);

	ctx->rightWallChance = UINT32_MAX >> 1;
	ctx->bottomWallChance = UINT32_MAX >> 1;
	ctx->width = width;
	ctx->height = height;
	ctx->nextRow = 0;
	ctx->lastSet = 1;

	// give it something if it's otherwise forgotten to be set
	// zero can give really bad results with rng
	ctx->seed = (uint64_t)memory + width + height;
	wb_mazegen_rng(&ctx->seed);
	return ctx;
}

static inline
int wb_mazegen_setScan(int rowWidth, int* row, int* sets)
{
	int numOut = 0;

	for(int i = 0; i < rowWidth; ++i) {
		bool found = 0;
		for(int j = 0; j < numOut; ++j) {
			// tile is in output sets
			if(row[i] == sets[j]) {
				found = true;
				break;
			}
		}
		if(found) continue;
		sets[numOut++] = row[i];
	}
	return numOut;
}

void wb_mazegen_GenerateRow(wbMazegenCtx* ctx, int y)
{
	uint64_t rwallchance = ctx->rightWallChance;
	uint64_t bwallchance = ctx->bottomWallChance;
	int w = ctx->width / 2;
	int h = ctx->height / 2;

	int* row = ctx->scratchRow;
	int* rwalls = ctx->scratchRwalls;
	int* bwalls = ctx->scratchBwalls;
	int set = ctx->lastSet;

	int tmw = ctx->width;
	int* tilemap = ctx->cells;

	for(int x = 0; x < w; ++x) {
		if(row[x] == 0) row[x] = set++;
	}

	for(int x = 0; x < w; ++x) {
		if(x == w - 1) {
			rwalls[x] = 1;
			break;
		}

		if((wb_mazegen_rng(&ctx->seed) >> 32) < rwallchance || row[x] == row[x+1]) {
			rwalls[x] = 1;
		} else {
			int s = row[x+1];
			for(int xx = 0; xx < w; xx++) {
				if(row[xx] == s) row[xx] = row[x];
			}
		}
	}

	for(int x = 0; x < w; ++x) {
		if(!rwalls[x] && row[x+1] != row[x]) {
			row[x+1] = row[x];
			int s = row[x+1];
			for(int xx = 0; xx < w; xx++) {
				if(row[xx] == s) row[xx] = row[x];
			}
		}
	}

	for(int x = 0; x < w; ++x) {
		if((wb_mazegen_rng(&ctx->seed) >> 32) < bwallchance || y == h - 1) {
			bwalls[x] = 1;
		}
	}

	int* sets = ctx->scratchSets;
	int numSets = wb_mazegen_setScan(w, row, sets);

	for(int i = 0; i < numSets; ++i) {
		int s = sets[i];
		bool hasConnection = false;
		for(int x = 0; x < w; ++x) {
			if(row[x] == s && bwalls[x] == 0) {
				hasConnection = true;
				break;
			}
		}

		if(!hasConnection) {
			for(int x = 0; x < w; ++x) {
				if(row[x] == s) {
					bwalls[x] = 0;
					break;
				}
			}
		}
	}

	if(y == h - 1) {
		for(int x = 0; x < w-1; ++x) {
			if(rwalls[x] && row[x] != row[x+1]) {
				rwalls[x] = 0;
				int s = row[x+1];
				for(int xx = 0; xx < w; xx++) {
					if(row[xx] == s) row[xx] = row[x];
				}
			}
		}
	}

	for(int x = 0; x < w; ++x) {
		tilemap[y*2 * tmw + x*2] = 1;
		if(!rwalls[x]) {
			tilemap[y*2 * tmw + x*2+1] = 1;
		}

		if(!bwalls[x] && y < h - 1) {
			tilemap[(y*2+1) * tmw + x*2] = 1;
		}
	}


	WB_MAZEGEN_memset(rwalls, 0, sizeof(int) * w);
	for(int x = 0; x < w; ++x) {
		if(bwalls[x]) row[x] = 0;
	}
	WB_MAZEGEN_memset(bwalls, 0, sizeof(int) * w);
	ctx->lastSet = set;
}

void wb_mazegen_Generate(wbMazegenCtx* ctx, uint64_t seed)
{
	ctx->seed = seed;
	for(int i = 0; i < 16; ++i) {
		wb_mazegen_rng(&ctx->seed);
	}

	for(int y = 0; y < ctx->height/2; ++y) {
		wb_mazegen_GenerateRow(ctx, y);
	}
}
#endif
#endif

