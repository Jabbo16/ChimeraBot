//////////////////////////////////////////////////////////////////////////
//
// This file is part of Iron's source files.
// Iron is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2016, 2017, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "massHydra.h"
#include "strategy.h"
#include "zerglingRush.h"
#include "expand.h"
#include "stone.h"
#include "../units/army.h"
#include "../Iron.h"

namespace { auto & bw = Broodwar; }




namespace iron
{


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class MassHydra
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


MassHydra::MassHydra()
{
}


string MassHydra::StateDescription() const
{
	if (!m_active) return "-";
	if (m_active) return "active : " + to_string(me().Army().ValueAgainstMassHydra()).substr(0, 4);

	return "-";
}


bool MassHydra::CanExpand() const
{
//	if (m_canExpand) return true;

	if (me().Bases().size() >= 2) return true;

	if (me().CompletedBuildings(Terran_Factory) >= 3)
		if (me().Army().ValueAgainstMassHydra() + me().CompletedUnits(Terran_Marine)/3 >= 10)
			if (me().Army().MyGroundUnitsAhead() > me().Army().HisGroundUnitsAhead() + 3)
				return true;

	return false;
}




void MassHydra::OnFrame_v()
{
	if (ai()->GetStrategy()->Active<Stone>())
		return Discard();

	if (him().IsTerran() || him().IsProtoss()) return Discard();

	if (him().HasLurkers())
	{
		DO_ONCE
			if (me().CompletedBuildings(Terran_Factory) == 2)
				for (const auto & b : me().Buildings(Terran_Factory))
					if (!b->Completed())
						if (b->CanAcceptCommand())
							b->CancelConstruction();

		if (!(me().HasResearched(TechTypes::Tank_Siege_Mode) || me().Player()->isResearching(TechTypes::Tank_Siege_Mode)))
			for (const auto & b : me().Buildings(Terran_Machine_Shop))
				if (b->Unit()->isUpgrading() ||
					b->Unit()->isResearching() && b->Unit()->getTech() != TechTypes::Tank_Siege_Mode)
					if (b->CanAcceptCommand())
						b->CancelResearch();
	}

	if (m_active)
	{
		if (me().Bases().size() >= 3)
			return Discard();

		if (him().CreationCount(Zerg_Hydralisk) == 0)
			if (me().Units(Terran_Vulture).size() >= 6)
			{
				m_active = false;
				return;
			}


		if (me().CompletedBuildings(Terran_Command_Center) >= 2)
			if (me().Army().ValueAgainstMassHydra() > 12)
				if (me().Army().MyGroundUnitsAhead() > me().Army().HisGroundUnitsAhead() + 3)
					return Discard();

		if (!him().MayMuta())
		{
			DO_ONCE
				for (const auto & b : me().Buildings(Terran_Engineering_Bay))
					if (!b->Completed())
						if (b->CanAcceptCommand())
							b->CancelConstruction();
		}
	}
	else
	{
		if (me().CompletedBuildings(Terran_Command_Center) >= 2) return Discard();

		const bool severalHatches = 
			(him().Buildings(Zerg_Hatchery).size() >= 2) ||
				((him().Buildings(Zerg_Hatchery).size() == 1) &&
				!contains(ai()->GetMap().StartingLocations(), him().Buildings(Zerg_Hatchery).front()->TopLeft()));

		if (!ai()->GetStrategy()->Detected<ZerglingRush>())
			if (him().AllUnits(Zerg_Hydralisk).size() >= 1 && me().Army().ValueAgainstMassHydra() < 2.0 ||
				severalHatches
					&& (him().AllUnits(Zerg_Hydralisk).size() >= 1 ||
						him().Buildings(Zerg_Hydralisk_Den).size() >= 1 && me().Units(Terran_Vulture).size() < 6)
					&& him().Buildings(Zerg_Hive).size() == 0
					&& him().Buildings(Zerg_Defiler_Mound).size() == 0
					&& him().Buildings(Zerg_Evolution_Chamber).size() == 0
					&& him().Buildings(Zerg_Greater_Spire).size() == 0
					&& him().Buildings(Zerg_Lair).size() == 0
					&& him().Buildings(Zerg_Spire).size() == 0
					&& him().Buildings(Zerg_Ultralisk_Cavern).size() == 0
					&& him().CreationCount(Zerg_Mutalisk) == 0
					&& him().CreationCount(Zerg_Lurker) == 0
					&& him().CreationCount(Zerg_Lurker_Egg) == 0
				)
			{
			///	bw << Name() << " active!" << endl; //ai()->SetDelay(100);

				m_active = true;
				return;
			}
	}
}


} // namespace iron



