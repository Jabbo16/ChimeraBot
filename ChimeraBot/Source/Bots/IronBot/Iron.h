//////////////////////////////////////////////////////////////////////////
//
// This file is part of Iron's source files.
// Iron is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2016, 2017, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef IRON_H
#define IRON_H

#include <BWAPI.h>
#include "territory/vmap.h"
#include "units/me.h"
#include "units/him.h"
#include "units/critters.h"
#include "ironutils.h"
#include "debug.h"
#include "defs.h"
#include <memory>

// Remember not to use "Broodwar" in any global class constructor!


namespace iron
{

class Strategy;
class VGridMap;


class Iron : public BWAPI::AIModule
{
public:
	int			m_logLines = 0;
	static Iron *		Instance()	{ return m_pInstance; }

	// Virtual functions for callbacks, leave these as they are.
	void onStart() override;
	void onEnd(bool isWinner) override;
	void onFrame() override;
	void onSendText(std::string text) override;
	void onReceiveText(BWAPI::Player player, std::string text) override;
	void onPlayerLeft(BWAPI::Player player) override;
	void onNukeDetect(BWAPI::Position target) override;
	void onUnitDiscover(BWAPI::Unit unit) override;
	void onUnitEvade(BWAPI::Unit unit) override;
	void onUnitShow(BWAPI::Unit unit) override;
	void onUnitHide(BWAPI::Unit unit) override;
	void onUnitCreate(BWAPI::Unit unit) override;
	void onUnitDestroy(BWAPI::Unit unit) override;
	void onUnitMorph(BWAPI::Unit unit) override;
	void onUnitRenegade(BWAPI::Unit unit) override;
	void onSaveGame(std::string gameName) override;
	void onUnitComplete(BWAPI::Unit unit) override;
	// Everything below this line is safe to modify.

									Iron();
									~Iron();

	BWEM::Map &						GetMap() const				{ return m_Map; }
	VMap &							GetVMap() const				{ return *m_pVMap.get(); }
	VGridMap &						GetGridMap() const			{ return *m_pGridMap.get(); }

	const class Me &				Me() const					{ return m_Me; }
	class Me &						Me()						{ return m_Me; }

	const class Him &				Him() const					{ return m_Him; }
	class Him &						Him()						{ return m_Him; }

	const class Critters &			Critters() const			{ return m_Critters; }
	class Critters &				Critters()					{ return m_Critters; }

	Strategy *						GetStrategy() const			{ return m_pStrategy.get(); }

	frame_t							Latency() const				{ return m_latency; }
	frame_t							Frame() const				{ return m_frame; }
	gameTime_t						Time() const				{ return m_time; }
	delay_t							Delay() const				{ return m_delay; }
	void							SetDelay(delay_t delay);

#if DEV && BWEM_USE_WINUTILS
	const map<string, TimerStats> &	OnFrameTimes() const	{ return m_OnFrameTimes; }
#endif

	void							OnMineralDestroyed(BWAPI::Unit u);
	void							OnStaticBuildingDestroyed(BWAPI::Unit u);

	Iron &							operator=(const Iron &) = delete;
private:

	void							CheckLeaveGame() const;
	void							Update();
	void							RunExperts();
	void							RunBehaviors();
	void							RunProduction();
	bool							HeHasLeft() const					{ return m_heHasLeft; }

	BWEM::Map &				m_Map;
	unique_ptr<VMap>		m_pVMap;
	unique_ptr<VGridMap>	m_pGridMap;
	class Me				m_Me;
	class Him				m_Him;
	class Critters			m_Critters;
	unique_ptr<Strategy>	m_pStrategy;

	frame_t					m_latency;
	frame_t					m_frame = 0;
	gameTime_t				m_time = 0;
	delay_t					m_delay;
	bool					m_heHasLeft = false;

#if DEV && BWEM_USE_WINUTILS
	map<string, TimerStats>	m_OnFrameTimes;
#endif

	static Iron *			m_pInstance;
};


inline Iron * ai() { return Iron::Instance(); }

	
} // namespace iron




#endif

