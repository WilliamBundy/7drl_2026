#include "sprite.h"

// the forcedSize stuff is for font rendering with non-bitmap fonts
// allowing you to force the shape of the destination rect even if the
// input is slightly off

/*
static inline
bool atlasGetForceSize(SpriteAtlas* atlas, int clip, float* forcedScale)
{
	if(forcedScale) *forcedScale = 1.0;
	return false;
}
*/

void drawSprite(Sprite* s, SpriteAtlas* atlas, Xform camera, bool intLock, SDL_Renderer* renderer)
{
	if(s->clip == 0) {
		return;
	}

	SDL_FRect src;
	uint32_t color = s->argb;

	if(s->clip == -1) {
		src = atlasGetSrcRect(atlas, 0);
	} else if (s->clip >= atlas->numClips || s->clip < -1) {
		src = atlasGetSrcRect(atlas, 0);
		color = 0xFFFF00FF;
	} else {
		src = atlasGetSrcRect(atlas, s->clip);
	}


	float4 region = s->region;
	if(region.z == 0 && region.w == 0) {
		region.z = src.w;
		region.w = src.h;
	}	
	region.xy += f2rot((-s->anchor - 0.5f) * region.zw, s->rot);

	SDL_FRect dst = pXformRect(region.xy, region.zw, camera);

	float2 fcenter = s->center;
	fcenter -= s->anchor;
	SDL_FPoint center = (SDL_FPoint){fcenter.x, fcenter.y};
	center.x *= dst.w;
	center.y *= dst.h;

	/*
	float forcedScale;
	if(s->clip >= 0 && atlasGetForceSize(atlas, s->clip, &forcedScale)) {
		dst.w = src.w * forcedScale * camera.scale;
		dst.h = src.h * forcedScale * camera.scale;
	}
	*/

	if(intLock) {
		dst.x = SDL_floorf(dst.x);
		dst.y = SDL_floorf(dst.y);
	}

	SDL_SetTextureColorMod(atlas->texture, 
		(color >> 16) & 0xFF, 
		(color >> 8) & 0xFF, 
		color & 0xFF);
	SDL_SetTextureAlphaMod(atlas->texture, 
		(color >> 24) & 0xFF);
	SDL_RenderTextureRotated(
		renderer, 
		atlas->texture, 
		&src, 
		&dst, 
		SDL_atan2f(s->rot.y, s->rot.x) / SDL_PI_F * 180.0f, 
		&center,
		(s->flags & Sprite_FlipHoriz ? SDL_FLIP_HORIZONTAL : 0) | 
		(s->flags & Sprite_FlipVert ? SDL_FLIP_VERTICAL : 0));
}

Sprite* nextSprite(SpriteBatch* batch)
{
	if(batch->numSprites >= batch->maxSprites) {
		SDL_assert(0);
		return nullptr;
	}

	Sprite* s = batch->sprites + batch->numSprites++;
	initSprite(s);
	return s;
}

void addSprite(SpriteBatch* batch, Sprite* sprite)
{
	if(batch->numSprites >= batch->maxSprites) {
		SDL_assert(0);
		return;
	}

	Sprite* s = batch->sprites + batch->numSprites++;
	*s = *sprite;
}



void drawSpriteBatch(SpriteBatch* batch, SDL_Renderer* renderer)
{
	for(int i = 0; i < batch->numSprites; ++i) {
		drawSprite(&batch->sprites[i], batch->atlas, batch->camera, batch->intLock, renderer);
	}
	batch->numSprites = 0;
}

SpriteBatch* createSpriteBatch(int maxSprites, SpriteAtlas* atlas)
{
	SDL_assert(maxSprites > 0);

	size_t size = sizeof(SpriteBatch) + sizeof(Sprite) * maxSprites;
	SpriteBatch* batch = SDL_calloc(1, size);
	batch->sprites = (void*)(batch + 1);
	batch->atlas = atlas;
	batch->camera = Xzero;
	batch->maxSprites = maxSprites;

	return batch;
}

SpriteAtlas* createSpriteAtlas(int maxClips, SDL_Texture* texture)
{
	size_t size = sizeof(SpriteAtlas) + sizeof(SpriteClip) * maxClips;
	SpriteAtlas* atlas = SDL_calloc(1, size);
	atlas->clips = (void*)(atlas + 1);
	atlas->maxClips = maxClips;
	atlas->numClips = 1;
	atlas->texture = texture;
	return atlas;
}

void atlasAdd(SpriteAtlas* atlas, SpriteClip clip)
{
	if(atlas->numClips >= atlas->maxClips) {
		return;
	}
	atlas->clips[atlas->numClips++] = clip;
}

int atlasAddGrid(SpriteAtlas* atlas, int x, int y, int clipW, int clipH, int gridW, int gridH)
{
	int start = atlas->numClips;
	for(int i = 0; i < gridH; i += clipH) {
		for(int j = 0; j < gridW; j += clipW) {
			int lx = x + j;
			int ly = y + i;
			atlasAdd(atlas, (SpriteClip){{lx, ly, clipW, clipH}, 0, 0, 0});
		}
	}
	return start;
}
