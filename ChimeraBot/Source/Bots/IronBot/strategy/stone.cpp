//////////////////////////////////////////////////////////////////////////
//
// This file is part of Iron's source files.
// Iron is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2016, 2017, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "stone.h"
#include "strategy.h"
#include "zerglingRush.h"
#include "expand.h"
#include "../units/army.h"
#include "../Iron.h"

namespace { auto & bw = Broodwar; }




namespace iron
{


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Stone
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////


Stone::Stone()
{
}


Stone::~Stone()
{
	ai()->GetStrategy()->SetMinScoutingSCVs(1);
}


string Stone::StateDescription() const
{
	if (!m_active) return "-";
	if (m_active) return "active" + string(m_demolish ? " (demolish)" : "");

	return "-";
}


bool Stone::TimeToScout() const
{
	return ai()->Frame() >= m_timeToScout;
}


bool Stone::Demolish() const
{
	return true;
}


bool Stone::Berserk() const
{
	if (m_berserk)
	{
		if (me().SupplyUsed() >= 150)
			return true;
	
		m_berserk = false;
	}
	else if (me().SupplyUsed() >= 180)
	{
		m_berserk = true;
		return true;
	}

	return false;
}


bool Stone::CanExpand() const
{
	if (me().MineralsAvailable() >= m_expandThreshold)
		return true;

	return false;
}


int Stone::SCVsoldiersObjective() const
{
//	if (me().Buildings(Terran_Command_Center).size() < 2) return 0;

	int n = max(3, (int)(me().CompletedUnits(Terran_SCV) * m_soldiersTenth / 10));
	ai()->GetStrategy()->SetMinScoutingSCVs(n);
	return n;
}


void Stone::OnFrame_v()
{
//	if (me().SupplyUsed() >= 10) return Discard();


	if (ai()->Frame() == 0)
	{
		bw << "Stone unchained" << endl;

		if (rand() % 4 == 0)
		{
			if (rand() % 2 == 0)
			{
				m_ironNow = true;
			}
			else
			{
				m_ironAt = 500 +  rand() % 40000;
			///	bw << "iron at " << m_ironAt << endl;
			}
		}

		if (rand() % 3 == 0)
		{
			m_timeToScout = rand() % 4000;
		///	bw << "scout delay = " << m_timeToScout << endl;
		}

		if (rand() % 2 == 0)
		{
			m_soldiersTenth = 5 + rand() % 4;
		///	bw << "soldiers force = " << m_soldiersTenth << endl;
		}

		if (rand() % 2 == 0)
		{
			m_expandThreshold = 150 + 50 * (rand() % 6);
		///	bw << "expand threshold = " << m_expandThreshold << endl;
		}

		if (rand() % 2 == 0)
		{
			m_demolish = true;
		///	bw << "demolish" << endl;
		}
	}


	if (m_ironNow || ai()->Frame() >= m_ironAt)
	{
		bw << "Iron researched" << endl;
		return Discard();
	}

	for (int ironTenth = 5 ; ironTenth <= 5 ; ++ironTenth)
		if (ai()->Frame() == m_ironAt * ironTenth / 10)
			bw << ironTenth << "0%" << endl;

	if (ai()->Frame() >= 5000)
		if (ai()->Frame() % 1000 == 0)
			m_demolish = (rand() % 2 == 0);


	if (m_active)
	{
	}
	else
	{
//		if (me().SupplyUsed() >= 7)
		{
		///	bw << Name() << " active!" << endl; //ai()->SetDelay(100);

			m_active = true;
			return;
		}
	}
}


} // namespace iron



