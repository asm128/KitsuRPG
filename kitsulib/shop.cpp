//#define NOMINMAX

#include "Game.h"
#include "draw.h"

#include "Accessory.h"
#include "Armor.h"
#include "Weapon.h"
#include "Profession.h"
#include "Vehicle.h"
#include "Facility.h"
#include "StageProp.h"
#include "Item.h"
#include "Enemy.h"

using namespace klib;

struct SBuyable
{
	int16_t			Definition; 
	int16_t			Grade; 
	int32_t			Price; 
	int32_t			MaintenanceCost; 
	std::string		Name;
} selectedChoice;


static klib::SMenuItem<SBuyable>	menuItemsAccessory	[nwol::size(definitionsAccessory	)] = {};
static klib::SMenuItem<SBuyable>	menuItemsStageProp	[nwol::size(definitionsStageProp	)] = {};
static klib::SMenuItem<SBuyable>	menuItemsFacility	[nwol::size(definitionsFacility		)] = {};
static klib::SMenuItem<SBuyable>	menuItemsVehicle	[nwol::size(definitionsVehicle		)] = {};
static klib::SMenuItem<SBuyable>	menuItemsProfession	[nwol::size(definitionsProfession	)] = {};
static klib::SMenuItem<SBuyable>	menuItemsWeapon		[nwol::size(definitionsWeapon		)] = {};
static klib::SMenuItem<SBuyable>	menuItemsArmor		[nwol::size(definitionsArmor		)] = {};
static klib::SMenuItem<SBuyable>	menuItemsItem		[nwol::size(itemDescriptions		)] = {};
static klib::SMenuItem<SBuyable>	menuItemsAgent		[nwol::size(enemyDefinitions		)] = {};

#define SHOP_EXIT_VALUE 0x7FFF
static klib::SMenu<SBuyable>		menuAccessory		({SHOP_EXIT_VALUE},	"Accessory"		" a la carte"	, 48);
static klib::SMenu<SBuyable>		menuStageProp		({SHOP_EXIT_VALUE},	"Stage Prop"	" a la carte"	, 48);
static klib::SMenu<SBuyable>		menuFacility		({SHOP_EXIT_VALUE},	"Facility"		" a la carte"	, 48);
static klib::SMenu<SBuyable>		menuVehicle			({SHOP_EXIT_VALUE},	"Vehicle"		" a la carte"	, 48);
static klib::SMenu<SBuyable>		menuProfession		({SHOP_EXIT_VALUE},	"Job License"	" a la carte"	, 48);
static klib::SMenu<SBuyable>		menuWeapon			({SHOP_EXIT_VALUE},	"Weapon"		" a la carte"	, 48);
static klib::SMenu<SBuyable>		menuArmor			({SHOP_EXIT_VALUE},	"Armor"			" a la carte"	, 48);
static klib::SMenu<SBuyable>		menuItem			({SHOP_EXIT_VALUE},	"Item"			" a la carte"	, 48);
static klib::SMenu<SBuyable>		menuAgent			({SHOP_EXIT_VALUE},	"Agent"			" a la carte"	, 48);

static ::nwol::glabel				namesAccessory		[nwol::size(definitionsAccessory	)] = {};
static ::nwol::glabel				namesStageProp		[nwol::size(definitionsStageProp	)] = {};
static ::nwol::glabel				namesFacility		[nwol::size(definitionsFacility		)] = {};
static ::nwol::glabel				namesVehicle		[nwol::size(definitionsVehicle		)] = {};
static ::nwol::glabel				namesProfession		[nwol::size(definitionsProfession	)] = {};
static ::nwol::glabel				namesWeapon			[nwol::size(definitionsWeapon		)] = {};
static ::nwol::glabel				namesArmor			[nwol::size(definitionsArmor		)] = {};
static ::nwol::glabel				namesItem			[nwol::size(itemDescriptions		)] = {};
static ::nwol::glabel				namesAgent			[nwol::size(enemyDefinitions		)] = {};


