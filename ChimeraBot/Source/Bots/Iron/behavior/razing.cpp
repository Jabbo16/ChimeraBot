//////////////////////////////////////////////////////////////////////////
//
// This file is part of Iron's source files.
// Iron is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2016, 2017, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "razing.h"
#include "repairing.h"
#include "fleeing.h"
#include "kiting.h"
#include "chasing.h"
#include "exploring.h"
#include "defaultBehavior.h"
#include "../strategy/strategy.h"
#include "../strategy/stone.h"
#include "../Iron.h"

namespace { auto & bw = Broodwar; }


namespace iron
{

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Razing
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

vector<Razing *> Razing::m_Instances;


HisBuilding * Razing::FindBuilding(MyUnit * u)
{//return nullptr;
	if (!u->GetArea(check_t::no_check)) return nullptr;
	if (u->GroundAttack() + u->AirAttack() == 0) return nullptr;

	map<HisBuilding *, int> AttackersPerBuilding;
	for (const Razing * razing : Razing::Instances())
		++AttackersPerBuilding[razing->m_pTarget];

	bool berserkSCV = false;
	if (auto s = ai()->GetStrategy()->Active<Stone>())
		if (s->Berserk())
			berserkSCV = true;

	if (auto s = ai()->GetStrategy()->Active<Stone>())
		if (s->Demolish())
		{
			static map<HisBuilding *, frame_t> AttackedSince;

			map<int, HisBuilding *> Candidates;
			for (auto & b : him().Buildings())
				if (!b->InFog())
					if (!b->Flying())
						if (b->GetArea() == u->GetArea())
						{
						///	for (int i = 0 ; i < AttackersPerBuilding[b.get()] ; ++i)
						///		bw->drawCircleMap(b->Pos(), 10 + 2*i, Colors::White);

							if (!(u->GetBehavior() && u->GetBehavior()->IsRazing() && u->GetBehavior()->IsRazing()->m_pTarget == b.get()))
								if (AttackersPerBuilding[b.get()] >= 8) continue;

							int score = b->LifeWithShields();

							if (b->Is(Zerg_Spawning_Pool)) score = 10;
							if (b->Is(Zerg_Sunken_Colony))
								if (!b->Completed())
									score = 20;
								else if (berserkSCV)
									 score = -10;
								else continue;
							if (b->Is(Zerg_Hatchery)) score = 30;
							if (b->Is(Zerg_Creep_Colony)) score = 40;


							if (b->Is(Terran_Bunker))
								if (!b->Completed())
									score = 10;
								else if (berserkSCV)
									 score = -10;
								else continue;
							if (b->Is(Terran_Factory)) score = 10;
							if (b->Is(Terran_Barracks)) score = 20;
							if (b->Is(Terran_Command_Center)) score = 30;
							if (b->Is(Terran_Supply_Depot)) score = 40;


							if (b->Is(Protoss_Pylon)) score = 10;
							if (b->Is(Protoss_Forge)) score = 20;
							if (b->Is(Protoss_Gateway)) score = 30;
							if (b->Is(Protoss_Nexus)) score = 40;

							if (b->LifeWithShields() < 100)
							{
								if (b->Completed()) score = 0;
								else score = b->LifeWithShields();
							}
/*
							if (AttackedSince[b.get()])
								if (!b->PrevDamage())
								{
									if (ai()->Frame() - AttackedSince[b.get()] > 300)
										score += 5000;

									if (ai()->Frame() - AttackedSince[b.get()] > 600)
										continue;
								}
*/
							Candidates[score] = b.get();
						}

			if (!Candidates.empty())
			{
				HisBuilding * pBestTarget = Candidates.begin()->second;
				if (!AttackedSince[pBestTarget]) AttackedSince[pBestTarget] = ai()->Frame();
				return pBestTarget;
			}
		}

	{
		vector<HisBuilding *> Candidates;
		for (const FaceOff & faceOff : u->FaceOffs())
			if (faceOff.DistanceToMyRange() < 6*32)
				if (HisBuilding * pTarget = faceOff.His()->IsHisBuilding())
					if (!pTarget->InFog())
						if (!pTarget->Flying())
							if (!faceOff.HisAttack() || berserkSCV)
								Candidates.push_back(pTarget);

		if (Candidates.empty())
			for (auto & b : him().Buildings())
				if (!b->InFog())
					if (!b->Flying())
						if (b->GetArea() == u->GetArea())
							if (!(u->Flying() ? b->AirAttack() : b->GroundAttack()) || berserkSCV)
								Candidates.push_back(b.get());


		HisBuilding * pBestTarget = nullptr;
		bool mostLife = (u->GetArea() == him().GetArea()) && !u->Is(Terran_SCV);
		int bestTargetLife = mostLife ? numeric_limits<int>::min() : numeric_limits<int>::max();
		for (HisBuilding * b : Candidates)
			if (mostLife == (b->LifeWithShields() > bestTargetLife))
			{
	//			assert_throw(b->Unit()->exists());
				bestTargetLife = b->LifeWithShields();
				pBestTarget = b;
			//	if (b->Type().isResourceDepot()) break;
			}

		return pBestTarget;
	}
}


Razing::Razing(MyUnit * pAgent, const Area * pWhere)
	: Behavior(pAgent, behavior_t::Razing), m_pWhere(pWhere)
{
//	assert_throw(pAgent->Flying() || pWhere->AccessibleFrom(pAgent->GetArea()));
	assert_throw(pAgent->GetArea(check_t::no_check) == pWhere);
//	assert_throw(Razing::Condition(pAgent));

	PUSH_BACK_UNCONTAINED_ELEMENT(m_Instances, this);

	m_inStateSince = ai()->Frame();

//	static bool done = false; if (!done) { done = true; ai()->SetDelay(500); bw << Agent()->NameWithId() << " starts " << Name() << endl; }
}


Razing::~Razing()
{
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


void Razing::ChangeState(state_t st)
{CI(this);
	assert_throw(m_state != st);
	
	m_state = st; OnStateChanged();
}


void Razing::OnOtherBWAPIUnitDestroyed_v(BWAPIUnit * other)
{CI(this);
	if (m_pTarget == other)
		m_pTarget = nullptr;
}


bool Razing::CanRepair(const MyBWAPIUnit * , int) const
{
	if (auto s = ai()->GetStrategy()->Active<Stone>())
		if (s->Demolish())
			return false;

	return true;
}


void Razing::OnFrame_v()
{CI(this);
#if DEV
	if (m_pTarget) drawLineMap(Agent()->Pos(), m_pTarget->Pos(), GetColor());//1
#endif

	bool keepRazing = false;
	if (auto s = ai()->GetStrategy()->Active<Stone>())
		if (s->Demolish())
			keepRazing = true;


	if (!Agent()->CanAcceptCommand()) return;

	if (keepRazing)
	{
		if (Agent()->Life() < Agent()->PrevLife(10))
		{
	//		auto Threats2 = findThreats(Agent(), 1*32);
	//		if (Threats2.size() >= 1)
	//			return Agent()->ChangeBehavior<Chasing>(Agent(), Threats2.back()->His(), !bool("insist"));

			auto Threats2 = findThreatsButBuildings(Agent(), 2*32);
			if (Threats2.size() == 1)
			{
				if (Threats2.front()->FramesToKillHim() < Threats2.front()->FramesToKillMe() - 5)
					return Agent()->ChangeBehavior<Chasing>(Agent(), Threats2.front()->His());
			}

			return Agent()->ChangeBehavior<Fleeing>(Agent());
		}

		if (Agent()->Life() <= Agent()->MaxLife() / 2)
		{
			int enemies1 = (int)findThreatsButBuildings(Agent(), 16).size();
			int enemies2 = (int)findThreatsButBuildings(Agent(), 32).size();
			int enemies3 = (int)findThreatsButBuildings(Agent(), 48).size();
			if ((enemies1 >= 1) ||
				(enemies2 >= 2) ||
				(enemies3 >= 3) ||
				(enemies2 >= 1) && (Agent()->Life() <= 15) ||
				(enemies3 >= 1) && (Agent()->Life() <= 10))
			{
				return Agent()->ChangeBehavior<Fleeing>(Agent());
			}
		}
	}
	else
	{
		if (Agent()->Life() < Agent()->PrevLife(10))
			return Agent()->ChangeBehavior<Fleeing>(Agent());

		if (Agent()->GroundRange() > 3*32)
		{
			if (Kiting::KiteCondition(Agent()) || Kiting::AttackCondition(Agent()))
				return Agent()->ChangeBehavior<Kiting>(Agent());
		}
		else
		{
			auto Threats3 = findThreats(Agent(), 3*32);
			if (!Threats3.empty())
				return Agent()->ChangeBehavior<Fleeing>(Agent());
		}
	}

	if (!keepRazing)
		if (Agent()->Type().isMechanical())
			if (Agent()->Life() < Agent()->MaxLife())
				if (Agent()->RepairersCount() < Agent()->MaxRepairers())
					if (Repairing * pRepairer = Repairing::GetRepairer(Agent(),
								(Agent()->Life()*4 > Agent()->MaxLife()*3) ? 16*32 :
								(Agent()->Life()*4 > Agent()->MaxLife()*2) ? 32*32 :
								(Agent()->Life()*4 > Agent()->MaxLife()*1) ? 64*32 : 1000000))
						return Agent()->ChangeBehavior<Repairing>(Agent(), pRepairer);


	m_pTarget = FindBuilding();
	if (m_pTarget)
	{
		if (m_pLastAttackedTarget != m_pTarget)
			Agent()->Attack(m_pLastAttackedTarget = m_pTarget, check_t::no_check);
	}
	else
		Agent()->ChangeBehavior<Exploring>(Agent(), Where());
}



} // namespace iron



