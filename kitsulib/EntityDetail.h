#include "Game.h"

namespace klib
{
	template <typename _TEntity, size_t _Width, size_t _Depth, size_t _SizeDefinitions, size_t _SizeModifiers> 
	void drawEntityDetail(SWeightedDisplay<_Width, _Depth>& display_, int32_t offsetY, int32_t offsetX, const SEntity& entity, const SEntityRecord<_TEntity> (&definitions)[_SizeDefinitions], const SEntityRecord<_TEntity> (&modifiers)[_SizeModifiers], const ::nwol::glabel& entityTypeName)
	{
		std::string					nameAndLevelText = "-- " + std::string(entityTypeName) + ": ";	
		lineToGridColored(display_.Screen, display_.TextAttributes, COLOR_GREEN , offsetY++, offsetX, nwol::SCREEN_LEFT, nameAndLevelText.c_str(), (uint32_t)nameAndLevelText.size());
		nameAndLevelText = getEntityName(entity, definitions, modifiers) + " Lv. " + std::to_string(entity.Level);	
		lineToGridColored(display_.Screen, display_.TextAttributes, COLOR_YELLOW, ++offsetY, offsetX, nwol::SCREEN_LEFT, nameAndLevelText.c_str(), (uint32_t)nameAndLevelText.size());

		const SEntityPoints			entityPoints = getEntityPoints(entity, definitions, modifiers);
		static const ::nwol::glabel formatPoints	= "%-21.21s: %-10.10s";
		static const ::nwol::glabel formatCoins		= "%-21.21s: %-11.11s";
		char formattedGauge[32] = {};
		offsetY+=2;
		sprintf_s(formattedGauge, "%i", entityPoints.LifeMax.Health				);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Max Health"				, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.LifeMax.Shield				);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Max Shield"				, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.LifeMax.Mana				);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Max Mana"				, formattedGauge);
		offsetY+=1;	//																																												
		sprintf_s(formattedGauge, "%i", entityPoints.LifeCurrent.Health			);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Health per turn"			, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.LifeCurrent.Shield			);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Shield per turn"			, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.LifeCurrent.Mana			);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Mana per turn"			, formattedGauge);
		offsetY+=1;	//																																												
		sprintf_s(formattedGauge, "%i", entityPoints.Attack.Hit					);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Hit Chance"				, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.Attack.Damage				);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Damage"					, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.Attack.DirectDamage.Health	);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Direct Damage Health"	, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.Attack.DirectDamage.Shield	);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Direct Damage Shield"	, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.Attack.DirectDamage.Mana	);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Direct Damage Mana"		, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.Attack.Absorption			);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Absorption"				, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.Attack.Range				);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Range"					, formattedGauge);
		offsetY+=1;	//																																												
		sprintf_s(formattedGauge, "%i", entityPoints.Fitness.Attack				);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Attack Speed"			, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.Fitness.Movement			);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Movement Speed"			, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.Fitness.Reflexes			);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Reflexes"				, formattedGauge);
		sprintf_s(formattedGauge, "%i", entityPoints.Fitness.Sight				);	printfToGrid(display_.Screen, offsetY++	, offsetX, nwol::SCREEN_LEFT, formatPoints.c_str()	, "Sight"					, formattedGauge);

		uint16_t color;
		sprintf_s(formattedGauge, "%i", entityPoints.Coins						);	printfToGrid(display_.Screen, ++offsetY	, offsetX, nwol::SCREEN_LEFT, formatCoins.c_str()	, "Coins per turn"			, formattedGauge);
		valueToGrid(display_.TextAttributes, offsetY, offsetX+23, nwol::SCREEN_LEFT, &(color = (entityPoints.Coins >= 0) ? COLOR_ORANGE : COLOR_RED), 1, 11);												
		sprintf_s(formattedGauge, "%i", entityPoints.PriceBuy/2					);	printfToGrid(display_.Screen, ++offsetY	, offsetX, nwol::SCREEN_LEFT, formatCoins.c_str()	, "Sell Price"				, formattedGauge);
		valueToGrid(display_.TextAttributes, offsetY, offsetX+23, nwol::SCREEN_LEFT, &(color = COLOR_ORANGE), 1, 11);																						
		sprintf_s(formattedGauge, "%i", entityPoints.CostMaintenance			);	printfToGrid(display_.Screen, ++offsetY	, offsetX, nwol::SCREEN_LEFT, formatCoins.c_str()	, "Maintenance Cost"		, formattedGauge);
		valueToGrid(display_.TextAttributes, offsetY, offsetX+23, nwol::SCREEN_LEFT, &(color = COLOR_ORANGE), 1, 11);
	}
}