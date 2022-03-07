#pragma once
#include "bus.h"
#include "device.h"
#include <SDL.h>
#include <SDL_ttf.h>

enum
{
	VIDEOCONTROLLER_MODE = BUS_VIDEO_START,
	VIDEOCONTROLLER_TICKSDELAY,
	VIDEOCONTROLLER_T1,
	VIDEOCONTROLLER_T2 = VIDEOCONTROLLER_T1 + 2,
	
	VIDEOCONTROLLER_END
};
static_assert(VIDEOCONTROLLER_END <= (BUS_VIDEO_END + 1));

enum VideoMode
{
	VIDEOCONTROLLER_MODE_NO,
	VIDEOCONTROLLER_MODE_1, // 150x104, T1 monochrome bitmap
};

class VideoController : public Device
{
	SDL_Window* window;
	SDL_Renderer* renderer;

	uint8_t modeSet = 0;

public:
	~VideoController(void) override;
	void process(void) override;

	// should be called from same thread as process()
	void setVideoMode(VideoMode vmode);
};