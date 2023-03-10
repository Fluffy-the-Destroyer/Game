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
				blueprint = stringFromFile(&spellBlueprints);
				if (blueprint == "EMPTY") {
					throw 3;
				}
				if (getTag(&spellBlueprints) != "/name") {
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
		flatDamageMin = flatDamageMax = flatMagicDamageMin = flatMagicDamageMax = flatArmourPiercingDamageMin = flatArmourPiercingDamageMax = flatSelfDamageMin = flatSelfDamageMax = flatSelfMagicDamageMin = flatSelfMagicDamageMax = flatSelfArmourPiercingDamageMin = flatSelfArmourPiercingDamageMax = manaChangeEnemy = manaChange = projectileChange = poison = selfPoison = bleed = selfBleed = maxHealthModifierEnemy = maxHealthModifier = maxManaModifierEnemy = maxManaModifier = turnManaRegenModifierEnemy = turnManaRegenModifier = battleManaRegenModifier = tempRegen = tempRegenSelf = constRegenModifierEnemy = constRegenModifier = battleRegenModifier = flatArmourModifierEnemy = flatArmourModifier = flatMagicArmourModifierEnemy = flatMagicArmourModifier = flatDamageModifierEnemy = flatDamageModifier = flatMagicDamageModifierEnemy = flatMagicDamageModifier = flatArmourPiercingDamageModifierEnemy = flatArmourPiercingDamageModifier = bonusActionsModifierEnemy = bonusActionsModifier = healthChange = battleManaRegenModifierEnemy = battleRegenModifierEnemy = initiativeModifier = 0;
		propDamage = propSelfDamage = poisonResistModifierEnemy = poisonResistModifier = bleedResistModifierEnemy = bleedResistModifier = propArmourModifierEnemy = propArmourModifier = propMagicArmourModifierEnemy = propMagicArmourModifier = propDamageModifierEnemy = propDamageModifier = propMagicDamageModifierEnemy = propMagicDamageModifier = propArmourPiercingDamageModifierEnemy = propArmourPiercingDamageModifier = evadeChanceModifierEnemy = evadeChanceModifier = 0;
		hitCount = cooldown = 1;
		counterHits = currentCooldown = spellType = timing = counterSpell = 0;
		noEvade = canCounterAttack = noCounter = lifelink = selfOverheal = targetOverheal = false;
		upgrade = "EMPTY";
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
				name = stringFromFile(&spellBlueprints);
			}
			else if (stringbuffer == "description") {
				description = stringFromFile(&spellBlueprints);
			}
			else if (stringbuffer == "flatDamageMin") {
				flatDamageMin = numFromFile(&spellBlueprints);
			}
			else if (stringbuffer == "flatDamageMax") {
				flatDamageMax = numFromFile(&spellBlueprints);
			}
			else if (stringbuffer == "flatDamage") {
				flatDamageMax = numFromFile(&spellBlueprints);
				flatDamageMin = flatDamageMax;
			}
			else if (stringbuffer == "flatMagicDamageMin") {
				flatMagicDamageMin = numFromFile(&spellBlueprints);
			}
			else if (stringbuffer == "flatMagicDamageMax") {
				flatMagicDamageMax = numFromFile(&spellBlueprints);
			}
			else if (stringbuffer == "flatMagicDamage") {
				flatMagicDamageMax = numFromFile(&spellBlueprints);
				flatMagicDamageMin = flatMagicDamageMax;
			}
			else if (stringbuffer == "flatArmourPiercingDamageMin") {
				flatArmourPiercingDamageMin = numFromFile(&spellBlueprints);
			}
			else if (stringbuffer == "flatArmourPiercingDamageMax") {
				flatArmourPiercingDamageMax = numFromFile(&spellBlueprints);
			}
			else if (stringbuffer == "flatArmourPiercingDamage") {
				flatArmourPiercingDamageMax = numFromFile(&spellBlueprints);
				flatArmourPiercingDamageMin = flatArmourPiercingDamageMax;
			}
			else if (stringbuffer == "propDamage") {
				propDamage = floatFromFile(&spellBlueprints);
				if (propDamage > 1) {
					propDamage = 1;
				}
				else if (propDamage < -1) {
					propDamage = -1;
				}
			}
			else if (stringbuffer == "flatSelfDamageMin") {
				flatSelfDamageMin = numFromFile(&spellBlueprints);
			}
			else if (stringbuffer == "flatSelfDamageMax") {
				flatSelfDamageMax = numFromFile(&spellBlueprints);
			}
			else if (stringbuffer == "flatSelfDamage") {
				flatSelfDamageMax = numFromFile(&spellBlueprints);
				flatSelfDamageMin = flatSelfDamageMax;
			}
			else if (stringbuffer == "flatSelfMagicDamageMin") {
				flatSelfMagicDamageMin = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "flatSelfMagicDamageMax") {
				flatSelfMagicDamageMax = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "flatSelfMagicDamage") {
				flatSelfMagicDamageMax = numFromFile(&spellBlueprints);;
				flatSelfMagicDamageMin = flatSelfMagicDamageMax;
			}
			else if (stringbuffer == "flatSelfArmourPiercingDamageMin") {
				flatSelfArmourPiercingDamageMin = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "flatSelfArmourPiercingDamageMax") {
				flatSelfArmourPiercingDamageMax = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "flatSelfArmourPiercingDamage") {
				flatSelfArmourPiercingDamageMax = numFromFile(&spellBlueprints);;
				flatSelfArmourPiercingDamageMin = flatSelfArmourPiercingDamageMax;
			}
			else if (stringbuffer == "propSelfDamage") {
				propSelfDamage = floatFromFile(&spellBlueprints);;
				if (propSelfDamage < -1) {
					propSelfDamage = -1;
				}
				else if (propSelfDamage > 1) {
					propSelfDamage = 1;
				}
			}
			else if (stringbuffer == "hitCount") {
				charBuf = numFromFile(&spellBlueprints);;
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
				manaChangeEnemy = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "manaChange") {
				manaChange = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "projectileChange") {
				projectileChange = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "poison") {
				poison = numFromFile(&spellBlueprints);;
				if (poison < -255) {
					poison = -255;
				}
				else if (poison > 255) {
					poison = 255;
				}
			}
			else if (stringbuffer == "selfPoison") {
				selfPoison = numFromFile(&spellBlueprints);;
				if (selfPoison < -255) {
					selfPoison = -255;
				}
				else if (selfPoison > 255) {
					selfPoison = 255;
				}
			}
			else if (stringbuffer == "bleed") {
				bleed = numFromFile(&spellBlueprints);;
				if (bleed < -255) {
					bleed = -255;
				}
				else if (bleed > 255) {
					bleed = 255;
				}
			}
			else if (stringbuffer == "selfBleed") {
				selfBleed = numFromFile(&spellBlueprints);;
				if (selfBleed < -255) {
					selfBleed = -255;
				}
				else if (selfBleed > 255) {
					selfBleed = 255;
				}
			}
			else if (stringbuffer == "maxHealthModifierEnemy") {
				maxHealthModifierEnemy = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "maxHealthModifier") {
				maxHealthModifier = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "maxManaModifierEnemy") {
				maxManaModifierEnemy = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "maxManaModifier") {
				maxManaModifier = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "turnManaRegenModifierEnemy") {
				turnManaRegenModifierEnemy = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "turnManaRegenModifier") {
				turnManaRegenModifier = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "battleManaRegenModifier") {
				battleManaRegenModifier = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "poisonResistModifierEnemy") {
				poisonResistModifierEnemy = floatFromFile(&spellBlueprints);;
				if (poisonResistModifierEnemy < -1) {
					poisonResistModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "poisonResistModifier") {
				poisonResistModifier = floatFromFile(&spellBlueprints);;
				if (poisonResistModifier < -1) {
					poisonResistModifier = -1;
				}
			}
			else if (stringbuffer == "bleedResistModifierEnemy") {
				bleedResistModifierEnemy = floatFromFile(&spellBlueprints);;
				if (bleedResistModifierEnemy < -1) {
					bleedResistModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "bleedResistModifier") {
				bleedResistModifier = floatFromFile(&spellBlueprints);;
				if (bleedResistModifier < -1) {
					bleedResistModifier = -1;
				}
			}
			else if (stringbuffer == "tempRegen") {
				tempRegen = numFromFile(&spellBlueprints);;
				if (tempRegen < -255) {
					tempRegen = -255;
				}
				else if (tempRegen > 255) {
					tempRegen = 255;
				}
			}
			else if (stringbuffer == "tempRegenSelf") {
				tempRegenSelf = numFromFile(&spellBlueprints);;
				if (tempRegenSelf < -255) {
					tempRegenSelf = -255;
				}
				else if (tempRegenSelf > 255) {
					tempRegenSelf = 255;
				}
			}
			else if (stringbuffer == "constRegenModifierEnemy") {
				constRegenModifierEnemy = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "constRegenModifier") {
				constRegenModifier = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "battleRegenModifier") {
				battleRegenModifier = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "flatArmourModifierEnemy") {
				flatArmourModifierEnemy = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "flatArmourModifier") {
				flatArmourModifier = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "propArmourModifierEnemy") {
				propArmourModifierEnemy = floatFromFile(&spellBlueprints);;
				if (propArmourModifierEnemy < -1) {
					propArmourModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "propArmourModifier") {
				propArmourModifier = floatFromFile(&spellBlueprints);;
				if (propArmourModifier < -1) {
					propArmourModifier = -1;
				}
			}
			else if (stringbuffer == "flatMagicArmourModifierEnemy") {
				flatMagicArmourModifierEnemy = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "flatMagicArmourModifier") {
				flatMagicArmourModifier = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "propMagicArmourModifierEnemy") {
				propMagicArmourModifierEnemy = floatFromFile(&spellBlueprints);;
				if (propMagicArmourModifierEnemy < -1) {
					propMagicArmourModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "propMagicArmourModifier") {
				propMagicArmourModifier = floatFromFile(&spellBlueprints);;
				if (propMagicArmourModifier < -1) {
					propMagicArmourModifier = -1;
				}
			}
			else if (stringbuffer == "flatDamageModifierEnemy") {
				flatDamageModifierEnemy = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "flatDamageModifier") {
				flatDamageModifier = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "propDamageModifierEnemy") {
				propDamageModifierEnemy = floatFromFile(&spellBlueprints);;
				if (propDamageModifierEnemy < -1) {
					propDamageModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "propDamageModifier") {
				propDamageModifier = floatFromFile(&spellBlueprints);;
				if (propDamageModifier < -1) {
					propDamageModifier = -1;
				}
			}
			else if (stringbuffer == "flatMagicDamageModifierEnemy") {
				flatMagicDamageModifierEnemy = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "flatMagicDamageModifier") {
				flatMagicDamageModifier = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "propMagicDamageModifierEnemy") {
				propMagicDamageModifierEnemy = floatFromFile(&spellBlueprints);;
				if (propMagicDamageModifierEnemy < -1) {
					propMagicDamageModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "propMagicDamageModifier") {
				propMagicDamageModifier = floatFromFile(&spellBlueprints);;
				if (propMagicDamageModifier < -1) {
					propMagicDamageModifier = -1;
				}
			}
			else if (stringbuffer == "flatArmourPiercingDamageModifierEnemy") {
				flatArmourPiercingDamageModifierEnemy = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "flatArmourPiercingDamageModifier") {
				flatArmourPiercingDamageModifier = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "propArmourPiercingDamageModifierEnemy") {
				propArmourPiercingDamageModifierEnemy = floatFromFile(&spellBlueprints);;
				if (propArmourPiercingDamageModifierEnemy < -1) {
					propArmourPiercingDamageModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "propArmourPiercingDamageModifier") {
				propArmourPiercingDamageModifier = floatFromFile(&spellBlueprints);;
				if (propArmourPiercingDamageModifier < -1) {
					propArmourPiercingDamageModifier = -1;
				}
			}
			else if (stringbuffer == "evadeChanceModifierEnemy") {
				evadeChanceModifierEnemy = floatFromFile(&spellBlueprints);;
				if (evadeChanceModifierEnemy < -1) {
					evadeChanceModifierEnemy = -1;
				}
			}
			else if (stringbuffer == "evadeChanceModifier") {
				evadeChanceModifier = floatFromFile(&spellBlueprints);;
				if (evadeChanceModifier < -1) {
					evadeChanceModifier = -1;
				}
			}
			else if (stringbuffer == "cooldown") {
				charBuf = numFromFile(&spellBlueprints);;
				if (charBuf < 1) {
					charBuf = 1;
				}
				else if (charBuf > 255) {
					charBuf = 255;
				}
				cooldown = static_cast<unsigned char>(charBuf);
			}
			else if (stringbuffer == "spellType") {
				charBuf = numFromFile(&spellBlueprints);;
				if (charBuf < 1 || charBuf > SPELL_TYPES_NO) {
					charBuf = 0;
				}
				spellType = static_cast<unsigned char>(charBuf);
			}
			else if (stringbuffer == "counterHits") {
				charBuf = numFromFile(&spellBlueprints);;
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
				charBuf = numFromFile(&spellBlueprints);;
				if (charBuf < 0 || charBuf > 2) {
					charBuf = 0;
				}
				timing = static_cast<unsigned char>(charBuf);
			}
			else if (stringbuffer == "counterSpell") {
				charBuf = numFromFile(&spellBlueprints);;
				if (charBuf < 0 || charBuf > 3) {
					charBuf = 0;
				}
				counterSpell = static_cast<unsigned char>(charBuf);
			}
			else if (stringbuffer == "bonusActionsModifierEnemy") {
				bonusActionsModifierEnemy = numFromFile(&spellBlueprints);;
				if (bonusActionsModifierEnemy < -255) {
					bonusActionsModifierEnemy = -255;
				}
				else if (bonusActionsModifierEnemy > 255) {
					bonusActionsModifierEnemy = 255;
				}
			}
			else if (stringbuffer == "bonusActionsModifier") {
				bonusActionsModifier = numFromFile(&spellBlueprints);;
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
				battleManaRegenModifierEnemy = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "battleRegenModifierEnemy") {
				battleRegenModifierEnemy = numFromFile(&spellBlueprints);;
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
				healthChange = numFromFile(&spellBlueprints);;
			}
			else if (stringbuffer == "selfOverheal/") {
				selfOverheal = true;
				ignoreLine(&spellBlueprints);
				if (!spellBlueprints) {
					throw 1;
				}
				stringbuffer = getTag(&spellBlueprints);
				continue;
			}
			else if (stringbuffer == "targetOverheal/") {
				targetOverheal = true;
				ignoreLine(&spellBlueprints);
				if (!spellBlueprints) {
					throw 1;
				}
				stringbuffer = getTag(&spellBlueprints);
				continue;
			}
			else if (stringbuffer == "upgrade") {
				upgrade = stringFromFile(&spellBlueprints);
			}
			else if (stringbuffer == "initiativeModifier") {
				initiativeModifier = numFromFile(&spellBlueprints);;
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
		setEffectType();
		spellBlueprints.close();
	}
	catch (int err) {
		spellBlueprints.close();
		spellName = upgrade = "EMPTY";
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
		cout << "None\n";
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
	if (targetOverheal) {
		cout << "May overheal target\n";
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
	if (selfOverheal) {
		cout << "May overheal caster\n";
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
	if (currentCooldown > 0) {
		cout << "- on cooldown ";
	}
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
		cout << '\b' << '\b' << ')';
	}
}

void spell::decCooldown() {
	if (currentCooldown > 0) {
		currentCooldown--;
	}
}

bool spell::checkSelfEffect() {
	if (propSelfDamage != 0) {
		return true;
	}
	if (selfPoison != 0 || selfBleed != 0 || tempRegenSelf != 0) {
		return true;
	}
	if (maxHealthModifier != 0 || maxManaModifier != 0) {
		return true;
	}
	if (turnManaRegenModifier != 0 || constRegenModifier != 0) {
		return true;
	}
	if (poisonResistModifier != 0 || bleedResistModifier != 0) {
		return true;
	}
	if (flatArmourModifier != 0 || propArmourModifier != 0 || flatMagicArmourModifier != 0 || propMagicArmourModifier != 0) {
		return true;
	}
	if (flatDamageModifier != 0 || flatMagicDamageModifier != 0 || flatArmourPiercingDamageModifier != 0) {
		return true;
	}
	if (propDamageModifier != 0 || propMagicDamageModifier != 0 || propArmourPiercingDamageModifier != 0) {
		return true;
	}
	if (evadeChanceModifier != 0 || bonusActionsModifier != 0) {
		return true;
	}
	return false;
}

bool spell::checkTargetEffect() {
	if (propDamage != 0) {
		return true;
	}
	if (manaChangeEnemy != 0) {
		return 0;
	}
	if (poison != 0 || bleed != 0 || tempRegen != 0) {
		return true;
	}
	if (maxHealthModifierEnemy != 0 || maxManaModifierEnemy != 0) {
		return true;
	}
	if (turnManaRegenModifierEnemy != 0 || constRegenModifierEnemy != 0) {
		return true;
	}
	if (poisonResistModifierEnemy != 0 || bleedResistModifierEnemy != 0) {
		return true;
	}
	if (flatArmourModifierEnemy != 0 || propArmourModifierEnemy != 0 || flatMagicArmourModifierEnemy != 0 || propMagicArmourModifierEnemy != 0) {
		return true;
	}
	if (flatDamageModifierEnemy != 0 || flatMagicDamageModifierEnemy != 0 || flatArmourPiercingDamageModifierEnemy != 0) {
		return true;
	}
	if (propDamageModifierEnemy != 0 || propMagicDamageModifierEnemy != 0 || propArmourPiercingDamageModifierEnemy != 0) {
		return true;
	}
	if (evadeChanceModifierEnemy != 0 || bonusActionsModifierEnemy != 0) {
		return true;
	}
	return false;
}

void spell::setEffectType() {
	if (checkSelfEffect()) {
		if (checkTargetEffect()) {
			effectType = 2;
		}
		else {
			effectType = 1;
		}
	}
	else {
		if (checkTargetEffect()) {
			effectType = 3;
		}
		else {
			effectType = 0;
		}
	}
	if (checkSelfDamage()) {
		if (checkTargetDamage()) {
			effectType += 20;
		}
		else {
			effectType += 10;
		}
	}
	else {
		if (checkTargetDamage()) {
			effectType += 30;
		}
	}
}

bool spell::checkTargetDamage() {
	if (flatDamageMin != 0 || flatDamageMax != 0) {
		return true;
	}
	if (flatMagicDamageMin != 0 || flatMagicDamageMax != 0) {
		return true;
	}
	if (flatArmourPiercingDamageMin != 0 || flatArmourPiercingDamageMax != 0) {
		return true;
	}
	return false;
}

bool spell::checkSelfDamage() {
	if (flatSelfDamageMin != 0 || flatSelfDamageMax != 0) {
		return true;
	}
	if (flatSelfMagicDamageMin != 0 || flatSelfMagicDamageMax != 0) {
		return true;
	}
	if (flatSelfArmourPiercingDamageMin != 0 || flatSelfArmourPiercingDamageMax != 0) {
		return true;
	}
	return false;
}

bool spell::upgradeItem() {
	if (!real) {
		cout << "Cannot upgrade empty slot!\n";
		return false;
	}
	if (upgrade == "EMPTY") {
		cout << name << " cannot be upgraded\n";
		return false;
	}
	spell newItem(upgrade);
	cout << "Current version:\n";
	displayStats();
	cout << "Upgraded version:\n";
	newItem.displayStats();
	cout << "To upgrade this item, enter 1.\nTo choose a different upgrade, enter 2.\n";
	if (userChoice(1, 2) == 2) {
		return false;
	}
	*this = newItem;
	return true;
}