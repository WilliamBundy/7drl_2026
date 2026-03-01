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

This is a quick and dirty implementation of some standard text layout for 
ASCII spritefonts and english text. It accepts a bitmap with glyphs in ascii
order (starting with 33, '!'), breaks them out individually, trims transparency
along the edges, and stores that in the spritefont struct. When drawing, you 
have the option of using the original width or the trimmed width, which
for me corresponds to mono or proportional layout. 

I usually wrap wbsf_drawText in a few different ways to make it less awkward
to use, usually for each different kind of text/font and using a global 
renderer/texture/camera to cut down on args

Status:
Mostly tested, probably works.
*/


#ifndef _WB_SPRITEFONT_H_ 
#define _WB_SPRITEFONT_H_

#include <stdint.h>
#include <SDL3/SDL.h>
#include "wb_gamemath.h"

typedef struct Spritefont
{
	union {
		int4 region;
		struct {
			int x, y, w, h;
		};
	};
	int chrWidth[256];
	int chrLSB[256];
	int row, line;
} wbsf_Spritefont;

// region: x/y: pos of top left corner of char pixels
//		   z/w: size of an individula glyph 
// row: glyphs per row, must be a power of 2 (it's usually 16)
// line: actual line height for spacing
// pxw/pxh: width/height of bitmap
void wbsf_spritefontInit(wbsf_Spritefont* sfm, int4 region, int row, int line, uint32_t* pixels, int pxw, int pxh);
float4 wbsf_drawText(
	SDL_Renderer* renderer, 
	wbsf_Spritefont* sfm, 
	const char* text, 
	float2 p, 
	size_t len, 
	size_t numCharsToShow, 
	float maxWidth, 
	SDL_Texture* texture, 
	float2 cameraOffset, 
	float zoom, 
	bool mono, 
	bool doDraw);

static inline
SDL_FRect wbsf_glyphSrc(wbsf_Spritefont* sfm, int g)
{
	g -= 33;
	return (SDL_FRect) {
		(int)(g & (sfm->row-1)) * sfm->w + sfm->x,
		(int)(g / (sfm->row)) * sfm->h + sfm->y,
		sfm->w,
		sfm->h
	};
}
#endif


#ifdef WB_SPRITEFONT_IMPLEMENTATION
#ifndef _WB_SPRITEFONT_C_
#define _WB_SPRITEFONT_C_

static inline
bool wbsf_charIsUpper(int c)
{
	return c >= 'A' && c <= 'Z';
}

static inline 
bool wbsf_charIsLower(int c)
{
	return c >= 'a' && c <= 'z';
}


static inline
bool wbsf_charIsNumber(int c)
{
	return c >= '0' && c <= '9';
}

static inline
bool wbsf_charIsLetter(int c)
{
	return wbsf_charIsUpper(c) || wbsf_charIsLower(c);
}

static inline
bool wbsf_charIsAlphanumeric(int c)
{
	return wbsf_charIsLetter(c) || wbsf_charIsNumber(c);
}


static inline
SDL_FRect wbsf_pXformRect(float2 p, float2 size, float2 offset, float scale, float2 origin)
{
	float2 pos = (p - offset) * scale + origin;
	return (SDL_FRect){pos.x, pos.y, size.x * scale, size.y * scale};
}

void wbsf_spritefontInit(wbsf_Spritefont* sfm, int4 region, int row, int line, uint32_t* pixels, int pxw, int pxh)
{
	sfm->region = region;
	sfm->row = row;
	sfm->line = line;

	for(int i = 0; i < 256; ++i) {
		sfm->chrWidth[i] = sfm->w;
		sfm->chrLSB[i] = 0;
	}

	for(int i = 33; i <= 127; ++i) {
		int w = -1;
		int lsb = -1;
		int srcx = (int)((i-33) & (sfm->row-1)) * sfm->w + sfm->x;
		int srcy = (int)((i-33) / (sfm->row)) * sfm->h + sfm->y;
		for(int x = 0; x < sfm->w; ++x) {
			for(int y = 0; y < sfm->h; ++y) {
				uint32_t px = pixels[(srcy + y) * pxw + (srcx + x)];
				if(px != 0) {
					lsb = x;
					break;
				}
			}
			if(lsb != -1) {
				break;
			}
		}
		for(int x = sfm->w-1; x >= 0; x--) {
			for(int y = 0; y < sfm->h; ++y) {
				uint32_t px = pixels[(srcy + y) * pxw + (srcx + x)];
				if(px != 0) {
					w = x;
					break;
				}
			}
			if(w != -1) {
				break;
			}
		}
		w -= lsb;
		sfm->chrWidth[i] = w + 1;
		sfm->chrLSB[i] = lsb;
	}
}

float4 wbsf_drawText(SDL_Renderer* renderer, wbsf_Spritefont* sfm, const char* text, float2 p, size_t len, size_t numCharsToShow, float maxWidth, SDL_Texture* texture, float2 cameraOffset, float zoom, bool mono, bool doDraw)
{
	if(len == -1) {
		len = SDL_strlen(text);
	}

	if(numCharsToShow == -1 || numCharsToShow > len) {
		numCharsToShow = len - 1;
	}

	size_t lasti = 0;
	float2 pen = 0;
	float2 ext = {sfm->w, sfm->h};
	bool usedLine = false;
	for(size_t i = 0; i < len; ++i) {
		int c = text[i];
		if(c < 0) c += 128 + 127;
		if(c != '\n' && (c < ' ' || c > '~')) continue;

		if((wbsf_charIsAlphanumeric(c) || c == '\'') && i < len - 1) continue;
		size_t llen = i - lasti;
		if(usedLine && maxWidth > 0 && llen * sfm->w + pen.x > maxWidth) {
			ext = f2max(pen, ext);
			pen.x = 0;
			pen.y += sfm->line;
			usedLine = false;
		}

		bool nonSpace = false;
		for(size_t j = lasti; j <= i; ++j) {
			if(j > numCharsToShow) break;
			if(text[j] == ' ') {
				if(pen.x != 0) {
					pen.x += mono ? sfm->w : sfm->w / 2;
				}

				continue;
			} else if(text[j] == '\n') {
				ext = f2max(pen, ext);
				pen.x = 0;
				pen.y += sfm->line;
				continue;
			}
			nonSpace = true;
			int g = text[j];

			SDL_FRect src = wbsf_glyphSrc(sfm, g);
			if(!mono) {
				src.x += sfm->chrLSB[g];
				src.w = sfm->chrWidth[g];
			}
			
			SDL_FRect dst = wbsf_pXformRect(pen + p, (float2){src.w, sfm->h}, cameraOffset, zoom, 0);
			if(doDraw) SDL_RenderTexture(renderer, texture, &src, &dst);
			if(!mono) {
				pen.x += src.w + 1;
			} else {
				pen.x += sfm->w;
			}
		}
		lasti = i + 1;
		if(nonSpace) {
			usedLine = true;
		}

		if(lasti > numCharsToShow) break;
	}

	return (float4){pen.x, pen.y, ext.x, ext.y};
}

#endif
#endif