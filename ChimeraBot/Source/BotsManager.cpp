#include "BotsManager.hpp"

namespace ChimeraBot
{
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
			chosenBot.emplace<ZZZKBotAIModule>(ZZZKBotAIModule{});
			chosenBotEnum = ZZK;
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
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onStart();
			break;
		default:;
		}
	}

	void BotsManager::onEnd(bool isWinner)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onEnd(isWinner);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onEnd(isWinner);
			break;
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onEnd(isWinner);
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
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onFrame();
			break;
		default:;
		}
	}

	void BotsManager::onPlayerLeft(BWAPI::Player player)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onPlayerLeft(player);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onPlayerLeft(player);
			break;
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onPlayerLeft(player);
			break;
		default:;
		}
	}

	void BotsManager::onNukeDetect(BWAPI::Position target)
	{
		switch (chosenBotEnum)
		{
		case McRave:
			std::get<McRaveModule>(chosenBot).onNukeDetect(target);
			break;
		case Iron:
			std::get<iron::Iron*>(chosenBot)->onNukeDetect(target);
			break;
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onNukeDetect(target);
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
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onUnitDiscover(unit);
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
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onUnitEvade(unit);
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
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onUnitShow(unit);
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
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onUnitHide(unit);
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
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onUnitCreate(unit);
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
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onUnitDestroy(unit);
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
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onUnitMorph(unit);
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
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onUnitRenegade(unit);
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
		case ZZK:
			std::get<ZZZKBotAIModule>(chosenBot).onUnitComplete(unit);
			break;
		default:;
		}
	}
}
