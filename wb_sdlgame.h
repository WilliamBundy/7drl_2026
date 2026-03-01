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

This is a relatively quick and dirty helper for using SDL, esp. with 
their 2D accelerated renderer. 

It wraps SDL's window creation (and DPI scaling), handles input events for
single-player games, keeps track of elapsed frame time with some extra 
stability (hopefully), and offers "state" objects for quickly plugging in 
start/stop/update kinds of things, and also offers the ability to overlay them

Status:
Largely untested, probably works... mostly
*/


#define WB_SDLGAME_IMPLEMENTATION
#ifndef _WB_SDLGAME_H_
#define _WB_SDLGAME_H_
#include <stdint.h>
#include <SDL3/SDL.h>
#include "wb_gamemath.h"

typedef struct GameState GameState;
typedef struct GameContext GameContext;
typedef void (*GameProc)(GameState*, GameContext*);
typedef struct GameState
{
	int id;
	int totalSize;
	const char* name;
	GameProc start;
	GameProc update;
	GameProc tick;
	GameProc render;
	GameProc stop;
} GameSate;

typedef struct GameSettings
{
	int windowWidth, windowHeight;
	const char* title;
	bool bordlessNoMouseStyle;
	bool forceScale;
} GameSettings;

enum
{
	KEY_JUST_RELEASED = -1,
	KEY_RELEASED = 0,
	KEY_PRESSED = 1,
	KEY_JUST_PRESSED = 2,
};

typedef struct GameInput
{
	float2 mpos;
	float2 mwheel;
	int2 mwheeli;
	int8_t keys[SDL_SCANCODE_COUNT];
	int8_t mbtn[16];
	int8_t pad[SDL_GAMEPAD_BUTTON_COUNT];
	float axis[SDL_GAMEPAD_AXIS_COUNT];
	SDL_Gamepad* gamepad[16];
	int numGamepads;
} GameInput;

#define NUM_GAME_TIMES 16
#define GAME_TIME_MASK (NUM_GAME_TIMES-1)
typedef struct GameContext
{
	SDL_PropertiesID stateDict;
	GameState* state, *nextState, *overlay, *nextOverlay;

	SDL_Window* window;
	SDL_Renderer* renderer;

	bool stopOverlay;

	float minFrameTime;
	float maxFrameTime;
	float displayScale;
	float elapsed;
	double totalGameTime;
	uint64_t frameIndex, frameTimes[NUM_GAME_TIMES], frameAvg, lastTime;

	GameInput* input;
	int numInput, maxInput;

} GameContext;

// this is optional to use, but you can define and set this in your own
// main.c or equivalent, and then have a global instance to work with, which
// can make some function calls dramatically more concise 
extern GameContext* Game;

GameState* gamestateCreate(const char* name, int id, size_t totalSize);
void gamestateSetProcs(GameState* state, GameProc start, GameProc update, GameProc tick, GameProc render, GameProc stop);

GameContext* gameCreate(GameSettings settings);
void gamePreUpdate(GameContext* game);
void gamePostUpdate(GameContext* game);
SDL_AppResult gameHandleEvent(GameContext* game, SDL_Event* event);

void gameRegister(GameContext* game, GameState* state);
void gameSwitch(GameContext* game, const char* name);
void gameOverlay(GameContext* game, const char* name);
void gameStart(GameContext* game, const char* name);

#endif

#ifdef WB_SDLGAME_IMPLEMENTATION
#ifndef _WB_SDLGAME_C_
GameContext* Game;

GameState* gamestateCreate(const char* name, int id, size_t totalSize)
{
	GameState* state = SDL_calloc(1, totalSize);
	state->name = name;
	state->id = id;
	state->totalSize = totalSize;
	return state;
}

void gamestateSetProcs(GameState* state, GameProc start, GameProc update, GameProc tick, GameProc render, GameProc stop)
{
	if(start) state->start = start;
	if(update) state->update = update;
	if(tick) state->tick = tick;
	if(render) state->render = render;
	if(stop) state->stop = stop;
}

GameContext* gameCreate(GameSettings settings)
{
	GameContext* game = SDL_calloc(1, sizeof(GameContext));
	if(!Game) {
		Game = game;
	}

	game->maxInput = 1;
	game->input = SDL_calloc(game->maxInput, sizeof(GameInput));

	game->minFrameTime = 1.0f / 240.0f;
	game->maxFrameTime = 1.0f / 30.0f;

	bool forceScale = settings.forceScale;
	float forcedScale = 1.0f;
	game->displayScale = 1.0f;
	{
		int displayCount = 0;
		SDL_DisplayID* displays = SDL_GetDisplays(&displayCount);
		for(int i = 0; i < displayCount; ++i) {
			const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(displays[i]);
			game->displayScale = mode->pixel_density;
		}
		if(forceScale) {
			game->displayScale = forcedScale;
		}
	}

	int flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	if(settings.bordlessNoMouseStyle) {
		flags |= SDL_WINDOW_BORDERLESS;
	} else {
		flags |= SDL_WINDOW_RESIZABLE;
	}
	game->window = SDL_CreateWindow(settings.title, settings.windowWidth, settings.windowHeight, flags);

	SDL_ShowWindow(game->window);
	return game;
}

