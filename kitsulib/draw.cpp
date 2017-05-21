#include "draw.h"
#include "credits.h"

using namespace klib;

void drawIntro(SGame& instanceGame);

// Currently what this function is lacking is the ability to receive negative offsets.
template<typename _TCell, size_t _Width, size_t _Depth>
void blitGrid(::nwol::SGrid<_TCell, _Width, _Depth>& source, int32_t offsetY, uint32_t offsetX, _TCell* target, size_t targetWidth, size_t targetHeight, int32_t rowPitch=-1) {
	size_t								actualWidth					= std::min(_Width, std::max((size_t)0, targetWidth-offsetX));
	if(rowPitch < 0)
		rowPitch = (int32_t)targetWidth;

	for(int32_t z = 0, maxZ = (int32_t)std::min(_Depth, targetHeight-offsetY); z < maxZ; ++z)  {
		if( (offsetY+z) < 0 )
			continue;

		memcpy(&target[(offsetY+z)*rowPitch+offsetX], &source.Cells[z][0], actualWidth);
	}
};

template<typename _TCell, size_t _Width, size_t _Depth>
void drawDisplay(::nwol::SGrid<_TCell, _Width, _Depth>& source, uint32_t offsetY, uint32_t offsetX ) {
	blitGrid(source, offsetY, offsetX, nwol::getASCIIBackBuffer(), nwol::getASCIIBackBufferWidth(), nwol::getASCIIBackBufferHeight());
};

void drawStateBackground( SGame& instanceGame ) {
	switch(instanceGame.State.State) {
	case	GAME_STATE_MENU_MAIN		:	drawIntro(instanceGame);																	; break;
	case	GAME_STATE_CREDITS			:	//drawSnowBackground(instanceGame.GlobalDisplay, instanceGame.FrameTimer.LastTimeSeconds);	; break;
	case	GAME_STATE_MEMORIAL			:	drawSnowBackground(instanceGame.GlobalDisplay, instanceGame.FrameTimer.LastTimeSeconds);	; break;
	case	GAME_STATE_MENU_SQUAD_SETUP	:	//drawRainBackground(instanceGame.GlobalDisplay, instanceGame.FrameTimer.LastTimeSeconds);	; break;
	case	GAME_STATE_MENU_EQUIPMENT	:	drawRainBackground(instanceGame.GlobalDisplay, instanceGame.FrameTimer.LastTimeSeconds);	; break;
	}
}

template <typename... _Args>
int32_t lineToScreen( int32_t offsetLine, int32_t offsetColumn, ::nwol::ALIGN_SCREEN align, const char_t* format, _Args... args ) {
	return printfToRect(::nwol::getASCIIBackBuffer(), (size_t)::nwol::getASCIIBackBufferWidth(), (size_t)::nwol::getASCIIBackBufferHeight(), offsetLine, offsetColumn, align, format, args...);
}

