#include "events.h"
#include <fstream>
#include <iostream>
#include "inputs.h"
#include "rng.h"
#include "enemies.h"
#include "spells.h"
#include "battle.h"
#include "blueprints.h"
#include "armour.h"
#include "resources.h"
#include <thread>
using namespace std;

extern variables g_customVars;
extern resource g_manaName, g_projName;


//Error codes:
// 1: Bad XML, including premature end of file or nonsense values
// 2: Specified blueprint or list not found
// 3: Loading empty slot, not technically an error
// 4: Unable to open blueprint file
// 5: Empty blueprint list
// 8: Exceeded limit for custom variables

void Event::loadFromFile(string blueprint, bool custom) {
	//Reset attributes to default values
	eventName = preBattleSpell = enemyBlueprint = reward = "EMPTY";
	preBattleText = postBattleText = "";
	choices.resize(0);
	for (unsigned char i = 0; i < 18; i++) {
		statChanges[i] = 0;
	}
	varChanges.reset();
	xpChange = 0;
	ifstream events;
	string buffer = "", buffer2="";
	try {
		if (blueprint == "EMPTY") {
			throw 3;
		}
		if (custom) {
			events.open("custom\\events.xml");
		}
		else {
			events.open("data\\events.xml");
		}
		if (!events.is_open()) {
			throw 4;
		}
		string blueprintName = "eventList name=\"" + blueprint + '\"';
		{
			bool noList = false;
			streampos filePos = 0;
			short listCount = -1;
			while (buffer != blueprintName) {
				buffer = getTag(&events);
				ignoreLine(&events);
				if (events.eof()) {
					events.clear();
					noList = true;
					break;
				}
			}
			if (!noList) {
				filePos = events.tellg();
				blueprintName = "/eventList";
				do {
					listCount++;
					buffer = getTag(&events);
					ignoreLine(&events);
				} while (buffer != blueprintName);
				events.clear();
				if (listCount == 0) {
					throw 5;
				}
				listCount = rng(1, listCount);
				events.seekg(filePos);
				for (int i = 1; i < listCount; i++) {
					ignoreLine(&events);
				}
				if (getTag(&events) != "name") {
					throw 1;
				}
				blueprint = stringFromFile(&events);
				if (blueprint == "EMPTY") {
					throw 3;
				}
				if (getTag(&events) != "/name") {
					throw 1;
				}
			}
			events.seekg(0);
			buffer = "";
		}
		blueprintName = "event name=\"" + blueprint + '\"';
		while (buffer != blueprintName) {
			buffer = getTag(&events);
			ignoreLine(&events);
			if (events.eof()) {
				throw 2;
			}
		}
		blueprintName = "/event";
		buffer = getTag(&events);
		short varBuffer = 0;
		while (buffer != blueprintName) {
			if (events.eof()) {
				throw 1;
			}
			if (buffer == "preBattleText") {
				preBattleText = stringFromFile(&events);
			}
			else if (buffer == "preBattleSpell") {
				preBattleSpell = stringFromFile(&events);
			}
			else if (buffer.substr(0, 14) == "enemyBlueprint") {
				buffer.erase(0, 14);
				enemyBlueprint = stringFromFile(&events);
				if (buffer.empty()) {}
				else if (buffer == " first=\"player\"") {
					firstGo = 1;
				}
				else if (buffer == " first=\"enemy\"") {
					firstGo = -1;
				}
				else {
					throw 1;
				}
				buffer = "enemyBlueprint";
			}
			else if (buffer == "postBattleText") {
				postBattleText = stringFromFile(&events);
			}
			else if (buffer == "statChanges") {
				ignoreLine(&events);
				if (!events) {
					throw 1;
				}
				buffer = getTag(&events);
				while (buffer != "/statChanges") {
					if (events.eof()) {
						throw 1;
					}
					if (buffer == "health") {
						statChanges[0] = numFromFile(&events);
					}
					else if (buffer == "mana") {
						statChanges[4] = numFromFile(&events);
					}
					else if (buffer == "projectiles") {
						statChanges[8] = numFromFile(&events);
					}
					else if (buffer == "maxHealth") {
						statChanges[1] = numFromFile(&events);
					}
					else if (buffer == "maxMana") {
						statChanges[5] = numFromFile(&events);
					}
					else if (buffer == "turnManaRegen") {
						statChanges[6] = numFromFile(&events);
					}
					else if (buffer == "battleManaRegen") {
						statChanges[7] = numFromFile(&events);
					}
					else if (buffer == "constRegen") {
						statChanges[2] = numFromFile(&events);
					}
					else if (buffer == "battleRegen") {
						statChanges[3] = numFromFile(&events);
					}
					else if (buffer == "flatArmour") {
						statChanges[9] = numFromFile(&events);
					}
					else if (buffer == "flatMagicArmour") {
						statChanges[10] = numFromFile(&events);
					}
					else if (buffer == "flatDamageModifier") {
						statChanges[11] = numFromFile(&events);
					}
					else if (buffer == "flatMagicDamageModifier") {
						statChanges[12] = numFromFile(&events);
					}
					else if (buffer == "flatArmourPiercingDamageModifier") {
						statChanges[13] = numFromFile(&events);
					}
					else if (buffer == "bonusActions") {
						statChanges[14] = numFromFile(&events);
						if (statChanges[14] > 254) {
							statChanges[14] = 254;
						}
						else if (statChanges[14] < -254) {
							statChanges[14] = -254;
						}
					}
					else if (buffer == "initiative") {
						statChanges[15] = numFromFile(&events);
					}
					else if (buffer == "statPoints") {
						statChanges[16] = numFromFile(&events);
					}
					else if (buffer == "upgradePoints") {
						statChanges[17] = numFromFile(&events);
					}
					else if (buffer == "xpChange") {
						xpChange = numFromFile(&events);
					}
					else {
						throw 1;
					}
					if (getTag(&events) != '/' + buffer) {
						throw 1;
					}
					ignoreLine(&events);
					if (!events) {
						throw 1;
					}
					buffer = getTag(&events);
				}
				ignoreLine(&events);
				if (!events) {
					throw 1;
				}
				buffer = getTag(&events);
				continue;
			}
			else if (buffer == "reward") {
				reward = stringFromFile(&events);
			}
			else if (buffer == "varChanges") {
				ignoreLine(&events);
				if (!events) {
					throw 1;
				}
				buffer = getTag(&events);
				while (buffer != "/varChanges") {
					if (events.eof()) {
						throw 1;
					}
					if (buffer.substr(0, 10) == "var name=\"") {
						buffer.erase(0, 10);
						if (buffer.empty() || buffer.back() != '\"') {
							throw 1;
						}
						buffer.pop_back();
						if (buffer.empty()) {
							throw 1;
						}
						clearSpace(&events);
						varBuffer = numFromFile(&events);
						*varChanges.value(buffer) += varBuffer;
						if (getTag(&events) != "/var") {
							throw 1;
						}
						ignoreLine(&events);
						if (!events) {
							throw 1;
						}
						buffer = getTag(&events);
					}
					else {
						throw 1;
					}
				}
				ignoreLine(&events);
				if (!events) {
					throw 1;
				}
				buffer = getTag(&events);
				continue;
			}
			else if (buffer == "choice") {
				ignoreLine(&events);
				if (choices.size() < SHRT_MAX) {
					choices.emplace_back();
				}
				buffer = getTag(&events);
				while (buffer != "/choice") {
					if (events.eof()) {
						throw 1;
					}
					if (buffer == "text") {
						choices.back().text = stringFromFile(&events);
					}
					else if (buffer == "healthChange") {
						choices.back().healthChange = numFromFile(&events);
					}
					else if (buffer == "manaChange") {
						choices.back().manaChange = numFromFile(&events);
					}
					else if (buffer == "projectileChange") {
						choices.back().projectileChange = numFromFile(&events);
					}
					else if (buffer == "req") {
						choices.back().req = stringFromFile(&events);
					}
					else if (buffer == "hidden/") {
						choices.back().hidden = true;
						ignoreLine(&events);
						if (!events) {
							throw 1;
						}
						buffer = getTag(&events);
						continue;
					}
					else if (buffer == "eventName") {
						choices.back().eventName = stringFromFile(&events);
					}
					else {
						throw 1;
					}
					if (getTag(&events) != '/' + buffer) {
						throw 1;
					}
					ignoreLine(&events);
					if (!events) {
						throw 1;
					}
					buffer = getTag(&events);
				}
				ignoreLine(&events);
				if (!events) {
					throw 1;
				}
				buffer = getTag(&events);
				continue;
			}
			else {
				throw 1;
			}
			if (getTag(&events) != '/' + buffer) {
				throw 1;
			}
			ignoreLine(&events);
			if (!events) {
				throw 1;
			}
			buffer = getTag(&events);
		}
		events.close();
		eventName = blueprint;
	}
	catch (int err) {
		events.close();
		eventName = preBattleSpell = enemyBlueprint = reward = "EMPTY";
		preBattleText = postBattleText = "";
		choices.resize(0);
		varChanges.reset();
		switch (err) {
		case 1:
			cout << "Unable to parse event or eventList " << blueprint << '\n';
			break;
		case 2:
			if (custom) {
				loadFromFile(blueprint, false);
				return;
			}
			cout << "No event or eventList found with name " << blueprint << '\n';
			break;
		case 4:
			if (custom) {
				loadFromFile(blueprint, false);
				return;
			}
			cout << "Could not open events.xml\n";
			break;
		case 5:
			cout << "eventList " << blueprint << " contains no entries\n";
			break;
		case 8:
			cout << "Maximum number of variables reached (32767)\n";
			break;
		}
	}
}

