//////////////////////////////////////////////////////////////////////////
//
// This file is part of Iron's source files.
// Iron is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2016, 2017, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef STONE_H
#define STONE_H

#include <BWAPI.h>
#include "strat.h"
#include "../defs.h"
#include "../ironutils.h"


namespace iron
{

class MyUnit;
	
//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Stone
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class Stone : public Strat
{
public:
									Stone();
									~Stone();

	string							Name() const override { return "Stone"; }
	string							StateDescription() const override;

	bool							Active() const		{ return m_active; }
	bool							CanExpand() const;
	bool							TimeToScout() const;
	bool							Berserk() const;
	bool							Demolish() const;
	int								SCVsoldiersObjective() const;

private:
	void							OnFrame_v() override;
	bool							Detection() const;

	bool							m_active = false;
	mutable bool					m_berserk = false;
	bool							m_ironNow = false;
	frame_t							m_ironAt = 1000000;
	frame_t							m_timeToScout = 0;
	int								m_soldiersTenth = 7;
	int								m_expandThreshold = 250;
	bool							m_demolish = false;
};


} // namespace iron


#endif

