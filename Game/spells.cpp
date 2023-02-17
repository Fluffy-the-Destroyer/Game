#include "spells.h"
#include "resources.h"
#include <fstream>
#include <iostream>
#include "inputs.h"
using namespace std;

//Error codes
// 1: Bad XML
// 2: Specified blueprint or list not found
// 3: Loading empty slot
// 4: Unable to oopen blueprint file
// 5: Empty blueprint list

extern resource g_projName, g_manaName;

string spell::getName() {
	if (real) {
		return name;
	}
	else {
		return "None";
	}
}

void spell::loadFromFile(string blueprint, bool custom) {
	ifstream spellBlueprints;
	string stringbuffer = "";
	short charBuf = 0;
	try {
		if (blueprint == "EMPTY") {
			throw 3;
		}
		//Open file
		if (custom) {
			spellBlueprints.open("custom\\spellBlueprints.xml");
		}
		else {
			spellBlueprints.open("data\\spellBlueprints.xml");
		}
		if (!spellBlueprints.is_open()) {
			throw 4;
		}
		string blueprintName = "spellBlueprintList name=\"" + blueprint + '\"';
		//Check for a list
		{
			bool noList = false; //Set if we do not find a list
			streampos filePos = 0; //Position in file
			short listCount = -1; //Number of items in list, also holding which one we have picked
			while (stringbuffer != blueprintName) {
				stringbuffer = getTag(&spellBlueprints);
				ignoreLine(&spellBlueprints);
				if (spellBlueprints.eof()) {
					spellBlueprints.clear();
					noList = true;
					break;
				}
			}
			if (!noList) {
				filePos = spellBlueprints.tellg();
				do {
					if (spellBlueprints.eof()) {
						throw 1;
					}
					listCount++;
					stringbuffer = getTag(&spellBlueprints);
					ignoreLine(&spellBlueprints);
				} while (stringbuffer != "/spellBlueprintList");
				spellBlueprints.clear();
				if (listCount == 0) {
					throw 5;
				}
				listCount = rng(1, listCount);
				spellBlueprints.seekg(filePos);
				for (int i = 1; i < listCount; i++) {
					ignoreLine(&spellBlueprints);
				}
				if (getTag(&spellBlueprints) != "name") {
					throw 1;
				}
				getline(spellBlueprints, blueprint, '<');
				if (blueprint == "EMPTY") {
					throw 3;
				}
				getline(spellBlueprints, stringbuffer, '>');
				if (stringbuffer != "/name") {
					throw 1;
				}
			}
			spellBlueprints.seekg(0);
			stringbuffer = "";
		}
		//Reset attributes to default values
		real = true;
		spellName = blueprint;
		name = description = "";
		flatDamageMin = flatDamageMax = flatMagicDamageMin = flatMagicDamageMax = flatArmourPiercingDamageMin = flatArmourPiercingDamageMax = flatSelfDamageMin = flatSelfDamageMax = flatSelfMagicDamageMin = flatSelfMagicDamageMax = flatSelfArmourPiercingDamageMin = flatSelfArmourPiercingDamageMax = manaChangeEnemy = manaChange = projectileChange = poison = selfPoison = bleed = selfBleed = maxHealthModifierEnemy = maxHealthModifier = maxManaModifierEnemy = maxManaModifier = turnManaRegenModifierEnemy = turnManaRegenModifier = battleManaRegenModifier = tempRegen = tempRegenSelf = constRegenModifierEnemy = constRegenModifier = battleRegenModifier = flatArmourModifierEnemy = flatArmourModifier = flatMagicArmourModifierEnemy = flatMagicArmourModifier = flatDamageModifierEnemy = flatDamageModifier = flatMagicDamageModifierEnemy = flatMagicDamageModifier = flatArmourPiercingDamageModifierEnemy = flatArmourPiercingDamageModifier = bonusActionsModifierEnemy = bonusActionsModifier = healthChange = 0;
		propDamage = propSelfDamage = poisonResistModifierEnemy = poisonResistModifier = bleedResistModifierEnemy = bleedResistModifier = propArmourModifierEnemy = propArmourModifier = propMagicArmourModifierEnemy = propMagicArmourModifier = propDamageModifierEnemy = propDamageModifier = propMagicDamageModifierEnemy = propMagicDamageModifier = propArmourPiercingDamageModifierEnemy = propArmourPiercingDamageModifier = evadeChanceModifierEnemy = evadeChanceModifier = 0;
		hitCount = cooldown = 1;
		counterHits = currentCooldown = spellType = timing = counterSpell = 0;
		noEvade = canCounterAttack = noCounter = lifelink = false;
		blueprintName = "spellBlueprint name=\"" + blueprint + '\"';
		//Read blueprint
		while (stringbuffer != blueprintName) {
			stringbuffer = getTag(&spellBlueprints);
			ignoreLine(&spellBlueprints);
			if (spellBlueprints.eof()) {
				throw 2;
			}
		}
		stringbuffer = getTag(&spellBlueprints);
		while (stringbuffer != "/spellBlueprint") {
			if (spellBlueprints.eof()) {
				throw 1;
			}
			if (stringbuffer == "name") {
				getline(spellBlueprints, name, '<');
				spellBlueprints.seekg(-1, ios_base::cur);
			}
			else if (stringbuffer == "description") {
				getline(spellBlueprints, description, '<');
				spellBlueprints.seekg(-1, ios_base::cur);
			}
			else if (stringbuffer == "flatDamageMin") {
				spellBlueprints >> flatDamageMin;
			}
			else if (stringbuffer == "flatDamageMax") {
				spellBlueprints >> flatDamageMax;
			}
			else if (stringbuffer == "flatDamage") {
				spellBlueprints >> flatDamageMax;
				flatDamageMin = flatDamageMax;
			}
			else if (stringbuffer == "flatMagicDamageMin") {
				spellBlueprints >> flatMagicDamageMin;
			}
			else if (stringbuffer == "flatMagicDamageMax") {
				spellBlueprints >> flatMagicDamageMax;
			}
			else if (stringbuffer == "flatMagicDamage") {
				spellBlueprints >> flatMagicDamageMax;
				flatMagicDamageMin = flatMagicDamageMax;
			}
			else if (stringbuffer == "flatArmourPiercingDamageMin") {
				spellBlueprints >> flatArmourPiercingDamageMin;
			}
			else if (stringbuffer == "flatArmourPiercingDamageMax") {
				spellBlueprints >> flatArmourPiercingDamageMax;
			}
			else if (stringbuffer == "flatArmourPiercingDamage") {
				spellBlueprints >> flatArmourPiercingDamageMax;
				flatArmourPiercingDamageMin = flatArmourPiercingDamageMax;
			}
			else if (stringbuffer == "propDamage") {
				spellBlueprints >> propDamage;
				if (propDamage > 1) {
					propDamage = 1;
				}
				else if (propDamage < -1) {
					propDamage = -1;
				}
			}
			else if (stringbuffer == "flatSelfDamageMin") {
				spellBlueprints >> flatSelfDamageMin;
			}
			else if (stringbuffer == "flatSelfDamageMax") {
				spellBlueprints >> flatSelfDamageMax;
			}
			else if (stringbuffer == "flatSelfDamage") {
				spellBlueprints >> flatSelfDamageMax;
				flatSelfDamageMin = flatSelfDamageMax;
			}
			else if (stringbuffer == "flatSelfMagicDamageMin") {
				spellBlueprints >> flatSelfMagicDamageMin;
			}
			else if (stringbuffer == "flatSelfMagicDamageMax") {
				spellBlueprints >> flatSelfMagicDamageMax;
			}
			else if (stringbuffer == "flatSelfMagicDamage") {
				spellBlueprints >> flatSelfMagicDamageMax;
				flatSelfMagicDamageMin = flatSelfMagicDamageMax;
			}
			else if (stringbuffer == "flatSelfArmourPiercingDamageMin") {
				spellBlueprints >> flatSelfArmourPiercingDamageMin;
			}
			else if (stringbuffer == "flatSelfArmourPiercingDamageMax") {
				spellBlueprints >> flatSelfArmourPiercingDamageMax;
			}
			else if (stringbuffer == "flatSelfArmourPiercingDamage") {
				spellBlueprints >> flatSelfArmourPiercingDamageMax;
				flatSelfArmourPiercingDamageMin = flatSelfArmourPiercingDamageMax;
			}
			else if (stringbuffer == "propSelfDamage") {
				spellBlueprints >> propSelfDamage;
				if (propSelfDamage < -1) {
					propSelfDamage = -1;
				}
				else if (propSelfDamage > 1) {
					propSelfDamage = 1;
				}
			}
			else if (stringbuffer == "hitCount") {
				spellBlueprints >> charBuf;
				if (charBuf < 0) {
					charBuf = 0;
				}
				else if (charBuf > 255) {
					charBuf = 255;
				}
				hitCount = static_cast<unsigned char>(charBuf);
			}
			else if (stringbuffer == "noEvade/") {
				noEvade = true;
				ignoreLine(&spellBlueprints);
				if (!spellBlueprints) {
					throw 1;
				}
				stringbuffer = getTag(&spellBlueprints);
				continue;
			}
			else if (stringbuffer == "manaChangeEnemy") {
				spellBlueprints >> manaChangeEnemy;
			}
			else if (stringbuffer == "manaChange") {
				spellBlueprints >> manaChange;
			}
			else if (stringbuffer == "projectileChange") {
				spellBlueprints >> projectileChange;
			}
			else if (stringbuffer == "poison") {
				spellBlueprints >> poison;
				if (poison < -255) {
					poison = -255;
				}
				else if (poison > 255) {
					poison = 255;
				}
			}
			else if (stringbuffer == "selfPoison") {
				spellBlueprints >> selfPoison;
				if (selfPoison < -255) {
					selfPoison = -255;
				}
				else if (selfPoison > 255) {
					selfPoison = 255;
				}
			}
			else if (stringbuffer == "bleed") {
				spellBlueprints >> bleed;
				if (bleed < -255) {
					bleed = -255;
				}
				else if (bleed > 255) {
					bleed = 255;
				}
			}
			else if (stringbuffer == "selfBleed") {
				spellBlueprints >> selfBleed;
				if (selfBleed < -255) {
					selfBleed = -255;
				}
				else if (selfBleed > 255) {
					selfBleed = 255;
				}
			}
			else if (stringbuffer == "maxHealthModifierEnemy") {
				spellBlueprints >> maxHealthModifierEnemy;
			}
			else if (stringbuffer == "maxHealthModifier") {
				spellBlueprints >> maxHealthModifier;
			}
			else if (stringbuffer == "maxManaModifierEnemy") {
				spellBlueprints >> maxManaModifierEnemy;
			}
			else if (stringbuffer == "maxManaModifier") {
				spellBlueprints >> maxManaModifier;
			}
			else if (stringbuffer == "turnManaRegenModifierEnemy") {
				spellBlueprints >> turnManaRegenModifierEnemy;
			}
			else if (stringbuffer == "turnManaRegenModifier") {
				spellBlueprints >> turnManaRegenModifier;
			}
			else if (stringbuffer == "battleManaRegenModifier") {
				spellBlueprints >> battleManaRegenModifier;
			}
			else if (stringbuffer == "poisonResistModifierEnemy") {
				spellBlueprints >> poisonResistModifierEnemy;
				if (poisonResistModifierEnemy < -1) {
					poisonResistModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "poisonResistModifier") {
				spellBlueprints >> poisonResistModifier;
				if (poisonResistModifier < -1) {
					poisonResistModifier = -1;
				}
			}
			else if (stringbuffer == "bleedResistModifierEnemy") {
				spellBlueprints >> bleedResistModifierEnemy;
				if (bleedResistModifierEnemy < -1) {
					bleedResistModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "bleedResistModifier") {
				spellBlueprints >> bleedResistModifier;
				if (bleedResistModifier < -1) {
					bleedResistModifier = -1;
				}
			}
			else if (stringbuffer == "tempRegen") {
				spellBlueprints >> tempRegen;
				if (tempRegen < -255) {
					tempRegen = -255;
				}
				else if (tempRegen > 255) {
					tempRegen = 255;
				}
			}
			else if (stringbuffer == "tempRegenSelf") {
				spellBlueprints >> tempRegenSelf;
				if (tempRegenSelf < -255) {
					tempRegenSelf = -255;
				}
				else if (tempRegenSelf > 255) {
					tempRegenSelf = 255;
				}
			}
			else if (stringbuffer == "constRegenModifierEnemy") {
				spellBlueprints >> constRegenModifierEnemy;
			}
			else if (stringbuffer == "constRegenModifier") {
				spellBlueprints >> constRegenModifier;
			}
			else if (stringbuffer == "battleRegenModifier") {
				spellBlueprints >> battleRegenModifier;
			}
			else if (stringbuffer == "flatArmourModifierEnemy") {
				spellBlueprints >> flatArmourModifierEnemy;
			}
			else if (stringbuffer == "flatArmourModifier") {
				spellBlueprints >> flatArmourModifier;
			}
			else if (stringbuffer == "propArmourModifierEnemy") {
				spellBlueprints >> propArmourModifierEnemy;
				if (propArmourModifierEnemy < -1) {
					propArmourModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "propArmourModifier") {
				spellBlueprints >> propArmourModifier;
				if (propArmourModifier < -1) {
					propArmourModifier = -1;
				}
			}
			else if (stringbuffer == "flatMagicArmourModifierEnemy") {
				spellBlueprints >> flatMagicArmourModifierEnemy;
			}
			else if (stringbuffer == "flatMagicArmourModifier") {
				spellBlueprints >> flatMagicArmourModifier;
			}
			else if (stringbuffer == "propMagicArmourModifierEnemy") {
				spellBlueprints >> propMagicArmourModifierEnemy;
				if (propMagicArmourModifierEnemy < -1) {
					propMagicArmourModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "propMagicArmourModifier") {
				spellBlueprints >> propMagicArmourModifier;
				if (propMagicArmourModifier < -1) {
					propMagicArmourModifier = -1;
				}
			}
			else if (stringbuffer == "flatDamageModifierEnemy") {
				spellBlueprints >> flatDamageModifierEnemy;
			}
			else if (stringbuffer == "flatDamageModifier") {
				spellBlueprints >> flatDamageModifier;
			}
			else if (stringbuffer == "propDamageModifierEnemy") {
				spellBlueprints >> propDamageModifierEnemy;
				if (propDamageModifierEnemy < -1) {
					propDamageModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "propDamageModifier") {
				spellBlueprints >> propDamageModifier;
				if (propDamageModifier < -1) {
					propDamageModifier = -1;
				}
			}
			else if (stringbuffer == "flatMagicDamageModifierEnemy") {
				spellBlueprints >> flatMagicDamageModifierEnemy;
			}
			else if (stringbuffer == "flatMagicDamageModifier") {
				spellBlueprints >> flatMagicDamageModifier;
			}
			else if (stringbuffer == "propMagicDamageModifierEnemy") {
				spellBlueprints >> propMagicDamageModifierEnemy;
				if (propMagicDamageModifierEnemy < -1) {
					propMagicDamageModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "propMagicDamageModifier") {
				spellBlueprints >> propMagicDamageModifier;
				if (propMagicDamageModifier < -1) {
					propMagicDamageModifier = -1;
				}
			}
			else if (stringbuffer == "flatArmourPiercingDamageModifierEnemy") {
				spellBlueprints >> flatArmourPiercingDamageModifierEnemy;
			}
			else if (stringbuffer == "flatArmourPiercingDamageModifier") {
				spellBlueprints >> flatArmourPiercingDamageModifier;
			}
			else if (stringbuffer == "propArmourPiercingDamageModifierEnemy") {
				spellBlueprints >> propArmourPiercingDamageModifierEnemy;
				if (propArmourPiercingDamageModifierEnemy < -1) {
					propArmourPiercingDamageModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "propArmourPiercingDamageModifier") {
				spellBlueprints >> propArmourPiercingDamageModifier;
				if (propArmourPiercingDamageModifier < -1) {
					propArmourPiercingDamageModifier = -1;
				}
			}
			else if (stringbuffer == "evadeChanceModifierEnemy") {
				spellBlueprints >> evadeChanceModifierEnemy;
				if (evadeChanceModifierEnemy < -1) {
					evadeChanceModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "evadeChanceModifier") {
				spellBlueprints >> evadeChanceModifier;
				if (evadeChanceModifier < -1) {
					evadeChanceModifier = -1;
				}
			}
			else if (stringbuffer == "cooldown") {
				spellBlueprints >> charBuf;
				if (charBuf < 1) {
					charBuf = 1;
				}
				else if (charBuf > 255) {
					charBuf = 255;
				}
				cooldown = static_cast<unsigned char>(charBuf);
			}
			else if (stringbuffer == "spellType") {
				spellBlueprints >> charBuf;
				if (charBuf < 1 || charBuf > SPELL_TYPES_NO) {
					charBuf = 0;
				}
				spellType = static_cast<unsigned char>(charBuf);
			}
			else if (stringbuffer == "counterHits") {
				spellBlueprints >> charBuf;
				if (charBuf < 0) {
					charBuf = 0;
				}
				else if (charBuf > 255) {
					charBuf = 255;
				}
				counterHits = static_cast<unsigned char>(charBuf);
			}
			else if (stringbuffer == "canCounterAttack/") {
				canCounterAttack = true;
				ignoreLine(&spellBlueprints);
				if (!spellBlueprints) {
					throw 1;
				}
				stringbuffer = getTag(&spellBlueprints);
				continue;
			}
			else if (stringbuffer == "timing") {
				spellBlueprints >> charBuf;
				if (charBuf < 0 || charBuf > 2) {
					charBuf = 0;
				}
				timing = static_cast<unsigned char>(charBuf);
			}
			else if (stringbuffer == "counterSpell") {
				spellBlueprints >> charBuf;
				if (charBuf < 0 || charBuf > 3) {
					charBuf = 0;
				}
				counterSpell = static_cast<unsigned char>(charBuf);
			}
			else if (stringbuffer == "bonusActionsModifierEnemy") {
				spellBlueprints >> bonusActionsModifierEnemy;
				if (bonusActionsModifierEnemy < -255) {
					bonusActionsModifierEnemy = -255;
				}
				else if (bonusActionsModifierEnemy > 255) {
					bonusActionsModifierEnemy = 255;
				}
			}
			else if (stringbuffer == "bonusActionsModifier") {
				spellBlueprints >> bonusActionsModifier;
				if (bonusActionsModifier < -255) {
					bonusActionsModifier = -255;
				}
				else if (bonusActionsModifier > 255) {
					bonusActionsModifier = 255;
				}
			}
			else if (stringbuffer == "noCounter/") {
				noCounter = true;
				ignoreLine(&spellBlueprints);
				if (!spellBlueprints) {
					throw 1;
				}
				stringbuffer = getTag(&spellBlueprints);
				continue;
			}
			else if (stringbuffer == "battleManaRegenModifierEnemy") {
				spellBlueprints >> battleManaRegenModifierEnemy;
			}
			else if (stringbuffer == "battleRegenModifierEnemy") {
				spellBlueprints >> battleRegenModifierEnemy;
			}
			else if (stringbuffer == "lifelink/") {
				lifelink = true;
				ignoreLine(&spellBlueprints);
				if (!spellBlueprints) {
					throw 1;
				}
				stringbuffer = getTag(&spellBlueprints);
				continue;
			}
			else if (stringbuffer == "healthChange") {
				spellBlueprints >> healthChange;
			}
			else {
				throw 1;
			}
			if (getTag(&spellBlueprints) != '/' + stringbuffer) {
				throw 1;
			}
			ignoreLine(&spellBlueprints);
			if (!spellBlueprints) {
				throw 1;
			}
			stringbuffer = getTag(&spellBlueprints);
		}
		if (flatDamageMin > flatDamageMax || flatMagicDamageMin > flatMagicDamageMax || flatArmourPiercingDamageMin > flatArmourPiercingDamageMax || flatSelfDamageMin > flatSelfDamageMax || flatSelfMagicDamageMin > flatSelfMagicDamageMax || flatSelfArmourPiercingDamageMin > flatSelfArmourPiercingDamageMax) {
			throw 1;
		}
		if (spellType == 0) { //Set based on stats
			setSpellType();
		}
		spellBlueprints.close();
	}
	catch (int err) {
		spellBlueprints.close();
		spellName = "EMPTY";
		real = false;
		name = "";
		description = "";
		switch (err) {
		case 1:
			cout << "Unable to parse blueprint or blueprintList " << blueprint << "\n";
			break;
		case 2:
			if (custom) {
				loadFromFile(blueprint, false);
				return;
			}
			cout << "No blueprint or blueprintList found with name " << blueprint << '\n';
			break;
		case 4:
			if (custom) {
				loadFromFile(blueprint, false);
				return;
			}
			cout << "Could not open spellBlueprints.xml\n";
			break;
		case 5:
			cout << "blueprintList " << blueprint << " contains no entries\n";
		}
	}
}

void spell::displayStats() {
	if (!real) {
		return;
	}
	//Name
	cout << name << "\n\n";
	//Description
	cout << description << '\n';
	{
		int healingMin = 0, healingMax = 0;
	//Flat Damage
		if (flatDamageMin == flatDamageMax) {
			if (flatDamageMax > 0) {
				cout << "Deals " << flatDamageMax << " physical damage\n";
			}
			else if (flatDamageMax < 0) {
				healingMin -= flatDamageMax;
				healingMax -= flatDamageMax;
			}
		}
		else if (flatDamageMin >= 0) {
			cout << "Deals " << flatDamageMin << " to " << flatDamageMax << " physical damage\n";
		}
		else if (flatDamageMax <= 0) {
			healingMin -= flatDamageMax;
			healingMax -= flatDamageMin;
		}
		else {
			cout << "Deals " << flatDamageMin << " to " << flatDamageMax << " physical damage, negative damage will heal the target\n";
		}
	//Magic damage
		if (flatMagicDamageMin == flatMagicDamageMax) {
			if (flatMagicDamageMax > 0) {
				cout << "Deals " << flatMagicDamageMax << " magic damage\n";
			}
			else if (flatMagicDamageMax < 0) {
				healingMin -= flatMagicDamageMax;
				healingMax -= flatMagicDamageMax;
			}
		}
		else if (flatMagicDamageMin >= 0) {
			cout << "Deals " << flatMagicDamageMin << " to " << flatMagicDamageMax << " magic damage\n";
		}
		else if (flatMagicDamageMax <= 0) {
			healingMin -= flatMagicDamageMax;
			healingMax -= flatMagicDamageMin;
		}
		else {
			cout << "Deals " << flatMagicDamageMin << " to " << flatMagicDamageMax << " magic damage, negative damage will heal the target\n";
		}
	//Flat AP damage
		if (flatArmourPiercingDamageMin == flatArmourPiercingDamageMax) {
			if (flatArmourPiercingDamageMax > 0) {
				cout << "Deals " << flatArmourPiercingDamageMax << " armour piercing damage\n";
			}
			else if (flatArmourPiercingDamageMax < 0) {
				healingMin -= flatArmourPiercingDamageMax;
				healingMax -= flatArmourPiercingDamageMax;
			}
		}
		else if (flatArmourPiercingDamageMin >= 0) {
			cout << "Deals " << flatArmourPiercingDamageMin << " to " << flatArmourPiercingDamageMax << " armour piercing damage\n";
		}
		else if (flatArmourPiercingDamageMax <= 0) {
			healingMin -= flatArmourPiercingDamageMax;
			healingMax -= flatArmourPiercingDamageMin;
		}
		else {
			cout << "Deals " << flatArmourPiercingDamageMin << " to " << flatArmourPiercingDamageMax << " armour piercing damage, negative damage will heal the target\n";
		}
		if (healingMax > 0) {
			cout << "Heals the target for " << healingMin;
			if (healingMin != healingMax) {
				cout << " to " << healingMax;
			}
			cout << '\n';
		}
	}
	//Proportional damage
	if (propDamage > 0) {
		cout << "Reduces target's health by " << 100 * propDamage << "% per hit\n";
	}
	else if (propDamage < 0) {
		cout << "Heals target for " << -100 * propDamage << "% of their maximum health per hit\n";
	}
	{
		int selfHealingMin = 0, selfHealingMax = 0;
	//Flat self damage
		if (flatSelfDamageMin == flatSelfDamageMax) {
			if (flatSelfDamageMax > 0) {
				cout << "Deals " << flatSelfDamageMax << " physical damage to user on cast\n";
			}
			else if (flatSelfDamageMax < 0) {
				selfHealingMin -= flatSelfDamageMax;
				selfHealingMax -= flatSelfDamageMax;
			}
		}
		else if (flatSelfDamageMin >= 0) {
			cout << "Deals " << flatSelfDamageMin << " to " << flatSelfDamageMax << " physical damage to user on cast\n";
		}
		else if (flatSelfDamageMax <= 0) {
			selfHealingMin -= flatSelfDamageMax;
			selfHealingMax -= flatSelfDamageMin;
		}
		else {
			cout << "Deals " << flatDamageMin << " to " << flatSelfDamageMax << " physical damage to user on cast, negative damage will heal\n";
		}
	//Flat self magic damage
		if (flatSelfMagicDamageMin == flatSelfMagicDamageMax) {
			if (flatSelfMagicDamageMax > 0) {
				cout << "Deals " << flatSelfMagicDamageMax << " magic damage to user on cast\n";
			}
			else if (flatSelfMagicDamageMax < 0) {
				selfHealingMin -= flatSelfMagicDamageMax;
				selfHealingMax -= flatSelfMagicDamageMax;
			}
		}
		else if (flatSelfMagicDamageMin >= 0) {
			cout << "Deals " << flatSelfMagicDamageMin << " to " << flatSelfMagicDamageMax << " magic damage to user on cast\n";
		}
		else if (flatSelfMagicDamageMax <= 0) {
			selfHealingMin -= flatSelfMagicDamageMax;
			selfHealingMax -= flatSelfMagicDamageMin;
		}
		else {
			cout << "Deals " << flatSelfMagicDamageMin << " to " << flatSelfMagicDamageMax << " magic damage to user on cast, negative damage will heal\n";
		}
	//Flat self AP damage
		if (flatSelfArmourPiercingDamageMin == flatSelfArmourPiercingDamageMax) {
			if (flatSelfArmourPiercingDamageMax > 0) {
				cout << "Deals " << flatSelfArmourPiercingDamageMax << " armour piercing damage to user on cast\n";
			}
			else if (flatSelfArmourPiercingDamageMax < 0) {
				selfHealingMin -= flatSelfArmourPiercingDamageMax;
				selfHealingMax -= flatSelfArmourPiercingDamageMax;
			}
		}
		else if (flatSelfArmourPiercingDamageMin >= 0) {
			cout << "Deals " << flatSelfArmourPiercingDamageMin << " to " << flatSelfArmourPiercingDamageMax << " armour piercing damage to user on cast\n";
		}
		else if (flatSelfArmourPiercingDamageMax <= 0) {
			selfHealingMin -= flatSelfArmourPiercingDamageMax;
			selfHealingMax -= flatSelfArmourPiercingDamageMin;
		}
		else {
			cout << "Deals " << flatSelfArmourPiercingDamageMin << " to " << flatSelfArmourPiercingDamageMax << " armour piercing damage to user on cast, negative damage will heal\n";
		}
		if (selfHealingMax > 0) {
			cout << "Heals the user for " << selfHealingMin;
			if (selfHealingMax != selfHealingMin) {
				cout << " to " << selfHealingMax;
			}
			cout << " on cast\n";
		}
	}
	//Prop self damage
	if (propSelfDamage > 0) {
		cout << "Reduces user's health by " << 100 * propSelfDamage << "% on cast\n";
	}
	else if (propSelfDamage < 0) {
		cout << "Heals user for " << -100 * propSelfDamage << "% of their maximum health on cast\n";
	}
	//Health change cost
	if (healthChange > 0) {
		cout << "User is healed for " << healthChange << ", even if attack is countered\n";
	}
	else if (healthChange < 0) {
		cout << "Costs " << -healthChange << " health to attack (even if countered)\n";
	}
	//Lifelink
	if (lifelink) {
		cout << "On dealing damage to target, heals the caster by that much\n";
	}
	//Hit count
	if (hitCount == 0) {
		cout << "Cannot attack\n";
	}
	else if (hitCount != 1) {
		cout << "Hits " << +hitCount << " times per cast\n";
	}
	//Counter hits
	if (counterHits == 1) {
		cout << "Usable for counter attacks, hits once\n";
	}
	else if (counterHits > 1) {
		cout << "Usable for counter attacks, hits " << +counterHits << " times\n";
	}
	//Evasion
	if (noEvade) {
		cout << "Cannot be dodged\n";
	}
	//Counter attacks
	if (canCounterAttack) {
		cout << "Can be counter attacked\n";
	}
	//No counter
	if (noCounter) {
		cout << "Cannot be countered\n";
	}
	//Spell timing
	if (timing == 1) {
		cout << "Can be cast in response to enemy action\n";
	}
	else if (timing == 2) {
		cout << "Can only be cast in response to enemy action\n";
	}
	//Counter spell
	if (counterSpell == 1) {
		cout << "Can counter spells if cast in response, preventing their effects\n";
	}
	else if (counterSpell == 2) {
		cout << "Can counter weapon attacks if cast in response, preventing their effects\n";
	}
	else if (counterSpell == 3) {
		cout << "Can counter weapon attacks or spells if cast in response, preventing their effects\n";
	}
	//Enemy bonus action modifier
	if (bonusActionsModifierEnemy == 1) {
		cout << "Target gains an additional bonus action (applied on hit)\n";
	}
	else if (bonusActionsModifierEnemy > 1) {
		cout << "Target gains " << bonusActionsModifierEnemy << " additional bonus actions (applied on hit)\n";
	}
	else if (bonusActionsModifierEnemy == -1) {
		cout << "Target loses a bonus action (applied on hit)\n";
	}
	else if (bonusActionsModifierEnemy < -1) {
		cout << "Target loses " << -bonusActionsModifierEnemy << " bonus actions (applied on hit)\n";
	}
	//Bonus action modifier
	if (bonusActionsModifier == 1) {
		cout << "Gain an additional bonus action on hit\n";
	}
	else if (bonusActionsModifier > 1) {
		cout << "Gain " << bonusActionsModifier << " additional bonus actions on cast\n";
	}
	else if (bonusActionsModifier == -1) {
		cout << "Lose a bonus action on cast\n";
	}
	else if (bonusActionsModifier < -1) {
		cout << "Lose " << -bonusActionsModifier << " bonus actions on cast\n";
	}
	//Enemy mana
	if (manaChangeEnemy == -1) {
		cout << "Target loses 1 " << g_manaName.singular() << " per hit\n";
	}
	else if (manaChangeEnemy < -1) {
		cout << "Target loses " << -manaChangeEnemy << ' ' << g_manaName.plural() << " per hit\n";
	}
	else if (manaChangeEnemy == 1) {
		cout << "Target gains 1 " << g_manaName.singular() << " per hit\n";
	}
	else if (manaChangeEnemy > 1) {
		cout << "Target gains " << manaChangeEnemy << ' ' << g_manaName.plural() << " per hit\n";
	}
	//Mana cost
	if (manaChange == -1) {
		cout << "Costs 1 " << g_manaName.singular() << " to cast\n";
	}
	else if (manaChange < -1) {
		cout << "Costs " << -manaChange << ' ' << g_manaName.plural() << " to cast\n";
	}
	else if (manaChange == 1) {
		cout << "Gain 1 " << g_manaName.singular() << " on cast\n";
	}
	else if (manaChange > 1) {
		cout << "Gain " << manaChange << ' ' << g_manaName.plural() << " on cast\n";
	}
	//Projectile cost
	if (projectileChange == -1) {
		cout << "Requires 1 " << g_projName.singular() << " to cast\n";
	}
	else if (projectileChange < -1) {
		cout << "Requires " << -projectileChange << ' ' << g_projName.plural() << " to cast\n";
	}
	else if (projectileChange == -1) {
		cout << "Regain 1" << g_projName.singular() << " on cast\n";
	}
	else if (projectileChange > 1) {
		cout << "Regain " << projectileChange << ' ' << g_projName.plural() << " on cast\n";
	}
	//Cooldown
	cout << "Cooldown: " << +cooldown << '\n';
	//Poison
	if (poison > 0) {
		cout << "Applies " << poison << " to target on hit\n";
	}
	else if (poison == -255) {
		cout << "Removes all poison from target on hit\n";
	}
	else if (poison < 0) {
		cout << "Removes " << poison << " from target on hit\n";
	}
	//Self poison
	if (selfPoison > 0) {
		cout << "Applies " << selfPoison << " to user on cast\n";
	}
	else if (selfPoison == -255) {
		cout << "Removes all poison from user on cast\n";
	}
	else if (selfPoison < 0) {
		cout << "Removes " << -selfPoison << " poison from user on cast\n";
	}
	//Enemy poison resist
	if (poisonResistModifierEnemy > 0) {
		cout << "On hit, target gets +" << 100 * poisonResistModifierEnemy << "% poison resist\n";
	}
	else if (poisonResistModifierEnemy < 0) {
		cout << "On hit, target gets " << 100 * poisonResistModifierEnemy << "% poison resist\n";
	}
	//Poison resist
	if (poisonResistModifier > 0) {
		cout << "On cast, user gets +" << 100 * poisonResistModifier << "% poison resist\n";
	}
	else if (poisonResistModifier < 0) {
		cout << "On cast, user gets " << 100 * poisonResistModifier << "% poison resist\n";
	}
	//Bleed
	if (bleed > 0) {
		cout << "Applies " << bleed << " bleed to target on hit\n";
	}
	else if (bleed == -255) {
		cout << "Removes all bleed from target on hit\n";
	}
	else if (bleed < 0) {
		cout << "Removes " << -bleed << " bleed from target on hit\n";
	}
	//Self bleed
	if (selfBleed > 0) {
		cout << "Applies " << selfBleed << " bleed to user on cast\n";
	}
	else if (selfBleed == -255) {
		cout << "Removes all bleed from user on cast\n";
	}
	else if (selfBleed < 0) {
		cout << "Removes " << -selfBleed << " bleed from user on cast\n";
	}
	//Enemy bleed resist
	if (bleedResistModifierEnemy > 0) {
		cout << "On hit, target gets +" << 100 * bleedResistModifierEnemy << "% bleed resist\n";
	}
	else if (bleedResistModifierEnemy < 0) {
		cout << "On hit, target gets " << 100 * bleedResistModifierEnemy << "% bleed resist\n";
	}
	//Bleed resist
	if (bleedResistModifier > 0) {
		cout << "On cast, user gets +" << 100 * bleedResistModifier << "% bleed resist\n";
	}
	else if (bleedResistModifier < 0) {
		cout << "On cast, user gets " << 100 * bleedResistModifier << "% bleed resist\n";
	}
	//Enemy max health
	if (maxHealthModifierEnemy > 0) {
		cout << "Increases target's maximum health by " << maxHealthModifierEnemy << " on hit\n";
	}
	else if (maxHealthModifierEnemy < 0) {
		cout << "Reduces target's maximum health by " << -maxHealthModifierEnemy << " on hit\n";
	}
	//Max health
	if (maxHealthModifier > 0) {
		cout << "Increases user's maximum health by " << maxHealthModifier << " on cast\n";
	}
	else if (maxHealthModifier < 0) {
		cout << "Reduces user's maximum health by " << -maxHealthModifier << " on cast\n";
	}
	//Enemy temp regen
	if (tempRegen > 0) {
		cout << "Applies " << tempRegen << " regeneration to target on hit\n";
	}
	else if (tempRegen == -255) {
		cout << "Removes all regeneration from target on hit\n";
	}
	else if (tempRegen < 0) {
		cout << "Removes " << -tempRegen << " regeneration from target on hit\n";
	}
	//Temp regen
	if (tempRegenSelf > 0) {
		cout << "Applies " << tempRegenSelf << " regeneration to user on cast\n";
	}
	else if (tempRegenSelf == -255) {
		cout << "Removes all regeneration from user on cast\n";
	}
	else if (tempRegenSelf < 0) {
		cout << "Removes " << -tempRegenSelf << " regeneration from user on cast\n";
	}
	//Enemy regen
	if (constRegenModifierEnemy > 0) {
		cout << "Target gets +" << constRegenModifierEnemy << " health per turn, (applied on hit)\n";
	}
	else if (constRegenModifierEnemy < 0) {
		cout << "Target gets " << constRegenModifierEnemy << " health per turn, (applpied on hit)\n";
	}
	//Regen
	if (constRegenModifier > 0) {
		cout << "User gets +" << constRegenModifier << " health per turn\n";
	}
	else if (constRegenModifier < 0) {
		cout << "User gets " << constRegenModifier << " health per turn\n";
	}
	//Enemy battle regen
	if (battleRegenModifierEnemy > 0) {
		cout << "Target gains " << battleRegenModifierEnemy << " health at end of battle (applied on hit)\n";
	}
	else if (battleRegenModifierEnemy < 0) {
		cout << "Target loses " << -battleRegenModifierEnemy << " health at end of battle (applied on hit)\n";
	}
	//Battle regen
	if (battleRegenModifier > 0) {
		cout << '+' << battleRegenModifier << " health at end of battle\n";
	}
	else if (battleRegenModifier < 0) {
		cout << battleRegenModifier << " health at end of battle\n";
	}
	//Enemy max mana
	if (maxManaModifierEnemy > 0) {
		cout << "Increases target's maximum " << g_manaName.plural() << " by " << maxManaModifierEnemy << " on hit\n";
	}
	else if (maxManaModifierEnemy < 0) {
		cout << "Reduces target's maximum " << g_manaName.plural() << " by " << -maxManaModifierEnemy << " on hit\n";
	}
	//Max mana
	if (maxManaModifier > 0) {
		cout << "Increases user's maximum " << g_manaName.plural() << " by " << maxManaModifier << " on cast\n";
	}
	else if (maxManaModifier < 0) {
		cout << "Reduces user's maximum " << g_manaName.plural() << " by " << -maxManaModifier << " on cast\n";
	}
	//Turn mana regen Enemy
	if (turnManaRegenModifierEnemy > 0) {
		cout << "Increases target's " << g_manaName.singular() << " recovery by " << turnManaRegenModifierEnemy << " on hit\n";
	}
	else if (turnManaRegenModifierEnemy < 0) {
		cout << "Reduces target's " << g_manaName.singular() << " recovery by " << -turnManaRegenModifierEnemy << " on hit\n";
	}
	//Turn mana regen
	if (turnManaRegenModifier > 0) {
		cout << "Increases user's " << g_manaName.singular() << " recovery by " << turnManaRegenModifier << " on cast\n";
	}
	else if (turnManaRegenModifier < 0) {
		cout << "Reduces user's " << g_manaName.singular() << " recovery by " << -turnManaRegenModifier << " on cast\n";
	}
	//Enemy battle mana regen
	if (battleManaRegenModifierEnemy == 1) {
		cout << "Target recovers 1 " << g_manaName.singular() << " at end of battle (applied on hit)\n";
	}
	else if (battleManaRegenModifierEnemy > 1) {
		cout << "Target recovers " << battleManaRegenModifierEnemy << ' ' << g_manaName.plural() << " at end of battle (applied on hit)\n";
	}
	else if (battleManaRegenModifierEnemy == -1) {
		cout << "Target loses 1 " << g_manaName.singular() << " at end of battle (applied on hit)\n";
	}
	else if (battleManaRegenModifierEnemy < -1) {
		cout << "Target loses " << -battleManaRegenModifierEnemy << ' ' << g_manaName.plural() << " at end of battle (applied on hit)\n";
	}
	//Battle mana regen
	if (battleManaRegenModifier == 1) {
		cout << "+1 " << g_manaName.singular() << " recovered at end of battle\n";
	}
	else if (battleManaRegenModifier > 1) {
		cout << '+' << battleManaRegenModifier << ' ' << g_manaName.plural() << " recovered at end of battle\n";
	}
	else if (battleManaRegenModifier == -1) {
		cout << "-1 " << g_manaName.singular() << " recovered at end of battle\n";
	}
	else if (battleManaRegenModifier < -1) {
		cout << battleManaRegenModifier << ' ' << g_manaName.plural() << " recovered at end of battle\n";
	}
	//Enemy flat armour
	if (flatArmourModifierEnemy > 0) {
		cout << "Increases target's physical armour rating by " << flatArmourModifierEnemy << " on hit\n";
	}
	else if (flatArmourModifierEnemy < 0) {
		cout << "Reduces target's physical armour rating by " << -flatArmourModifierEnemy << " on hit\n";
	}
	//Flat armour
	if (flatArmourModifier > 0) {
		cout << "Increases user's physical armour rating by " << flatArmourModifier << " on cast\n";
	}
	else if (flatArmourModifier < 0) {
		cout << "Reduces user's physical armour rating by " << -flatArmourModifier << " on cast\n";
	}
	//Enemy prop armour
	if (propArmourModifierEnemy > 0) {
		cout << "Target receives " << 100 * propArmourModifierEnemy << "% more physical damage, (applied on hit)\n";
	}
	else if (propArmourModifierEnemy < 0) {
		cout << "Target receives " << -100 * propArmourModifierEnemy << "% less physical damage, (applied on hit)\n";
	}
	//Prop armour
	if (propArmourModifier > 0) {
		cout << "User receives " << 100 * propArmourModifier << "% more physical damage\n";
	}
	else if (propArmourModifier < 0) {
		cout << "User receives " << -100 * propArmourModifier << "% less physical damage\n";
	}
	//Enemy flat magic armour
	if (flatMagicArmourModifierEnemy > 0) {
		cout << "Increases target's magic armour rating by " << flatMagicArmourModifierEnemy << " on hit\n";
	}
	else if (flatMagicArmourModifierEnemy < 0) {
		cout << "Reduces target's magic armour rating by " << -flatMagicArmourModifierEnemy << " on hit\n";
	}
	//Flat magic armour
	if (flatMagicArmourModifier > 0) {
		cout << "Increases user's magic armour rating by " << flatMagicArmourModifier << " on cast\n";
	}
	else if (flatMagicArmourModifier < 0) {
		cout << "Reduces user's magic armour rating by " << -flatMagicArmourModifier << " on cast\n";
	}
	//Enemy prop magic armour
	if (propMagicArmourModifierEnemy > 0) {
		cout << "Target receives " << 100 * propMagicArmourModifierEnemy << "% more magic damage, (applied on hit)\n";
	}
	else if (propMagicArmourModifierEnemy < 0) {
		cout << "Target receives " << -100 * propMagicArmourModifierEnemy << "% less magic damage, (applied on hit)\n";
	}
	//Prop magic armour
	if (propMagicArmourModifier > 0) {
		cout << "User receives " << 100 * propMagicArmourModifier << "% more magic damage\n";
	}
	else if (propMagicArmourModifier < 0) {
		cout << "User receives " << -100 * propMagicArmourModifier << "% less magic damage\n";
	}
	//Enemy flat damage
	if (flatDamageModifierEnemy > 0) {
		cout << "Target deals " << flatDamageModifierEnemy << " more physical damage, (applied on hit)\n";
	}
	else if (flatDamageModifierEnemy < 0) {
		cout << "Target deals " << -flatDamageModifierEnemy << " less physical damage, (applied on hit)\n";
	}
	//Flat damage
	if (flatDamageModifier > 0) {
		cout << "User deals " << flatDamageModifier << " more physical damage\n";
	}
	else if (flatDamageModifier < 0) {
		cout << "User deals " << -flatDamageModifier << " less physical damage\n";
	}
	//Enemy prop damage
	if (propDamageModifierEnemy > 0) {
		cout << "Target deals " << 100 * propDamageModifierEnemy << "% more physical damage, (applied on hit)\n";
	}
	else if (propDamageModifierEnemy < 0) {
		cout << "Target deals " << -100 * propDamageModifierEnemy << "% less physical damage, (applied on hit)\n";
	}
	//Prop damage
	if (propDamageModifier > 0) {
		cout << "User deals " << 100 * propDamageModifier << "% more physical damage\n";
	}
	else if (propDamageModifier < 0) {
		cout << "User deals " << -100 * propDamageModifier << "% less physical damage\n";
	}
	//Enemy flat magic damage
	if (flatMagicDamageModifierEnemy > 0) {
		cout << "Target deals " << flatMagicDamageModifierEnemy << " more magic damage, (applied on hit)\n";
	}
	else if (flatMagicDamageModifierEnemy < 0) {
		cout << "Target deals " << -flatMagicDamageModifierEnemy << " less magic damage, (applied on hit)\n";
	}
	//Flat magic damage
	if (flatMagicDamageModifier > 0) {
		cout << "User deals " << flatMagicDamageModifier << " more magic damage\n";
	}
	else if (flatMagicDamageModifier < 0) {
		cout << "User deals " << -flatMagicDamageModifier << " less magic damage\n";
	}
	//Enemy prop magic damage
	if (propMagicDamageModifierEnemy > 0) {
		cout << "Target deals " << 100 * propMagicDamageModifierEnemy << "% more magic damage, (applied on hit)\n";
	}
	else if (propMagicDamageModifierEnemy < 0) {
		cout << "Target deals " << -100 * propMagicDamageModifierEnemy << "% less magic damage, (applied on hit)\n";
	}
	//Prop magic damage
	if (propMagicDamageModifier > 0) {
		cout << "User deals " << 100 * propMagicDamageModifier << "% more magic damage\n";
	}
	else if (propMagicDamageModifier < 0) {
		cout << "User deals " << -100 * propMagicDamageModifier << "% less magic damage\n";
	}
	//enemy flat AP
	if (flatArmourPiercingDamageModifierEnemy > 0) {
		cout << "Target deals " << flatArmourPiercingDamageModifierEnemy << " more armour piercing damage, (applied on hit)\n";
	}
	else if (flatArmourPiercingDamageModifierEnemy < 0) {
		cout << "Target deals " << -flatArmourPiercingDamageModifierEnemy << " less armour piercing damage, (applied on hit)\n";
	}
	//Flat AP
	if (flatArmourPiercingDamageModifier > 0) {
		cout << "User deals " << flatArmourPiercingDamageModifier << " more armour piercing damage\n";
	}
	else if (flatArmourPiercingDamageModifier < 0) {
		cout << "User deals " << -flatArmourPiercingDamageModifier << " less armour piercing damage\n";
	}
	//Enemy prop AP
	if (propArmourPiercingDamageModifierEnemy > 0) {
		cout << "Target deals " << 100 * propArmourPiercingDamageModifierEnemy << "% more armour piercing damage, (applied on hit)\n";
	}
	else if (propArmourPiercingDamageModifierEnemy < 0) {
		cout << "Target deals " << -100 * propArmourPiercingDamageModifierEnemy << "% less armour piercing damage, (applied on hit)\n";
	}
	//Prop AP
	if (propArmourPiercingDamageModifier > 0) {
		cout << "User deals " << 100 * propArmourPiercingDamageModifier << "% more armour piercing damage\n";
	}
	else if (propArmourPiercingDamageModifier < 0) {
		cout << "User deals " << -100 * propArmourPiercingDamageModifier << "% less armour piercing damage\n";
	}
	//Enemy evade
	if (evadeChanceModifierEnemy > 0) {
		cout << "Increases target's evasion chance by " << 100 * evadeChanceModifierEnemy << "%, (applied on hit)\n";
	}
	else if (evadeChanceModifierEnemy < 0) {
		cout << "Reduces target's evasion chance by " << -100 * evadeChanceModifierEnemy << "%, (applied on hit)\n";
	}
	//Evade
	if (evadeChanceModifier > 0) {
		cout << "Increases user's evasion chance by " << 100 * evadeChanceModifier << "%\n";
	}
	else if (evadeChanceModifier < 0) {
		cout << "Reduces user's evasion chance by " << -100 * evadeChanceModifier << "%\n";
	}
}

void spell::setSpellType() {
	{
		int flatDamage = (flatDamageMin + flatDamageMax + flatMagicDamageMin + flatMagicDamageMax + flatArmourPiercingDamageMin + flatArmourPiercingDamageMax) / 2; //Average total flat damage
		if (flatDamage >= ATTACK_SPELL_FLAT_CUTOFF || propDamage >= ATTACK_SPELL_PROP_CUTOFF) { //Does minimum damage
			int flatHealing = -(flatSelfDamageMin + flatSelfDamageMax + flatSelfMagicDamageMin + flatSelfMagicDamageMax + flatSelfArmourPiercingDamageMin + flatSelfArmourPiercingDamageMax) / 2; //Average total flat healing
			if (flatHealing >= HEALING_SPELL_FLAT_CUTOFF || -propSelfDamage >= HEALING_SPELL_PROP_CUTOFF) { //Does minimum healing
				spellType = 4;
				return;
			}
			spellType = 1;
			return;
		}
	}
	{
		int flatHealing = -(flatSelfDamageMin + flatSelfDamageMax + flatSelfMagicDamageMin + flatSelfMagicDamageMax + flatSelfArmourPiercingDamageMin + flatSelfArmourPiercingDamageMax) / 2; //Average total flat healing
		if (flatHealing >= HEALING_SPELL_FLAT_CUTOFF || -propSelfDamage >= HEALING_SPELL_PROP_CUTOFF) { //Does minimum healing
			spellType = 2;
			return;
		}
	}
	spellType = 3;
}

bool spell::checkSpellType(unsigned char type) {
	switch (type) {
	case 1:
		if (spellType == 1 || spellType == 4) { //Attack or attack+healing
			return true;
		}
		break;
	case 2:
		if (spellType == 2 || spellType == 4) { //Healing or attack+healing
			return true;
		}
		break;
	default:
		return (type == spellType);
	}
	return false;
}

void spell::displayName() {
	if (!real) {
		cout << "None";
		return;
	}
	cout << name << ' ';
	if (manaChange < 0 || projectileChange < 0 || healthChange < 0) {
		cout << '(';
		if (healthChange < 0) {
			cout << healthChange << " health, ";
		}
		if (manaChange == -1) {
			cout << "-1 " << g_manaName.singular() << ", ";
		}
		else if (manaChange < -1) {
			cout << manaChange << ' ' << g_manaName.plural() << ", ";
		}
		if (projectileChange == -1) {
			cout << "-1 " << g_projName.singular() << ", ";
		}
		else if (projectileChange < -1) {
			cout << projectileChange << ' ' << g_projName.plural() << ", ";
		}
		cout << (char)8 << (char)8 << ')';
	}
}

void spell::decCooldown() {
	if (currentCooldown > 0) {
		currentCooldown--;
	}
}