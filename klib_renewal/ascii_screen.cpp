/// Copyright 2014-2017 - asm128
#include "ascii_screen.h"
#include "nwol_ascii_display.h"

static struct SASCIIDisplayBuffered {
	::nwol::SASCIITarget							BackBuffer							= {};
	::nwol::SASCIITarget							FrontBuffer							= {};
	bool											bCreated							= false;

													~SASCIIDisplayBuffered						()																		{
		if( bCreated )	
			::nwol::shutdownASCIIScreen();
	}
}												__g_ASCIIScreen;	// Main instance (Windows applications seem to support only one console per process, as well as many custom smaller systems)

::nwol::error_t									nwol::getASCIIBackBuffer			(::nwol::SASCIITarget& target)											{ target = __g_ASCIIScreen.BackBuffer	; return 0; }
::nwol::error_t									nwol::getASCIIFrontBuffer			(::nwol::SASCIITarget& target)											{ target = __g_ASCIIScreen.FrontBuffer	; return 0; }
::nwol::error_t									nwol::setASCIIScreenTitle			(const char_t* title)													{ return ::nwol::asciiDisplayTitleSet({title, (uint32_t)strlen(title)}); }
::nwol::error_t									nwol::clearASCIIBackBuffer			(char_t value, uint16_t colorValue)										{ return ::nwol::asciiTargetClear(__g_ASCIIScreen.BackBuffer, value, colorValue); }
::nwol::error_t									nwol::presentASCIIFrontBuffer		()																		{ return ::nwol::asciiDisplayPresent(__g_ASCIIScreen.FrontBuffer); }
::nwol::error_t									nwol::presentASCIIBackBuffer		()																		{
	error_if(errored(::nwol::swapASCIIBuffers()), "Why would this ever happen? Is the memory corrupt?");
	return ::nwol::asciiDisplayPresent(__g_ASCIIScreen.FrontBuffer);
}

::nwol::error_t									nwol::swapASCIIBuffers				()																		{
	::nwol::SASCIITarget								bB									= __g_ASCIIScreen.BackBuffer;
	__g_ASCIIScreen.BackBuffer 						= __g_ASCIIScreen.FrontBuffer;
	__g_ASCIIScreen.FrontBuffer						= bB;
	return 0;
}

::nwol::error_t									nwol::shutdownASCIIScreen			()																		{
	if(false == __g_ASCIIScreen.bCreated)
		return 1;
	error_if(errored(::nwol::asciiDisplayDestroy()							 ), "Why would this evern happen?");
	error_if(errored(::nwol::asciiTargetDestroy(__g_ASCIIScreen.BackBuffer	)), "Why would this evern happen?");
	error_if(errored(::nwol::asciiTargetDestroy(__g_ASCIIScreen.FrontBuffer	)), "Why would this evern happen?");
	__g_ASCIIScreen.bCreated						= false;
	return 0;
}

::nwol::error_t									nwol::initASCIIScreen				(uint32_t width, uint32_t height)										{
	if(false == __g_ASCIIScreen.bCreated) {
		nwol_necall(::nwol::asciiTargetCreate(__g_ASCIIScreen.BackBuffer	, width, height), "Out of memory or something weird is going on.");
		nwol_necall(::nwol::asciiTargetCreate(__g_ASCIIScreen.FrontBuffer	, width, height), "Out of memory or something weird is going on.");
		nwol_necall(::nwol::asciiDisplayCreate(width, height), "Out of memory or something weird is going on.");
		__g_ASCIIScreen.bCreated						= true;
	}
	return 0;
}
