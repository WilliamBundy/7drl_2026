#pragma once
#include <SDL3/SDL.h>
#include "wb_gamemath.h"

typedef struct Xform
{
	float2 pos, origin;
	float scale, reserved;
} Xform;
static const Xform Xzero = (Xform){0, 0, 1}, Xdouble = {0, 0, 2}, Xtriple = {0, 0, 3}; 

static inline
float2 pXform(float2 p, Xform c)
{
	return (p - c.pos) * c.scale + c.origin;
}

static inline
float2 pXformInv(float2 p, Xform c)
{
	return (p - c.origin) / c.scale + c.pos;
}
static inline
float2 iXform(int2 p, float2 size, float2 offset, Xform c)
{
	return ((float2){(float)p.x, (float)p.y} * size + offset - c.pos) * c.scale + c.origin;
}

static inline
SDL_FRect pXformRect(float2 p, float2 size, Xform c)
{
	float2 pos = pXform(p, c);
	return (SDL_FRect){pos.x, pos.y, size.x * c.scale, size.y * c.scale};
}

static inline
SDL_FRect rXformRect(SDL_FRect fr, Xform c)
{
	float2 pos = (float2){fr.x, fr.y};
	float2 size = (float2){fr.w, fr.h};
	return pXformRect(pos, size, c);
}

static inline
SDL_FRect pXformRectInv(float2 p, float2 size, Xform c)
{
	float2 pos = pXformInv(p, c);
	return (SDL_FRect){pos.x, pos.y, size.x / c.scale, size.y / c.scale};
}

static inline
SDL_FRect iXformRect(int2 p, float2 size, float2 offset, Xform c)
{
	float2 pos = iXform(p, size, offset, c);
	return (SDL_FRect){pos.x, pos.y, size.x * c.scale, size.y * c.scale};
}

static inline
SDL_FRect iXformRectXY(int x, int y, float2 size, float2 offset, Xform c)
{
	return iXformRect((int2){x, y}, size, offset, c);
}

typedef struct SpriteClip
{
	float4 region;
	float2 offset;
	int flags;
	int reserved;
} SpriteClip;

typedef struct SpriteAtlas
{
	SpriteClip* clips;
	int numClips, maxClips;

	SDL_Texture* texture;
	void* reserved;
} SpriteAtlas;

SpriteAtlas* createSpriteAtlas(int maxClips, SDL_Texture* texture);
void atlasAdd(SpriteAtlas* atlas, SpriteClip clip);
int atlasAddGrid(SpriteAtlas* atlas, int x, int y, int clipW, int clipH, int gridW, int gridH);

static inline
SDL_FRect atlasGetSrcRect(SpriteAtlas* atlas, int clip)
{
	float4 r = atlas->clips[clip].region;
	SDL_FRect fr = {r[0], r[1], r[2], r[3]};
	return fr;
}

enum 
{
	Sprite_FlipHoriz = 1<<4,
	Sprite_FlipVert = 1<<5,
};

typedef struct Sprite
{
	union {
		struct { float x, y, w, h; };
		struct { float2 pos, size; };
		float4 region;
		SDL_FRect dst;
	};

	float2 rot;
	float2 center;
	float2 anchor;
	float2 offset;

	uint32_t argb;
	int clip;

	uint32_t flags;
	int reserved;
} Sprite;

static inline
void initSprite(Sprite* s)
{
	Sprite ss = {0};
	ss.rot.x = 1.0f;
	ss.argb = 0xFFFFFFFFu;
	ss.clip = -2;
	*s = ss;
}

typedef struct SpriteBatch
{
	Sprite* sprites;
	int numSprites, maxSprites;

	Xform camera;
	SpriteAtlas* atlas;
} SpriteBatch;

SpriteBatch* createSpriteBatch(int maxSprites, SpriteAtlas* atlas);

void drawSprite(Sprite* s, SpriteAtlas* atlas, Xform camera, bool intLock, SDL_Renderer* renderer);
Sprite* nextSprite(SpriteBatch* batch);
void addSprite(SpriteBatch* batch, Sprite* sprite);
void drawSpriteBatch(SpriteBatch* batch, SDL_Renderer* renderer);


