#include "BotsManager.hpp"

namespace ChimeraBot
{

	BWAPI::AIModule* BotsManager::loadAIModule(const string& moduleName) {
		//the signature of the function that creates an AI module
		using PFNCreateAI = BWAPI::AIModule* (*)(BWAPI::Game*);
		using PFNGameInit = void(*)(BWAPI::Game*);
		const auto path = "bwapi-data/AI/" + moduleName + ".dll";
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
			chosenBot.emplace<McRaveModule>(McRaveModule{});
			chosenBotEnum = McRave;
			break;
		case Races::Terran:
			chosenBot.emplace<iron::Iron*>(new iron::Iron{});
			chosenBotEnum = Iron;
			break;
		case Races::Zerg:
			chosenBot.emplace<AIModule*>(loadAIModule("ZZZKBot"));
			chosenBotEnum = ZZZKBot;
			break;
		}
	}

	BotsManager::BotsManager()
	{
		chooseBot();
	}

	void BotsManager::onStart()
	{
		switch(chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onStart();
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onStart();
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onStart();
			break;
		default:;
		}
	}

	void BotsManager::onEnd(const bool isWinner)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onEnd(isWinner);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onEnd(isWinner);
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onEnd(isWinner);
			break;
		default:;
		}
	}

	void BotsManager::onFrame()
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onFrame();
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onFrame();
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onFrame();
			break;
		default:;
		}
	}

	void BotsManager::onPlayerLeft(const BWAPI::Player player)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onPlayerLeft(player);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onPlayerLeft(player);
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onPlayerLeft(player);
			break;
		default:;
		}
	}

	void BotsManager::onNukeDetect(const BWAPI::Position target)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onNukeDetect(target);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onNukeDetect(target);
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onNukeDetect(target);
			break;
		default:;
		}
	}

	void BotsManager::onUnitDiscover(BWAPI::Unit unit)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onUnitDiscover(unit);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onUnitDiscover(unit);
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onUnitDiscover(unit);
			break;
		default:;
		}
	}

	void BotsManager::onUnitEvade(BWAPI::Unit unit)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onUnitEvade(unit);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onUnitEvade(unit);
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onUnitEvade(unit);
			break;
		default:;
		}
	}

	void BotsManager::onUnitShow(BWAPI::Unit unit)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onUnitShow(unit);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onUnitShow(unit);
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onUnitShow(unit);
			break;
		default:;
		}
	}

	void BotsManager::onUnitHide(BWAPI::Unit unit)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onUnitHide(unit);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onUnitHide(unit);
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onUnitHide(unit);
			break;
		default:;
		}
	}

	void BotsManager::onUnitCreate(BWAPI::Unit unit)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onUnitCreate(unit);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onUnitCreate(unit);
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onUnitCreate(unit);
			break;
		default:;
		}
	}

	void BotsManager::onUnitDestroy(BWAPI::Unit unit)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onUnitDestroy(unit);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onUnitDestroy(unit);
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onUnitDestroy(unit);
			break;
		default:;
		}
	}

	void BotsManager::onUnitMorph(BWAPI::Unit unit)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onUnitMorph(unit);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onUnitMorph(unit);
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onUnitMorph(unit);
			break;
		default:;
		}
	}

	void BotsManager::onUnitRenegade(BWAPI::Unit unit)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onUnitRenegade(unit);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onUnitRenegade(unit);
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onUnitRenegade(unit);
			break;
		default:;
		}
	}

	void BotsManager::onUnitComplete(BWAPI::Unit unit)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onUnitComplete(unit);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onUnitComplete(unit);
			break;
		case ZZZKBot:
			std::get<AIModule*>(chosenBot)->onUnitComplete(unit);
			break;
		default:;
		}
	}
}
