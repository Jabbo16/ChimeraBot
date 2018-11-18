//////////////////////////////////////////////////////////////////////////
//
// This file is part of Iron's source files.
// Iron is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2016, 2017, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#ifndef MASS_HYDRA_H
#define MASS_HYDRA_H

#include <BWAPI.h>
#include "strat.h"
#include "../defs.h"
#include "../ironutils.h"


namespace iron
{

class MyUnit;
	
//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
//                                  class MassHydra
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
//

class MassHydra : public Strat
{
public:
									MassHydra();

	string							Name() const override { return "MassHydra"; }
	string							StateDescription() const override;

	bool							Active() const		{ return m_active; }
	bool							CanExpand() const;

private:
	void							OnFrame_v() override;
	bool							Detection() const;

	bool							m_active = false;
//	mutable bool					m_canExpand = false;
	mutable frame_t					m_lastNeedVultureFrame = 0;

};


} // namespace iron


#endif

