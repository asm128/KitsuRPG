/// Copyright 2014-2017 - asm128
#include "nwol_ascii_target.h"

#ifndef ASCIISCREEN_H_20983472903740928374234
#define ASCIISCREEN_H_20983472903740928374234

#define DEFAULT_ASCII_SCREEN_HEIGHT		83
#define DEFAULT_ASCII_SCREEN_WIDTH		((uint32_t)(DEFAULT_ASCII_SCREEN_HEIGHT*2.666666f))

namespace nwol
{
	::nwol::error_t			initASCIIScreen					(uint32_t width = DEFAULT_ASCII_SCREEN_WIDTH, uint32_t height = DEFAULT_ASCII_SCREEN_HEIGHT);
	::nwol::error_t			shutdownASCIIScreen				();
	::nwol::error_t			setASCIIScreenTitle				(const char_t* title);
	::nwol::error_t			getASCIIBackBuffer				(::nwol::SASCIITarget& target);
	::nwol::error_t			getASCIIFrontBuffer				(::nwol::SASCIITarget& target);
	::nwol::error_t			swapASCIIBuffers				();
	::nwol::error_t			clearASCIIBackBuffer			(char_t value, uint16_t colorValue=15);
	::nwol::error_t			presentASCIIFrontBuffer			();
	::nwol::error_t			presentASCIIBackBuffer			();
};

#endif // ASCIISCREEN_H_20983472903740928374234