static int32_t initBuyMenus()
{
	char preformatted[256] = {};
	for(size_t i=0, itemCount=nwol::size(	definitionsAccessory	); i<itemCount; ++i){	sprintf_s(preformatted, "%-28.28s $%8.8s",	definitionsAccessory	[i].Name.c_str(), std::to_string(	definitionsAccessory	[i].Points.PriceBuy	).c_str()); 	menuItemsAccessory	[i] = { { (int16_t)i, 1, definitionsAccessory	[i].Points.PriceBuy	, definitionsAccessory	[i].Points.CostMaintenance	, definitionsAccessory	[i].Name},	""	};	namesAccessory		[i] = preformatted; };	
	for(size_t i=0, itemCount=nwol::size(	definitionsStageProp	); i<itemCount; ++i){	sprintf_s(preformatted, "%-28.28s $%8.8s",	definitionsStageProp	[i].Name.c_str(), std::to_string(	definitionsStageProp	[i].Points.PriceBuy	).c_str()); 	menuItemsStageProp	[i] = { { (int16_t)i, 1, definitionsStageProp	[i].Points.PriceBuy	, definitionsStageProp	[i].Points.CostMaintenance	, definitionsStageProp	[i].Name},	""	};	namesStageProp		[i] = preformatted; };	
	for(size_t i=0, itemCount=nwol::size(	definitionsFacility		); i<itemCount; ++i){	sprintf_s(preformatted, "%-28.28s $%8.8s",	definitionsFacility		[i].Name.c_str(), std::to_string(	definitionsFacility		[i].Points.PriceBuy	).c_str()); 	menuItemsFacility	[i] = { { (int16_t)i, 1, definitionsFacility	[i].Points.PriceBuy	, definitionsFacility	[i].Points.CostMaintenance	, definitionsFacility	[i].Name},	""	};	namesFacility		[i] = preformatted; };	
	for(size_t i=0, itemCount=nwol::size(	definitionsVehicle		); i<itemCount; ++i){	sprintf_s(preformatted, "%-28.28s $%8.8s",	definitionsVehicle		[i].Name.c_str(), std::to_string(	definitionsVehicle		[i].Points.PriceBuy	).c_str()); 	menuItemsVehicle	[i] = { { (int16_t)i, 1, definitionsVehicle		[i].Points.PriceBuy	, definitionsVehicle	[i].Points.CostMaintenance	, definitionsVehicle	[i].Name},	""	};	namesVehicle		[i] = preformatted; };	
	for(size_t i=0, itemCount=nwol::size(	definitionsProfession	); i<itemCount; ++i){	sprintf_s(preformatted, "%-28.28s $%8.8s",	definitionsProfession	[i].Name.c_str(), std::to_string(	definitionsProfession	[i].Points.PriceBuy	).c_str()); 	menuItemsProfession	[i] = { { (int16_t)i, 1, definitionsProfession	[i].Points.PriceBuy	, definitionsProfession	[i].Points.CostMaintenance	, definitionsProfession	[i].Name},	""	};	namesProfession		[i] = preformatted; };	
	for(size_t i=0, itemCount=nwol::size(	definitionsWeapon		); i<itemCount; ++i){	sprintf_s(preformatted, "%-28.28s $%8.8s",	definitionsWeapon		[i].Name.c_str(), std::to_string(	definitionsWeapon		[i].Points.PriceBuy	).c_str()); 	menuItemsWeapon		[i] = { { (int16_t)i, 1, definitionsWeapon		[i].Points.PriceBuy	, definitionsWeapon		[i].Points.CostMaintenance	, definitionsWeapon		[i].Name},	""	};	namesWeapon			[i] = preformatted; };	
	for(size_t i=0, itemCount=nwol::size(	definitionsArmor		); i<itemCount; ++i){	sprintf_s(preformatted, "%-28.28s $%8.8s",	definitionsArmor		[i].Name.c_str(), std::to_string(	definitionsArmor		[i].Points.PriceBuy	).c_str()); 	menuItemsArmor		[i] = { { (int16_t)i, 1, definitionsArmor		[i].Points.PriceBuy	, definitionsArmor		[i].Points.CostMaintenance	, definitionsArmor		[i].Name},	""	};	namesArmor			[i] = preformatted; };	
	for(size_t i=0, itemCount=nwol::size(	itemDescriptions		); i<itemCount; ++i){	sprintf_s(preformatted, "%-28.28s $%8.8s",	itemDescriptions		[i].Name.c_str(), std::to_string(	itemDescriptions		[i].Price			).c_str()); 	menuItemsItem		[i] = { { (int16_t)i, 1, itemDescriptions		[i].Price			, 0													, itemDescriptions		[i].Name},	""	};	namesItem			[i] = preformatted; };	
	for(size_t i=0, itemCount=nwol::size(	enemyDefinitions		); i<itemCount; ++i){	sprintf_s(preformatted, "%-28.28s $%8.8s",	enemyDefinitions		[i].Name.c_str(), std::to_string(	enemyDefinitions		[i].Points.PriceBuy	).c_str()); 	menuItemsAgent		[i] = { { (int16_t)i, 1, enemyDefinitions		[i].Points.PriceBuy	, enemyDefinitions		[i].Points.CostMaintenance	, enemyDefinitions		[i].Name},	""	};	namesAgent			[i] = preformatted; };	

	return 0;
}

