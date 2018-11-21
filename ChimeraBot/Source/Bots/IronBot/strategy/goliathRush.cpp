//////////////////////////////////////////////////////////////////////////
//
// This file is part of Iron's source files.
// Iron is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2016, 2017, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "goliathRush.h"
#include "strategy.h"
#include "stone.h"
#include "../units/army.h"
#include "../behavior/VChasing.h"
#include "../Iron.h"

namespace { auto & bw = Broodwar; }




namespace iron
{


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class GoliathRush
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


GoliathRush::GoliathRush()
{
}


string GoliathRush::StateDescription() const
{
	if (!m_detected) return "-";
	if (m_detected) return "detected";

	return "-";
}


bool GoliathRush::CanExpand() const
{
//	if (m_canExpand) return true;

	if (me().Bases().size() >= 2) return true;

	if (me().CompletedBuildings(Terran_Factory) >= 3)
		if (me().Units(Terran_Vulture).size() + 4*me().Units(Terran_Siege_Tank_Tank_Mode).size() + 3*me().Units(Terran_Goliath).size() > 8 +
 		    him().AllUnits(Terran_Vulture).size() + 4*him().AllUnits(Terran_Siege_Tank_Tank_Mode).size() + 3*him().AllUnits(Terran_Goliath).size())
			if (me().Army().MyGroundUnitsAhead() > me().Army().HisGroundUnitsAhead() + 3)
				return true;

	return false;
}


bool GoliathRush::Detection() const
{
///	if (ai()->Frame() >= 7000) return true;

	if (me().Units(Terran_Vulture).size() > 2 * him().AllUnits(Terran_Vulture).size())
		if (him().AllUnits(Terran_Goliath).size() > 2 * him().AllUnits(Terran_Siege_Tank_Tank_Mode).size())
			if (him().AllUnits(Terran_Goliath).size() >= 2)
				return true;

	return false;
}


void GoliathRush::OnFrame_v()
{
	if (ai()->GetStrategy()->Active<Stone>())
		return Discard();

	if (him().IsProtoss() || him().IsZerg()) return Discard();

//	if (me().Units(Terran_Goliath).size() >= 4) return Discard();

	if (m_detected)
	{
		if (me().CompletedBuildings(Terran_Command_Center) >= 2) return Discard();

		for (const auto & b : me().Buildings(Terran_Starport))
			if (!b->Completed())
				if (b->CanAcceptCommand())
					DO_ONCE
						return b->CancelConstruction();

		static frame_t lastCancel = 0;
		if (ai()->Frame() - lastCancel > 3*bw->getRemainingLatencyFrames())
		for (const auto & b : me().Buildings(Terran_Starport))
			if (b->Unit()->isTraining())
				if (b->CanAcceptCommand())
				{
				///	bw << "CancelTrain Wraith" << endl;
				///	ai()->SetDelay(5000);
					lastCancel = ai()->Frame();
					return b->CancelTrain();
				}
/*
		for (HisUnit * wraith : him().Units(Terran_Wraith))
			if (findMyClosestBase(wraith->Pos(), 4))
				for (const auto & u : me().Units(Terran_Marine))
					if (!u->GetBehavior()->IsVChasing())
						u->ChangeBehavior<VChasing>(u.get(), wraith, bool("dontFlee"));
*/
	}
	else
	{
		if (me().Bases().size() >= 2) return Discard();
//		if (me().CreationCount(Terran_Wraith) >= 3) return Discard();

		if (Detection())
		{
		///	bw << Name() << " detected!" << endl; ai()->SetDelay(100);

			m_detected = true;
			return;
		}
	}
}


} // namespace iron



