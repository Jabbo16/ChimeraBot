//////////////////////////////////////////////////////////////////////////
//
// This file is part of Iron's source files.
// Iron is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2016, 2017, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef PROTOSS_GREEDY_H
#define PROTOSS_GREEDY_H

#include <BWAPI.h>
#include "strat.h"
#include "../defs.h"
#include "../ironutils.h"


namespace iron
{

class MyUnit;
	
//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class ProtossGreedy
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class ProtossGreedy : public Strat
{
public:
									ProtossGreedy();

	string							Name() const override { return "ProtossGreedy"; }
	string							StateDescription() const override;

	bool							Active() const		{ return m_active; }
	bool							CanExpand() const;

private:
	void							OnFrame_v() override;
	bool							Detection() const;

	bool							m_active = false;
	mutable bool					m_canExpand = false;
};


} // namespace iron


#endif

