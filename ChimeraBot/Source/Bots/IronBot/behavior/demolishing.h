//////////////////////////////////////////////////////////////////////////
//
// This file is part of Iron's source files.
// Iron is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2016, 2017, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef DEMOLISHING_H
#define DEMOLISHING_H

#include <BWAPI.h>
#include "behavior.h"
#include "../ironutils.h"


namespace iron
{

class MyUnit;
class HisBuilding;

//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class Demolishing
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class Demolishing : public Behavior<MyUnit>
{
public:
	static const vector<Demolishing *> &	Instances()					{ return m_Instances; }

								Demolishing(MyUnit * pAgent, HisBuilding * pTarget);
								~Demolishing();

	enum state_t {reachingTarget, destroying};

	const Demolishing *			IsDemolishing() const override			{ return this; }
	Demolishing *				IsDemolishing() override				{ return this; }

	string						Name() const override				{ return "demolishing"; }
	string						StateName() const override;


	BWAPI::Color				GetColor() const override			{CI(this); return Colors::Blue; }
	Text::Enum					GetTextColor() const override		{CI(this); return Text::Blue; }

	void						OnFrame_v() override;

	bool						CanRepair(const MyBWAPIUnit * , int) const override	{ return false; }
	bool						CanChase(const HisUnit * ) const override			{ return true; }

	state_t						State() const						{CI(this); return m_state; }
	HisBuilding *				Target() const						{ return m_pTarget; }

private:
	void						OnOtherBWAPIUnitDestroyed_v(BWAPIUnit * other) override;
	void						ChangeState(state_t st);
	void						OnFrame_reachingTarget();
	void						OnFrame_destroying();

	state_t						m_state = reachingTarget;
	frame_t						m_inStateSince;
	HisBuilding *				m_pTarget;
	int							m_hisBuildingCount;

	static vector<Demolishing *>	m_Instances;
};



} // namespace iron


#endif

