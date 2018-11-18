#include "McRave.h"

namespace McRave
{
	void ProductionManager::onFrame()
	{
		Display().startClock();
		updateReservedResources();
		updateProduction();
		Display().performanceTest(__FUNCTION__);
	}

	void ProductionManager::updateProduction()
	{
		for (auto &b : Buildings().getMyBuildings()) {
			BuildingInfo &building = b.second;
			if (!building.unit() || !building.unit()->isCompleted() || Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
				continue;

			bool latencyIdle = building.getRemainingTrainFrames() < Broodwar->getRemainingLatencyFrames() || building.unit()->isIdle();

			if (latencyIdle && (!building.getType().isResourceDepot() || (building.getType() != UnitTypes::Zerg_Hatchery && building.getType().getRace() == Races::Zerg))) {
				idleProduction.erase(building.unit());
				idleUpgrade.erase(building.unit());
				idleTech.erase(building.unit());

				produce(building);
				research(building);
				upgrade(building);
			}

			// CC/Nexus
			else if (building.getType().isResourceDepot() && latencyIdle) {
				for (auto &unit : building.getType().buildsWhat()) {
					if (unit.isAddon() && !building.unit()->getAddon() && BuildOrder().getItemQueue().find(unit) != BuildOrder().getItemQueue().end() && BuildOrder().getItemQueue().at(unit).getActualCount() > Broodwar->self()->visibleUnitCount(unit)) {
						building.unit()->buildAddon(unit);
						continue;
					}
					if (unit.isWorker() && Broodwar->self()->completedUnitCount(unit) < 75 && (isAffordable(unit) || unit == UnitTypes::Terran_SCV) && (!Resources().isGasSaturated() || !Resources().isMinSaturated())) {
						building.unit()->train(unit);
						building.setRemainingTrainFrame(unit.buildTime());
					}
				}
			}

			// MadMix
			if (Broodwar->self()->getRace() == Races::Zerg)
				MadMix(building);
		}
	}

	void ProductionManager::MadMix(BuildingInfo& building)
	{
		// Check current overlord count
		int ovie = 0;
		for (auto &u : Units().getMyUnits()) {
			UnitInfo &unit = u.second;
			if (unit.getType() == UnitTypes::Zerg_Egg && unit.unit()->getBuildType() == UnitTypes::Zerg_Overlord)
				ovie++;
		}

		if (building.unit()->getLarva().size() == 0)
			idleProduction[building.unit()] = UnitTypes::Zerg_Larva;
		else
			idleProduction.erase(building.unit());

		for (auto &larva : building.unit()->getLarva()) {

			if ((Broodwar->self()->visibleUnitCount(UnitTypes::Zerg_Overlord) + ovie - 1) < min(22, (int)floor((Units().getSupply() / max(14, (16 - Broodwar->self()->allUnitCount(UnitTypes::Zerg_Overlord))))))) {
				building.unit()->morph(UnitTypes::Zerg_Overlord);
				ovie++;
			}
			else {
				double best = 0.0;
				UnitType typeBest;

				for (auto &type : Strategy().getUnitScores()) {
					UnitType unit = type.first;
					double mineral = unit.mineralPrice() > 0 ? max(0.0, min(2.0, double(Broodwar->self()->minerals() - reservedMineral - Buildings().getQueuedMineral()) / (double)unit.mineralPrice())) : 1.0;
					double gas = unit.gasPrice() > 0 ? max(0.0, min(2.0, double(Broodwar->self()->gas() - reservedGas - Buildings().getQueuedGas()) / (double)unit.gasPrice())) : 1.0;
					double score = Strategy().getUnitScore(unit);
					double value = score * mineral * gas;

					if (BuildOrder().isUnitUnlocked(type.first) && value > best && isCreateable(building.unit(), type.first) && (isAffordable(type.first) || type.first == Strategy().getHighestUnitScore()) && isSuitable(type.first)) {
						best = value, typeBest = type.first;
					}
				}

				if (typeBest != UnitTypes::None) {
					if (isAffordable(typeBest)) {
						larva->morph(typeBest);
						return;	// Only produce 1 unit per frame to allow for scores to be re-calculated
					}
					else {
						idleProduction[building.unit()] = typeBest;
						reservedMineral += typeBest.mineralPrice();
						reservedGas += typeBest.gasPrice();
					}
				}
				else if (BuildOrder().isRush() && BuildOrder().isOpener())
					larva->morph(UnitTypes::Zerg_Zergling);
				else if (isCreateable(building.unit(), UnitTypes::Zerg_Drone) && isAffordable(UnitTypes::Zerg_Drone) && isSuitable(UnitTypes::Zerg_Drone))
					larva->morph(UnitTypes::Zerg_Drone);
			}
		}

	}

	void ProductionManager::produce(BuildingInfo& building)
	{
		double best = 0.0;
		UnitType bestType = UnitTypes::None;
		for (auto &unit : building.getType().buildsWhat()) {

			double mineral = unit.mineralPrice() > 0 ? max(0.0, min(1.0, double(Broodwar->self()->minerals() - reservedMineral - Buildings().getQueuedMineral()) / (double)unit.mineralPrice())) : 1.0;
			double gas = unit.gasPrice() > 0 ? max(0.0, min(1.0, double(Broodwar->self()->gas() - reservedGas - Buildings().getQueuedGas()) / (double)unit.gasPrice())) : 1.0;
			double score = max(0.01, Strategy().getUnitScore(unit));
			double value = score * mineral * gas;

			if (unit.isAddon() && BuildOrder().getItemQueue().find(unit) != BuildOrder().getItemQueue().end() && BuildOrder().getItemQueue().at(unit).getActualCount() > Broodwar->self()->visibleUnitCount(unit)) {
				building.unit()->buildAddon(unit);
				break;
			}

			// If we teched to DTs, try to create as many as possible
			if (unit == UnitTypes::Protoss_Dark_Templar && BuildOrder().getTechList().size() == 1 && isCreateable(building.unit(), unit) && isSuitable(unit)) {				
				best = DBL_MAX;
				bestType = unit;
			}
			else if (unit == BuildOrder().getTechUnit() && isCreateable(building.unit(), unit) && isSuitable(unit) && Broodwar->self()->visibleUnitCount(unit) == 0) {
				best = DBL_MAX;
				bestType = unit;
			}
			else if (unit == UnitTypes::Protoss_Observer && isCreateable(building.unit(), unit) && isSuitable(unit) && Broodwar->self()->visibleUnitCount(unit) < Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Nexus)) {
				best = DBL_MAX;
				bestType = unit;
			}
			else if (value >= best && isCreateable(building.unit(), unit) && isSuitable(unit) && (isAffordable(bestType) || BuildOrder().getTechUnit() == bestType || BuildOrder().getTechList().find(bestType) != BuildOrder().getTechList().end())) {
				best = value;
				bestType = unit;
			}
		}

		if (bestType != UnitTypes::None) {

			// If we can afford it, train it
			if (isAffordable(bestType)) {
				building.unit()->train(bestType);
				building.setRemainingTrainFrame(bestType.buildTime());
				idleProduction.erase(building.unit());				
			}

			if (bestType == UnitTypes::Protoss_Dark_Templar && !isAffordable(UnitTypes::Protoss_Dark_Templar) && Players().vP() && Broodwar->self()->minerals() > 300)
				bestType = UnitTypes::Protoss_Zealot;

			// Else if this is a tech unit, add it to idle production
			else if (BuildOrder().getTechUnit() == bestType || BuildOrder().getTechList().find(bestType) != BuildOrder().getTechList().end()) {
				if (Units().getSupply() < 380)
					idleFrame = Broodwar->getFrameCount();

				idleProduction[building.unit()] = bestType;
				reservedMineral += bestType.mineralPrice();
				reservedGas += bestType.gasPrice();
			}			
		}
	}

