#pragma once
#include <variant>
#include <Windows.h>
#include <BWAPI.h>
#include "Bots/McRave/McRave/Header.h"
#include "Bots/IronBot/Iron.h"

namespace ChimeraBot
{
	class BotsManager
	{
		AIModule* chosenBot;
		BWAPI::AIModule * loadAIModule(const string& moduleName);
		void chooseBot();
	public:
		BotsManager();
		void onStart();
		void onEnd(bool isWinner);
		void onFrame();
		void onPlayerLeft(BWAPI::Player player);
		void onNukeDetect(BWAPI::Position target);
		void onUnitDiscover(BWAPI::Unit unit);
		void onUnitEvade(BWAPI::Unit unit);
		void onUnitShow(BWAPI::Unit unit);
		void onUnitHide(BWAPI::Unit unit);
		void onUnitCreate(BWAPI::Unit unit);
		void onUnitDestroy(BWAPI::Unit unit);
		void onUnitMorph(BWAPI::Unit unit);
		void onUnitRenegade(BWAPI::Unit unit);
		void onUnitComplete(BWAPI::Unit unit);
	};
}