SGameState	drawMemorial				(SGame& instanceGame, const SGameState& returnState)	{ return returnState;}
void		klib::drawAndPresentGame	( SGame& instanceGame )									{
	static ::nwol::STimer				frameMeasure;
	static ::nwol::STimer				animTimer;
	static ::nwol::SAccumulator<double>	animTimerAccum				= {0.0, 1.00};
	// increase time
	animTimer.Frame();
	// Check for help display
	if(animTimerAccum.Accumulate(animTimer.LastTimeSeconds))
		animTimerAccum.Value = 0;

	drawStateBackground(instanceGame);
	showMenu(instanceGame);

	drawDisplay(instanceGame.GlobalDisplay.Screen, 0, 0);
	::nwol::SASCIITarget				target;
	::nwol::getASCIIBackBuffer			(target);

	uint32_t							bbWidth						= target.Width		();
	uint32_t							bbHeight					= target.Height		();
	char_t								* bbChar					= target.Text		.begin();
	uint16_t							* bbColor					= target.Attributes	.begin();

	//drawDisplay(instanceGame.MenuDisplay, 0);
	switch(instanceGame.State.State) { 
	//case GAME_STATE_MENU_ACTION:
	case GAME_STATE_MENU_LAN_MISSION:
	case GAME_STATE_TACTICAL_CONTROL:
	case GAME_STATE_START_MISSION:
		drawDisplay(instanceGame.TacticalDisplay.Screen, TACTICAL_DISPLAY_POSY, (instanceGame.GlobalDisplay.Screen.Width>>1)-(instanceGame.TacticalDisplay.Width>>1));
		break;
	case GAME_STATE_CREDITS:
		drawCredits(bbChar, bbWidth, bbHeight, instanceGame.FrameTimer.LastTimeSeconds, namesSpecialThanks, instanceGame.State);
		break;
	case GAME_STATE_MEMORIAL:
		drawMemorial(bbChar, bbWidth, bbHeight, 
			&instanceGame.GlobalDisplay.TextAttributes.Cells[0][0],
			instanceGame.FrameTimer.LastTimeSeconds, instanceGame.Players[0].Memorial, instanceGame.State);
		break;
	case GAME_STATE_WELCOME_COMMANDER: 
	case GAME_STATE_MENU_SQUAD_SETUP: 
	case GAME_STATE_MENU_EQUIPMENT: 
		break;
	default:
		drawDisplay(instanceGame.PostEffectDisplay.Screen, TACTICAL_DISPLAY_POSY, (instanceGame.GlobalDisplay.Screen.Width>>1)-(instanceGame.PostEffectDisplay.Width>>1));
	}

	memcpy(bbColor, &instanceGame.GlobalDisplay.TextAttributes.Cells[0][0], instanceGame.GlobalDisplay.TextAttributes.Width*instanceGame.GlobalDisplay.TextAttributes.Depth*sizeof(uint16_t));
	uint32_t							y							= 0;

	switch(instanceGame.State.State) { 
	//case GAME_STATE_MENU_ACTION:
	case GAME_STATE_MENU_LAN_MISSION:
	case GAME_STATE_TACTICAL_CONTROL:
	case GAME_STATE_START_MISSION:
		for(y=0; y<instanceGame.PostEffectDisplay.TextAttributes.Depth; ++y)
			memcpy(&bbColor[(TACTICAL_DISPLAY_POSY+y)*bbWidth+((bbWidth>>1)-(instanceGame.TacticalDisplay.TextAttributes.Width>>1))], &instanceGame.TacticalDisplay.TextAttributes.Cells[y][0], instanceGame.TacticalDisplay.TextAttributes.Width*sizeof(uint16_t));
		break;
	case GAME_STATE_CREDITS:
	case GAME_STATE_MEMORIAL:
	case GAME_STATE_WELCOME_COMMANDER: 
	case GAME_STATE_MENU_SQUAD_SETUP: 
	case GAME_STATE_MENU_EQUIPMENT: 
		break;
	default:
		for(y=0; y<instanceGame.PostEffectDisplay.TextAttributes.Depth; ++y)
			memcpy(&bbColor[(TACTICAL_DISPLAY_POSY+y)*bbWidth+((bbWidth>>1)-(instanceGame.PostEffectDisplay.TextAttributes.Width>>1))], &instanceGame.PostEffectDisplay.TextAttributes.Cells[y][0], instanceGame.PostEffectDisplay.TextAttributes.Width*sizeof(uint16_t));
	}

	// Frame timer
	frameMeasure.Frame();
	instanceGame.FrameTimer.Frame();
	if(instanceGame.FrameTimer.LastTimeSeconds >= 0.150001)
		instanceGame.FrameTimer.LastTimeSeconds = 0.150001;

	int32_t								actualOffsetX;
	// Print log
	const int32_t						MAX_LOG_LINES				= iif(instanceGame.State.State == GAME_STATE_WELCOME_COMMANDER || nwol::bit_true(instanceGame.Flags, GAME_FLAGS_TACTICAL)) 30 : 4;
	int32_t								logSize						= (int32_t)instanceGame.UserLog.size();
	for(uint32_t iLogLine=0, logLineCount = std::min(MAX_LOG_LINES, logSize); iLogLine< logLineCount; ++iLogLine)
		actualOffsetX = nwol::lineToRectColored(target, instanceGame.UserLog[logSize-1-iLogLine].Color, bbHeight-9-iLogLine, 1, nwol::SCREEN_LEFT, instanceGame.UserLog[logSize-1-iLogLine].Message.c_str());	

	// Print some debugging information 
	if(nwol::bit_true(instanceGame.Flags,GAME_FLAGS_HELPON)) {
		int32_t								offsetYHelp					= TACTICAL_DISPLAY_POSY+instanceGame.TacticalDisplay.Depth;
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_YELLOW	, offsetYHelp+18-5, 2, ::nwol::SCREEN_RIGHT, "Help!!! Press F1 to hide.");
		//										::																																		 
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_ORANGE	, offsetYHelp+20-5, bbWidth/4*3, nwol::SCREEN_LEFT, "Keys:");
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_DARKGREY	, offsetYHelp+21-5, bbWidth/4*3, nwol::SCREEN_LEFT, "Up    : Menu cursor up.");
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_DARKGREY	, offsetYHelp+22-5, bbWidth/4*3, nwol::SCREEN_LEFT, "Down  : Menu cursor down.");
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_DARKGREY	, offsetYHelp+23-5, bbWidth/4*3, nwol::SCREEN_LEFT, "Left  : Previous menu page.");
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_DARKGREY	, offsetYHelp+24-5, bbWidth/4*3, nwol::SCREEN_LEFT, "Right : Next menu page.");
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_DARKGREY	, offsetYHelp+25-5, bbWidth/4*3, nwol::SCREEN_LEFT, "Enter : Select menu option.");
		//										::																																		 
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_ORANGE	, offsetYHelp+27-5, bbWidth/4*3, nwol::SCREEN_LEFT, "Mouse:");
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_DARKGREY	, offsetYHelp+28-5, bbWidth/4*3, nwol::SCREEN_LEFT, "Left Click  : Select menu option");
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_DARKGREY	, offsetYHelp+29-5, bbWidth/4*3, nwol::SCREEN_LEFT, "Left Click  : Select move destination");
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_DARKGREY	, offsetYHelp+30-5, bbWidth/4*3, nwol::SCREEN_LEFT, "Left Click  : Select attack target");
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_DARKGREY	, offsetYHelp+31-5, bbWidth/4*3, nwol::SCREEN_LEFT, "Right Click : Return to the previous screen.");
		//										::																																		 
		actualOffsetX						= ::nwol::lineToRectColored(target, (animTimerAccum.Value > 0.5)? COLOR_CYAN	:	COLOR_DARKBLUE		, offsetYHelp+33-5, bbWidth/4*3, nwol::SCREEN_LEFT, "\x0C, \x0B: Blue characters are your own squad.");
		actualOffsetX						= ::nwol::lineToRectColored(target, (animTimerAccum.Value > 0.5)? COLOR_MAGENTA	:	COLOR_DARKMAGENTA	, offsetYHelp+34-5, bbWidth/4*3, nwol::SCREEN_LEFT, "\x0C, \x0B: Magenta characters are your ally squads.");
		actualOffsetX						= ::nwol::lineToRectColored(target, (animTimerAccum.Value > 0.5)? COLOR_RED		:	COLOR_DARKRED		, offsetYHelp+35-5, bbWidth/4*3, nwol::SCREEN_LEFT, "\x0C, \x0B: Red characters are your enemies.");
		actualOffsetX						= ::nwol::lineToRectColored(target, (animTimerAccum.Value > 0.5)? COLOR_ORANGE	:	COLOR_DARKYELLOW	, offsetYHelp+36-5, bbWidth/4*3, nwol::SCREEN_LEFT, "\x04: Money is picked up when you walk over it.");
		actualOffsetX						= ::nwol::lineToRectColored(target, (animTimerAccum.Value > 0.5)? COLOR_WHITE	:	COLOR_DARKGREY		, offsetYHelp+37-5, bbWidth/4*3, nwol::SCREEN_LEFT, "\xB9: Obstacles turn gray when destroyed.");
		//										::																																		 
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_YELLOW		, offsetYHelp+39-5, bbWidth/4*3, nwol::SCREEN_LEFT, "Melee attacks and grenades currently require ");
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_YELLOW		, offsetYHelp+40-5, bbWidth/4*3, nwol::SCREEN_LEFT, "a valid target selected in order to work!");
			
	}
	else
		actualOffsetX						= ::nwol::lineToRectColored(target, COLOR_YELLOW, TACTICAL_DISPLAY_POSY+instanceGame.TacticalDisplay.Depth+18-5, 2, ::nwol::SCREEN_RIGHT, "Press F1 to display help.");	
	
	actualOffsetX						= ::nwol::printfToRectColored(target, COLOR_DARKGREEN	, bbHeight-4, 1, nwol::SCREEN_LEFT, "Mouse: %i, %i."			, instanceGame.FrameInput.MouseX, instanceGame.FrameInput.MouseY);
	actualOffsetX						= ::nwol::printfToRectColored(target, COLOR_GREEN		, bbHeight-3, 1, nwol::SCREEN_LEFT, "Frame time: %.5f seconds."	, instanceGame.FrameTimer.LastTimeSeconds	);
	actualOffsetX						= ::nwol::printfToRectColored(target, COLOR_CYAN		, bbHeight-2, 1, nwol::SCREEN_LEFT, "Frames last second: %f."	, instanceGame.FrameTimer.FramesLastSecond	);
	time_t									curTimeWithUnreliableSize	= 0; 
	{ 
		::nwol::CLock							thelock						(instanceGame.ServerTimeMutex);
		curTimeWithUnreliableSize			= instanceGame.ServerTime;
	}
	char									send_buffer[64]				= {};
	ctime_s(send_buffer, sizeof(send_buffer), &curTimeWithUnreliableSize);

	::std::string							serverTime					= ::std::string("Server time: ") + send_buffer;
	serverTime							= serverTime.substr(0, serverTime .size()-2);
	actualOffsetX						= nwol::printfToRectColored(bbChar, bbWidth, bbHeight, bbColor, COLOR_CYAN		, bbHeight-2, 1, ::nwol::SCREEN_RIGHT, "%s."	, serverTime.c_str());	
	actualOffsetX						= nwol::printfToRectColored(bbChar, bbWidth, bbHeight, bbColor, COLOR_DARKGREY	, bbHeight-1, 1, ::nwol::SCREEN_CENTER, "%s."	, instanceGame.StateMessage.c_str()); 

	// Print user error messages and draw cursor.
	if(instanceGame.State.State != GAME_STATE_CREDITS) {
		int32_t								finalMissionCost			= missionCost(instanceGame.Players[PLAYER_INDEX_USER], instanceGame.Players[PLAYER_INDEX_USER].Squad, instanceGame.Players[PLAYER_INDEX_USER].Squad.Size);
		int32_t								playerFunds					= instanceGame.Players[PLAYER_INDEX_USER].Money;
		::nwol::printfToRectColored(target, (finalMissionCost > playerFunds) ? COLOR_ORANGE : COLOR_CYAN	, bbHeight-5, 1, ::nwol::SCREEN_RIGHT, "Squad size: %i."	, instanceGame.Players[PLAYER_INDEX_USER].Squad.Size);
		::nwol::printfToRectColored(target, (finalMissionCost > playerFunds) ? COLOR_RED : COLOR_ORANGE		, bbHeight-4, 1, ::nwol::SCREEN_RIGHT, "Mission cost: %i"	, finalMissionCost);
		::nwol::printfToRectColored(target, (playerFunds < 0) ? COLOR_RED : COLOR_YELLOW					, bbHeight-3, 1, ::nwol::SCREEN_RIGHT, "Funds: %i"			, playerFunds);

		::nwol::printfToRectColored(target, COLOR_YELLOW	, bbHeight-5, 0, ::nwol::SCREEN_CENTER, "%s", instanceGame.UserMessage.c_str());
		::nwol::printfToRectColored(target, COLOR_CYAN		, bbHeight-4, 0, ::nwol::SCREEN_CENTER, "%s", instanceGame.UserSuccess.c_str());
		::nwol::printfToRectColored(target, COLOR_RED		, bbHeight-3, 0, ::nwol::SCREEN_CENTER, "%s", instanceGame.UserMiss.c_str());
		::nwol::printfToRectColored(target, COLOR_RED		, bbHeight-2, 0, ::nwol::SCREEN_CENTER, "%s", instanceGame.UserError.c_str());

		// Draw cursor
		//int32_t mouseX = instanceGame.FrameInput.MouseX, mouseY = instanceGame.FrameInput.MouseY;
		//bbColor[mouseY*bbWidth+mouseX] = 
		//	( ((bbColor[mouseY*bbWidth+mouseX] & 0xF0) >> 4)
		//	| ((bbColor[mouseY*bbWidth+mouseX] & 0x0F) << 4)
		//	);
	}

	// increase coins
	static ::nwol::STimer				profitTimer;
	static ::nwol::SAccumulator<double>	profitTimerAccum			= {0.0, 3.0};

	profitTimer.Frame();
	if( profitTimerAccum.Accumulate(profitTimer.LastTimeSeconds) ) {
		instanceGame.Players[PLAYER_INDEX_USER].Money++;
		profitTimerAccum.Value				= 0;
	}

	static ::nwol::STimer				keyTimer;
	static ::nwol::SAccumulator<double>	keyTimerAccum				= {0.0, .50};
	// increase time
	keyTimer.Frame();
	// Check for help display
	if(keyTimerAccum.Accumulate(keyTimer.LastTimeSeconds) && instanceGame.FrameInput.Keys[VK_F1]) {
		if(::nwol::bit_true(instanceGame.Flags, GAME_FLAGS_HELPON))
			::nwol::bit_clear(instanceGame.Flags, GAME_FLAGS_HELPON);
		else
			::nwol::bit_set(instanceGame.Flags, GAME_FLAGS_HELPON);
		keyTimerAccum.Value = 0;
	}

}; 	// 

void drawIntro( SGame& instanceGame ) {
	drawFireBackground(instanceGame.PostEffectDisplay, instanceGame.FrameTimer.LastTimeSeconds);

	int32_t								displayWidth				= (int32_t)instanceGame.PostEffectDisplay.Width;
	int32_t								displayDepth				= (int32_t)instanceGame.PostEffectDisplay.Depth;

	static ::std::string				words			[]			= {"Vulgar", "Display", "of", "Power"};
	for( uint32_t i=0; i < ::nwol::size(words); ++i) {
		uint32_t							offsetY						= (uint32_t)((displayDepth >> 1)-(::nwol::size(words) >> 1) + i * 2);
		uint32_t							offsetX						= printfToGridColored(instanceGame.PostEffectDisplay.Screen, instanceGame.PostEffectDisplay.TextAttributes, COLOR_ORANGE, offsetY, 0, ::nwol::SCREEN_CENTER, "%s", words[i].c_str());
	}
};
