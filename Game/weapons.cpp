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
	string buffer = "";
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
		ignoreLine(&weaponBlueprints, '<');
		if (custom && weaponBlueprints.eof()) {
			throw 4;
		}
		weaponBlueprints.seekg(-1, ios_base::cur);
		string blueprintName = "weaponBlueprintList name=\"" + blueprint + '\"';
		bool customFile = custom;
		//Check for list
		{
			bool noList = false; //Set if we do not find a list
			streampos filePos = 0; //Position in file
			short listCount = -1; //For tracking number of entries in a list and then which one we have chosen
			while (true) {
				while (buffer != blueprintName) { //Haven't found a list
					buffer = getTag(&weaponBlueprints);
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
						buffer = getTag(&weaponBlueprints);
						ignoreLine(&weaponBlueprints);
					} while (buffer != "/weaponBlueprintList");
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
				else if (customFile) {
					weaponBlueprints.close();
					weaponBlueprints.open("data\\weaponBlueprints.xml");
					if (!weaponBlueprints.is_open()) {
						weaponBlueprints.open("custom\\weaponBlueprints.xml");
						if (!weaponBlueprints.is_open()) {
							custom = false;
							throw 4;
						}
						break;
					}
					customFile = false;
					noList = false;
					filePos = 0;
					listCount = -1;
					continue;
				}
				break;
			}
		}
		if (customFile != custom) {
			weaponBlueprints.close();
			weaponBlueprints.open("custom\\weaponBlueprints.xml");
			if (!weaponBlueprints.is_open()) {
				weaponBlueprints.open("data\\weaponBlueprints.xml");
				if (!weaponBlueprints.is_open()) {
					custom = false;
					throw 4;
				}
			}
			else {
				customFile = true;
			}
		}
		weaponBlueprints.seekg(0);
		buffer = "";
		//Reset attributes to default values
		real = true;
		name = description = "";
		//weaponName = blueprint;
		flatDamageMin = flatDamageMax = flatMagicDamageMin = flatMagicDamageMax = flatArmourPiercingDamageMin = flatArmourPiercingDamageMax = flatSelfDamageMin = flatSelfDamageMax = flatSelfMagicDamageMin = flatSelfMagicDamageMax = flatSelfArmourPiercingDamageMin = flatSelfArmourPiercingDamageMax = manaChange = projectileChange = healthChange = flatMagicDamageModifier = 0;
		propDamage = propSelfDamage = 0;
		hitCount = 1;
		counterHits = poison = bleed = selfPoison = selfBleed = 0;
		noEvade = noCounter = noCounterAttack = lifeLink = dualWield = selfOverHeal = targetOverHeal = false;
		upgrade = "EMPTY";
		blueprintName = "weaponBlueprint name=\"" + blueprint + '\"';
		while (true) {
			while (buffer != blueprintName) {
				buffer = getTag(&weaponBlueprints);
				ignoreLine(&weaponBlueprints);
				if (weaponBlueprints.eof()) {
					break;
				}
			}
			if (weaponBlueprints.eof()) {
				if (customFile) {
					weaponBlueprints.close();
					weaponBlueprints.open("data\\weaponBlueprints.xml");
					if (!weaponBlueprints.is_open()) {
						custom = false;
						throw 4;
					}
					customFile = false;
					continue;
				}
				throw 2;
			}
			break;
		}
		{
			short charBuf = 0; //Using chars to store integers, so I can't extract them directly as they would extract as characters. Using this to buffer them
			buffer = getTag(&weaponBlueprints);
			while (buffer != "/weaponBlueprint") {
				if (buffer == "name") {
					name = stringFromFile(&weaponBlueprints);
				}
				else if (buffer == "description") {
					description = stringFromFile(&weaponBlueprints);
				}
				else if (buffer == "flatDamageMin") {
					flatDamageMin = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "flatDamageMax") {
					flatDamageMax = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "flatDamage") { //For setting a fixed damage
					flatDamageMin = numFromFile(&weaponBlueprints);;
					flatDamageMax = flatDamageMin;
				}
				else if (buffer == "flatMagicDamageMin") {
					flatMagicDamageMin = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "flatMagicDamageMax") {
					flatMagicDamageMax = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "flatMagicDamage") {
					flatMagicDamageMin = numFromFile(&weaponBlueprints);;
					flatMagicDamageMax = flatMagicDamageMin;
				}
				else if (buffer == "flatArmourPiercingDamageMin") {
					flatArmourPiercingDamageMin = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "flatArmourPiercingDamageMax") {
					flatArmourPiercingDamageMax = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "flatArmourPiercingDamage") {
					flatArmourPiercingDamageMin = numFromFile(&weaponBlueprints);;
					flatArmourPiercingDamageMax = flatArmourPiercingDamageMin;
				}
				else if (buffer == "propDamage") {
					propDamage = floatFromFile(&weaponBlueprints);;
					if (propDamage < -1) {
						propDamage = -1;
					}
					else if (propDamage > 1) {
						propDamage = 1;
					}
				}
				else if (buffer == "flatSelfDamageMin") {
					flatSelfDamageMin = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "flatSelfDamageMax") {
					flatSelfDamageMax = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "flatSelfDamage") {
					flatSelfDamageMin = numFromFile(&weaponBlueprints);;
					flatSelfDamageMax = flatSelfDamageMin;
				}
				else if (buffer == "flatSelfMagicDamageMin") {
					flatSelfMagicDamageMin = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "flatSelfMagicDamageMax") {
					flatSelfMagicDamageMax = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "flatSelfMagicDamage") {
					flatSelfMagicDamageMin = numFromFile(&weaponBlueprints);;
					flatSelfMagicDamageMax = flatSelfMagicDamageMin;
				}
				else if (buffer == "flatSelfArmourPiercingDamageMin") {
					flatSelfArmourPiercingDamageMin = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "flatSelfArmourPiercingDamageMax") {
					flatSelfArmourPiercingDamageMax = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "flatSelfArmourPiercingDamage") {
					flatSelfArmourPiercingDamageMin = numFromFile(&weaponBlueprints);;
					flatSelfArmourPiercingDamageMax = flatArmourPiercingDamageMin;
				}
				else if (buffer == "propSelfDamage") {
					propSelfDamage = floatFromFile(&weaponBlueprints);;
					if (propSelfDamage < -1) {
						propSelfDamage = -1;
					}
					else if (propSelfDamage > 1) {
						propSelfDamage = 1;
					}
				}
				else if (buffer == "hitCount") {
					charBuf = numFromFile(&weaponBlueprints);;
					if (charBuf < 0) {
						charBuf = 0;
					}
					else if (charBuf > 255) {
						charBuf = 255;
					}
					hitCount = static_cast<uint8_t>(charBuf);
				}
				else if (buffer == "noEvade/") { //Self closing tag, indicates it should be true
					noEvade = true;
					ignoreLine(&weaponBlueprints);
					buffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (buffer == "manaChange") {
					manaChange = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "projectileChange") {
					projectileChange = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "poison") {
					charBuf = numFromFile(&weaponBlueprints);;
					if (charBuf < 0) {
						charBuf = 0;
					}
					else if (charBuf > 255) {
						charBuf = 255;
					}
					poison = static_cast<uint8_t>(charBuf);
				}
				else if (buffer == "selfPoison") {
					charBuf = numFromFile(&weaponBlueprints);;
					if (charBuf < 0) {
						charBuf = 0;
					}
					else if (charBuf > 255) {
						charBuf = 255;
					}
					selfPoison = static_cast<uint8_t>(charBuf);
				}
				else if (buffer == "bleed") {
					charBuf = numFromFile(&weaponBlueprints);;
					if (charBuf < 0) {
						charBuf = 0;
					}
					else if (charBuf > 255) {
						charBuf = 255;
					}
					bleed = static_cast<uint8_t>(charBuf);
				}
				else if (buffer == "selfBleed") {
					charBuf = numFromFile(&weaponBlueprints);;
					if (charBuf < 0) {
						charBuf = 0;
					}
					else if (charBuf > 255) {
						charBuf = 255;
					}
					selfBleed = static_cast<uint8_t>(charBuf);
				}
				else if (buffer == "counterHits") {
					charBuf = numFromFile(&weaponBlueprints);;
					if (charBuf < 0) {
						charBuf = 0;
					}
					else if (charBuf > 255) {
						charBuf = 255;
					}
					counterHits = static_cast<uint8_t>(charBuf);
				}
				else if (buffer == "noCounter/") {
					noCounter = true;
					ignoreLine(&weaponBlueprints);
					buffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (buffer == "noCounterAttack/") {
					noCounterAttack = true;
					ignoreLine(&weaponBlueprints);
					buffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (buffer == "lifeLink/") {
					lifeLink = true;
					ignoreLine(&weaponBlueprints);
					buffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (buffer == "healthChange") {
					healthChange = numFromFile(&weaponBlueprints);;
				}
				else if (buffer == "dualWield/") {
					dualWield = true;
					ignoreLine(&weaponBlueprints);
					buffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (buffer == "selfOverHeal/") {
					selfOverHeal = true;
					ignoreLine(&weaponBlueprints);
					buffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (buffer == "targetOverHeal/") {
					targetOverHeal = true;
					ignoreLine(&weaponBlueprints);
					buffer = getTag(&weaponBlueprints);
					continue;
				}
				else if (buffer == "upgrade") {
					upgrade = stringFromFile(&weaponBlueprints);
				}
				else if (buffer == "flatMagicDamageModifier") {
					flatMagicDamageModifier = numFromFile(&weaponBlueprints);
				}
				else {
					throw 1;
				}
				if (getTag(&weaponBlueprints) != '/' + buffer) {
					throw 1;
				}
				ignoreLine(&weaponBlueprints);
				buffer = getTag(&weaponBlueprints);
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
		//weaponName = upgrade = "EMPTY";
		name = "";
		description = "";
		real = false;
		switch (err) {
		case 1:
			cout << "Unable to parse blueprint or blueprintList " << blueprint << ". Using default weapon.\n";
			break;
		case 2:
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
	if (targetOverHeal) {
		cout << "Attacks may over heal the target\n";
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
	if (selfOverHeal) {
		cout << "May over heal user\n";
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
	//LifeLink
	if (lifeLink) {
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
	//Magic damage modifier
	if (flatMagicDamageModifier != 0) {
		cout << showpos << flatMagicDamageModifier << " magic damage dealt (passive effect)\n" << noshowpos;
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

void weapon::save(ofstream* saveFile) {
	if (real) { //Save weapon data
		*saveFile << "\t\t<weapon>\n";
			*saveFile << "\t\t\t<name>" << addEscapes(name) << "</name>\n";
			*saveFile << "\t\t\t<description>" << addEscapes(description) << "</description>\n";
			if (flatDamageMin != 0) {
				*saveFile << "\t\t\t<flatDamageMin>" << flatDamageMin << "</flatDamageMin>\n";
			}
			if (flatDamageMax != 0) {
				*saveFile << "\t\t\t<flatDamageMax>" << flatDamageMax << "</flatDamageMax>\n";
			}
			if (flatMagicDamageMin != 0) {
				*saveFile << "\t\t\t<flatMagicDamageMin>" << flatMagicDamageMin << "</flatMagicDamageMin>\n";
			}
			if (flatMagicDamageMax != 0) {
				*saveFile << "\t\t\t<flatMagicDamageMax>" << flatMagicDamageMax << "</flatMagicDamageMax>\n";
			}
			if (flatArmourPiercingDamageMin != 0) {
				*saveFile << "\t\t\t<flatArmourPiercingDamageMin>" << flatArmourPiercingDamageMin << "</flatArmourPiercingDamageMin>\n";
			}
			if (flatArmourPiercingDamageMax != 0) {
				*saveFile << "\t\t\t<flatArmourPiercingDamageMax>" << flatArmourPiercingDamageMax << "</flatArmourPiercingDamageMax>\n";
			}
			if (propDamage != 0) {
				*saveFile << "\t\t\t<propDamage>" << propDamage << "</propDamage>\n";
			}
			if (flatSelfDamageMin != 0) {
				*saveFile << "\t\t\t<flatSelfDamageMin>" << flatSelfDamageMin << "</flatSelfDamageMin>\n";
			}
			if (flatSelfDamageMax != 0) {
				*saveFile << "\t\t\t<flatSelfDamageMax>" << flatSelfDamageMax << "</flatSelfDamageMax>\n";
			}
			if (flatSelfMagicDamageMin != 0) {
				*saveFile << "\t\t\t<flatSelfMagicDamageMin>" << flatSelfMagicDamageMin << "</flatSelfMagicDamageMin>\n";
			}
			if (flatSelfMagicDamageMax != 0) {
				*saveFile << "\t\t\t<flatSelfMagicDamageMax>" << flatSelfMagicDamageMax << "</flatSelfMagicDamageMax>\n";
			}
			if (flatSelfArmourPiercingDamageMin != 0) {
				*saveFile << "\t\t\t<flatSelfArmourPiercingDamageMin>" << flatSelfArmourPiercingDamageMin << "</flatSelfArmourPiercingDamageMin>\n";
			}
			if (flatSelfArmourPiercingDamageMax != 0) {
				*saveFile << "\t\t\t<flatSelfArmourPiercingDamageMax>" << flatSelfArmourPiercingDamageMax << "</flatSelfArmourPiercingDamageMax>\n";
			}
			if (propSelfDamage != 0) {
				*saveFile << "\t\t\t<propSelfDamage>" << propSelfDamage << "</propSelfDamage>\n";
			}
			if (healthChange != 0) {
				*saveFile << "\t\t\t<healthChange>" << healthChange << "</healthChange>\n";
			}
			if (manaChange != 0) {
				*saveFile << "\t\t\t<manaChange>" << manaChange << "</manaChange>\n";
			}
			if (projectileChange != 0) {
				*saveFile << "\t\t\t<projectileChange>" << projectileChange << "</projectileChange>\n";
			}
			if (hitCount != 1) {
				*saveFile << "\t\t\t<hitCount>" << +hitCount << "</hitCount>\n";
			}
			if (counterHits != 0) {
				*saveFile << "\t\t\t<counterHits>" << +counterHits << "</counterHits>\n";
			}
			if (noEvade) {
				*saveFile << "\t\t\t<noEvade/>\n";
			}
			if (noCounter) {
				*saveFile << "\t\t\t<noCounter/>\n";
			}
			if (noCounterAttack) {
				*saveFile << "\t\t\t<noCounterAttack/>\n";
			}
			if (poison != 0) {
				*saveFile << "\t\t\t<poison>" << +poison << "</poison>\n";
			}
			if (selfPoison != 0) {
				*saveFile << "\t\t\t<selfPoison>" << +selfPoison << "</selfPoison>\n";
			}
			if (bleed != 0) {
				*saveFile << "\t\t\t<bleed>" << +bleed << "</bleed>\n";
			}
			if (selfBleed != 0) {
				*saveFile << "\t\t\t<selfBleed>" << +selfBleed << "</selfBleed>\n";
			}
			if (lifeLink) {
				*saveFile << "\t\t\t<lifeLink/>\n";
			}
			if (dualWield) {
				*saveFile << "\t\t\t<dualWield/>\n";
			}
			if (selfOverHeal) {
				*saveFile << "\t\t\t<selfOverHeal/>\n";
			}
			if (targetOverHeal) {
				*saveFile << "\t\t\t<targetOverHeal/>\n";
			}
			if (upgrade != "EMPTY") {
				*saveFile << "\t\t\t<upgrade>" << addEscapes(upgrade) << "</upgrade>\n";
			}
			if (flatMagicDamageModifier != 0) {
				*saveFile << "\t\t\t<flatMagicDamageModifier>" << flatMagicDamageModifier << "</flatMagicDamageModifier>\n";
			}
			*saveFile << "\t\t\t<effectType>" << +effectType << "</effectType>\n";
		*saveFile << "\t\t</weapon>\n";
	}
	else { //Empty slot
		*saveFile << "\t\t<weapon/>\n";
	}
}

void weapon::loadSave(ifstream* saveFile) {
	string buffer = getTag(saveFile);
	short charBuf;
	if (buffer == "weapon/") {
		real = false;
		return;
	}
	else if (buffer == "weapon") {
		real = true;
	}
	else {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer != "name") {
		throw 1;
	}
	name = stringFromFile(saveFile);
	if (getTag(saveFile) != '/' + buffer) {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer != "description") {
		throw 1;
	}
	description = stringFromFile(saveFile);
	if (getTag(saveFile) != '/' + buffer) {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer == "flatDamageMin") {
		*saveFile >> flatDamageMin;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatDamageMin = 0;
	}
	if (buffer == "flatDamageMax") {
		*saveFile >> flatDamageMax;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatDamageMax = 0;
	}
	if (buffer == "flatMagicDamageMin") {
		*saveFile >> flatMagicDamageMin;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatMagicDamageMin = 0;
	}
	if (buffer == "flatMagicDamageMax") {
		*saveFile >> flatMagicDamageMax;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatMagicDamageMax = 0;
	}
	if (buffer == "flatArmourPiercingDamageMin") {
		*saveFile >> flatArmourPiercingDamageMin;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatArmourPiercingDamageMin = 0;
	}
	if (buffer == "flatArmourPiercingDamageMax") {
		*saveFile >> flatArmourPiercingDamageMax;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatArmourPiercingDamageMax = 0;
	}
	if (buffer == "propDamage") {
		*saveFile >> propDamage;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		propDamage = 0;
	}
	if (buffer == "flatSelfDamageMin") {
		*saveFile >> flatSelfDamageMin;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatSelfDamageMin = 0;
	}
	if (buffer == "flatSelfDamageMax") {
		*saveFile >> flatSelfDamageMax;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatSelfDamageMax = 0;
	}
	if (buffer == "flatSelfMagicDamageMin") {
		*saveFile >> flatSelfMagicDamageMin;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatSelfMagicDamageMin = 0;
	}
	if (buffer == "flatSelfMagicDamageMax") {
		*saveFile >> flatSelfMagicDamageMax;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatSelfMagicDamageMax = 0;
	}
	if (buffer == "flatSelfArmourPiercingDamageMin") {
		*saveFile >> flatSelfArmourPiercingDamageMin;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatSelfArmourPiercingDamageMin = 0;
	}
	if (buffer == "flatSelfArmourPiercingDamageMax") {
		*saveFile >> flatSelfArmourPiercingDamageMax;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatSelfArmourPiercingDamageMax = 0;
	}
	if (buffer == "propSelfDamage") {
		*saveFile >> propSelfDamage;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		propSelfDamage = 0;
	}
	if (buffer == "healthChange") {
		*saveFile >> healthChange;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		healthChange = 0;
	}
	if (buffer == "manaChange") {
		*saveFile >> manaChange;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		manaChange = 0;
	}
	if (buffer == "projectileChange") {
		*saveFile >> projectileChange;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		projectileChange = 0;
	}
	if (buffer == "hitCount") {
		*saveFile >> charBuf;
		hitCount = static_cast<uint8_t>(charBuf);
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		hitCount = 1;
	}
	if (buffer == "counterHits") {
		*saveFile >> charBuf;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		counterHits = 0;
	}
	if (buffer == "noEvade") {
		noEvade = true;
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		noEvade = false;
	}
	if (buffer == "noCounter/") {
		noCounter = true;
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		noCounter = false;
	}
	if (buffer == "noCounterAttack/") {
		noCounterAttack = true;
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		noCounterAttack = false;
	}
	if (buffer == "poison") {
		*saveFile >> charBuf;
		poison = static_cast<uint8_t>(charBuf);
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		poison = 0;
	}
	if (buffer == "selfPoison") {
		*saveFile >> charBuf;
		selfPoison = static_cast<uint8_t>(charBuf);
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		selfPoison = 0;
	}
	if (buffer == "bleed") {
		*saveFile >> charBuf;
		bleed = static_cast<uint8_t>(charBuf);
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		bleed = 0;
	}
	if (buffer == "selfBleed") {
		*saveFile >> charBuf;
		selfBleed = static_cast<uint8_t>(charBuf);
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		selfBleed = 0;
	}
	if (buffer == "lifeLink/") {
		lifeLink = true;
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		lifeLink = false;
	}
	if (buffer == "dualWield/") {
		dualWield = true;
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		dualWield = false;
	}
	if (buffer == "selfOverHeal/") {
		selfOverHeal = true;
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		selfOverHeal = false;
	}
	if (buffer == "targetOverHeal/") {
		targetOverHeal = true;
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		targetOverHeal = false;
	}
	if (buffer == "upgrade") {
		upgrade = stringFromFile(saveFile);
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		upgrade = "EMPTY";
	}
	if (buffer == "flatMagicDamageModifier") {
		*saveFile >> flatMagicDamageModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatMagicDamageModifier = 0;
	}
	if (buffer != "effectType") {
		throw 1;
	}
	*saveFile >> charBuf;
	effectType = static_cast<uint8_t>(charBuf);
	if (getTag(saveFile) != '/' + buffer) {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer != "/weapon") {
		throw 1;
	}
	ignoreLine(saveFile);
}