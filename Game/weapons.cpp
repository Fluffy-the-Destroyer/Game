#include "weapons.h"
#include <fstream>
#include "inputs.h"
#include <iostream>
#include "resources.h"
using namespace std;


//Error codes:
// 1: Bad XML, including premature end of file or nonsense values
// 2: Specified blueprint or list not found
// 3: Loading empty slot, not technically an error
// 4: Unable to open blueprint file
// 5: Empty blueprint list

extern resource g_projName, g_manaName;

string weapon::getName() {
	if (real) {
		return name;
	}
	else {
		return "None";
	}
}

void weapon::loadFromFile(string blueprint, bool custom) { //Mostly the same as the functions to load armour from files
	ifstream weaponBlueprints;
	string stringbuffer = "";
	try { //Throws exception if cannot find properly formed blueprint
		if (blueprint == "EMPTY") {
			throw 3;
		}
		//Open blueprint file
		if (custom) {
			weaponBlueprints.open("custom\\weaponBlueprints.xml");
		}
		else {
			weaponBlueprints.open("data\\weaponBlueprints.xml");
		}
		if (!weaponBlueprints.is_open()) {
			throw 4;
		}
		string blueprintName = "weaponBlueprintList name=\"" + blueprint + '\"';
		//Check for list
		{
			bool noList = false; //Set if we do not find a list
			streampos filePos = 0; //Position in file
			short listCount = -1; //For tracking number of entries in a list and then which one we have chosen
			while (stringbuffer != blueprintName) { //Haven't found a list
				stringbuffer = getTag(&weaponBlueprints);
				ignoreLine(&weaponBlueprints);
				if (weaponBlueprints.eof()) { //Reached end of file without finding list
					weaponBlueprints.clear();
					noList = true;
					break;
				}
			}
			if (!noList) {
				filePos = weaponBlueprints.tellg();
				do {
					listCount++;
					stringbuffer = getTag(&weaponBlueprints);
					ignoreLine(&weaponBlueprints);
				} while (stringbuffer != "/weaponBlueprintList");
				weaponBlueprints.clear();
				if (listCount == 0) {
					throw 5;
				}
				listCount = rng(1, listCount);
				weaponBlueprints.seekg(filePos);
				for (int i = 1; i < listCount; i++) {
					ignoreLine(&weaponBlueprints);
				}
				if (getTag(&weaponBlueprints) != "name") {
					throw 1;
				}
				blueprint = stringFromFile(&weaponBlueprints);
				if (blueprint == "EMPTY") {
					throw 3;
				}
				if (getTag(&weaponBlueprints) != "/name") {
					throw 1;
				}
			}
			weaponBlueprints.seekg(0);
			stringbuffer = "";
		}
		//Reset attributes to default values
		real = true;
		name = description = "";
		weaponName = blueprint;
		flatDamageMin = flatDamageMax = flatMagicDamageMin = flatMagicDamageMax = flatArmourPiercingDamageMin = flatArmourPiercingDamageMax = flatSelfDamageMin = flatSelfDamageMax = flatSelfMagicDamageMin = flatSelfMagicDamageMax = flatSelfArmourPiercingDamageMin = flatSelfArmourPiercingDamageMax = manaChange = projectileChange = healthChange = 0;
		propDamage = propSelfDamage = 0;
		hitCount = 1;
		counterHits = poison = bleed = selfPoison = selfBleed = 0;
		noEvade = noCounter = noCounterAttack = lifelink = dualWield = selfOverheal = targetOverheal = false;
		upgrade = "EMPTY";
		blueprintName = "weaponBlueprint name=\"" + blueprint + '\"';
		while (stringbuffer != blueprintName) {
			stringbuffer = getTag(&weaponBlueprints);
			ignoreLine(&weaponBlueprints);
			if (weaponBlueprints.eof()) {
				throw 2;
			}
		}
		{
			short charBuf = 0; //Using chars to store integers, so I can't extract them directly as they would extract as characters. Using this to buffer them
			stringbuffer = getTag(&weaponBlueprints);
			while (stringbuffer != "/weaponBlueprint") {
				if (weaponBlueprints.eof()) {
					throw 1;
				}
				if (stringbuffer == "name") {
					name = stringFromFile(&weaponBlueprints);
				}
				else if (stringbuffer == "description") {
					description = stringFromFile(&weaponBlueprints);
				}
				else if (stringbuffer == "flatDamageMin") {
					flatDamageMin = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "flatDamageMax") {
					flatDamageMax = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "flatDamage") { //For setting a fixed damage
					flatDamageMin = numFromFile(&weaponBlueprints);;
					flatDamageMax = flatDamageMin;
				}
				else if (stringbuffer == "flatMagicDamageMin") {
					flatMagicDamageMin = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "flatMagicDamageMax") {
					flatMagicDamageMax = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "flatMagicDamage") {
					flatMagicDamageMin = numFromFile(&weaponBlueprints);;
					flatMagicDamageMax = flatMagicDamageMin;
				}
				else if (stringbuffer == "flatArmourPiercingDamageMin") {
					flatArmourPiercingDamageMin = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "flatArmourPiercingDamageMax") {
					flatArmourPiercingDamageMax = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "flatArmourPiercingDamage") {
					flatArmourPiercingDamageMin = numFromFile(&weaponBlueprints);;
					flatArmourPiercingDamageMax = flatArmourPiercingDamageMin;
				}
				else if (stringbuffer == "propDamage") {
					propDamage = floatFromFile(&weaponBlueprints);;
					if (propDamage < -1) {
						propDamage = -1;
					}
					else if (propDamage > 1) {
						propDamage = 1;
					}
				}
				else if (stringbuffer == "flatSelfDamageMin") {
					flatSelfDamageMin = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "flatSelfDamageMax") {
					flatSelfDamageMax = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "flatSelfDamage") {
					flatSelfDamageMin = numFromFile(&weaponBlueprints);;
					flatSelfDamageMax = flatSelfDamageMin;
				}
				else if (stringbuffer == "flatSelfMagicDamageMin") {
					flatSelfMagicDamageMin = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "flatSelfMagicDamageMax") {
					flatSelfMagicDamageMax = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "flatSelfMagicDamage") {
					flatSelfMagicDamageMin = numFromFile(&weaponBlueprints);;
					flatSelfMagicDamageMax = flatSelfMagicDamageMin;
				}
				else if (stringbuffer == "flatSelfArmourPiercingDamageMin") {
					flatSelfArmourPiercingDamageMin = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "flatSelfArmourPiercingDamageMax") {
					flatSelfArmourPiercingDamageMax = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "flatSelfArmourPiercingDamage") {
					flatSelfArmourPiercingDamageMin = numFromFile(&weaponBlueprints);;
					flatSelfArmourPiercingDamageMax = flatArmourPiercingDamageMin;
				}
				else if (stringbuffer == "propSelfDamage") {
					propSelfDamage = floatFromFile(&weaponBlueprints);;
					if (propSelfDamage < -1) {
						propSelfDamage = -1;
					}
					else if (propSelfDamage > 1) {
						propSelfDamage = 1;
					}
				}
				else if (stringbuffer == "hitCount") {
					charBuf = numFromFile(&weaponBlueprints);;
					if (charBuf < 0) {
						charBuf = 0;
					}
					else if (charBuf > 255) {
						charBuf = 255;
					}
					hitCount = static_cast<unsigned char>(charBuf);
				}
				else if (stringbuffer == "noEvade/") { //Self closing tag, indicates it should be true
					noEvade = true;
					ignoreLine(&weaponBlueprints);
					if (!weaponBlueprints) {
						throw 1;
					}
					stringbuffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (stringbuffer == "manaChange") {
					manaChange = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "projectileChange") {
					projectileChange = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "poison") {
					charBuf = numFromFile(&weaponBlueprints);;
					if (charBuf < 0) {
						charBuf = 0;
					}
					else if (charBuf > 255) {
						charBuf = 255;
					}
					poison = static_cast<unsigned char>(charBuf);
				}
				else if (stringbuffer == "selfPoison") {
					charBuf = numFromFile(&weaponBlueprints);;
					if (charBuf < 0) {
						charBuf = 0;
					}
					else if (charBuf > 255) {
						charBuf = 255;
					}
					selfPoison = static_cast<unsigned char>(charBuf);
				}
				else if (stringbuffer == "bleed") {
					charBuf = numFromFile(&weaponBlueprints);;
					if (charBuf < 0) {
						charBuf = 0;
					}
					else if (charBuf > 255) {
						charBuf = 255;
					}
					bleed = static_cast<unsigned char>(charBuf);
				}
				else if (stringbuffer == "selfBleed") {
					charBuf = numFromFile(&weaponBlueprints);;
					if (charBuf < 0) {
						charBuf = 0;
					}
					else if (charBuf > 255) {
						charBuf = 255;
					}
					selfBleed = static_cast<unsigned char>(charBuf);
				}
				else if (stringbuffer == "counterHits") {
					charBuf = numFromFile(&weaponBlueprints);;
					if (charBuf < 0) {
						charBuf = 0;
					}
					else if (charBuf > 255) {
						charBuf = 255;
					}
					counterHits = static_cast<unsigned char>(charBuf);
				}
				else if (stringbuffer == "noCounter/") {
					noCounter = true;
					ignoreLine(&weaponBlueprints);
					if (!weaponBlueprints) {
						throw 1;
					}
					stringbuffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (stringbuffer == "noCounterAttack/") {
					noCounterAttack = true;
					ignoreLine(&weaponBlueprints);
					if (!weaponBlueprints) {
						throw 1;
					}
					stringbuffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (stringbuffer == "lifelink/") {
					lifelink = true;
					ignoreLine(&weaponBlueprints);
					if (!weaponBlueprints) {
						throw 1;
					}
					stringbuffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (stringbuffer == "healthChange") {
					healthChange = numFromFile(&weaponBlueprints);;
				}
				else if (stringbuffer == "dualWield/") {
					dualWield = true;
					ignoreLine(&weaponBlueprints);
					if (!weaponBlueprints) {
						throw 1;
					}
					stringbuffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (stringbuffer == "selfOverheal/") {
					selfOverheal = true;
					ignoreLine(&weaponBlueprints);
					if (!weaponBlueprints) {
						throw 1;
					}
					stringbuffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (stringbuffer == "targetOverheal/") {
					targetOverheal = true;
					ignoreLine(&weaponBlueprints);
					if (!weaponBlueprints) {
						throw 1;
					}
					continue;
				}
				else if (stringbuffer == "upgrade") {
					upgrade = stringFromFile(&weaponBlueprints);
				}
				else {
					throw 1;
				}
				if (getTag(&weaponBlueprints) != '/' + stringbuffer) {
					throw 1;
				}
				ignoreLine(&weaponBlueprints);
				if (!weaponBlueprints) {
					throw 1;
				}
				stringbuffer = getTag(&weaponBlueprints);
			}
		}
		//Check all max damages are at least their corresponding min
		if (flatDamageMax < flatDamageMin || flatMagicDamageMax < flatMagicDamageMin || flatArmourPiercingDamageMax < flatArmourPiercingDamageMin || flatSelfDamageMax < flatSelfDamageMin || flatSelfMagicDamageMax < flatSelfMagicDamageMin || flatSelfArmourPiercingDamageMax < flatSelfArmourPiercingDamageMin) {
			throw 1;
		}
		setEffectType();
		weaponBlueprints.close();
	}
	catch (int err) {
		weaponBlueprints.close();
		weaponName = upgrade = "EMPTY";
		name = "";
		description = "";
		real = false;
		switch (err) {
		case 1:
			cout << "Unable to parse blueprint or blueprintList " << blueprint << ". Using default weapon.\n";
			break;
		case 2:
			if (custom) {
				loadFromFile(blueprint, false);
				return;
			}
			cout << "No blueprint or blueprintList found with name " << blueprint << ". Using default weapon.\n";
			break;
		case 4:
			if (custom) {
				loadFromFile(blueprint, false);
				return;
			}
			cout << "Could not open weaponBlueprints.xml, using default weapon.\n";
			break;
		case 5:
			cout << "blueprintList " << blueprint << " contains no entries, using default weapon.\n";
			break;
		}
	}
}

void weapon::displayStats() {
	if (!real) {
		cout << "None\n";
		return;
	}
	//Name
	cout << name << "\n\n";
	//Description
	cout << description << '\n';
	{
		int healingMin = 0, healingMax = 0; //For enemy healing
	//Flat damage
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
	//Flat magic damage
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
		cout << "Attacks may overheal the target\n";
	}
	//Proportional damage
	if (propDamage > 0) {
		cout << "Reduces target's health by " << 100 * propDamage << "%\n";
	}
	else if (propDamage < 0) {
		cout << "Heals target for " << -100 * propDamage << "% of their maximum health\n";
	}
	{
		int selfHealingMin = 0, selfHealingMax = 0;
	//Flat self damage
		if (flatSelfDamageMin == flatSelfDamageMax) {
			if (flatSelfDamageMax > 0) {
				cout << "Deals " << flatSelfDamageMax << " physical damage to user on attack\n";
			}
			else if (flatSelfDamageMax < 0) {
				selfHealingMin -= flatSelfDamageMax;
				selfHealingMax -= flatSelfDamageMax;
			}
		}
		else if (flatSelfDamageMin >= 0) {
			cout << "Deals " << flatSelfDamageMin << " to " << flatSelfDamageMax << " physical damage to user on attack\n";
		}
		else if (flatSelfDamageMax <= 0) {
			selfHealingMin -= flatSelfDamageMax;
			selfHealingMax -= flatSelfDamageMin;
		}
		else {
			cout << "Deals " << flatDamageMin << " to " << flatSelfDamageMax << " physical damage to user on attack, negative damage will heal\n";
		}
	//Flat self magic damage
		if (flatSelfMagicDamageMin == flatSelfMagicDamageMax) {
			if (flatSelfMagicDamageMax > 0) {
				cout << "Deals " << flatSelfMagicDamageMax << " magic damage to user on attack\n";
			}
			else if (flatSelfMagicDamageMax < 0) {
				selfHealingMin -= flatSelfMagicDamageMax;
				selfHealingMax -= flatSelfMagicDamageMax;
			}
		}
		else if (flatSelfMagicDamageMin >= 0) {
			cout << "Deals " << flatSelfMagicDamageMin << " to " << flatSelfMagicDamageMax << " magic damage to user on attack\n";
		}
		else if (flatSelfMagicDamageMax <= 0) {
			selfHealingMin -= flatSelfMagicDamageMax;
			selfHealingMax -= flatSelfMagicDamageMin;
		}
		else {
			cout << "Deals " << flatSelfMagicDamageMin << " to " << flatSelfMagicDamageMax << " magic damage to user on attack, negative damage will heal\n";
		}
	//Flat self AP damage
		if (flatSelfArmourPiercingDamageMin == flatSelfArmourPiercingDamageMax) {
			if (flatSelfArmourPiercingDamageMax > 0) {
				cout << "Deals " << flatSelfArmourPiercingDamageMax << " armour piercing damage to user on attack\n";
			}
			else if (flatSelfArmourPiercingDamageMax < 0) {
				selfHealingMin -= flatSelfArmourPiercingDamageMax;
				selfHealingMax -= flatSelfArmourPiercingDamageMax;
			}
		}
		else if (flatSelfArmourPiercingDamageMin >= 0) {
			cout << "Deals " << flatSelfArmourPiercingDamageMin << " to " << flatSelfArmourPiercingDamageMax << " armour piercing damage to user on attack\n";
		}
		else if (flatSelfArmourPiercingDamageMax <= 0) {
			selfHealingMin -= flatSelfArmourPiercingDamageMax;
			selfHealingMax -= flatSelfArmourPiercingDamageMin;
		}
		else {
			cout << "Deals " << flatSelfArmourPiercingDamageMin << " to " << flatSelfArmourPiercingDamageMax << " armour piercing damage to user on attack, negative damage will heal\n";
		}
		if (selfHealingMax > 0) {
			cout << "Heals the user for " << selfHealingMin;
			if (selfHealingMax != selfHealingMin) {
				cout << " to " << selfHealingMax;
			}
			cout << " on attack\n";
		}
	}
	if (selfOverheal) {
		cout << "May overheal user\n";
	}
	//Prop self damage
	if (propSelfDamage > 0) {
		cout << "Reduces user's health by " << 100 * propSelfDamage << "% on attack\n";
	}
	else if (propSelfDamage < 0) {
		cout << "Heals user for " << -100 * propSelfDamage << "% of their maximum health on attack\n";
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
		cout << "On dealing damage to target, heals the user by that much\n";
	}
	//Hit count
	if (hitCount == 0) {
		cout << "Cannot attack\n";
	}
	else if (hitCount != 1) {
		cout << "Hits " << +hitCount << " times per attack\n";
	}
	//Counter attack
	if (counterHits == 1) {
		cout << "Usable for counter attacks, hits once\n";
	}
	else if (counterHits > 0) {
		cout << "Usable for counter attacks, hits " << +counterHits << " times\n";
	}
	//Dual wielding
	if (dualWield) {
		cout << "Can be dual wielded\n";
	}
	//Evasion
	if (noEvade) {
		cout << "Cannot be dodged\n";
	}
	//Counter attacks
	if (noCounterAttack) {
		cout << "Cannot be counter attacked\n";
	}
	//Attack countering
	if (noCounter) {
		cout << "Effects cannot be countered by spells\n";
	}
	//Mana cost
	if (manaChange == -1) {
		cout << "Costs 1 " << g_manaName.singular() << " to attack\n";
	}
	else if (manaChange < -1) {
		cout << "Costs " << -manaChange << ' ' << g_manaName.plural() << " to attack\n";
	}
	else if (manaChange == 1) {
		cout << "Gain 1 " << g_manaName.singular() << " on attack\n";
	}
	else if (manaChange > 1) {
		cout << "Gain " << manaChange << ' ' << g_manaName.plural() << " on attack\n";
	}
	//Projectile cost
	if (projectileChange == -1) {
		cout << "Requires 1 " << g_projName.singular() << " to attack\n";
	}
	else if (projectileChange < -1) {
		cout << "Requires " << -projectileChange << ' ' << g_projName.plural() << " to attack\n";
	}
	else if (projectileChange == 1) {
		cout << "Regain 1" << g_projName.singular() << " on attack\n";
	}
	else if (projectileChange > 1) {
		cout << "Regain " << projectileChange << ' ' << g_projName.plural() << " on attack\n";
	}
	//Poison
	if (poison > 0) {
		cout << "Applies " << +poison << " poison on hit\n";
	}
	//Self poison
	if (selfPoison > 0) {
		cout << "Applies " << +selfPoison << " poison to user on attack\n";
	}
	//Bleed
	if (bleed > 0) {
		cout << "Applies " << +bleed << " bleed on hit\n";
	}
	//Self bleed
	if (selfBleed > 0) {
		cout << "Applies " << +selfBleed << " bleed to user on attack\n";
	}
}

void weapon::displayName() {
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
		cout << '\b' << '\b' << ')';
	}
}

bool weapon::checkSelfEffect() {
	if (propSelfDamage != 0) {
		return true;
	}
	if (selfPoison != 0 || selfBleed != 0) {
		return true;
	}
	return false;
}

bool weapon::checkTargetEffect() {
	if (propDamage != 0) {
		return true;
	}
	if (poison != 0 || bleed != 0) {
		return true;
	}
	return false;
}

void weapon::setEffectType() {
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

bool weapon::checkTargetDamage() {
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

bool weapon::checkSelfDamage() {
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

bool weapon::upgradeItem() {
	if (!real) {
		cout << "Cannot upgrade empty slot!\n";
		return false;
	}
	if (upgrade == "EMPTY") {
		cout << name << " cannot be upgraded\n";
		return false;
	}
	weapon newItem(upgrade);
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