static int32_t reinitBuyMenus(SGame& instanceGame)
{
	char preformatted[256] = {};

	SCharacterInventory& playerInventory = instanceGame.Players[PLAYER_INDEX_USER].Goods.Inventory;

	for(size_t i=0, itemCount=nwol::size(	definitionsAccessory	); i<itemCount; ++i){	int32_t indexElement	=	playerInventory.Accessory	.FindElement({	menuItemsAccessory	[i].ReturnValue.Definition, 0, menuItemsAccessory	[i].ReturnValue.Grade, -1}); sprintf_s(preformatted, "x%3.3s: %s", std::to_string((indexElement != -1) ?	playerInventory.Accessory	[indexElement].Count : 0).c_str(), namesAccessory		[i].c_str() );	menuItemsAccessory	[i].Text = preformatted; }
	for(size_t i=0, itemCount=nwol::size(	definitionsStageProp	); i<itemCount; ++i){	int32_t indexElement	=	playerInventory.StageProp	.FindElement({	menuItemsStageProp	[i].ReturnValue.Definition, 0, menuItemsStageProp	[i].ReturnValue.Grade, -1}); sprintf_s(preformatted, "x%3.3s: %s", std::to_string((indexElement != -1) ?	playerInventory.StageProp	[indexElement].Count : 0).c_str(), namesStageProp		[i].c_str() );	menuItemsStageProp	[i].Text = preformatted; }
	for(size_t i=0, itemCount=nwol::size(	definitionsFacility		); i<itemCount; ++i){	int32_t indexElement	=	playerInventory.Facility	.FindElement({	menuItemsFacility	[i].ReturnValue.Definition, 0, menuItemsFacility	[i].ReturnValue.Grade, -1}); sprintf_s(preformatted, "x%3.3s: %s", std::to_string((indexElement != -1) ?	playerInventory.Facility	[indexElement].Count : 0).c_str(), namesFacility		[i].c_str() );	menuItemsFacility	[i].Text = preformatted; }
	for(size_t i=0, itemCount=nwol::size(	definitionsVehicle		); i<itemCount; ++i){	int32_t indexElement	=	playerInventory.Vehicle		.FindElement({	menuItemsVehicle	[i].ReturnValue.Definition, 0, menuItemsVehicle		[i].ReturnValue.Grade, -1}); sprintf_s(preformatted, "x%3.3s: %s", std::to_string((indexElement != -1) ?	playerInventory.Vehicle		[indexElement].Count : 0).c_str(), namesVehicle			[i].c_str() );	menuItemsVehicle	[i].Text = preformatted; }
	for(size_t i=0, itemCount=nwol::size(	definitionsProfession	); i<itemCount; ++i){	int32_t indexElement	=	playerInventory.Profession	.FindElement({	menuItemsProfession	[i].ReturnValue.Definition, 0, menuItemsProfession	[i].ReturnValue.Grade, -1}); sprintf_s(preformatted, "x%3.3s: %s", std::to_string((indexElement != -1) ?	playerInventory.Profession	[indexElement].Count : 0).c_str(), namesProfession		[i].c_str() );	menuItemsProfession	[i].Text = preformatted; }
	for(size_t i=0, itemCount=nwol::size(	definitionsWeapon		); i<itemCount; ++i){	int32_t indexElement	=	playerInventory.Weapon		.FindElement({	menuItemsWeapon		[i].ReturnValue.Definition, 0, menuItemsWeapon		[i].ReturnValue.Grade, -1}); sprintf_s(preformatted, "x%3.3s: %s", std::to_string((indexElement != -1) ?	playerInventory.Weapon		[indexElement].Count : 0).c_str(), namesWeapon			[i].c_str() );	menuItemsWeapon		[i].Text = preformatted; }
	for(size_t i=0, itemCount=nwol::size(	definitionsArmor		); i<itemCount; ++i){	int32_t indexElement	=	playerInventory.Armor		.FindElement({	menuItemsArmor		[i].ReturnValue.Definition, 0, menuItemsArmor		[i].ReturnValue.Grade, -1}); sprintf_s(preformatted, "x%3.3s: %s", std::to_string((indexElement != -1) ?	playerInventory.Armor		[indexElement].Count : 0).c_str(), namesArmor			[i].c_str() );	menuItemsArmor		[i].Text = preformatted; }
	for(size_t i=0, itemCount=nwol::size(	itemDescriptions		); i<itemCount; ++i){	int32_t indexElement	=	playerInventory.Items		.FindElement({	menuItemsItem		[i].ReturnValue.Definition, 0, menuItemsItem		[i].ReturnValue.Grade, -1}); sprintf_s(preformatted, "x%3.3s: %s", std::to_string((indexElement != -1) ?	playerInventory.Items		[indexElement].Count : 0).c_str(), namesItem			[i].c_str() );	menuItemsItem		[i].Text = preformatted; }
	for(size_t i=0, itemCount=nwol::size(	enemyDefinitions		); i<itemCount; ++i){	menuItemsAgent[i].Text	=	namesAgent[i]; }

	return 0;
}



