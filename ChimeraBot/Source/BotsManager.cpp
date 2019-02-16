#include "BotsManager.hpp"

namespace ChimeraBot
{

	BWAPI::AIModule* BotsManager::loadAIModule(const string& moduleName) {
		//the signature of the function that creates an AI module
		using PFNCreateAI = BWAPI::AIModule* (*)(BWAPI::Game*);
		using PFNGameInit = void(*)(BWAPI::Game*);
		const auto path = "bwapi-data/AI/bots/" + moduleName + ".dll";
		const auto hDLL = LoadLibrary(path.c_str());
		if (hDLL != NULL) {
			// Obtain the AI module function
			const auto newAIModule = PFNCreateAI(GetProcAddress(hDLL, "newAIModule"));
			// The two lines below are needed in BWAPI >= 4.1.2
			const auto gameInitFunction = PFNGameInit(GetProcAddress(hDLL, "gameInit"));
			gameInitFunction(BroodwarPtr);
			if (newAIModule) {
				return newAIModule(BroodwarPtr);
			}
		}
		return new AIModule();
	}

	void BotsManager::chooseBot()
	{
		switch (Broodwar->self()->getRace())
		{
		case Races::Protoss:
			chosenBot = new McRaveModule{};
			break;
		case Races::Terran:
			chosenBot = new iron::Iron{};
			break;
		case Races::Zerg:
			chosenBot = loadAIModule("ZZZKBot");
			break;
		default: ;
		}
	}

	BotsManager::BotsManager()
	{
		chooseBot();
	}

	void BotsManager::onStart()
	{
		chosenBot->onStart();
	}

	void BotsManager::onEnd(const bool isWinner)
	{
		chosenBot->onEnd(isWinner);
	}

	void BotsManager::onFrame()
	{
		chosenBot->onFrame();
	}

	void BotsManager::onPlayerLeft(const BWAPI::Player player)
	{
		chosenBot->onPlayerLeft(player);
	}

	void BotsManager::onNukeDetect(const BWAPI::Position target)
	{
		chosenBot->onNukeDetect(target);
	}

	void BotsManager::onUnitDiscover(BWAPI::Unit unit)
	{
		chosenBot->onUnitDiscover(unit);
	}

	void BotsManager::onUnitEvade(BWAPI::Unit unit)
	{
		chosenBot->onUnitEvade(unit);
	}

	void BotsManager::onUnitShow(BWAPI::Unit unit)
	{
		chosenBot->onUnitShow(unit);
	}

	void BotsManager::onUnitHide(BWAPI::Unit unit)
	{
		chosenBot->onUnitHide(unit);
	}

	void BotsManager::onUnitCreate(BWAPI::Unit unit)
	{
		chosenBot->onUnitCreate(unit);
	}

	void BotsManager::onUnitDestroy(BWAPI::Unit unit)
	{
		chosenBot->onUnitDestroy(unit);
	}

	void BotsManager::onUnitMorph(BWAPI::Unit unit)
	{
		chosenBot->onUnitMorph(unit);
	}

	void BotsManager::onUnitRenegade(BWAPI::Unit unit)
	{
		chosenBot->onUnitRenegade(unit);
	}

	void BotsManager::onUnitComplete(BWAPI::Unit unit)
	{
		chosenBot->onUnitComplete(unit);
	}
}