	void ProductionManager::research(BuildingInfo& building)
	{
		for (auto &research : building.getType().researchesWhat()) {
			if (isCreateable(research) && isSuitable(research)) {
				if (isAffordable(research))
					building.unit()->research(research), idleTech.erase(building.unit());
				else
					idleTech[building.unit()] = research;
				reservedMineral += research.mineralPrice();
				reservedGas += research.gasPrice();
			}
		}
	}

	void ProductionManager::upgrade(BuildingInfo& building)
	{
		for (auto &upgrade : building.getType().upgradesWhat()) {
			if (isCreateable(upgrade) && isSuitable(upgrade)) {
				if (isAffordable(upgrade))
					building.unit()->upgrade(upgrade), idleUpgrade.erase(building.unit());
				else
					idleUpgrade[building.unit()] = upgrade;
				reservedMineral += upgrade.mineralPrice();
				reservedGas += upgrade.gasPrice();
			}
		}
	}

	bool ProductionManager::isAffordable(UnitType unit)
	{
		// If tech
		if (BuildOrder().getTechList().find(unit) != BuildOrder().getTechList().end()) {

			// If a tech unit and we can afford it including buildings queued
			if (Broodwar->self()->minerals() >= unit.mineralPrice() + Buildings().getQueuedMineral() && (Broodwar->self()->gas() >= unit.gasPrice() + Buildings().getQueuedGas() || unit.gasPrice() == 0)) {
				return true;
			}
		}
		// If we can afford it including buildings queued and tech units queued
		else if (Broodwar->self()->minerals() >= unit.mineralPrice() + reservedMineral + Buildings().getQueuedMineral() && (Broodwar->self()->gas() >= unit.gasPrice() + reservedGas + Buildings().getQueuedGas() || unit.gasPrice() == 0)) {
			return true;
		}
		return false;
	}