SGameState drawBuyMenu(SGame& instanceGame, const SGameState& returnState)
{
#define MAX_BUY_ITEMS 64

	static const int32_t initedMenus = initBuyMenus();
	reinitBuyMenus(instanceGame);

	static klib::SMenuItem<SBuyable> menuItems[MAX_BUY_ITEMS+1] = {};

	std::string menuTitle;
	switch(instanceGame.State.Substate)
	{
	case GAME_SUBSTATE_ACCESSORY	:	selectedChoice = drawMenu(instanceGame.GlobalDisplay.Screen, &instanceGame.GlobalDisplay.TextAttributes.Cells[0][0], menuAccessory	,	menuItemsAccessory		,  instanceGame.FrameInput, {-1});	break;
	case GAME_SUBSTATE_STAGEPROP	:	selectedChoice = drawMenu(instanceGame.GlobalDisplay.Screen, &instanceGame.GlobalDisplay.TextAttributes.Cells[0][0], menuStageProp	,	menuItemsStageProp		,  instanceGame.FrameInput, {-1});	break;
	case GAME_SUBSTATE_FACILITY		:	selectedChoice = drawMenu(instanceGame.GlobalDisplay.Screen, &instanceGame.GlobalDisplay.TextAttributes.Cells[0][0], menuFacility	,	menuItemsFacility		,  instanceGame.FrameInput, {-1});	break;
	case GAME_SUBSTATE_VEHICLE		:	selectedChoice = drawMenu(instanceGame.GlobalDisplay.Screen, &instanceGame.GlobalDisplay.TextAttributes.Cells[0][0], menuVehicle	,	menuItemsVehicle		,  instanceGame.FrameInput, {-1});	break;
	case GAME_SUBSTATE_PROFESSION	:	selectedChoice = drawMenu(instanceGame.GlobalDisplay.Screen, &instanceGame.GlobalDisplay.TextAttributes.Cells[0][0], menuProfession	,	menuItemsProfession		,  instanceGame.FrameInput, {-1});	break;
	case GAME_SUBSTATE_WEAPON		:	selectedChoice = drawMenu(instanceGame.GlobalDisplay.Screen, &instanceGame.GlobalDisplay.TextAttributes.Cells[0][0], menuWeapon		,	menuItemsWeapon			,  instanceGame.FrameInput, {-1});	break;
	case GAME_SUBSTATE_ARMOR		:	selectedChoice = drawMenu(instanceGame.GlobalDisplay.Screen, &instanceGame.GlobalDisplay.TextAttributes.Cells[0][0], menuArmor		,	menuItemsArmor			,  instanceGame.FrameInput, {-1});	break;
	case GAME_SUBSTATE_ITEM			:	selectedChoice = drawMenu(instanceGame.GlobalDisplay.Screen, &instanceGame.GlobalDisplay.TextAttributes.Cells[0][0], menuItem		,	menuItemsItem			,  instanceGame.FrameInput, {-1});	break;
	case GAME_SUBSTATE_CHARACTER	:	selectedChoice = drawMenu(instanceGame.GlobalDisplay.Screen, &instanceGame.GlobalDisplay.TextAttributes.Cells[0][0], menuAgent		,	menuItemsAgent			,  4, instanceGame.FrameInput, {-1});	break;
	default:
		break;
	}

	if(selectedChoice.Definition == SHOP_EXIT_VALUE)
		return {GAME_STATE_MENU_BUY};
	else if( selectedChoice.Definition == -1 )
		return returnState;

	SGameState retVal = returnState;

	SPlayer& player = instanceGame.Players[PLAYER_INDEX_USER];

	instanceGame.ClearMessages();
	if(selectedChoice.Price > player.Money)
	{
		instanceGame.UserError		= "You don't have enough money for " + selectedChoice.Name + "!!";
		instanceGame.LogError();
		return retVal;
	}

	SCharacterInventory& playerInventory = player.Goods.Inventory;
	int32_t iCharacterInArmy = 0, armySize = player.Army.size();
	bool bFoundFreeCharacterSlot = false;
	bool bSold = false;
	GPtrObj(CCharacter) newCharacter;
	switch(instanceGame.State.Substate)
	{
	case GAME_SUBSTATE_ACCESSORY	:	if(playerInventory.Accessory	.AddElement({selectedChoice.Definition, 0, selectedChoice.Grade, -1})) 	{ instanceGame.UserSuccess = "You have successfully bought " + selectedChoice.Name + " for " + std::to_string(selectedChoice.Price) + " Coins.";				bSold = true; } break;
	case GAME_SUBSTATE_STAGEPROP	:	if(playerInventory.StageProp	.AddElement({selectedChoice.Definition, 0, selectedChoice.Grade, -1})) 	{ instanceGame.UserSuccess = "You have successfully bought " + selectedChoice.Name + " for " + std::to_string(selectedChoice.Price) + " Coins.";				bSold = true; } break;
	case GAME_SUBSTATE_FACILITY		:	if(playerInventory.Facility		.AddElement({selectedChoice.Definition, 0, selectedChoice.Grade, -1})) 	{ instanceGame.UserSuccess = "You have successfully bought " + selectedChoice.Name + " for " + std::to_string(selectedChoice.Price) + " Coins.";				bSold = true; } break;
	case GAME_SUBSTATE_VEHICLE		:	if(playerInventory.Vehicle		.AddElement({selectedChoice.Definition, 0, selectedChoice.Grade, -1})) 	{ instanceGame.UserSuccess = "You have successfully bought " + selectedChoice.Name + " for " + std::to_string(selectedChoice.Price) + " Coins.";				bSold = true; } break;
	case GAME_SUBSTATE_PROFESSION	:	if(playerInventory.Profession	.AddElement({selectedChoice.Definition, 0, selectedChoice.Grade, -1})) 	{ instanceGame.UserSuccess = "You have successfully bought " + selectedChoice.Name + " Job License for " + std::to_string(selectedChoice.Price) + " Coins."	;	bSold = true; } break;
	case GAME_SUBSTATE_WEAPON		:	if(playerInventory.Weapon		.AddElement({selectedChoice.Definition, 0, selectedChoice.Grade, -1})) 	{ instanceGame.UserSuccess = "You have successfully bought " + selectedChoice.Name + " for " + std::to_string(selectedChoice.Price) + " Coins.";				bSold = true; } break;
	case GAME_SUBSTATE_ARMOR		:	if(playerInventory.Armor		.AddElement({selectedChoice.Definition, 0, selectedChoice.Grade, -1})) 	{ instanceGame.UserSuccess = "You have successfully bought " + selectedChoice.Name + " for " + std::to_string(selectedChoice.Price) + " Coins.";				bSold = true; } break;
	case GAME_SUBSTATE_ITEM			:	if(playerInventory.Items		.AddElement({selectedChoice.Definition, 0, selectedChoice.Grade, -1})) 	{ instanceGame.UserSuccess = "You have successfully bought " + selectedChoice.Name + " for " + std::to_string(selectedChoice.Price) + " Coins.";				bSold = true; } break;
	case GAME_SUBSTATE_CHARACTER	:	
		newCharacter = GPtrObj(CCharacter)(enemyDefinitions[selectedChoice.Definition]);
		for(iCharacterInArmy ; iCharacterInArmy < armySize; ++iCharacterInArmy) {
			if(0 == player.Army[iCharacterInArmy]) {
				player.Army.set(newCharacter, iCharacterInArmy);
				bFoundFreeCharacterSlot = true;
				break;
			}
		}
		if(!bFoundFreeCharacterSlot)
			player.Army.push_back(newCharacter);	
		setupAgent(*newCharacter.get_address(), *newCharacter.get_address());
		instanceGame.UserSuccess = "You have successfully hired " + selectedChoice.Name + " for " + std::to_string(selectedChoice.MaintenanceCost) + " Coins/Mission."; 
		bSold = true;
		break;
	default:
		break;
	}

	if(bSold)
	{
		instanceGame.LogSuccess();
		player.Money -= selectedChoice.Price;
		player.Score.MoneySpent += selectedChoice.Price;
	}
	else
	{
		instanceGame.UserError = "There is not enough space in your inventory!"; 
		instanceGame.LogError();
	}




	return retVal;
}

