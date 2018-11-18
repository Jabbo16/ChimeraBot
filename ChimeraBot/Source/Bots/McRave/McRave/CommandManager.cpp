#include "McRave.h"

namespace McRave
{
	void CommandManager::onFrame()
	{
		Display().startClock();
		updateEnemyCommands();
		updateGoals();
		updateAlliedUnits();
		Display().performanceTest(__FUNCTION__);
	}

	void CommandManager::updateEnemyCommands()
	{
		// Clear cache
		enemyCommands.clear();

		// Store bullets as enemy units if they can affect us too
		for (auto &b : Broodwar->getBullets()) {
			if (b && b->exists()) {
				if (b->getType() == BulletTypes::Psionic_Storm)
					addCommand(nullptr, b->getPosition(), TechTypes::Psionic_Storm, true);

				if (b->getType() == BulletTypes::EMP_Missile)
					addCommand(nullptr, b->getTargetPosition(), TechTypes::EMP_Shockwave, true);
			}
		}

		// Store enemy detection and assume casting orders
		for (auto &u : Units().getEnemyUnits()) {
			UnitInfo& unit = u.second;

			if (!unit.unit() || (unit.unit()->exists() && (unit.unit()->isLockedDown() || unit.unit()->isMaelstrommed() || unit.unit()->isStasised() || !unit.unit()->isCompleted())))
				continue;

			if (unit.getType().isDetector())
				Commands().addCommand(unit.unit(), unit.getPosition(), unit.getType(), true);

			if (unit.unit()->exists()) {
				Order enemyOrder = unit.unit()->getOrder();
				Position enemyTarget = unit.unit()->getOrderTargetPosition();

				if (enemyOrder == Orders::CastEMPShockwave)
					Commands().addCommand(unit.unit(), enemyTarget, TechTypes::EMP_Shockwave, true);
				if (enemyOrder == Orders::CastPsionicStorm)
					Commands().addCommand(unit.unit(), enemyTarget, TechTypes::Psionic_Storm, true);
			}
		}

		// Store nuke dots
		for (auto &dot : Broodwar->getNukeDots())
			Commands().addCommand(nullptr, dot, TechTypes::Nuclear_Strike, true);
	}

