#pragma once
#include <BWAPI.h>
#include "Singleton.h"
#include <sstream>

using namespace BWAPI;
using namespace std;

namespace McRave
{
	class Item
	{
		int actualCount, reserveCount;
	public:
		Item() {};

		Item(int actual, int reserve = -1) {
			actualCount = actual, reserveCount = (reserve == -1 ? actual : reserve);
		}

		int const getReserveCount() { return reserveCount; }
		int const getActualCount() { return actualCount; }
	};

	class BuildOrderManager
	{
		map <UnitType, Item> itemQueue;
		bool getOpening, getTech, bioBuild, wallNat, wallMain, scout, productionSat, techSat;
		bool fastExpand, proxy, hideTech, playPassive, rush;
		UpgradeType firstUpgrade;
		TechType firstTech;
		UnitType techUnit;
		UnitType productionUnit;
		set <UnitType> techList, unlockedType;
		vector <string> buildNames;
		string currentBuild = "None";
		string currentVariant = "";
		stringstream ss;
		int satVal, prodVal, techVal, baseVal;
		int gasLimit = INT_MAX;
		int zealotLimit = INT_MAX;
		int dragoonLimit = INT_MAX;

		void getDefaultBuild();
		bool isBuildAllowed(Race, string);
		bool isBuildPossible(string);
		void updateBuild();

		void getNewTech();
		void checkNewTech();
		void checkAllTech();
		void checkExoticTech();

		void checkUnitLimits();

	public:
		bool shouldAddProduction(), shouldAddGas(), techComplete();
		bool shouldExpand();
		map<UnitType, Item>& getItemQueue() { return itemQueue; }

		UnitType getTechUnit() { return techUnit; }
		string getCurrentBuild() { return currentBuild; }
		string getCurrentVariant() { return currentVariant; }
		UpgradeType getFirstUpgrade() { return firstUpgrade; }
		TechType getFirstTech() { return firstTech; }
		set <UnitType>& getTechList() { return techList; }	
		set <UnitType>& getUnlockedList() { return unlockedType; }

		int buildCount(UnitType);
		int gasWorkerLimit() { return gasLimit; }

		bool isUnitUnlocked(UnitType unit) { return (unlockedType.find(unit) != unlockedType.end()); }
		bool isOpener() { return getOpening; }
		bool isBioBuild() { return bioBuild; }
		bool isFastExpand() { return fastExpand; }
		bool shouldScout() { return scout; }
		bool isWallNat() { return wallNat; }
		bool isWallMain() { return wallMain; }
		bool isProxy() { return proxy; }
		bool isHideTech() { return hideTech; }
		bool isPlayPassive() { return playPassive; }
		bool isRush() { return rush; }

		bool firstReady();

		void onEnd(bool), onStart(), onFrame();
		void protossOpener(), protossTech(), protossSituational(), protossUnlocks(), protossIslandPlay();
		void terranOpener(), terranTech(), terranSituational();
		void zergOpener(), zergTech(), zergSituational();										
	
		void PScoutMemes(), PDWEBMemes(), PArbiterMemes();	// Gimmick builds	

		void PFFE();									// FFE - "http://liquipedia.net/starcraft/Protoss_FE_(vs._Zerg)"		
		void PZZCore(), PZCore(), PNZCore();			// 1Gate Core - "http://liquipedia.net/starcraft/1_Gate_Core_(vs._Protoss)"	
		void PProxy99();								// 2Gate Proxy - "http://liquipedia.net/starcraft/2_Gateway_(vs._Zerg)"		
		void PProxy6();
		void P4Gate();									// 4Gate - "http://liquipedia.net/starcraft/4_Gate_Goon_(vs._Protoss)"
		void P2GateExpand();							// 2Gate Expand		
		void P12Nexus();								// 12 Nexus - "http://liquipedia.net/starcraft/12_Nexus"		
		void P21Nexus();								// 21 Nexus - "http://liquipedia.net/starcraft/21_Nexus"		
		void PDTExpand();								// DT Expand - "http://liquipedia.net/starcraft/DT_Fast_Expand_(vs._Terran)"		
		void P2GateDragoon();							// 2 Gate Dragoon - "http://liquipedia.net/starcraft/10/15_Gates_(vs._Terran)"		
		void P1GateRobo();								// 1 Gate Robo - "http://liquipedia.net/starcraft/1_Gate_Reaver"
		void P3Nexus();									// Triple Nexus
		void PZealotDrop();								
		void P1GateCorsair();							// 1 Gate Corsair

		void Reaction2GateDefensive();
		void Reaction2GateAggresive();
		void Reaction2Gate();
		void Reaction4Gate();

		void T2Fact();
		void TSparks();
		void T2PortWraith();
		void T1RaxFE();
		void TNukeMemes();
		void TBCMemes();
		void T2RaxFE();
		void T1FactFE();

		void Z2HatchMuta();
		void Z3HatchLing();
		void Z4Pool();
		void Z9Pool();
		void Z2HatchHydra();
		void Z3HatchBeforePool();
		void ZLurkerTurtle();
	};

}

typedef Singleton<McRave::BuildOrderManager> BuildOrderSingleton;