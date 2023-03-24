#include "armour.h"
#include <fstream>
#include "inputs.h"
#include "rng.h"
#include <iostream>
#include "resources.h"
using namespace std;

//Error codes:
// 1: Bad XML, including premature end of file or nonsense values
// 2: Specified blueprint or list not found
// 3: Loading empty slot, not technically an error
// 4: Unable to open blueprint file
// 5: Empty blueprint list

extern resource g_manaName;

string armour::getName() {
	if (real) {
		return name;
	}
	else {
		return "None";
	}
}

void armour::loadFromFile(string blueprint, bool custom) {
	ifstream armourBlueprints;
	string buffer = "";
	string type = "";
	switch (armourType()) {
	case 1:
		type = "armourHead";
		break;
	case 2:
		type = "armourTorso";
		break;
	case 3:
		type = "armourLegs";
		break;
	case 4:
		type = "armourFeet";
		break;
	}
	try { //Will throw an exception if it fails to find a properly formed blueprint
		if (blueprint == "EMPTY") { //Refers to an empty armour slot
			throw 3;
		}
		//Open blueprint file
		if (custom) {
			armourBlueprints.open("custom\\armourBlueprints.xml");
		}
		else {
			armourBlueprints.open("data\\armourBlueprints.xml");
		}
		if (!armourBlueprints.is_open()) { //Could not open file
			throw 4;
		}
		ignoreLine(&armourBlueprints, '<');
		if (custom && armourBlueprints.eof()) {
			throw 4;
		}
		armourBlueprints.seekg(-1, ios_base::cur);
		string blueprintName = type + "BlueprintList name=\"" + blueprint + '\"';
		bool customFile = custom;
		//Check for a blueprint list
		{
			bool noList = false; //Set when end of file is reached without finding a list
			streampos filePos = 0; //Position in file
			short listCount = -1; //Number of items in a list, initialising to -1 to streamline later code, also using to store which entry we have chosen
			while (true) {
				while (buffer != blueprintName) { //Haven't found a list
					buffer = getTag(&armourBlueprints);
					ignoreLine(&armourBlueprints);
					if (armourBlueprints.eof()) { //Reached end of file without finding list. If the last line of the file is the opening tag of a matching list, it would be bad XML so that case can be ignored.
						armourBlueprints.clear(); //Reset eof bit and (if necessary) fail bit
						noList = true;
						break;
					}
				}
				if (!noList) { //Found a list, position in the file is start of line containing first item in list
					filePos = armourBlueprints.tellg(); //Store file position
					blueprintName = '/' + type + "BlueprintList";
					do {
						listCount++;
						buffer = getTag(&armourBlueprints);
						ignoreLine(&armourBlueprints);
					} while (buffer != blueprintName);
					armourBlueprints.clear(); //In case the closing tag of the list was the end of the file
					if (listCount == 0) { //Empty list
						throw 5;
					}
					listCount = rng(1, listCount); //Pick a random entry in the list
					armourBlueprints.seekg(filePos); //Go back to start of first entry
					for (int i = 1; i < listCount; i++) { //Ignore lines up to the one we picked
						ignoreLine(&armourBlueprints);
					}
					if (getTag(&armourBlueprints) != "name") { //It should be this
						throw 1;
					}
					blueprint = stringFromFile(&armourBlueprints);
					if (blueprint == "EMPTY") { //Selected entry is the empty slot
						throw 3;
					}
					if (getTag(&armourBlueprints) != "/name") {
						throw 1;
					}
				}
				else if (customFile) {
					armourBlueprints.close();
					armourBlueprints.open("data\\armourBlueprints.xml");
					if (!armourBlueprints.is_open()) {
						armourBlueprints.open("custom\\armourBlueprints.xml");
						if (!armourBlueprints.is_open()) {
							custom = false;
							throw 4;
						}
						break;
					}
					customFile = noList = false;
					filePos = 0;
					listCount = -1;
					continue;
				}
				break;
			}
		}
		if (customFile != custom) {
			armourBlueprints.close();
			armourBlueprints.open("custom\\armourBlueprints.xml");
			if (!armourBlueprints.is_open()) {
				armourBlueprints.open("data\\armourBlueprints.xml");
				if (!armourBlueprints.is_open()) {
					custom = false;
					throw 4;
				}
			}
			else {
				customFile = true;
			}
		}
		armourBlueprints.seekg(0); //Go back to beginning of file
		buffer = "";
		//Reset attributes to default values
		real = true;
		maxHealthModifier = maxManaModifier = turnManaRegenModifier = battleManaRegenModifier = turnRegenModifier = battleRegenModifier = flatArmourModifier = flatMagicArmourModifier = flatDamageModifier = flatMagicDamageModifier = flatArmourPiercingDamageModifier = bonusActionsModifier = initiativeModifier = 0;
		propArmourModifier = propMagicArmourModifier = propDamageModifier = propMagicDamageModifier = propArmourPiercingDamageModifier = evadeChanceModifier = poisonResistModifier = bleedResistModifier = counterAttackChanceModifier = 0;
		name = description = "";
		upgrade = "EMPTY";
		//Find and read actual blueprint
		blueprintName = type + "Blueprint name=\"" + blueprint + '\"';
		while (true) {
			while (buffer != blueprintName) { //Haven't found a blueprint
				buffer = getTag(&armourBlueprints);
				ignoreLine(&armourBlueprints);
				if (armourBlueprints.eof()) { //Reached end of file without finding blueprint. If the last line of the file is the opening tag of a matching blueprint, it would be bad XML so that case can be ignored.
					break;
				}
			}
			if (armourBlueprints.eof()) {
				if (customFile) {
					armourBlueprints.close();
					armourBlueprints.open("data\\armourBlueprints.xml");
					if (!armourBlueprints.is_open()) {
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
		blueprintName = '/' + type + "Blueprint";
		buffer = getTag(&armourBlueprints); //Get the tag
		while (buffer != blueprintName) { //Keep reading data until we reach the end of the blueprint, this is what getTag will return
			if (buffer == "maxHealthModifier") { //Set the appropriate attribute, then ignore the rest of the line.
				maxHealthModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "maxManaModifier") {
				maxManaModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "turnManaRegenModifier") {
				turnManaRegenModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "battleManaRegenModifier") {
				battleManaRegenModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "turnRegenModifier") {
				turnRegenModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "battleRegenModifier") {
				battleRegenModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "flatArmourModifier") {
				flatArmourModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "propArmourModifier") {
				propArmourModifier = floatFromFile(&armourBlueprints);
				if (propArmourModifier < -1) {
					propArmourModifier = -1;
				}
			}
			else if (buffer == "flatMagicArmourModifier") {
				flatMagicArmourModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "propMagicArmourModifier") {
				propMagicArmourModifier = floatFromFile(&armourBlueprints);
				if (propMagicArmourModifier < -1) {
					propMagicArmourModifier = -1;
				}
			}
			else if (buffer == "flatDamageModifier") {
				flatDamageModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "propDamageModifier") {
				propDamageModifier = floatFromFile(&armourBlueprints);
				if (propDamageModifier < -1) {
					propDamageModifier = -1;
				}
			}
			else if (buffer == "evadeChanceModifier") {
				evadeChanceModifier = floatFromFile(&armourBlueprints);
				if (evadeChanceModifier < -1) {
					evadeChanceModifier = -1;
				}
			}
			else if (buffer == "poisonResistModifier") {
				poisonResistModifier = floatFromFile(&armourBlueprints);
				if (poisonResistModifier < -1) {
					poisonResistModifier = -1;
				}
			}
			else if (buffer == "bleedResistModifier") {
				bleedResistModifier = floatFromFile(&armourBlueprints);
				if (bleedResistModifier < -1) {
					bleedResistModifier = -1;
				}
			}
			else if (buffer == "flatMagicDamageModifier") {
				flatMagicDamageModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "propMagicDamageModifier") {
				propMagicDamageModifier = floatFromFile(&armourBlueprints);
				if (propMagicDamageModifier < -1) {
					propMagicDamageModifier = -1;
				}
			}
			else if (buffer == "flatArmourPiercingDamageModifier") {
				flatArmourPiercingDamageModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "propArmourPiercingDamageModifier") {
				propArmourPiercingDamageModifier = floatFromFile(&armourBlueprints);
				if (propMagicDamageModifier < -1) {
					propMagicDamageModifier = -1;
				}
			}
			else if (buffer == "counterAttackChanceModifier") {
				counterAttackChanceModifier = floatFromFile(&armourBlueprints);
				if (counterAttackChanceModifier < -1) {
					counterAttackChanceModifier = -1;
				}
			}
			else if (buffer == "name") {
				name = stringFromFile(&armourBlueprints);
			}
			else if (buffer == "description") {
				description = stringFromFile(&armourBlueprints);
			}
			else if (buffer == "bonusActionsModifier") {
				bonusActionsModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "initiativeModifier") {
				initiativeModifier = numFromFile(&armourBlueprints);
			}
			else if (buffer == "upgrade") {
				upgrade = stringFromFile(&armourBlueprints);
			}
			else {
				throw 1;
			}
			if (getTag(&armourBlueprints) != '/' + buffer) { //Closing tag is different from opening tag
				throw 1;
			}
			ignoreLine(&armourBlueprints);
			buffer = getTag(&armourBlueprints); //Get the tag
		}
		armourBlueprints.close();
	}
	catch (int err) { //Default values for an empty slot
		armourBlueprints.close();
		real = false;
		name = "";
		description = "";
		switch (err) {
		case 1:
			cout << "Unable to parse blueprint or blueprintList " << blueprint << ". Using default armour.\n";
			break;
		case 2:
			cout << "No blueprint or blueprintList found with name " << blueprint << ". Using default armour.\n";
			break;
		case 4:
			if (custom) {
				loadFromFile(blueprint, false);
				return;
			}
			cout << "Could not open armourBlueprints.xml, using default armour.\n";
			break;
		case 5:
			cout << "blueprintList " << blueprint << " contains no entries, using default armour.\n";
			break;
		}
	}
}

void armour::displayStats() {
	if (!real) {
		cout << "None\n";
		return;
	}
	cout << showpos;
	//Name
	cout << name << '\n';
	//Type of armour
	cout << "Equip location: ";
	switch (armourType()) {
	case 1:
		cout << "Head\n";
		break;
	case 2:
		cout << "Torso\n";
		break;
	case 3:
		cout << "Legs\n";
		break;
	case 4:
		cout << "Feet\n";
		break;
	}
	//Description
	cout << description << '\n';
	//Max health
	if (maxHealthModifier != 0) {
		cout << maxHealthModifier << " maximum health\n";
	}
	//Health regen per turn
	if (turnRegenModifier != 0) {
		cout << turnRegenModifier << " health per turn\n";
	}
	//Health regen at end of battle
	if (battleRegenModifier != 0) {
		cout << battleRegenModifier << " healing after battle\n";
	}
	//Max mana
	if (maxManaModifier != 0) {
		cout << maxManaModifier << " maximum " << g_manaName.plural() << '\n';
	}
	//Mana regen per turn
	if (turnManaRegenModifier == 1) {
		cout << "+1 " << g_manaName.singular() << " per turn\n";
	}
	else if (turnManaRegenModifier == -1) {
		cout << "-1 " << g_manaName.singular() << " per turn\n";
	}
	else if (turnManaRegenModifier != 0) {
		cout << turnManaRegenModifier << ' ' << g_manaName.plural() << " per turn\n";
	}
	//Post battle mana regeneration
	if (battleManaRegenModifier == 1) {
		cout << "+1 " << g_manaName.singular() << " recovered after battle\n";
	}
	else if (battleManaRegenModifier == -1) {
		cout << "-1 " << g_manaName.singular() << " recovered after battle\n";
	}
	else if (battleManaRegenModifier != 0) {
		cout << battleManaRegenModifier << ' ' << g_manaName.plural() << " recovered after battle\n";
	}
	//Flat armour
	if (flatArmourModifier != 0) {
		cout << flatArmourModifier << " physical armour rating\n";
	}
	//Proportional armour
	if (propArmourModifier != 0) {
		cout << 100 * propArmourModifier << "% physical damage received\n";
	}
	//Flat magic armour
	if (flatMagicArmourModifier != 0) {
		cout << flatMagicArmourModifier << " magic armour rating\n";
	}
	//Proportional magic armour
	if (propMagicArmourModifier != 0) {
		cout << 100 * propMagicArmourModifier << "% magic damage received\n";
	}
	//Flat damage modifier
	if (flatDamageModifier != 0) {
		cout << flatDamageModifier << " physical damage\n";
	}
	//Prop damage modifier
	if (propDamageModifier != 0) {
		cout << 100 * propDamageModifier << "% physical damage\n";
	}
	//Flat magic damage
	if (flatMagicDamageModifier != 0) {
		cout << flatMagicDamageModifier << " magic damage\n";
	}
	//Prop magic damage
	if (propMagicDamageModifier != 0) {
		cout << 100 * propMagicDamageModifier << "% magic damage\n";
	}
	//Flat AP damage
	if (flatArmourPiercingDamageModifier != 0) {
		cout << flatArmourPiercingDamageModifier << " armour piercing damage\n";
	}
	//Prop AP damage
	if (propArmourPiercingDamageModifier != 0) {
		cout << 100 * propArmourPiercingDamageModifier << "% armour piercing damage\n";
	}
	//Evade chance
	if (evadeChanceModifier != 0) {
		cout << 100 * evadeChanceModifier << "% evade chance\n";
	}
	//Counter attack chance
	if (counterAttackChanceModifier != 0) {
		cout << 100 * counterAttackChanceModifier << "% chance to counter attack\n";
	}
	//Poison resist
	if (poisonResistModifier != 0) {
		cout << 100 * poisonResistModifier << "% poison resistance\n";
	}
	//Bleed resist
	if (bleedResistModifier != 0) {
		cout << 100 * bleedResistModifier << "% bleed resistance\n";
	}
	//Bonus actions
	if (bonusActionsModifier == 1) {
		cout << "+1 bonus action\n";
	}
	else if (bonusActionsModifier == -1) {
		cout << "-1 bonus action\n";
	}
	else if (bonusActionsModifier != 0) {
		cout << bonusActionsModifier << " bonus actions\n";
	}
	if (initiativeModifier != 0) {
		cout << initiativeModifier << " speed\n";
	}
	cout << noshowpos;
}

bool armourHead::upgradeItem() {
	if (!real) {
		cout << "Cannot upgrade empty slot!\n";
		return false;
	}
	if (upgrade == "EMPTY") {
		cout << name << " cannot be upgraded\n";
		return false;
	}
	armourHead newItem(upgrade);
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

bool armourTorso::upgradeItem() {
	if (!real) {
		cout << "Cannot upgrade empty slot!\n";
		return false;
	}
	if (upgrade == "EMPTY") {
		cout << name << " cannot be upgraded\n";
		return false;
	}
	armourTorso newItem(upgrade);
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

bool armourLegs::upgradeItem() {
	if (!real) {
		cout << "Cannot upgrade empty slot!\n";
		return false;
	}
	if (upgrade == "EMPTY") {
		cout << name << " cannot be upgraded\n";
		return false;
	}
	armourLegs newItem(upgrade);
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

bool armourFeet::upgradeItem() {
	if (!real) {
		cout << "Cannot upgrade empty slot!\n";
		return false;
	}
	if (upgrade == "EMPTY") {
		cout << name << " cannot be upgraded\n";
		return false;
	}
	armourFeet newItem(upgrade);
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

void armour::save(ofstream* saveFile) {
	string type;
	switch (armourType()) {
	case 1:
		type = "Head";
		break;
	case 2:
		type = "Torso";
		break;
	case 3:
		type = "Legs";
		break;
	case 4:
		type = "Feet";
		break;
	default:
		return;
	}
	if (real) {
		*saveFile << "\t\t<armour" << type << ">\n";
			*saveFile << "\t\t\t<name>" << addEscapes(name) << "</name>\n";
			*saveFile << "\t\t\t<description>" << addEscapes(description) << "</description>\n";
			if (maxHealthModifier != 0) {
				*saveFile << "\t\t\t<maxHealthModifier>" << maxHealthModifier << "</maxHealthModifier>\n";
			}
			if (maxManaModifier != 0) {
				*saveFile << "\t\t\t<maxManaModifier>" << maxManaModifier << "</maxManaModifier>\n";
			}
			if (turnManaRegenModifier != 0) {
				*saveFile << "\t\t\t<turnManaRegenModifier>" << turnManaRegenModifier << "</turnManaRegenModifier>\n";
			}
			if (battleManaRegenModifier != 0) {
				*saveFile << "\t\t\t<battleManaRegenModifier>" << battleManaRegenModifier << "</battleManaRegenModifier>\n";
			}
			if (turnRegenModifier != 0) {
				*saveFile << "\t\t\t<turnRegenModifier>" << turnRegenModifier << "</turnRegenModifier>\n";
			}
			if (battleRegenModifier != 0) {
				*saveFile << "\t\t\t<battleRegenModifier>" << battleRegenModifier << "</battleRegenModifier>\n";
			}
			if (flatArmourModifier != 0) {
				*saveFile << "\t\t\t<flatArmourModifier>" << flatArmourModifier << "</flatArmourModifier>\n";
			}
			if (propArmourModifier != 0) {
				*saveFile << "\t\t\t<propArmourModifier>" << propArmourModifier << "</propArmourModifier>\n";
			}
			if (flatMagicArmourModifier != 0) {
				*saveFile << "\t\t\t<flatMagicArmourModifier>" << flatMagicArmourModifier << "</flatMagicArmourModifier>\n";
			}
			if (propMagicArmourModifier != 0) {
				*saveFile << "\t\t\t<propMagicArmourModifier>" << propMagicArmourModifier << "</propMagicArmourModifier>\n";
			}
			if (flatDamageModifier != 0) {
				*saveFile << "\t\t\t<flatDamageModifier>" << flatDamageModifier << "</flatDamageModifier>\n";
			}
			if (propDamageModifier != 0) {
				*saveFile << "\t\t\t<propDamageModifier>" << propDamageModifier << "</propDamageModifier>\n";
			}
			if (flatMagicDamageModifier != 0) {
				*saveFile << "\t\t\t<flatMagicDamageModifier>" << flatMagicDamageModifier << "</flatMagicDamageModifier>\n";
			}
			if (propMagicDamageModifier != 0) {
				*saveFile << "\t\t\t<propMagicDamageModifier>" << propMagicDamageModifier << "</propMagicDamageModifier>\n";
			}
			if (flatArmourPiercingDamageModifier != 0) {
				*saveFile << "\t\t\t<flatArmourPiercingDamageModifier>" << flatArmourPiercingDamageModifier << "</flatArmourPiercingDamageModifier>\n";
			}
			if (propArmourPiercingDamageModifier != 0) {
				*saveFile << "\t\t\t<propArmourPiercingDamageModifier>" << propArmourPiercingDamageModifier << "</propArmourPiercingDamageModifier>\n";
			}
			if (evadeChanceModifier != 0) {
				*saveFile << "\t\t\t<evadeChanceModifier>" << evadeChanceModifier << "</evadeChanceModifier>\n";
			}
			if (poisonResistModifier != 0) {
				*saveFile << "\t\t\t<poisonResistModifier>" << poisonResistModifier << "</poisonResistModifier>\n";
			}
			if (bleedResistModifier != 0) {
				*saveFile << "\t\t\t<bleedResistModifier>" << bleedResistModifier << "</bleedResistModifier>\n";
			}
			if (counterAttackChanceModifier != 0) {
				*saveFile << "\t\t\t<counterAttackChanceModifier>" << counterAttackChanceModifier << "</counterAttackChanceModifier>\n";
			}
			if (bonusActionsModifier != 0) {
				*saveFile << "\t\t\t<bonusActionsModifier>" << bonusActionsModifier << "</bonusActionsModifier>\n";
			}
			if (initiativeModifier != 0) {
				*saveFile << "\t\t\t<initiativeModifier>" << initiativeModifier << "</initiativeModifier>\n";
			}
			if (upgrade != "EMPTY") {
				*saveFile << "\t\t\t<upgrade>" << addEscapes(upgrade) << "</upgrade>\n";
			}
		*saveFile << "\t\t</armour" << type << ">\n";
	}
	else {
		*saveFile << "\t\t<amour" << type << "/>\n";
	}
}

void armour::loadSave(ifstream* saveFile) {
	string type, buffer;
	switch (armourType()) {
	case 1:
		type = "armourHead";
		break;
	case 2:
		type = "armourTorso";
		break;
	case 3:
		type = "armourLegs";
		break;
	case 4:
		type = "armourFeet";
		break;
	default:
		return;
	}
	buffer = getTag(saveFile);
	if (buffer == type + '/') {
		real = false;
		ignoreLine(saveFile);
		return;
	}
	else if (buffer == type) {
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
	if (getTag(saveFile) != "/name") {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer != "description") {
		throw 1;
	}
	description = stringFromFile(saveFile);
	if (getTag(saveFile) != "/description") {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer == "maxHealthModifier") {
		*saveFile >> maxHealthModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		maxHealthModifier = 0;
	}
	if (buffer == "maxManaModifier") {
		*saveFile >> maxManaModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		maxManaModifier = 0;
	}
	if (buffer == "turnManaRegenModifier") {
		*saveFile >> turnManaRegenModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		turnManaRegenModifier = 0;
	}
	if (buffer == "battleManaRegenModifier") {
		*saveFile >> battleManaRegenModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		battleManaRegenModifier = 0;
	}
	if (buffer == "turnRegenModifier") {
		*saveFile >> turnRegenModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		turnRegenModifier = 0;
	}
	if (buffer == "battleRegenModifier") {
		*saveFile >> battleRegenModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		battleRegenModifier = 0;
	}
	if (buffer == "flatArmourModifier") {
		*saveFile >> flatArmourModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatArmourModifier = 0;
	}
	if (buffer == "propArmourModifier") {
		*saveFile >> propArmourModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		propArmourModifier = 0;
	}
	if (buffer == "flatMagicArmourModifier") {
		*saveFile >> flatMagicArmourModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatMagicArmourModifier = 0;
	}
	if (buffer == "propMagicArmourModifier") {
		*saveFile >> propMagicArmourModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		propMagicArmourModifier = 0;
	}
	if (buffer == "flatDamageModifier") {
		*saveFile >> flatDamageModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatDamageModifier = 0;
	}
	if (buffer == "propDamageModifier") {
		*saveFile >> propDamageModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		propDamageModifier = 0;
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
	if (buffer == "propMagicDamageModifier") {
		*saveFile >> propMagicDamageModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		propMagicDamageModifier = 0;
	}
	if (buffer == "flatArmourPiercingDamageModifier") {
		*saveFile >> flatArmourPiercingDamageModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatArmourPiercingDamageModifier = 0;
	}
	if (buffer == "propArmourPiercingDamageModifier") {
		*saveFile >> propArmourPiercingDamageModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		propArmourPiercingDamageModifier = 0;
	}
	if (buffer == "evadeChanceModifier") {
		*saveFile >> evadeChanceModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		evadeChanceModifier = 0;
	}
	if (buffer == "poisonResistModifier") {
		*saveFile >> poisonResistModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		poisonResistModifier = 0;
	}
	if (buffer == "bleedResistModifier") {
		*saveFile >> bleedResistModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		bleedResistModifier = 0;
	}
	if (buffer == "counterAttackChanceModifier") {
		*saveFile >> counterAttackChanceModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		counterAttackChanceModifier = 0;
	}
	if (buffer == "bonusActionsModifier") {
		*saveFile >> bonusActionsModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		bonusActionsModifier = 0;
	}
	if (buffer == "initiativeModifier") {
		*saveFile >> initiativeModifier;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		initiativeModifier = 0;
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
	if (buffer != '/' + type) {
		throw 1;
	}
	ignoreLine(saveFile);
}