	bool ProductionManager::isAffordable(TechType tech)
	{
		return Broodwar->self()->minerals() >= tech.mineralPrice() && Broodwar->self()->gas() >= tech.gasPrice();
	}

	bool ProductionManager::isAffordable(UpgradeType upgrade)
	{
		return Broodwar->self()->minerals() >= upgrade.mineralPrice() && Broodwar->self()->gas() >= upgrade.gasPrice();
	}

	bool ProductionManager::isCreateable(Unit building, UnitType unit)
	{
		if (!BuildOrder().isUnitUnlocked(unit))
			return false;

		switch (unit)
		{
			// Gateway Units
		case UnitTypes::Enum::Protoss_Zealot:
			return true;
		case UnitTypes::Enum::Protoss_Dragoon:
			return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Cybernetics_Core) > 0;
		case UnitTypes::Enum::Protoss_Dark_Templar:
			return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Templar_Archives) > 0;
		case UnitTypes::Enum::Protoss_High_Templar:
			return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Templar_Archives) > 0;

			// Robo Units
		case UnitTypes::Enum::Protoss_Shuttle:
			return true;
		case UnitTypes::Enum::Protoss_Reaver:
			return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Robotics_Support_Bay) > 0;
		case UnitTypes::Enum::Protoss_Observer:
			return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Observatory) > 0;

			// Stargate Units
		case UnitTypes::Enum::Protoss_Corsair:
			return true;
		case UnitTypes::Enum::Protoss_Scout:
			return true;
		case UnitTypes::Enum::Protoss_Carrier:
			return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Fleet_Beacon) > 0;
		case UnitTypes::Enum::Protoss_Arbiter:
			return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Arbiter_Tribunal) > 0;

			// Barracks Units
		case UnitTypes::Enum::Terran_Marine:
			return true;
		case UnitTypes::Enum::Terran_Firebat:
			return Broodwar->self()->completedUnitCount(UnitTypes::Terran_Academy) > 0;
		case UnitTypes::Enum::Terran_Medic:
			return Broodwar->self()->completedUnitCount(UnitTypes::Terran_Academy) > 0;
		case UnitTypes::Enum::Terran_Ghost:
			return Broodwar->self()->completedUnitCount(UnitTypes::Terran_Covert_Ops) > 0;
		case UnitTypes::Enum::Terran_Nuclear_Missile:
			return Broodwar->self()->completedUnitCount(UnitTypes::Terran_Covert_Ops) > 0;

			// Factory Units
		case UnitTypes::Enum::Terran_Vulture:
			return true;
		case UnitTypes::Enum::Terran_Siege_Tank_Tank_Mode:
			return building->getAddon() != nullptr ? true : false;
		case UnitTypes::Enum::Terran_Goliath:
			return (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Armory) > 0);

			// Starport Units
		case UnitTypes::Enum::Terran_Wraith:
			return true;
		case UnitTypes::Enum::Terran_Valkyrie:
			return (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Armory) > 0 && building->getAddon() != nullptr) ? true : false;
		case UnitTypes::Enum::Terran_Battlecruiser:
			return (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Physics_Lab) && building->getAddon() != nullptr) ? true : false;
		case UnitTypes::Enum::Terran_Science_Vessel:
			return (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Science_Facility) > 0 && building->getAddon() != nullptr) ? true : false;
		case UnitTypes::Enum::Terran_Dropship:
			return building->getAddon() != nullptr ? true : false;

		case UnitTypes::Enum::Zerg_Drone:
			return true;
		case UnitTypes::Enum::Zerg_Zergling:
			return (Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Spawning_Pool) > 0);
		case UnitTypes::Enum::Zerg_Hydralisk:
			return (Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Hydralisk_Den) > 0);
		case UnitTypes::Enum::Zerg_Mutalisk:
			return (Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Spire) > 0);
		case UnitTypes::Enum::Zerg_Scourge:
			return (Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Spire) > 0);
		case UnitTypes::Enum::Zerg_Ultralisk:
			return (Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Ultralisk_Cavern) > 0);
		}
		return false;
	}

	bool ProductionManager::isCreateable(UpgradeType upgrade)
	{
		for (auto &unit : upgrade.whatUses())
			if (BuildOrder().isUnitUnlocked(unit) && Broodwar->self()->getUpgradeLevel(upgrade) != upgrade.maxRepeats() && !Broodwar->self()->isUpgrading(upgrade)) return true;
		return false;
	}

	bool ProductionManager::isCreateable(TechType tech)
	{
		for (auto &unit : tech.whatUses())
			if (BuildOrder().isUnitUnlocked(unit) && !Broodwar->self()->hasResearched(tech) && !Broodwar->self()->isResearching(tech)) return true;
		return false;
	}

	bool ProductionManager::isSuitable(UnitType unit)
	{
		using namespace UnitTypes;

		if (unit.isWorker()) {
			if (Broodwar->self()->completedUnitCount(unit) < 90 && (!Resources().isMinSaturated() || !Resources().isGasSaturated()))
				return true;
			else
				return false;
		}

		if (unit.getRace() == Races::Zerg)
			return true;

		bool needReavers = false;
		bool needShuttles = false;

		// Determine whether we want reavers or shuttles;
		if (!Strategy().needDetection()) {
			if ((Terrain().isIslandMap() && Broodwar->self()->visibleUnitCount(unit) < 2 * Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Nexus))
				|| (Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Reaver) > (Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Shuttle) * 2))
				|| (Broodwar->mapFileName().find("Great Barrier") != string::npos && Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Shuttle) < 1))
				needShuttles = true;
			if (!Terrain().isIslandMap() || (Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Reaver) <= (Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Shuttle) * 2)))
				needReavers = true;
		}

		// HACK: Want x reavers before a shuttle
		if (Players().vP() && Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Reaver) < (2 + int(Strategy().getEnemyBuild() == "P4Gate")))
			needShuttles = false;

		switch (unit)
		{
			// Gateway Units
		case Protoss_Zealot:
			return true;
		case Protoss_Dragoon:
			return true;
		case Protoss_Dark_Templar:
			return Broodwar->self()->visibleUnitCount(unit) < 4;
		case Protoss_High_Templar:
			return Broodwar->self()->visibleUnitCount(unit) < 10 && (Broodwar->self()->hasResearched(TechTypes::Psionic_Storm) || Broodwar->self()->isResearching(TechTypes::Psionic_Storm));

			// Robo Units
		case Protoss_Shuttle:
			return needShuttles;
		case Protoss_Reaver:
			return needReavers;
		case Protoss_Observer:
			return Broodwar->self()->visibleUnitCount(unit) < 1 + (Units().getSupply() / 100);

			// Stargate Units
		case Protoss_Corsair:
			return Broodwar->self()->visibleUnitCount(unit) < (10 + (Terrain().isIslandMap() * 10));
		case Protoss_Scout:
			return true;
		case Protoss_Carrier:
			return true;
		case Protoss_Arbiter:
			return (Broodwar->self()->visibleUnitCount(unit) < 10 && (Broodwar->self()->isUpgrading(UpgradeTypes::Khaydarin_Core) || Broodwar->self()->getUpgradeLevel(UpgradeTypes::Khaydarin_Core)));

			// Barracks Units
		case Terran_Marine:
			return true;
		case Terran_Firebat:
			return true;
		case Terran_Medic:
			return Broodwar->self()->completedUnitCount(unit) * 4 < Broodwar->self()->completedUnitCount(Terran_Marine);
		case Terran_Ghost:
			return BuildOrder().getCurrentBuild() == "TNukeMemes";
		case Terran_Nuclear_Missile:
			return BuildOrder().getCurrentBuild() == "TNukeMemes";

			// Factory Units
		case Terran_Vulture:
			return true;
		case Terran_Siege_Tank_Tank_Mode:
			return true;
		case Terran_Goliath:
			return true;

			// Starport Units
		case Terran_Wraith:
			return BuildOrder().getCurrentBuild() == "T2PortWraith";
		case Terran_Valkyrie:
			return BuildOrder().getCurrentBuild() == "T2PortWraith";
		case Terran_Battlecruiser:
			return true;
		case Terran_Science_Vessel:
			return Broodwar->self()->visibleUnitCount(unit) < 6;
		case Terran_Dropship:
			return Broodwar->self()->visibleUnitCount(unit) <= 0;
		}
		return false;
	}

	bool ProductionManager::isSuitable(UpgradeType upgrade)
	{
		using namespace UpgradeTypes;

		// If this is a specific unit upgrade, check if it's unlocked
		if (upgrade.whatUses().size() == 1) {
			for (auto &unit : upgrade.whatUses()) {
				if (!BuildOrder().isUnitUnlocked(unit))
					return false;
			}
		}

		// If this isn't the first tech/upgrade and we don't have our first tech/upgrade
		if (upgrade != BuildOrder().getFirstUpgrade()) {
			if (BuildOrder().getFirstUpgrade() != UpgradeTypes::None && Broodwar->self()->getUpgradeLevel(BuildOrder().getFirstUpgrade()) <= 0 && !Broodwar->self()->isUpgrading(BuildOrder().getFirstUpgrade()))
				return false;
			if (BuildOrder().getFirstTech() != TechTypes::None && !Broodwar->self()->hasResearched(BuildOrder().getFirstTech()) && !Broodwar->self()->isResearching(BuildOrder().getFirstTech()))
				return false;
		}

		// If we're playing Protoss, check Protoss upgrades
		if (Broodwar->self()->getRace() == Races::Protoss) {
			switch (upgrade) {
				// Energy upgrades
			case Khaydarin_Amulet:
				return (Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Assimilator) >= 4 && Broodwar->self()->hasResearched(TechTypes::Psionic_Storm));
			case Khaydarin_Core:
				return true;

				// Range upgrades
			case Singularity_Charge:
				return Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Dragoon) >= 1;

				// Sight upgrades
			case Apial_Sensors:
				return (Broodwar->self()->minerals() > 1500 && Broodwar->self()->gas() > 1000);
			case Sensor_Array:
				return (Broodwar->self()->minerals() > 1500 && Broodwar->self()->gas() > 1000);

				// Capacity upgrades
			case Carrier_Capacity:
				return true;
			case Reaver_Capacity:
				return (Broodwar->self()->minerals() > 1500 && Broodwar->self()->gas() > 1000);
			case Scarab_Damage:
				return (Broodwar->self()->minerals() > 1500 && Broodwar->self()->gas() > 1000);

				// Speed upgrades
			case Gravitic_Drive:
				return Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Shuttle) > 0;
			case Gravitic_Thrusters:
				return Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Scout) > 0;
			case Gravitic_Boosters:
				return (Broodwar->self()->minerals() > 1500 && Broodwar->self()->gas() > 1000);
			case Leg_Enhancements:
				return (Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Nexus) >= 2);

				// Ground unit upgrades
			case Protoss_Ground_Weapons:
				return !Terrain().isIslandMap() && (Units().getSupply() > 120 || Players().getNumberZerg() > 0);
			case Protoss_Ground_Armor:
				return !Terrain().isIslandMap() && (Broodwar->self()->getUpgradeLevel(UpgradeTypes::Protoss_Ground_Weapons) > Broodwar->self()->getUpgradeLevel(UpgradeTypes::Protoss_Ground_Armor) || Broodwar->self()->isUpgrading(UpgradeTypes::Protoss_Ground_Weapons));
			case Protoss_Plasma_Shields:
				return (Broodwar->self()->getUpgradeLevel(UpgradeTypes::Protoss_Ground_Weapons) >= 2 && Broodwar->self()->getUpgradeLevel(UpgradeTypes::Protoss_Ground_Armor) >= 2);

				// Air unit upgrades
			case Protoss_Air_Weapons:
				return (Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Corsair) > 0 || Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Scout) > 0);
			case Protoss_Air_Armor:
				return Broodwar->self()->getUpgradeLevel(UpgradeTypes::Protoss_Air_Weapons) > Broodwar->self()->getUpgradeLevel(UpgradeTypes::Protoss_Air_Armor);
			}
		}

		else if (Broodwar->self()->getRace() == Races::Terran) {
			switch (upgrade) {
			case Ion_Thrusters:
				return true;
			case Charon_Boosters:
				return Strategy().getUnitScore(UnitTypes::Terran_Goliath) > 1.00;
			case U_238_Shells:
				return Broodwar->self()->hasResearched(TechTypes::Stim_Packs);
			case Terran_Infantry_Weapons:
				return (BuildOrder().isBioBuild());
			case Terran_Infantry_Armor:
				return (Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Infantry_Weapons) > Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Infantry_Armor) || Broodwar->self()->isUpgrading(UpgradeTypes::Terran_Infantry_Weapons));

			case Terran_Vehicle_Weapons:
				return (Units().getGlobalAllyGroundStrength() > 20.0);
			case Terran_Vehicle_Plating:
				return (Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Vehicle_Weapons) > Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Vehicle_Plating) || Broodwar->self()->isUpgrading(UpgradeTypes::Terran_Vehicle_Weapons));
			case Terran_Ship_Weapons:
				return (Units().getGlobalAllyAirStrength() > 20.0);
			case Terran_Ship_Plating:
				return (Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Ship_Weapons) > Broodwar->self()->getUpgradeLevel(UpgradeTypes::Terran_Ship_Plating) || Broodwar->self()->isUpgrading(UpgradeTypes::Terran_Ship_Weapons));
			}
		}

		else if (Broodwar->self()->getRace() == Races::Zerg) {
			switch (upgrade)
			{
			case Metabolic_Boost:
				return true;
			case Grooved_Spines:
				return true;
			case Muscular_Augments:
				return Broodwar->self()->getUpgradeLevel(UpgradeTypes::Grooved_Spines);
			case Pneumatized_Carapace:
				return (Units().getSupply() > 160);
			case Anabolic_Synthesis:
				return true;
			case Adrenal_Glands:
				return true;

				// Ground unit upgrades
			case Zerg_Melee_Attacks:
				return (Units().getSupply() > 120);
			case Zerg_Missile_Attacks:
				return false;
			case Zerg_Carapace:
				return (Units().getSupply() > 120);

				// Air unit upgrades
			case Zerg_Flyer_Attacks:
				return (Units().getSupply() > 120);
			case Zerg_Flyer_Carapace:
				return (Units().getSupply() > 120);
			}
		}
		return false;
	}

	bool ProductionManager::isSuitable(TechType tech)
	{
		using namespace TechTypes;

		// If this is a specific unit tech, check if it's unlocked
		if (tech.whatUses().size() == 1) {
			for (auto &unit : tech.whatUses()) {
				if (!BuildOrder().isUnitUnlocked(unit))
					return false;
			}
		}

		// If this isn't the first tech/upgrade and we don't have our first tech/upgrade
		if (tech != BuildOrder().getFirstTech()) {
			if (BuildOrder().getFirstUpgrade() != UpgradeTypes::None && Broodwar->self()->getUpgradeLevel(BuildOrder().getFirstUpgrade()) <= 0 && !Broodwar->self()->isUpgrading(BuildOrder().getFirstUpgrade()))
				return false;
			if (BuildOrder().getFirstTech() != TechTypes::None && !Broodwar->self()->hasResearched(BuildOrder().getFirstTech()) && !Broodwar->self()->isResearching(BuildOrder().getFirstTech()))
				return false;
		}

		if (Broodwar->self()->getRace() == Races::Protoss) {
			switch (tech) {
			case Psionic_Storm:
				return true;
			case Stasis_Field:
				return Broodwar->self()->getUpgradeLevel(UpgradeTypes::Khaydarin_Core) > 0;
			case Recall:
				return (Broodwar->self()->minerals() > 1500 && Broodwar->self()->gas() > 1000);
			case Disruption_Web:
				return (Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Corsair) >= 10);
			}
		}

		else if (Broodwar->self()->getRace() == Races::Terran) {
			switch (tech) {
			case Stim_Packs:
				return BuildOrder().isBioBuild();
			case Spider_Mines:
				return Broodwar->self()->getUpgradeLevel(UpgradeTypes::Ion_Thrusters) > 0 || Broodwar->self()->isUpgrading(UpgradeTypes::Ion_Thrusters);
			case Tank_Siege_Mode:
				return Broodwar->self()->hasResearched(TechTypes::Spider_Mines) || Broodwar->self()->isResearching(TechTypes::Spider_Mines) || Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Siege_Tank_Tank_Mode) > 0;
			case Cloaking_Field:
				return Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Wraith) >= 2;
			case Yamato_Gun:
				return Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Battlecruiser) >= 0;
			case Personnel_Cloaking:
				return Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Ghost) >= 2;
			}
		}

		else if (Broodwar->self()->getRace() == Races::Zerg) {
			switch (tech) {
			case Lurker_Aspect:
				return true;
			}
		}
		return false;
	}

	void ProductionManager::updateReservedResources()
	{
		// Reserved minerals for idle buildings, tech and upgrades
		reservedMineral = 0, reservedGas = 0;

		for (auto &b : idleProduction) {
			reservedMineral += b.second.mineralPrice();
			reservedGas += b.second.gasPrice();
		}

		for (auto &t : idleTech) {
			reservedMineral += t.second.mineralPrice();
			reservedGas += t.second.gasPrice();
		}

		for (auto &u : idleUpgrade) {
			reservedMineral += u.second.mineralPrice();
			reservedGas += u.second.gasPrice();
		}
		return;
	}
}