	void CommandManager::updateGoals()
	{
		// Defend my expansions
		for (auto &s : Stations().getMyStations()) {
			Station station = s.second;
			BuildingInfo& base = Buildings().getMyBuildings().at(s.first);

			if (station.BWEMBase()->Location() != mapBWEB.getNaturalTile() && station.BWEMBase()->Location() != mapBWEB.getMainTile() && Grids().getDefense(station.BWEMBase()->Location()) == 0) {

				myGoals[station.ResourceCentroid()] = 0.1;
				UnitInfo* rangedUnit = Util().getClosestAllyUnit(base, Filter::GetType == UnitTypes::Protoss_Dragoon || Filter::GetType == UnitTypes::Terran_Siege_Tank_Tank_Mode);

				if (rangedUnit)
					rangedUnit->setDestination(station.ResourceCentroid());
			}
			else if (myGoals.find(station.ResourceCentroid()) != myGoals.end())
				myGoals.erase(station.ResourceCentroid());
		}

		// Attack enemy expansions with a small force
		// PvP / PvT
		if (!Players().vZ()) {
			auto distBest = 0.0;
			auto posBest = Positions::Invalid;
			for (auto &s : Stations().getEnemyStations()) {
				Station station = s.second;
				auto pos = station.BWEMBase()->Center();
				auto dist = pos.getDistance(Terrain().getEnemyStartingPosition());
				if (dist > distBest) {
					distBest = dist;
					posBest = pos;
				}
			}
			assignClosestToGoal(posBest, vector<UnitType> {UnitTypes::Protoss_Dragoon, 4});
		}

		// Secure our own future expansion position
		// PvT
		Position nextExpand(Buildings().getCurrentExpansion());
		if (nextExpand.isValid() && Players().vT()) {
			UnitType building = Broodwar->self()->getRace().getResourceDepot();
			if (BuildOrder().buildCount(building) > Broodwar->self()->visibleUnitCount(building)) {
				assignClosestToGoal(nextExpand, vector<UnitType> { UnitTypes::Protoss_Dragoon, 2 });
			}
		}

		// Deny enemy expansions
		if (Players().vT() && Stations().getMyStations().size() >= 3 && Stations().getMyStations().size() > Stations().getEnemyStations().size() && Terrain().getEnemyExpand().isValid() && Units().getSupply() >= 200)
			assignClosestToGoal((Position)Terrain().getEnemyExpand(), vector<UnitType> { UnitTypes::Protoss_Dragoon, 4 });

		// Send lurkers to expansions for fun
		if (Broodwar->self()->getRace() == Races::Zerg && !Stations().getMyStations().empty()) {
			auto lurkerPerBase = Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Lurker) / Stations().getMyStations().size();

			for (auto &base : Stations().getMyStations()) {
				assignClosestToGoal(base.second.ResourceCentroid(), vector<UnitType> { UnitTypes::Zerg_Lurker, UnitTypes::Zerg_Lurker, UnitTypes::Zerg_Lurker, UnitTypes::Zerg_Lurker });
			}
		}
	}

	void CommandManager::assignClosestToGoal(Position here, vector<UnitType> types)
	{
		// TODO: Remake this so that it grabs the correct number of units
		// Right now we only grab 1 type of unit, but may want multiple types later
		// Form concave and set destination
		map<double, UnitInfo*> unitByDist;

		for (auto &type : types) {
			for (auto &u : Units().getMyUnits()) {
				UnitInfo &unit = u.second;

				if (unit.unit() && unit.getType() == type)
					unitByDist[unit.getPosition().getDistance(here)] = &u.second;
			}
		}

		int i = 0;
		for (auto &u : unitByDist) {
			UnitInfo* unit = u.second;
			if (find(types.begin(), types.end(), u.second->getType()) != types.end() && !u.second->getDestination().isValid()) {
				u.second->setDestination(here);
				myGoals[here] += u.second->getVisibleGroundStrength();
				i++;
			}
			if (i == int(types.size()))
				break;
		}
	}

	void CommandManager::updateAlliedUnits()
	{
		myCommands.clear();
		for (auto &u : Units().getMyUnits()) {
			UnitInfo &unit = u.second;

			bool attackCooldown = (Broodwar->getFrameCount() - unit.getLastAttackFrame() <= unit.getMinStopFrame() - Broodwar->getRemainingLatencyFrames());
			//if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
				//continue;

			if (!unit.unit() || !unit.unit()->exists()																							// Prevent crashes	
				|| unit.getType() == UnitTypes::Protoss_Shuttle || unit.getType() == UnitTypes::Terran_Dropship 								// Transports have their own commands	
				|| unit.getType() == UnitTypes::Protoss_Interceptor																				// Interceptors don't need commands
				|| unit.unit()->isLockedDown() || unit.unit()->isMaelstrommed() || unit.unit()->isStasised() || !unit.unit()->isCompleted()		// If the unit is locked down, maelstrommed, stassised, or not completed	
				|| (unit.getType() != UnitTypes::Protoss_Carrier && attackCooldown))															// If the unit is not ready to perform an action after an attack (certain units have minimum frames after an attack before they can receive a new command)
				continue;

			// Temp variables to use
			if (unit.hasTarget()) {
				widths = unit.getTarget().getType().tileWidth() * 16.0 + unit.getType().tileWidth() * 16.0;
				allyRange = widths + (unit.getTarget().getType().isFlyer() ? unit.getAirRange() : unit.getGroundRange());
				enemyRange = widths + (unit.getType().isFlyer() ? unit.getTarget().getAirRange() : unit.getTarget().getGroundRange());
			}

			// Unstick a unit
			if (unit.isStuck() && unit.unit()->isMoving())
				unit.unit()->stop();

			// Units targeted by splash need to move away from the army
			else if (Units().getSplashTargets().find(unit.unit()) != Units().getSplashTargets().end()) {
				if (unit.hasTarget() && unit.unit()->getGroundWeaponCooldown() <= 0 && unit.hasTarget() && unit.getTarget().unit()->exists())
					attack(unit);
				else
					approach(unit);
			}

			// If this unit should use a special ability that requires a command
			else if (shouldUseSpecial(unit))
				continue;

			// If this unit should engage
			else if (unit.shouldEngage()) {
				unit.circleGreen();
				if (shouldAttack(unit))
					attack(unit);
				else if (shouldApproach(unit))
					approach(unit);
				else if (shouldKite(unit))
					kite(unit);
			}

			// If this unit should retreat
			else if (unit.shouldRetreat()) {
				if (shouldDefend(unit))
					defend(unit);
				else
					kite(unit);
			}

			else {
				unit.circleYellow();
				move(unit);
			}
		}
		return;
	}

	bool CommandManager::shouldAttack(UnitInfo& unit)
	{
		if (unit.getType() == UnitTypes::Zerg_Lurker)
			return false;

		if (unit.getType() == UnitTypes::Protoss_Carrier) {
			for (auto &i : unit.unit()->getInterceptors()) {
				if (!i || !i->isCompleted()) continue;
				UnitInfo interceptor = Units().getMyUnits()[i];
				if (Broodwar->getFrameCount() - interceptor.getLastAttackFrame() > 100)
					return true;
			}
			return false;
		}

		if ((!unit.getTarget().getType().isFlyer() && unit.unit()->getGroundWeaponCooldown() < Broodwar->getRemainingLatencyFrames())
			|| (unit.getTarget().getType().isFlyer() && unit.unit()->getAirWeaponCooldown() < Broodwar->getRemainingLatencyFrames())
			|| unit.getType() == UnitTypes::Terran_Medic)
			return true;
		return false;
	}

	bool CommandManager::shouldKite(UnitInfo& unit)
	{
		if (unit.getType() == UnitTypes::Protoss_Carrier || unit.getType() == UnitTypes::Zerg_Mutalisk)
			return true;

		else if (unit.getType() == UnitTypes::Protoss_Corsair)
			return false;

		else if (unit.getType() == UnitTypes::Protoss_Zealot && Terrain().getNaturalWall() && Position(Terrain().getNaturalWall()->getDoor()).getDistance(unit.getPosition()) < 96.0)
			return false;

		if ((unit.getPosition().getDistance(Terrain().getDefendPosition()) < 128.0 && allyRange <= 64.0 && Strategy().defendChoke())
			|| (unit.getTarget().getType().isBuilding()))
			return false;

		if (((unit.getTarget().isBurrowed() || unit.getTarget().unit()->isCloaked()) && !unit.getTarget().unit()->isDetected())		// Invisible Unit
			|| (unit.getType() == UnitTypes::Protoss_Reaver && unit.getTarget().getGroundRange() < unit.getGroundRange())			// Reavers always kite units with lower range
			|| (unit.getType() == UnitTypes::Terran_Vulture)																		// Vultures always kite
			|| (unit.getType() == UnitTypes::Zerg_Mutalisk)																			// Mutas always kite
			|| (unit.getType() == UnitTypes::Protoss_Carrier)
			|| (allyRange >= 32.0 && unit.unit()->isUnderAttack() && allyRange >= enemyRange)										// Ranged unit under attack by unit with lower range
			|| ((enemyRange <= allyRange && unit.unit()->getDistance(unit.getTarget().getPosition()) <= allyRange - enemyRange)))	// Ranged unit fighting lower range unit and not at max range
			return true;
		return false;
	}

	bool CommandManager::shouldApproach(UnitInfo& unit)
	{
		if (!unit.hasTarget())
			return false;

		if (unit.getSimValue() >= 10.0 && unit.getType() != UnitTypes::Protoss_Reaver && (!unit.getTarget().getType().isWorker() || unit.getGroundRange() <= 32))
			return true;

		if (unit.getType() == UnitTypes::Protoss_Carrier || unit.getType() == UnitTypes::Zerg_Mutalisk)
			return false;

		if (unit.getGroundRange() < 32 && unit.getTarget().getType().isWorker())
			return true;

		if (unit.getType() == UnitTypes::Zerg_Lurker)
			return true;

		if ((unit.getGroundRange() < unit.getTarget().getGroundRange() && !unit.getTarget().getType().isBuilding() && Grids().getMobility(WalkPosition(unit.getEngagePosition())) > 0)																					// Approach slower units with higher range
			|| (unit.getType() != UnitTypes::Terran_Battlecruiser && unit.getType() != UnitTypes::Zerg_Guardian && unit.getType().isFlyer() && unit.getTarget().getType() != UnitTypes::Zerg_Scourge))																												// Small flying units approach other flying units except scourge
			return true;
		return false;
	}

	bool CommandManager::shouldDefend(UnitInfo& unit)
	{
		bool isSafe = (!unit.getType().isFlyer() && Grids().getEGroundThreat(unit.getWalkPosition()) == 0.0) || (unit.getType().isFlyer() && Grids().getEAirThreat(unit.getWalkPosition()) == 0.0);
		bool closeToDefend = Terrain().getDefendPosition().getDistance(unit.getPosition()) < 320.0 || Terrain().isInAllyTerritory(unit.getTilePosition()) || Terrain().isInAllyTerritory((TilePosition)unit.getDestination()) || (!unit.getType().isFlyer() && !mapBWEM.GetArea(unit.getTilePosition()));
		if (/*(!unit.hasTarget() || !Units().isThreatening(unit.getTarget())) &&*/ isSafe && closeToDefend)
			return true;
		return false;
	}

	void CommandManager::move(UnitInfo& unit)
	{
		// If it's a tank, make sure we're unsieged before moving - TODO: Check that target has velocity and > 512 or no velocity and < tank range
		if (unit.hasTarget() && unit.getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode && unit.unit()->getDistance(unit.getTarget().getPosition()) > 512 && unit.getTarget().getSpeed() > 0.0)
			unit.unit()->unsiege();

		else if (unit.getDestination().isValid()) {

			if (!Terrain().isInEnemyTerritory((TilePosition)unit.getDestination())) {
				Position bestPosition = Util().getConcavePosition(unit, mapBWEM.GetArea(TilePosition(unit.getDestination())));

				if (bestPosition.isValid() && (bestPosition != unit.getPosition() || unit.unit()->getLastCommand().getType() == UnitCommandTypes::None)) {
					if (unit.unit()->getLastCommand().getTargetPosition() != Position(bestPosition) || unit.unit()->getLastCommand().getType() != UnitCommandTypes::Move)
						unit.unit()->move(Position(bestPosition));
				}
			}
			else if(unit.unit()->getLastCommand().getTargetPosition() != Position(unit.getDestination()) || unit.unit()->getLastCommand().getType() != UnitCommandTypes::Move)
				unit.unit()->move(unit.getDestination());
		}

		// If unit has a transport, move to it or load into it
		else if (unit.getTransport() && unit.getTransport()->exists()) {
			if (!isLastCommand(unit, UnitCommandTypes::Right_Click_Unit, unit.getTransport()->getPosition()))
				unit.unit()->rightClick(unit.getTransport());
		}

		// If target doesn't exist, move towards it
		else if (unit.hasTarget() && unit.getTarget().getPosition().isValid() && Grids().getMobility(WalkPosition(unit.getEngagePosition())) > 0 && (unit.getPosition().getDistance(unit.getTarget().getPosition()) < 320.0 || unit.getType().isFlyer())) {
			if (!isLastCommand(unit, UnitCommandTypes::Move, unit.getTarget().getPosition()))
				unit.unit()->move(unit.getTarget().getPosition());
		}

		else if (Terrain().getAttackPosition().isValid() && !unit.getType().isFlyer()) {
			if (!isLastCommand(unit, UnitCommandTypes::Move, Terrain().getAttackPosition()))
				unit.unit()->move(Terrain().getAttackPosition());
		}

		else if (unit.getType().isFlyer() && !unit.hasTarget() && unit.getPosition().getDistance(unit.getSimPosition()) < 640.0)
			kite(unit);

		// If no target and no enemy bases, move to a base location (random if we have found the enemy once already)
		else if (unit.unit()->isIdle()) {
			if (Terrain().getEnemyStartingPosition().isValid())
				unit.unit()->move(Terrain().randomBasePosition());
			else {
				for (auto &start : Broodwar->getStartLocations()) {
					if (start.isValid() && !Broodwar->isExplored(start) && !Commands().overlapsCommands(unit.unit(), unit.getType(), Position(start), 32)) {
						unit.unit()->move(Position(start));
						Commands().addCommand(unit.unit(), Position(start), unit.getType());
						return;
					}
				}
				// Catch in case no valid movement positions
				unit.unit()->move(Terrain().closestUnexploredStart());
			}
		}
	}

	void CommandManager::defend(UnitInfo& unit)
	{
		unit.circleRed();

		// HACK: Hardcoded cannon surround, testing
		if (unit.getType().isWorker() && Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Photon_Cannon) > 0) {
			auto enemy = Util().getClosestUnit(unit.getPosition(), Broodwar->enemy());
			if (enemy) {
				auto cannon = Util().getClosestAllyBuilding(*enemy, Filter::GetType == UnitTypes::Protoss_Photon_Cannon);
				if (cannon) {
					double distBest = DBL_MAX;
					auto walkBest = WalkPositions::Invalid;
					auto start = cannon->getWalkPosition();
					for (int x = start.x - 2; x < start.x + 10; x++) {
						for (int y = start.y - 2; y < start.y + 10; y++) {
							WalkPosition w(x, y);
							double dist = Position(w).getDistance(mapBWEM.Center());
							if (dist < distBest && Util().isMobile(unit.getWalkPosition(), w, unit.getType())) {
								distBest = dist;
								walkBest = w;
							}
						}
					}

					if (walkBest.isValid() && unit.unit()->getLastCommand().getTargetPosition() != Position(walkBest))
						unit.unit()->move(Position(walkBest));
					return;
				}
			}
		}

		// HACK: Flyers defend above a base
		// TODO: Choose a base instead of closest to enemy, sometimes fly over a base I dont own
		if (unit.getType().isFlyer()) {
			if (Terrain().getEnemyStartingPosition().isValid() && mapBWEB.getMainPosition().getDistance(Terrain().getEnemyStartingPosition()) < mapBWEB.getNaturalPosition().getDistance(Terrain().getEnemyStartingPosition()))
				unit.unit()->move(mapBWEB.getMainPosition());
			else
				unit.unit()->move(mapBWEB.getNaturalPosition());
		}

		else if (unit.getDestination().isValid()) {
			Position bestPosition = Util().getConcavePosition(unit, mapBWEM.GetArea(TilePosition(unit.getDestination())));

			if (bestPosition.isValid() && (bestPosition != unit.getPosition() || unit.unit()->getLastCommand().getType() == UnitCommandTypes::None)) {
				if (unit.unit()->getLastCommand().getTargetPosition() != bestPosition || unit.unit()->getLastCommand().getType() != UnitCommandTypes::Move)
					unit.unit()->move(bestPosition);
			}
		}

		//else if (!Strategy().defendChoke()) {
		//	// Get defend position
		//	Position bestPosition = Util().getConcavePosition(unit, nullptr, (Position)mapBWEB.getMainChoke()->Center());
		//	if (bestPosition.isValid()) {
		//		if (unit.unit()->getLastCommand().getTargetPosition() != bestPosition || unit.unit()->getLastCommand().getType() != UnitCommandTypes::Move) {
		//			unit.unit()->move(bestPosition);
		//		}
		//		unit.setDestination(bestPosition);
		//		addCommand(unit.unit(), bestPosition, UnitTypes::None);
		//	}			
		//}
		else {
			// Get defend position
			Position bestPosition = Util().getConcavePosition(unit, nullptr, Terrain().getDefendPosition());
			if (bestPosition.isValid()) {
				if (unit.getPosition().getDistance(bestPosition) > 16.0)
					unit.unit()->move(bestPosition);				
				else
					addCommand(unit.unit(), bestPosition, UnitTypes::None);
				unit.setDestination(bestPosition);
			}

			Broodwar->drawLineMap(unit.getPosition(), bestPosition, Colors::Purple);
		}
	}

	void CommandManager::kite(UnitInfo& unit)
	{
		// If unit has a transport, move to it or load into it - TODO: add scarab count upgrade check
		// HACK: This is just for Reavers atm, add HT before AIIDE hopefully
		if (unit.getType() == UnitTypes::Protoss_Reaver) {
			if (unit.getTransport() && unit.getTransport()->exists() && unit.unit()->getScarabCount() != 5 && Grids().getEGroundThreat(unit.getWalkPosition()) == 0.0) {
				if (!isLastCommand(unit, UnitCommandTypes::Right_Click_Unit, unit.getTransport()->getPosition()))
					unit.unit()->rightClick(unit.getTransport());
				return;
			}
		}

		// Workers use mineral fields to help with drilling
		if (unit.getType().isWorker() && Terrain().isInAllyTerritory(unit.getTilePosition())) {
			Unit mineral = unit.unit()->getClosestUnit(Filter::IsMineralField);
			if (mineral) {
				unit.unit()->gather(mineral);
				return;
			}
		}

		// Mechanical units run to SCVs to be repaired
		if (unit.getType().isMechanical() && unit.getPercentHealth() < 1.00 && unit.hasTarget() && unit.getPosition().getDistance(unit.getTarget().getPosition()) > 400) {
			UnitInfo* scv = Util().getClosestAllyUnit(unit, Filter::GetType == UnitTypes::Terran_SCV);
			if (scv) {
				unit.unit()->move(scv->getPosition());
				return;
			}
		}

		// Get kite position
		Position posBest = Util().getKitePosition(unit);

		// If a valid position was found that isn't the starting position
		if (posBest.isValid() && posBest != unit.getPosition()) {
			Grids().updateAllyMovement(unit.unit(), (WalkPosition)posBest);
			unit.setDestination(posBest);
			if (!isLastCommand(unit, UnitCommandTypes::Move, posBest))
				unit.unit()->move(posBest);
		}
	}

	void CommandManager::attack(UnitInfo& unit)
	{
		bool newOrder = Broodwar->getFrameCount() - unit.unit()->getLastCommandFrame() > Broodwar->getRemainingLatencyFrames();

		if (unit.getTarget().unit() && !unit.getTarget().unit()->exists())
			move(unit);

		// DT hold position vs spider mines
		else if (unit.getType() == UnitTypes::Protoss_Dark_Templar && unit.hasTarget() && unit.getTarget().getType() == UnitTypes::Terran_Vulture_Spider_Mine && unit.getTarget().hasTarget() && unit.getTarget().getTarget().unit() == unit.unit() && !unit.getTarget().isBurrowed()) {
			if (unit.unit()->getLastCommand().getType() != UnitCommandTypes::Hold_Position)
				unit.unit()->holdPosition();
		}

		// If close enough, set the current area as a psi storm area so units move and storms don't overlap
		else if (unit.getType() == UnitTypes::Protoss_High_Templar) {
			if (unit.getPosition().getDistance(unit.getTarget().getPosition()) <= 640 && !Commands().overlapsCommands(unit.unit(), TechTypes::Psionic_Storm, unit.getTarget().getPosition(), 96) && unit.unit()->getEnergy() >= 75 && (Grids().getEGroundCluster(unit.getTarget().getWalkPosition()) + Grids().getEAirCluster(unit.getTarget().getWalkPosition())) >= STORM_LIMIT) {
				unit.unit()->useTech(TechTypes::Psionic_Storm, unit.getTarget().unit());
				addCommand(unit.unit(), unit.getTarget().getPosition(), TechTypes::Psionic_Storm);
			}
			else if (unit.getPosition().getDistance(unit.getTarget().getPosition()) > 640.0)
				move(unit);
			else
				kite(unit);
		}

		else if (unit.getType() == UnitTypes::Terran_Medic) {
			if (!isLastCommand(unit, UnitCommandTypes::Attack_Move, unit.getTarget().getPosition()))
				unit.unit()->attack(unit.getTarget().getPosition());
		}

		//// HACK: intercept enemys scout movement
		//else if (Terrain().isInAllyTerritory(unit.getTarget().getTilePosition()) && unit.getTarget().getType().isWorker() && !Util().unitInRange(unit) && unit.getTarget().unit()->exists())
		//	unit.unit()->move(unit.getTarget().unit()->getOrderTargetPosition());

		// If unit is currently not attacking his assigned target, or last command isn't attack
		else if (unit.unit()->getLastCommand().getType() != UnitCommandTypes::Attack_Unit || unit.unit()->getLastCommand().getTarget() != unit.getTarget().unit())
			unit.unit()->attack(unit.getTarget().unit());

		// If unit can receive a new order and current order is wrong, re-issue
		else if (newOrder && unit.unit()->getOrderTarget() != unit.getTarget().unit())
			unit.unit()->attack(unit.getTarget().unit());

	}

	void CommandManager::approach(UnitInfo& unit)
	{
		if (unit.hasTarget() && unit.getTarget().getPosition().isValid() && !isLastCommand(unit, UnitCommandTypes::Move, unit.getTarget().getPosition()))
			unit.unit()->move(unit.getTarget().getPosition());
	}

	bool CommandManager::isLastCommand(UnitInfo& unit, UnitCommandType thisCommand, Position thisPosition)
	{
		if (unit.unit()->getLastCommand().getType() == thisCommand && unit.unit()->getLastCommand().getTargetPosition() == thisPosition)
			return true;
		return false;
	}

	bool CommandManager::overlapsCommands(Unit unit, TechType tech, Position here, int radius)
	{
		for (auto &command : myCommands) {
			if (command.unit != unit && command.tech == tech && command.pos.getDistance(here) <= radius * 2)
				return true;
		}
		return false;
	}

	bool CommandManager::overlapsCommands(Unit unit, UnitType type, Position here, int radius)
	{
		for (auto &command : myCommands) {
			if (command.unit != unit && command.type == type && command.pos.getDistance(here) <= radius * 2)
				return true;
		}
		return false;
	}

	bool CommandManager::overlapsAllyDetection(Position here)
	{
		for (auto &command : myCommands) {
			if (command.type == UnitTypes::Spell_Scanner_Sweep) {
				double range = 420.0;
				if (command.pos.getDistance(here) < range)
					return true;
			}
			else if (command.type.isDetector()) {
				double range = command.type.isBuilding() ? 256.0 : 360.0;
				if (command.pos.getDistance(here) < range)
					return true;
			}
		}
		return false;
	}

	bool CommandManager::overlapsEnemyDetection(Position here)
	{
		for (auto &command : enemyCommands) {
			if (command.type == UnitTypes::Spell_Scanner_Sweep) {
				double range = 420.0;
				double ff = 32;
				if (command.pos.getDistance(here) < range + ff)
					return true;
			}

			else if (command.type.isDetector()) {
				double range = command.type.isBuilding() ? 256.0 : 360.0;
				double ff = 32;
				if (command.pos.getDistance(here) < range + ff)
					return true;
			}
		}
		return false;
	}

	bool CommandManager::isInDanger(Position here)
	{
		for (auto &command : myCommands) {
			if (command.tech == TechTypes::Psionic_Storm && here.getDistance(command.pos) <= 128)
				return true;
			if (command.tech == TechTypes::Disruption_Web && here.getDistance(command.pos) <= 128)
				return true;
			if (command.tech == TechTypes::EMP_Shockwave && here.getDistance(command.pos) <= 128)
				return true;
			if (command.tech == TechTypes::Nuclear_Strike && here.getDistance(command.pos) <= 640)
				return true;
		}

		for (auto &command : enemyCommands) {
			if (command.tech == TechTypes::Psionic_Storm && here.getDistance(command.pos) <= 128)
				return true;
			if (command.tech == TechTypes::Disruption_Web && here.getDistance(command.pos) <= 128)
				return true;
			if (command.tech == TechTypes::EMP_Shockwave && here.getDistance(command.pos) <= 128)
				return true;
			if (command.tech == TechTypes::Nuclear_Strike && here.getDistance(command.pos) <= 640)
				return true;
		}

		return false;
	}

	bool CommandManager::isInDanger(UnitInfo& unit)
	{
		return isInDanger(unit.getPosition());
	}
}