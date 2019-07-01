#include "draw.h"

#include "gui.h"
#include "netlib_node.h"
#include "nwol_member_registry.h"
#include "nwol_runtime.h"
#include "application.h"

#ifndef __NWOA_H__983264902__
#define __NWOA_H__983264902__

GDEFINE_ENUM_TYPE	(NETWORK_STATE, uint8_t);
GDEFINE_ENUM_VALUE	(NETWORK_STATE, NONE	, 0);
GDEFINE_ENUM_VALUE	(NETWORK_STATE, ENABLED	, 1);
GDEFINE_ENUM_VALUE	(NETWORK_STATE, RUNNING	, 2);

struct SApplication	: public ::gddm::SFramework {
	NWOM(, SApplication, ::klib, SGame,	Game, ::nwol::GDATA_TYPE_OBJ, "Game instance", "Game information is stored in this object.");
	NWOM_REGISTRY
		(	NWOM_NAME(Game)
		);
	inline						SApplication	(::nwol::SRuntimeValues	* runtimeValues)	: SFramework(runtimeValues)	{ GUI = {{640, 480}, {::klib::SGlobalDisplay::Width, ::klib::SGlobalDisplay::Depth},}; }
};

int32_t						setup			(::SApplication	& instanceApp);
int32_t						update			(::SApplication	& instanceApp, bool exitRequested);
int32_t						render			(::SApplication	& instanceApp);
int32_t						cleanup			(::SApplication	& instanceApp);
//											 
::nwol::error_t				networkEnable	(::SApplication	& instanceApp);
::nwol::error_t				networkDisable	(::SApplication	& instanceApp);
#endif // __NWOA_H__983264902__
