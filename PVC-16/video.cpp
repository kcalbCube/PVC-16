#include "video.h"
#include "memory.h"
#include "interrupt.h"
#include "utility.h"

void VideoController::process(void)
{
	for(Operation& op : dc->operations)
		switch (op)
		{
		case Operation::VIDEOCONTROLLER_SET_MODE:
			setVideoMode(static_cast<VideoMode>(busRead(VIDEOCONTROLLER_MODE)));
			break;
		}
	if (const auto ticksDelay = busRead(VIDEOCONTROLLER_TICKSDELAY); tick <= ticksDelay)
		return;
	tick = 0;

	const auto mode = busRead(VIDEOCONTROLLER_MODE);
	if (!mode || mode != modeSet)
		return;

	switch (mode)
	{
	case VIDEOCONTROLLER_MODE_1:
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		size_t i = 0;
		auto t1 = busRead16(VIDEOCONTROLLER_T1);

		SDL_Point points[144 * 100]{};
		for (size_t y = 0; y < 100; ++y)
			for (size_t x = 0; x < 144 / 8; ++x)
			{
				auto byte = mc.read8(t1 + y * (144 / 8) + x);
				for (size_t j = 0; j < 8; ++j)
					if (byte & (1 << (7-j)))
						points[i++] = SDL_Point(x * 8 + j, y);
			}
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawPoints(renderer, points, i);

		delayedInterrupt(interrupts::VBI);
	}
	break;
	default:
		UNREACHABLE;
	}

	SDL_RenderPresent(renderer);
	SDL_PumpEvents();
	
}

void VideoController::setVideoMode(VideoMode vmode)
{
	static bool sdlInitialized = false;

	if (!sdlInitialized)
	{
		SDL_Init(SDL_INIT_EVERYTHING);
		TTF_Init();
		sdlInitialized = true;
	}
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);

	switch (vmode)
	{
	case VIDEOCONTROLLER_MODE_1:
	{
		window = SDL_CreateWindow("PVC-16", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 576, 400, SDL_WINDOW_SHOWN);
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		SDL_RenderSetScale(renderer, 4.f, 4.f);
	}
	break;
	}

	modeSet = vmode;
}

VideoController::~VideoController(void)
{
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);

	TTF_Quit();
	SDL_Quit();
}