static inline void stateStart(GameState* state, GameContext* game) 
{
	if(state && state->start) {
		state->start(state, game);
	}
}
static inline void stateUpdate(GameState* state, GameContext* game) 
{
	if(state && state->start) {
		state->start(state, game);
	}
}
static inline void stateRender(GameState* state, GameContext* game) 
{
	if(state && state->render) {
		state->render(state, game);
	}
}
static inline void stateTick(GameState* state, GameContext* game) 
{
	if(state && state->tick) {
		state->tick(state, game);
	}
}
static inline void stateStop(GameState* state, GameContext* game) 
{
	if(state && state->stop) {
		state->stop(state, game);
	}
}

void gamePreUpdate(GameContext* game)
{
	SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(game->renderer);
}	

void gameUpdate(GameContext* game)
{

	stateUpdate(game->state, game);
	stateUpdate(game->overlay, game);

	stateRender(game->state, game);
	stateRender(game->overlay, game);

	if(game->stopOverlay) {
		stateStop(game->overlay, game);
		game->overlay = nullptr;
		game->stopOverlay = false;
	}

	if(game->nextState) {
		stateStop(game->overlay, game);
		game->overlay = nullptr;
		game->stopOverlay = false;

		stateStop(game->state, game);
		game->state = game->nextState;
		stateStart(game->state, game);
	}

	if(game->nextOverlay) {
		stateStop(game->overlay, game);
		game->overlay = game->nextOverlay;
		stateStart(game->overlay, game);
	}
}

void gamePostUpdate(GameContext* game)
{
	SDL_RenderPresent(game->renderer);

	uint64_t currentTime = SDL_GetPerformanceCounter();
	game->frameTimes[game->frameIndex++ & GAME_TIME_MASK] = currentTime - game->lastTime;
	game->lastTime = currentTime;

	{
		for(int i = 0; i < NUM_GAME_TIMES; ++i) {
			game->frameAvg += game->frameTimes[i];
		}
		game->frameAvg /= NUM_GAME_TIMES;
		game->elapsed = (double)game->frameAvg / (double)SDL_GetPerformanceFrequency();
		game->elapsed = f32clamp(game->elapsed, game->minFrameTime, game->maxFrameTime);
		game->totalGameTime += (double)game->frameAvg / (double)SDL_GetPerformanceFrequency();
	}
}


SDL_AppResult gameHandleEvent(GameContext *game, SDL_Event *event)
{
	if(event->type == SDL_EVENT_QUIT) {
		return SDL_APP_SUCCESS;
	}

	GameInput* input = game->input;
	SDL_JoystickID which;
	SDL_Gamepad* pad;
	switch(event->type) {
	case SDL_EVENT_GAMEPAD_ADDED:
		if(input->numGamepads >= SDL_arraysize(input->gamepad)) break;
		input->gamepad[input->numGamepads++] = SDL_OpenGamepad(event->gdevice.which);
		break;
	case SDL_EVENT_GAMEPAD_REMOVED:
		which = event->gdevice.which;
		pad = SDL_GetGamepadFromID(which);
		if(pad) {
			for(int i = 0; i < input->numGamepads; ++i) {
				if(input->gamepad[i] == pad) {
					input->gamepad[i] = input->gamepad[--input->numGamepads];
					input->gamepad[input->numGamepads] = nullptr;
					break;
				}
			}
			SDL_CloseGamepad(pad);
		}
		break;

	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_KEY_UP:
		if(event->key.repeat) break;
		input->keys[event->key.scancode] = event->type == SDL_EVENT_KEY_DOWN ?
			KEY_JUST_PRESSED : 
			KEY_JUST_RELEASED;
		break;

	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP:
		input->mbtn[event->button.button] = event->type == SDL_EVENT_MOUSE_BUTTON_DOWN ?
			KEY_JUST_PRESSED : 
			KEY_JUST_RELEASED;

		break;
	case SDL_EVENT_MOUSE_MOTION:
		input->mpos = (float2){event->motion.x, event->motion.y};
		break;
	case SDL_EVENT_MOUSE_WHEEL:
		input->mwheel = (float2){event->wheel.x, event->wheel.y};
		input->mwheeli = (int2){event->wheel.integer_x, event->wheel.integer_y};
		break;

	case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
	case SDL_EVENT_GAMEPAD_BUTTON_UP:
		which = event->gbutton.which;
		pad = SDL_GetGamepadFromID(which);
		if(pad == input->gamepad[0]) {
			input->pad[event->gbutton.button] = event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ?
			KEY_JUST_PRESSED : 
			KEY_JUST_RELEASED;
		}
		break;
	case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		which = event->gaxis.which;
		pad = SDL_GetGamepadFromID(which);
		if(pad == input->gamepad[0]) {
			int v = event->gaxis.axis;
			float f = 0;
			if(v > 0) f = (float)v / INT16_MAX;
			if(v < 0) f = (float)v / INT16_MIN;
			input->axis[event->gaxis.axis] = f;
		}
		break;
	}



	return SDL_APP_CONTINUE;
}

void gameRegister(GameContext* game, GameState* state)
{
	SDL_SetPointerProperty(game->stateDict, state->name, state);
}

void gameSwitch(GameContext* game, const char* name)
{
	game->nextState = SDL_GetPointerProperty(game->stateDict, name, nullptr);
}

void gameOverlay(GameContext* game, const char* name)
{
	game->nextOverlay = SDL_GetPointerProperty(game->stateDict, name, nullptr);
}

void gameStart(GameContext* game, const char* name)
{
	game->state = SDL_GetPointerProperty(game->stateDict, name, nullptr);
	stateStart(game->state, game);
}

#endif
#endif


