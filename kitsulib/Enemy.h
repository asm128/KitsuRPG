#include "Character.h"

#ifndef __ENEMY_H__923409234902364897234__
#define __ENEMY_H__923409234902364897234__

//#define BEHEMOT_STATUS_INFLICT		((klib::COMBAT_STATUS)(klib::COMBAT_STATUS_POISON	|klib::COMBAT_STATUS_BLEEDING	))
//#define ZEUS_STATUS_INFLICT			((klib::COMBAT_STATUS)(klib::COMBAT_STATUS_BLIND	|klib::COMBAT_STATUS_SHOCK		))
//#define TIAMAT_STATUS_INFLICT		((klib::COMBAT_STATUS)(klib::COMBAT_STATUS_CHARMED	|klib::COMBAT_STATUS_STUN		|klib::COMBAT_STATUS_BLEEDING	|klib::COMBAT_STATUS_PETRIFY	))
#define TIAMAT_STATUS_IMMUNITY		((klib::COMBAT_STATUS)(klib::COMBAT_STATUS_BLIND	|klib::COMBAT_STATUS_BLEEDING	|klib::COMBAT_STATUS_BURN		|klib::COMBAT_STATUS_FROZEN		|klib::COMBAT_STATUS_POISON		|klib::COMBAT_STATUS_PETRIFY	))
//#define DEATH_STATUS_INFLICT		((klib::COMBAT_STATUS)(klib::COMBAT_STATUS_BLIND	|klib::COMBAT_STATUS_STUN		|klib::COMBAT_STATUS_BLEEDING	|klib::COMBAT_STATUS_BURN		|klib::COMBAT_STATUS_FREEZING	|klib::COMBAT_STATUS_POISON		|klib::COMBAT_STATUS_PETRIFY	|klib::COMBAT_STATUS_PANIC	))
#define DEATH_STATUS_IMMUNITY		((klib::COMBAT_STATUS)(klib::COMBAT_STATUS_BLIND	|klib::COMBAT_STATUS_BLEEDING	|klib::COMBAT_STATUS_BURN		|klib::COMBAT_STATUS_FROZEN		|klib::COMBAT_STATUS_POISON		|klib::COMBAT_STATUS_PANIC		))
//#define SELDON_STATUS_INFLICT		((klib::COMBAT_STATUS)(klib::COMBAT_STATUS_BLIND	|klib::COMBAT_STATUS_STUN		|klib::COMBAT_STATUS_SHOCK		|klib::COMBAT_STATUS_BLACKOUT	|klib::COMBAT_STATUS_PETRIFY	|klib::COMBAT_STATUS_SLOW		|klib::COMBAT_STATUS_SLEEP		))
#define SELDON_STATUS_IMMUNITY		((klib::COMBAT_STATUS)(klib::COMBAT_STATUS_BLIND	|klib::COMBAT_STATUS_BLEEDING	|klib::COMBAT_STATUS_FREEZING	|klib::COMBAT_STATUS_SLEEP		|klib::COMBAT_STATUS_SHOCK		|klib::COMBAT_STATUS_BLACKOUT	|klib::COMBAT_STATUS_PETRIFY	))

namespace klib
{
#pragma pack(push, 1)
	// (maxHP, hitChance, attack, coins, SEntityFitness, SEntityEffect, SEntityStatus, name)
	static const CCharacter enemyDefinitions[] =	
	{	{	    1,		   50,		    1,     100	,	{1, 4+ 2,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_NONE		},	"Normal guy"		}
	,	{	    5,		   50,		    5,    1000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_NONE		},	"Slave"				}
	,	{	   10,		   50,		   10,    4000	,	{1, 4+ 6,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_NONE		},	"Terrorist"			}
	,	{	   15,		   50,		   15,   16000	,	{1, 4+ 8,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_NONE		},	"Genocide"			}
	,	{10+10+10,		50+10,		15+10,	 20000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_POISON	},	"Behemoth"			}
	,	{10+10+11,		50+11,		15+11,	 21000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_POISON	},	"Jesus"				}
	,	{10+10+12,		50+12,		15+12,	 22000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_POISON	},	"Dragon"			}
	,	{10+10+13,		50+13,		15+13,	 23000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_POISON	},	"Gilgamesh"			}
	,	{10+10+14,		50+14,		15+14,	 24000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_POISON	},	"Fenrir"			}
	,	{10+10+15,		50+15,		15+15,	 25000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_POISON	},	"Perseus"			}
	,	{10+20+16,		50+16,		15+16,	 26000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_SLEEP		},	"Cthulhu"			}
	,	{10+20+17,		50+17,		15+17,	 27000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_BLIND		},	"Zeus"				}
	,	{10+20+18,		50+18,		15+18,	 28000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_STUN		},	"Thor"				}
	,	{10+20+19,		50+19,		15+19,	 29000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	COMBAT_STATUS_BLEEDING	},	"Anhur"				}
	,	{10+20+20,		50+20,		15+20,	 30000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	TIAMAT_STATUS_IMMUNITY	},	"Tiamat"			}
	,	{10+40+21,		50+21,		15+21,	 31000	,	{1, 4+ 4,  0, 10}, {}	,	{COMBAT_STATUS_NONE	,	DEATH_STATUS_IMMUNITY	},	"Death"				}
	,	{10+80+23,		50+23,		15+23,	 33000	,	{1, 4+ 4, 10, 10}, {}	,	{COMBAT_STATUS_NONE	,	SELDON_STATUS_IMMUNITY	},	"asm128"			}
	};

	// set
	void setupAgent				(const CCharacter& adventurer, CCharacter& currentEnemy);
	void completeAgentResearch	(CCharacter& agent);
#pragma pack(pop)
}	// namespace
#endif // __ENEMY_H__923409234902364897234__