SGameState drawBuy(SGame& instanceGame, const SGameState& returnState)
{
	static const ::nwol::glabel textToPrint = "Tell me how much money you have and I will tell you what you'll become.";

	bool bDonePrinting = ::nwol::getMessageSlow(instanceGame.SlowMessage, textToPrint.c_str(), textToPrint.size(), instanceGame.FrameTimer.LastTimeSeconds*3);
	memcpy(&instanceGame.PostEffectDisplay.Screen.Cells[instanceGame.PostEffectDisplay.Depth>>1][instanceGame.PostEffectDisplay.Width/2-(strlen(instanceGame.SlowMessage)+1)/2], instanceGame.SlowMessage, strlen(instanceGame.SlowMessage));
	if ( !bDonePrinting ) 
		return returnState;

	if(GAME_SUBSTATE_MAIN == instanceGame.State.Substate) {
		static const SMenu<SGameState> menuBuy({GAME_STATE_WELCOME_COMMANDER}, "Order Menu", 26);
		return drawMenu(instanceGame.GlobalDisplay.Screen, &instanceGame.GlobalDisplay.TextAttributes.Cells[0][0], menuBuy, optionsBuy, instanceGame.FrameInput, instanceGame.State);
	}
	else 
		return drawBuyMenu(instanceGame, returnState);

	return returnState;
};


