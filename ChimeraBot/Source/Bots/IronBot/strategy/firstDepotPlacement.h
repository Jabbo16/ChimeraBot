//////////////////////////////////////////////////////////////////////////
//
// This file is part of Iron's source files.
// Iron is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2016, 2017, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef FIRST_DEPOT_PLACEMENT_H
#define FIRST_DEPOT_PLACEMENT_H

#include <BWAPI.h>
#include "strat.h"
#include "../defs.h"
#include "../ironutils.h"


namespace iron
{

class MyUnit;
FORWARD_DECLARE_MY(Terran_SCV)
	
//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class FirstDepotPlacement
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class FirstDepotPlacement : public Strat
{
public:
									FirstDepotPlacement();
									~FirstDepotPlacement();

	string							Name() const override { return "FirstDepotPlacement"; }
	string							StateDescription() const override;

	bool							Active() const		{ return m_active; }
	TilePosition					Location() const	{ return m_location; }
	My<Terran_SCV> *				Builder() const		{ return m_pBuilder; }

private:
	void							OnBWAPIUnitDestroyed(BWAPIUnit * pBWAPIUnit) override;
	void							OnFrame_v() override;

	bool							m_active = false;
	frame_t							m_activeSince;
	My<Terran_SCV> *				m_pBuilder = nullptr;
	TilePosition					m_location = TilePositions::None;
};


} // namespace iron


#endif

