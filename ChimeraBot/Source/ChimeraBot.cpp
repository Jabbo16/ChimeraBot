#include "ChimeraBot.hpp"

namespace ChimeraBot
{
	void ChimeraBot::onStart()
	{
		botsManager.onStart();
	}

	void ChimeraBot::onEnd(bool isWinner)
	{
		botsManager.onEnd(isWinner);
	}

	void ChimeraBot::onFrame()
	{
		botsManager.onFrame();
	}

	void ChimeraBot::onSendText(std::string text)
	{
	}

	void ChimeraBot::onReceiveText(BWAPI::Player player, std::string text)
	{
	}

	void ChimeraBot::onPlayerLeft(BWAPI::Player player)
	{
		botsManager.onPlayerLeft(player);
	}

	void ChimeraBot::onNukeDetect(BWAPI::Position target)
	{
		botsManager.onNukeDetect(target);
	}

	void ChimeraBot::onUnitDiscover(BWAPI::Unit unit)
	{
		botsManager.onUnitDiscover(unit);
	}

	void ChimeraBot::onUnitEvade(BWAPI::Unit unit)
	{
		botsManager.onUnitEvade(unit);
	}

	void ChimeraBot::onUnitShow(BWAPI::Unit unit)
	{
		botsManager.onUnitShow(unit);
	}

	void ChimeraBot::onUnitHide(BWAPI::Unit unit)
	{
		botsManager.onUnitHide(unit);
	}

	void ChimeraBot::onUnitCreate(BWAPI::Unit unit)
	{
		botsManager.onUnitCreate(unit);
	}

	void ChimeraBot::onUnitDestroy(BWAPI::Unit unit)
	{
		botsManager.onUnitDestroy(unit);
	}

	void ChimeraBot::onUnitMorph(BWAPI::Unit unit)
	{
		botsManager.onUnitMorph(unit);
	}

	void ChimeraBot::onUnitRenegade(BWAPI::Unit unit)
	{
		botsManager.onUnitRenegade(unit);
	}

	void ChimeraBot::onSaveGame(std::string gameName)
	{
	}

	void ChimeraBot::onUnitComplete(BWAPI::Unit unit)
	{
		botsManager.onUnitComplete(unit);
	}
}

