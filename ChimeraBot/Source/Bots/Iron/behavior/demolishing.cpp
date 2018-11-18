//////////////////////////////////////////////////////////////////////////
//
// This file is part of Iron's source files.
// Iron is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2016, 2017, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "demolishing.h"
#include "fleeing.h"
#include "walking.h"
#include "defaultBehavior.h"
#include "../units/bunker.h"
#include "../strategy/strategy.h"
#include "../strategy/zerglingRush.h"
#include "../Iron.h"

namespace { auto & bw = Broodwar; }


namespace iron
{


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Demolishing
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

vector<Demolishing *> Demolishing::m_Instances;


Demolishing::Demolishing(MyUnit * pAgent, HisBuilding * pTarget)
	: Behavior(pAgent, behavior_t::Demolishing), m_pTarget(pTarget)
{
	assert_throw(pAgent->Is(Terran_SCV));


	PUSH_BACK_UNCONTAINED_ELEMENT(m_Instances, this);

	m_inStateSince = ai()->Frame();
}


Demolishing::~Demolishing()
{CI(this);
#if !DEV
	try //3
#endif
	{
		assert_throw(contains(m_Instances, this));
		really_remove(m_Instances, this);
	}
#if !DEV
	catch(...){} //3
#endif
}


void Demolishing::ChangeState(state_t st)
{CI(this);
	assert_throw(m_state != st);
	
	m_state = st; OnStateChanged();
}


string Demolishing::StateName() const
{CI(this);
	switch(State())
	{
	case reachingTarget:	return "reachingTarget";
	case destroying:		return "destroying";
	default:				return "?";
	}
}


void Demolishing::OnOtherBWAPIUnitDestroyed_v(BWAPIUnit * other)
{CI(this);
	if (other == Target())
	{
		m_pTarget = nullptr;
		Agent()->ChangeBehavior<DefaultBehavior>(Agent());
	}
}

void Demolishing::OnFrame_v()
{CI(this);
#if DEV
	drawLineMap(Agent()->Pos(), Target()->Pos(), GetColor());
#endif

	if (!Agent()->CanAcceptCommand()) return;

	if (Target()->Flying()) { m_pTarget = nullptr; return Agent()->ChangeBehavior<DefaultBehavior>(Agent()); }

	if (m_hisBuildingCount != (int)ai()->Him().Buildings().size())
		{ m_pTarget = nullptr; return Agent()->ChangeBehavior<DefaultBehavior>(Agent()); }

	switch (State())
	{
	case reachingTarget:	OnFrame_reachingTarget(); break;
	case destroying:		OnFrame_destroying(); break;
	default: assert_throw(false);
	}
}


void Demolishing::OnFrame_reachingTarget()
{CI(this);
	if (JustArrivedInState())
	{
		SetSubBehavior<Walking>(Agent(), Target()->Pos(), __FILE__ + to_string(__LINE__));
	}

//	if (!findPursuers(Agent()).empty())	return Agent()->ChangeBehavior<Fleeing>(Agent());
	if (Agent()->Life() < Agent()->PrevLife(10)) return Agent()->ChangeBehavior<Fleeing>(Agent());

	if (!findThreats(Agent(), 3*32).empty())
		return Agent()->ChangeBehavior<Fleeing>(Agent());

	if (dist(Agent()->Pos(), Target()->Pos()) < 32*10)
	{
		ResetSubBehavior();
		return ChangeState(destroying);
	}
}


void Demolishing::OnFrame_destroying()
{CI(this);
	if (JustArrivedInState())
	{
		m_inStateSince = ai()->Frame();
	}

	if (Agent()->Life() < Agent()->PrevLife(10))
	{
//		auto Threats2 = findThreats(Agent(), 1*32);
//		if (Threats2.size() >= 1)
//			return Agent()->ChangeBehavior<Chasing>(Agent(), Threats2.back()->His(), !bool("insist"));

		 return Agent()->ChangeBehavior<Fleeing>(Agent());
	}

	int enemies1 = (int)findThreatsButBuildings(Agent(), 1*32).size();
	int enemies2 = (int)findThreatsButBuildings(Agent(), 2*32).size();
	int enemies3 = (int)findThreatsButBuildings(Agent(), 3*32).size();
	if ((enemies1 >= 1) ||
		(enemies2 >= 2) ||
		(enemies3 >= 3) ||
		(enemies2 >= 1) && (Agent()->Life() <= 10) ||
		(enemies3 >= 1) && (Agent()->Life() <= 5))
	{
		return Agent()->ChangeBehavior<Fleeing>(Agent());
	}


	Agent()->Attack(Target());
}




} // namespace iron



