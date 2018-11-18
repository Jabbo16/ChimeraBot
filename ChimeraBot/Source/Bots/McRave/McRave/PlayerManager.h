#pragma once
#include "McRave.h"
#include "PlayerInfo.h"
#include "Singleton.h"

namespace McRave
{
	class PlayerManager
	{
		map <Player, PlayerInfo> thePlayers;
		int eZerg, eProtoss, eTerran, eRandom;
	public:
		map <Player, PlayerInfo>& getPlayers() { return thePlayers; }
		void onStart(), onFrame(), update(PlayerInfo&);

		int getNumberZerg() { return eZerg; }
		int getNumberProtoss() { return eProtoss; }
		int getNumberTerran() { return eTerran; }
		int getNumberRandom() { return eRandom; }

		bool vP() { return (thePlayers.size() == 1 && (eProtoss > 0 || Broodwar->enemy()->getRace() == Races::Protoss)); }
		bool vT() { return (thePlayers.size() == 1 && (eTerran > 0 || Broodwar->enemy()->getRace() == Races::Terran)); }
		bool vZ() { return (thePlayers.size() == 1 && (eZerg > 0 || Broodwar->enemy()->getRace() == Races::Zerg)); }
	};
}

typedef Singleton<McRave::PlayerManager> PlayerSingleton;
