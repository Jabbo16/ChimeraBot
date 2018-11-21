//////////////////////////////////////////////////////////////////////////
//
// This file is part of Iron's source files.
// Iron is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2016, 2017, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "protossGreedy.h"
#include "strategy.h"
#include "wraithRush.h"
#include "expand.h"
#include "stone.h"
#include "../units/army.h"
#include "../Iron.h"

namespace { auto & bw = Broodwar; }




namespace iron
{


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class ProtossGreedy
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


ProtossGreedy::ProtossGreedy()
{
}


string ProtossGreedy::StateDescription() const
{
	if (!m_active) return "-";
	if (m_active) return "m_active";

	return "-";
}


bool ProtossGreedy::CanExpand() const
{
	if (m_canExpand) return true;

	if (me().Bases().size() >= 3) return true;

	if (me().Buildings(Terran_Factory).size() >= 3)
		if (me().Units(Terran_Vulture).size() >= 18 ||
			me().Units(Terran_Vulture).size() >= 15 && me().MineralsAvailable() >= 300)
		{
		///	DO_ONCE { bw << "ProtossGreedy::CanExpand()" << endl; ai()->SetDelay(500); }
			return m_canExpand = true;
		}

	return false;
}


void ProtossGreedy::OnFrame_v()
{
	if (ai()->GetStrategy()->Active<Stone>())
		return Discard();

	if (him().IsTerran() || him().IsZerg()) return Discard();

	if (m_active)
	{
		if (me().Buildings(Terran_Command_Center).size() >= 3) return Discard();
	}
	else
	{
		if (me().Buildings(Terran_Command_Center).size() >= 2) return Discard();

		if (him().Buildings(Protoss_Forge).size() >= 1) return Discard();
		if (him().Buildings(Protoss_Photon_Cannon).size() >= 1) return Discard();

		if ((him().Buildings(Protoss_Nexus).size() >= 2) ||
			((him().Buildings(Protoss_Nexus).size() == 1) &&
			!contains(ai()->GetMap().StartingLocations(), him().Buildings(Protoss_Nexus).front()->TopLeft())) ||
			him().MayDarkTemplar())
		{
		///	bw << Name() << " active!" << endl; //ai()->SetDelay(100);

			m_active = true;
			return;
		}
	}
}


} // namespace iron