unsigned char Event::eventHandler(player* playerCharacter) {
	static spell eventSpell;
	static enemy opponent;
	if (!preBattleText.empty()) {
		cout << preBattleText << '\n';
		this_thread::sleep_for(chrono::milliseconds(500));
	}
	g_customVars += varChanges;
	eventSpell.loadFromFile(preBattleSpell);
	opponent.loadFromFile(enemyBlueprint);
	if (eventSpell.getReal()) {
		if (spellCast(&eventSpell, playerCharacter) == 1) {
			cout << "You are dead\n";
			return 2;
		}
		this_thread::sleep_for(chrono::milliseconds(500));
	}
	if (opponent.getReal()) { //REVIEW WORDING
		cout << opponent.getIntroduction() << '\n' << "To start the battle, enter 1.\n";
		userChoice(1, 1);
		if (battleHandler(playerCharacter, &opponent, firstGo) == 2) {
			return 2;
		}
		playerCharacter->reset();
		if (playerCharacter->getHealth() <= 0) {
			cout << "You defeat " << opponent.getName() << ", but succumb to your wounds after the battle\n";
			return 2;
		}
		cout << showpos << opponent.getXp() << " experience\n" << noshowpos;
		playerCharacter->giveXp(opponent.getXp());
	}
	if (!postBattleText.empty()) {
		cout << postBattleText << '\n';
		this_thread::sleep_for(chrono::milliseconds(500));
	}
	cout << showpos;
	if (statChanges[0] != 0) {
		playerCharacter->modifyHealth(statChanges[0]);
		cout << statChanges[0] << " health\n";
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[1] > 0) {
		cout << statChanges[1] << " maximum health\n";
		if (playerCharacter->maxHealthBase > SHRT_MAX - statChanges[1]) {
			playerCharacter->maxHealthBase = SHRT_MAX;
		}
		else {
			playerCharacter->maxHealthBase += statChanges[1];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	else if (statChanges[1] < 0) {
		cout << statChanges[1] << " maximum health\n";
		if (playerCharacter->maxHealthBase < SHRT_MIN - statChanges[1]) {
			playerCharacter->maxHealthBase = SHRT_MIN;
		}
		else {
			playerCharacter->maxHealthBase += statChanges[1];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[2] > 0) {
		cout << statChanges[2] << " health per turn\n";
		if (playerCharacter->constRegenBase > SHRT_MAX - statChanges[2]) {
			playerCharacter->constRegenBase = SHRT_MAX;
		}
		else {
			playerCharacter->constRegenBase += statChanges[2];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	else if (statChanges[2] < 0) {
		cout << statChanges[2] << " health per turn\n";
		if (playerCharacter->constRegenBase < SHRT_MIN - statChanges[2]) {
			playerCharacter->constRegenBase = SHRT_MIN;
		}
		else {
			playerCharacter->constRegenBase += statChanges[2];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[3] > 0) {
		cout << statChanges[3] << " health at end of battle\n";
		if (playerCharacter->battleRegenBase > SHRT_MAX - statChanges[3]) {
			playerCharacter->battleRegenBase = SHRT_MAX;
		}
		else {
			playerCharacter->battleRegenBase += statChanges[3];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	else if (statChanges[3] < 0) {
		cout << statChanges[3] << " health at end of battle\n";
		if (playerCharacter->battleRegenBase < SHRT_MIN - statChanges[3]) {
			playerCharacter->battleRegenBase = SHRT_MIN;
		}
		else {
			playerCharacter->battleRegenBase += statChanges[3];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[4] != 0) {
		playerCharacter->modifyMana(statChanges[4]);
		cout << statChanges[1] << ' ';
		if (statChanges[4] == 1 || statChanges[4] == -1) {
			cout << g_manaName.singular() << '\n';
		}
		else {
			cout << g_manaName.plural() << '\n';
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[5] > 0) {
		cout << statChanges[5] << " maximum " << g_manaName.plural() << '\n';
		if (playerCharacter->maxManaBase > SHRT_MAX - statChanges[5]) {
			playerCharacter->maxManaBase = SHRT_MAX;
		}
		else {
			playerCharacter->maxManaBase += statChanges[5];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	else if (statChanges[5] < 0) {
		cout << statChanges[5] << " maximum " << g_manaName.plural() << '\n';
		if (playerCharacter->maxManaBase < SHRT_MIN - statChanges[5]) {
			playerCharacter->maxManaBase = SHRT_MIN;
		}
		else {
			playerCharacter->maxManaBase += statChanges[5];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[6] > 0) {
		if (statChanges[6] == 1) {
			cout << statChanges[6] << ' ' << g_manaName.singular() << " per turn\n";
		}
		else {
			cout << statChanges[6] << ' ' << g_manaName.plural() << " per turn\n";
		}
		if (playerCharacter->turnManaRegenBase > SHRT_MAX - statChanges[6]) {
			playerCharacter->turnManaRegenBase = SHRT_MAX;
		}
		else {
			playerCharacter->turnManaRegenBase += statChanges[6];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	else if (statChanges[6] < 0) {
		if (statChanges[6] == -1) {
			cout << statChanges[6] << ' ' << g_manaName.singular() << " per turn\n";
		}
		else {
			cout << statChanges[6] << ' ' << g_manaName.plural() << " per turn\n";
		}
		if (playerCharacter->turnManaRegenBase < SHRT_MIN - statChanges[6]) {
			playerCharacter->turnManaRegenBase = SHRT_MIN;
		}
		else {
			playerCharacter->turnManaRegenBase += statChanges[6];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[7] > 0) {
		if (statChanges[7] == 1) {
			cout << statChanges[7] << ' ' << g_manaName.singular() << " at end of battle\n";
		}
		else {
			cout << statChanges[7] << ' ' << g_manaName.plural() << " at end of battle\n";
		}
		if (playerCharacter->battleManaRegenBase > SHRT_MAX - statChanges[7]) {
			playerCharacter->battleManaRegenBase = SHRT_MAX;
		}
		else {
			playerCharacter->battleManaRegenBase += statChanges[7];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	else if (statChanges[7] < 0) {
		if (statChanges[7] == -1) {
			cout << statChanges[7] << ' ' << g_manaName.singular() << " at end of battle\n";
		}
		else {
			cout << statChanges[7] << ' ' << g_manaName.plural() << " at end of battle\n";
		}
		if (playerCharacter->battleManaRegenBase < SHRT_MIN - statChanges[7]) {
			playerCharacter->battleManaRegenBase = SHRT_MIN;
		}
		else {
			playerCharacter->battleManaRegenBase += statChanges[7];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[8] != 0) {
		playerCharacter->modifyProjectiles(statChanges[8]);
		cout << statChanges[8] << ' ';
		if (statChanges[8] == 1 || statChanges[8] == -1) {
			cout << g_projName.singular() << '\n';
		}
		else {
			cout << g_projName.plural() << '\n';
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[9] > 0) {
		cout << statChanges[9] << " physical armour\n";
		if (playerCharacter->flatArmourBase > SHRT_MAX - statChanges[9]) {
			playerCharacter->flatArmourBase = SHRT_MAX;
		}
		else {
			playerCharacter->flatArmourBase += statChanges[9];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	else if (statChanges[9] < 0) {
		cout << statChanges[9] << " physical armour\n";
		if (playerCharacter->flatArmourBase < SHRT_MIN - statChanges[9]) {
			playerCharacter->flatArmourBase = SHRT_MIN;
		}
		else {
			playerCharacter->flatArmourBase += statChanges[9];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[10] > 0) {
		cout << statChanges[10] << " magic armour\n";
		if (playerCharacter->flatMagicArmourBase > SHRT_MAX - statChanges[10]) {
			playerCharacter->flatMagicArmourBase = SHRT_MAX;
		}
		else {
			playerCharacter->flatMagicArmourBase += statChanges[10];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	else if (statChanges[10] < 0) {
		cout << statChanges[10] << " magic armour\n";
		if (playerCharacter->flatMagicArmourBase < SHRT_MIN - statChanges[10]) {
			playerCharacter->flatMagicArmourBase = SHRT_MIN;
		}
		else {
			playerCharacter->flatMagicArmourBase += statChanges[10];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[11] > 0) {
		cout << statChanges[11] << " physical damage dealt\n";
		if (playerCharacter->flatDamageModifierBase > SHRT_MAX - statChanges[11]) {
			playerCharacter->flatDamageModifierBase = SHRT_MAX;
		}
		else {
			playerCharacter->flatDamageModifierBase += statChanges[11];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	else if (statChanges[11] < 0) {
		cout << statChanges[11] << " physical damage dealt\n";
		if (playerCharacter->flatDamageModifierBase < SHRT_MIN - statChanges[11]) {
			playerCharacter->flatDamageModifierBase = SHRT_MIN;
		}
		else {
			playerCharacter->flatDamageModifierBase += statChanges[11];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[12] > 0) {
		cout << statChanges[12] << " magic damage dealt\n";
		if (playerCharacter->flatMagicDamageModifierBase > SHRT_MAX - statChanges[12]) {
			playerCharacter->flatMagicDamageModifierBase = SHRT_MAX;
		}
		else {
			playerCharacter->flatMagicDamageModifierBase += statChanges[12];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	else if (statChanges[12] < 0) {
		cout << statChanges[12] << " magic damage dealt\n";
		if (playerCharacter->flatMagicDamageModifierBase < SHRT_MIN - statChanges[12]) {
			playerCharacter->flatMagicDamageModifierBase = SHRT_MIN;
		}
		else {
			playerCharacter->flatMagicDamageModifierBase += statChanges[12];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[13] > 0) {
		cout << statChanges[13] << " armour piercing damage dealt\n";
		if (playerCharacter->flatArmourPiercingDamageModifierBase > SHRT_MAX - statChanges[13]) {
			playerCharacter->flatArmourPiercingDamageModifierBase = SHRT_MAX;
		}
		else {
			playerCharacter->flatArmourPiercingDamageModifierBase += statChanges[13];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	else if (statChanges[13] < 0) {
		cout << statChanges[13] << " armour piercing damage dealt\n";
		if (playerCharacter->flatArmourPiercingDamageModifierBase < SHRT_MIN - statChanges[13]) {
			playerCharacter->flatArmourPiercingDamageModifierBase = SHRT_MIN;
		}
		else {
			playerCharacter->flatArmourPiercingDamageModifierBase += statChanges[13];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[14] != 0) {
		cout << statChanges[14] << " bonus actions\n";
		if (playerCharacter->bonusActionsBase + statChanges[14] > 127) {
			playerCharacter->bonusActionsBase = 127;
		}
		else if (playerCharacter->bonusActionsBase + statChanges[14] < -127) {
			playerCharacter->bonusActionsBase = -127;
		}
		else {
			playerCharacter->bonusActionsBase += statChanges[14];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	if (statChanges[15] > 0) {
		cout << statChanges[15] << " speed\n";
		if (playerCharacter->initiativeBase > SHRT_MAX - statChanges[15]) {
			playerCharacter->initiativeBase = SHRT_MAX;
		}
		else {
			playerCharacter->initiativeBase += statChanges[15];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	else if (statChanges[15] < 0) {
		cout << statChanges[15] << " speed\n";
		if (playerCharacter->initiativeBase < SHRT_MIN - statChanges[15]) {
			playerCharacter->initiativeBase = SHRT_MIN;
		}
		else {
			playerCharacter->initiativeBase += statChanges[15];
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	cout << noshowpos;
	playerCharacter->upgradeStats(statChanges[16]);
	playerCharacter->upgradeItems(statChanges[17]);
	if (xpChange != 0) {
		cout << showpos << xpChange << " experience points\n" << noshowpos;
		playerCharacter->giveXp(xpChange);
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	this_thread::sleep_for(chrono::milliseconds(500));
	switch (blueprintListSelector(&reward)) { //Find type of equipment and its blueprint name
	case 1: //Helmet
	{
		armourHead newHelmet(reward);
		cout << "New headwear: " << newHelmet.getName() << '\n';
		cout << "To view stats, enter 1.\nTo discard, enter 2.\n";
		if (userChoice(1, 2) == 1) {
			playerCharacter->equip(&newHelmet);
		}
		this_thread::sleep_for(chrono::milliseconds(500));
		break;
	}
	case 2: //Chestplate
	{
		armourTorso newChestplate(reward);
		cout << "New chestwear: " << newChestplate.getName() << '\n';
		cout << "To view stats, enter 1.\nTo discard, enter 2.\n";
		if (userChoice(1, 2) == 1) {
			playerCharacter->equip(&newChestplate);
		}
		this_thread::sleep_for(chrono::milliseconds(500));
		break;
	}
	case 3: //Legs
	{
		armourLegs newGreaves(reward);
		cout << "New legwear: " << newGreaves.getName() << '\n';
		cout << "To view stats, enter 1.\nTo discard, enter 2.\n";
		if (userChoice(1, 2) == 1) {
			playerCharacter->equip(&newGreaves);
		}
		this_thread::sleep_for(chrono::milliseconds(500));
		break;
	}
	case 4: //Boots
	{
		armourFeet newBoots(reward);
		cout << "New footwear: " << newBoots.getName() << '\n';
		cout << "To view stats, enter 1.\nTo discard, enter 2.\n";
		if (userChoice(1, 2) == 1) {
			playerCharacter->equip(&newBoots);
		}
		this_thread::sleep_for(chrono::milliseconds(500));
		break;
	}
	case 5: //Weapon
	{
		weapon newWeapon(reward);
		cout << "New weapon: " << newWeapon.getName() << '\n';
		cout << "To view stats, enter 1.\nTo discard, enter 2.\n";
		if (userChoice(1, 2) == 1) {
			playerCharacter->equip(&newWeapon);
		}
		this_thread::sleep_for(chrono::milliseconds(500));
		break;
	}
	case 6: //Spell
		eventSpell.loadFromFile(reward);
		cout << "New spell: " << eventSpell.getName() << '\n';
		cout << "To view stats, enter 1.\nTo discard, enter 2.\n";
		if (userChoice(1, 2) == 1) {
			playerCharacter->equip(&eventSpell);
		}
		this_thread::sleep_for(chrono::milliseconds(500));
		break;
	}
	//Recalculate modifiers, as new equipment may have been equipped, then check player is not dead
	playerCharacter->calculateModifiers();
	if (playerCharacter->getHealth() <= 0) {
		cout << "You are dead\n";
		return 2;
	}
	vector<choice> possibleChoices; //Choices which are allowed
	vector<Event> newEvents; //Events which the choices will load
	for (short i = 0; i < choices.size(); i++) {
		if (evalCond(choices[i].req, playerCharacter) && (choices[i].hidden || (choices[i].manaChange >= -playerCharacter->getMana() && choices[i].projectileChange >= -playerCharacter->getProjectiles() && choices[i].healthChange >= -playerCharacter->getHealth()))) { //Are requirements met for the choice
			possibleChoices.push_back(choices[i]); //Add choice to list
			newEvents.emplace_back(choices[i].eventName); //Load its event
		}
	}
	if (possibleChoices.empty()) { //If no choices allowed, add trivial one
		possibleChoices.emplace_back();
		newEvents.emplace_back();
	}
	cout << "Enter the number of the desired choice\n";
	for (short i = 0; i < possibleChoices.size(); i++) {
		cout << i + 1 << ". ";
		possibleChoices[i].display();
	}
	short chosen = userChoice(1, static_cast<int>(possibleChoices.size())) - 1; //Choose a choice
	//Apply stat changes
	playerCharacter->modifyHealth(possibleChoices[chosen].healthChange);
	if (playerCharacter->getHealth() <= 0) { //Check in case this killed the player
		return 2;
	}
	playerCharacter->modifyProjectiles(possibleChoices[chosen].projectileChange);
	playerCharacter->modifyMana(possibleChoices[chosen].manaChange);
	//Set next event
	if (newEvents[chosen].eventName == "EMPTY") {
		return 0;
	}
	*this = newEvents[chosen];
	return 1;
}

void choice::display() {
	cout << text << '\n';
	if ((healthChange != 0 || manaChange != 0 || projectileChange != 0) && !hidden) {
		cout << '(';
		if (healthChange > 0) {
			cout << "Gain " << healthChange << " health; ";
		}
		else if (healthChange < 0) {
			cout << "Lose " << healthChange << " health; ";
		}
		if (manaChange == 1) {
			cout << "Gain 1 " << g_manaName.singular() << "; ";
		}
		else if (manaChange > 1) {
			cout << "Gain " << manaChange << ' ' << g_manaName.plural() << "; ";
		}
		else if (manaChange == -1) {
			cout << "Lose 1 " << g_manaName.singular() << "; ";
		}
		else if (manaChange < -1) {
			cout << "Lose " << -manaChange << ' ' << g_manaName.plural() << "; ";
		}
		if (projectileChange == 1) {
			cout << "Gain 1 " << g_projName.singular() << "; ";
		}
		else if (projectileChange > 1) {
			cout << "Gain " << projectileChange << ' ' << g_projName.plural() << "; ";
		}
		else if (projectileChange == -1) {
			cout << "Lose 1 " << g_projName.singular() << "; ";
		}
		else if (projectileChange < -1) {
			cout << "Lose " << -projectileChange << ' ' << g_projName.plural() << "; ";
		}
		cout << '\b' << '\b' << ")\n";
	}
}