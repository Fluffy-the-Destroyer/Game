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
	string stringbuffer = "";
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
		string blueprintName = type + "BlueprintList name=\"" + blueprint + '\"';
		//Check for a blueprint list
		{
			bool noList = false; //Set when end of file is reached without finding a list
			streampos filePos = 0; //Position in file
			short listCount = -1; //Number of items in a list, initialising to -1 to streamline later code, also using to store which entry we have chosen
			while (stringbuffer != blueprintName) { //Haven't found a list
				stringbuffer = getTag(&armourBlueprints);
				ignoreLine(&armourBlueprints);
				if (armourBlueprints.eof()) { //Reached end of file without finding list. If the last line of the file is the opening tag of a matching list, it would be bad XML so that case can be ignored.
					armourBlueprints.clear(); //Reset eofbit and (if necessary) failbit
					noList = true;
					break;
				}
			}
			if (!noList) { //Found a list, position in the file is start of line containing first item in list
				filePos = armourBlueprints.tellg(); //Store file position
				blueprintName = '/' + type + "BlueprintList";
				do {
					listCount++;
					stringbuffer = getTag(&armourBlueprints);
					ignoreLine(&armourBlueprints);
				} while (stringbuffer != blueprintName);
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
				getline(armourBlueprints, blueprint, '<'); //Get the blueprint name
				if (blueprint == "EMPTY") { //Selected entry is the empty slot
					throw 3;
				}
				getline(armourBlueprints, stringbuffer, '>'); //Get the closing tag
				if (stringbuffer != "/name") {
					throw 1;
				}
			}
			armourBlueprints.seekg(0); //Go back to beginning of file
			stringbuffer = "";
		}
		//Reset attributes to default values
		real = true;
		armourName = blueprint;
		maxHealthModifier = maxManaModifier = turnManaRegenModifier = battleManaRegenModifier = constRegenModifier = battleRegenModifier = flatArmourModifier = flatMagicArmourModifier = flatDamageModifier = flatMagicDamageModifier = flatArmourPiercingDamageModifier = bonusActionsModifier = 0;
		propArmourModifier = propMagicArmourModifier = propDamageModifier = propMagicDamageModifier = propArmourPiercingDamageModifier = evadeChanceModifier = poisonResistModifier = bleedResistModifier = counterAttackChanceModifier = 0;
		name = description = "";
		//Find and read actual blueprint
		blueprintName = type + "Blueprint name=\"" + blueprint + '\"';
		while (stringbuffer != blueprintName) { //Haven't found a blueprint
			stringbuffer = getTag(&armourBlueprints);
			ignoreLine(&armourBlueprints);
			if (armourBlueprints.eof()) { //Reached end of file without finding blueprint. If the last line of the file is the opening tag of a matching blueprint, it would be bad XML so that case can be ignored.
				throw 2;
			}
		}
		blueprintName = '/' + type + "Blueprint";
		stringbuffer = getTag(&armourBlueprints); //Get the tag
		while (stringbuffer != blueprintName) { //Keep reading data until we reach the end of the blueprint, this is what getTag will return
			if (armourBlueprints.eof()) { //Reached end of file without finding proper closing tag for blueprint
				throw 1;
			}
			if (stringbuffer == "maxHealthModifier") { //Set the appropriate attribute, then ignore the rest of the line.
				armourBlueprints >> maxHealthModifier;
			}
			else if (stringbuffer == "maxManaModifier") {
				armourBlueprints >> maxManaModifier;
			}
			else if (stringbuffer == "turnManaRegenModifier") {
				armourBlueprints >> turnManaRegenModifier;
			}
			else if (stringbuffer == "battleManaRegenModifier") {
				armourBlueprints >> battleManaRegenModifier;
			}
			else if (stringbuffer == "constRegenModifier") {
				armourBlueprints >> constRegenModifier;
			}
			else if (stringbuffer == "battleRegenModifier") {
				armourBlueprints >> battleRegenModifier;
			}
			else if (stringbuffer == "flatArmourModifier") {
				armourBlueprints >> flatArmourModifier;
			}
			else if (stringbuffer == "propArmourModifier") {
				armourBlueprints >> propArmourModifier;
				if (propArmourModifier < -1) {
					propArmourModifier = -1;
				}
			}
			else if (stringbuffer == "flatMagicArmourModifier") {
				armourBlueprints >> flatMagicArmourModifier;
			}
			else if (stringbuffer == "propMagicArmourModifier") {
				armourBlueprints >> propMagicArmourModifier;
				if (propMagicArmourModifier < -1) {
					propMagicArmourModifier = -1;
				}
			}
			else if (stringbuffer == "flatDamageModifier") {
				armourBlueprints >> flatDamageModifier;
			}
			else if (stringbuffer == "propDamageModifier") {
				armourBlueprints >> propDamageModifier;
				if (propDamageModifier < -1) {
					propDamageModifier = -1;
				}
			}
			else if (stringbuffer == "evadeChanceModifier") {
				armourBlueprints >> evadeChanceModifier;
				if (evadeChanceModifier < -1) {
					evadeChanceModifier = -1;
				}
			}
			else if (stringbuffer == "poisonResistModifier") {
				armourBlueprints >> poisonResistModifier;
				if (poisonResistModifier < -1) {
					poisonResistModifier = -1;
				}
			}
			else if (stringbuffer == "bleedResistModifier") {
				armourBlueprints >> bleedResistModifier;
				if (bleedResistModifier < -1) {
					bleedResistModifier = -1;
				}
			}
			else if (stringbuffer == "flatMagicDamageModifier") {
				armourBlueprints >> flatMagicDamageModifier;
			}
			else if (stringbuffer == "propMagicDamageModifier") {
				armourBlueprints >> propMagicDamageModifier;
				if (propMagicDamageModifier < -1) {
					propMagicDamageModifier = -1;
				}
			}
			else if (stringbuffer == "flatArmourPiercingDamageModifier") {
				armourBlueprints >> flatArmourPiercingDamageModifier;
			}
			else if (stringbuffer == "propArmourPiercingDamageModifier") {
				armourBlueprints >> propArmourPiercingDamageModifier;
				if (propMagicDamageModifier < -1) {
					propMagicDamageModifier = -1;
				}
			}
			else if (stringbuffer == "counterAttackChanceModifier") {
				armourBlueprints >> counterAttackChanceModifier;
				if (counterAttackChanceModifier < -1) {
					counterAttackChanceModifier = -1;
				}
			}
			else if (stringbuffer == "name") {
				getline(armourBlueprints, name, '<');
				armourBlueprints.seekg(-1, ios_base::cur); //Goes back one character, to before the <
			}
			else if (stringbuffer == "description") {
				getline(armourBlueprints, description, '<');
				armourBlueprints.seekg(-1, ios_base::cur);
			}
			else if (stringbuffer == "bonusActionsModifier") {
				armourBlueprints >> bonusActionsModifier;
			}
			else {
				throw 1;
			}
			if (getTag(&armourBlueprints) != '/' + stringbuffer) { //Closing tag is different from opening tag
				throw 1;
			}
			ignoreLine(&armourBlueprints);
			if (!armourBlueprints) {
				throw 1;
			}
			stringbuffer = getTag(&armourBlueprints); //Get the tag
		}
		armourBlueprints.close();
	}
	catch (int err) { //Default values for an empty slot
		armourBlueprints.close();
		armourName = "EMPTY";
		real = false;
		name = "";
		description = "";
		switch (err) {
		case 1:
			cout << "Unable to parse blueprint or blueprintList " << blueprint << ". Using default armour.\n";
			break;
		case 2:
			if (custom) {
				loadFromFile(blueprint, false);
				return;
			}
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

//void armourTorso::loadFromFile(string blueprint) {
//	real = true;
//	ifstream armourBlueprints;
//	string stringbuffer = "";
//	try { //Will throw an exception if it fails to find a properly formed blueprint
//		if (blueprint == "EMPTY") { //Refers to an empty armour slot
//			throw 3;
//		}
//		//Open blueprint file
//		armourBlueprints.open("data\\armourBlueprints.xml");
//		if (!armourBlueprints.is_open()) { //Could not open file
//			throw 4;
//		}
//		//Check for a blueprint list
//		{
//			bool noList = false; //Set when end of file is reached without finding a list
//			streampos filePos = 0; //Position in file
//			short listCount = -1; //Number of items in a list, initialising to -1 to streamline later code, also using to store which entry we have chosen
//			while (stringbuffer != "armourTorsoBlueprintList name=\"" + blueprint + "\"") { //Haven't found a list
//				stringbuffer = getTag(&armourBlueprints);
//				ignoreLine(&armourBlueprints);
//				if (armourBlueprints.eof()) { //Reached end of file without finding list. If the last line of the file is the opening tag of a matching list, it would be bad XML so that case can be ignored.
//					armourBlueprints.clear(); //Reset eofbit and (if necessary) failbit
//					noList = true;
//					break;
//				}
//			}
//			if (!noList) { //Found a list, position in the file is start of line containing first item in list
//				filePos = armourBlueprints.tellg(); //Store file position
//				do {
//					if (armourBlueprints.eof()) { //File ends before list terminates
//						throw 1;
//					}
//					listCount++;
//					stringbuffer = getTag(&armourBlueprints);
//					ignoreLine(&armourBlueprints);
//				} while (stringbuffer != "/armourTorsoBlueprintList");
//				armourBlueprints.clear(); //In case the closing tag of the list was the end of the file
//				if (listCount == 0) { //Empty list
//					throw 5;
//				}
//				listCount = rng(1, listCount); //Pick a random entry in the list
//				armourBlueprints.seekg(filePos); //Go back to start of first entry
//				for (int i = 1; i < listCount; i++) { //Ignore lines up to the one we picked
//					ignoreLine(&armourBlueprints);
//				}
//				if (getTag(&armourBlueprints) != "name") { //It should be this
//					throw 1;
//				}
//				getline(armourBlueprints, blueprint, '<'); //Get the blueprint name
//				if (blueprint == "EMPTY") {
//					throw 3;
//				}
//				getline(armourBlueprints, stringbuffer, '>');
//				if (stringbuffer != "/name") {
//					throw 1;
//				}
//			}
//			armourBlueprints.seekg(0); //Go back to beginning of file
//			stringbuffer = "";
//		}
//		//Find and read actual blueprint
//		while (stringbuffer != "armourTorsoBlueprint name=\"" + blueprint + "\"") { //Haven't found a blueprint
//			stringbuffer = getTag(&armourBlueprints);
//			ignoreLine(&armourBlueprints);
//			if (armourBlueprints.eof()) { //Reached end of file without finding blueprint. If the last line of the file is the opening tag of a matching blueprint, it would be bad XML so that case can be ignored.
//				throw 2;
//			}
//		}
//		while (stringbuffer != "/armourTorsoBlueprint") { //Keep reading data until we reach the end of the blueprint, this is what getTag will return
//			if (armourBlueprints.eof()) { //Reached end of file without finding proper closing tag for blueprint
//				throw 1;
//			}
//			stringbuffer = getTag(&armourBlueprints); //Get the tag
//			if (stringbuffer == "maxHealthModifier") { //Set the appropriate attribute, then ignore the rest of the line.
//				armourBlueprints >> maxHealthModifier;
//			}
//			else if (stringbuffer == "maxManaModifier") {
//				armourBlueprints >> maxManaModifier;
//			}
//			else if (stringbuffer == "turnManaRegenModifier") {
//				armourBlueprints >> turnManaRegenModifier;
//			}
//			else if (stringbuffer == "battleManaRegenModifier") {
//				armourBlueprints >> battleManaRegenModifier;
//			}
//			else if (stringbuffer == "constRegenModifier") {
//				armourBlueprints >> constRegenModifier;
//			}
//			else if (stringbuffer == "battleRegenModifier") {
//				armourBlueprints >> battleRegenModifier;
//			}
//			else if (stringbuffer == "flatArmourModifier") {
//				armourBlueprints >> flatArmourModifier;
//			}
//			else if (stringbuffer == "propArmourModifier") {
//				armourBlueprints >> propArmourModifier;
//				if (propArmourModifier < -1) {
//					propArmourModifier = -1;
//				}
//			}
//			else if (stringbuffer == "flatMagicArmourModifier") {
//				armourBlueprints >> flatMagicArmourModifier;
//			}
//			else if (stringbuffer == "propMagicArmourModifier") {
//				armourBlueprints >> propMagicArmourModifier;
//				if (propMagicArmourModifier < -1) {
//					propMagicArmourModifier = -1;
//				}
//			}
//			else if (stringbuffer == "flatDamageModifier") {
//				armourBlueprints >> flatDamageModifier;
//			}
//			else if (stringbuffer == "propDamageModifier") {
//				armourBlueprints >> propDamageModifier;
//				if (propDamageModifier < -1) {
//					propDamageModifier = -1;
//				}
//			}
//			else if (stringbuffer == "evadeChanceModifier") {
//				armourBlueprints >> evadeChanceModifier;
//				if (evadeChanceModifier < -1) {
//					evadeChanceModifier = -1;
//				}
//			}
//			else if (stringbuffer == "poisonResistModifier") {
//				armourBlueprints >> poisonResistModifier;
//				if (poisonResistModifier < -1) {
//					poisonResistModifier = -1;
//				}
//			}
//			else if (stringbuffer == "bleedResistModifier") {
//				armourBlueprints >> bleedResistModifier;
//				if (bleedResistModifier < -1) {
//					bleedResistModifier = -1;
//				}
//			}
//			else if (stringbuffer == "flatMagicDamageModifier") {
//				armourBlueprints >> flatMagicDamageModifier;
//			}
//			else if (stringbuffer == "propMagicDamageModifier") {
//				armourBlueprints >> propMagicDamageModifier;
//				if (propMagicDamageModifier < -1) {
//					propMagicDamageModifier = -1;
//				}
//			}
//			else if (stringbuffer == "flatArmourPiercingDamageModifier") {
//				armourBlueprints >> flatArmourPiercingDamageModifier;
//			}
//			else if (stringbuffer == "propArmourPiercingDamageModifier") {
//				armourBlueprints >> propArmourPiercingDamageModifier;
//				if (propMagicDamageModifier < -1) {
//					propMagicDamageModifier = -1;
//				}
//			}
//			else if (stringbuffer == "name") {
//				getline(armourBlueprints, name, '<');
//				armourBlueprints.seekg(-1, ios_base::cur);
//			}
//			else if (stringbuffer == "description") {
//				getline(armourBlueprints, description, '<');
//				armourBlueprints.seekg(-1, ios_base::cur);
//			}
//			if (getTag(&armourBlueprints) != '/' + stringbuffer) {
//				throw 1;
//			}
//			ignoreLine(&armourBlueprints);
//			if (!armourBlueprints) {
//				throw 1;
//			}
//		}
//		armourBlueprints.close();
//	}
//	catch (int err) { //Default values for an empty slot
//		armourBlueprints.close();
//		real = false;
//		name = "";
//		description = "";
//		switch (err) {
//		case 1:
//			cout << "Unable to parse blueprint or blueprintList " << blueprint << ". Using default armour.\n";
//			break;
//		case 2:
//			cout << "No blueprint or blueprintList found with name " << blueprint << ". Using default armour.\n";
//			break;
//		case 4:
//			cout << "Could not open armourBlueprints.xml, using default armour.\n";
//			break;
//		case 5:
//			cout << "blueprintList " << blueprint << " contains no entries, using default armour.\n";
//			break;
//		}
//	}
//}
//
//void armourLegs::loadFromFile(string blueprint) {
//	real = true;
//	ifstream armourBlueprints;
//	string stringbuffer = "";
//	try { //Will throw an exception if it fails to find a properly formed blueprint
//		if (blueprint == "EMPTY") { //Refers to an empty armour slot
//			throw 3;
//		}
//		//Open blueprint file
//		armourBlueprints.open("data\\armourBlueprints.xml");
//		if (!armourBlueprints.is_open()) { //Could not open file
//			throw 4;
//		}
//		//Check for a blueprint list
//		{
//			bool noList = false; //Set when end of file is reached without finding a list
//			streampos filePos = 0; //Position in file
//			short listCount = -1; //Number of items in a list, initialising to -1 to streamline later code, also using to store which entry we have chosen
//			while (stringbuffer != "armourLegsBlueprintList name=\"" + blueprint + "\"") { //Haven't found a list
//				stringbuffer = getTag(&armourBlueprints);
//				ignoreLine(&armourBlueprints);
//				if (armourBlueprints.eof()) { //Reached end of file without finding list. If the last line of the file is the opening tag of a matching list, it would be bad XML so that case can be ignored.
//					armourBlueprints.clear(); //Reset eofbit and (if necessary) failbit
//					noList = true;
//					break;
//				}
//			}
//			if (!noList) { //Found a list, position in the file is start of line containing first item in list
//				filePos = armourBlueprints.tellg(); //Store file position
//				do {
//					if (armourBlueprints.eof()) { //File ends before list terminates
//						throw 1;
//					}
//					listCount++;
//					stringbuffer = getTag(&armourBlueprints);
//					ignoreLine(&armourBlueprints);
//				} while (stringbuffer != "/armourLegsBlueprintList");
//				armourBlueprints.clear(); //In case the closing tag of the list was the end of the file
//				if (listCount == 0) { //Empty list
//					throw 5;
//				}
//				listCount = rng(1, listCount); //Pick a random entry in the list
//				armourBlueprints.seekg(filePos); //Go back to start of first entry
//				for (int i = 1; i < listCount; i++) { //Ignore lines up to the one we picked
//					ignoreLine(&armourBlueprints);
//				}
//				if (getTag(&armourBlueprints) != "name") { //It should be this
//					throw 1;
//				}
//				getline(armourBlueprints, blueprint, '<'); //Get the blueprint name
//				if (blueprint == "EMPTY") {
//					throw 3;
//				}
//				getline(armourBlueprints, stringbuffer, '>');
//				if (stringbuffer != "/name") {
//					throw 1;
//				}
//			}
//			armourBlueprints.seekg(0); //Go back to beginning of file
//			stringbuffer = "";
//		}
//		//Find and read actual blueprint
//		while (stringbuffer != "armourLegsBlueprint name=\"" + blueprint + "\"") { //Haven't found a blueprint
//			stringbuffer = getTag(&armourBlueprints);
//			ignoreLine(&armourBlueprints);
//			if (armourBlueprints.eof()) { //Reached end of file without finding blueprint. If the last line of the file is the opening tag of a matching blueprint, it would be bad XML so that case can be ignored.
//				throw 2;
//			}
//		}
//		while (stringbuffer != "/armourLegsBlueprint") { //Keep reading data until we reach the end of the blueprint, this is what getTag will return
//			if (armourBlueprints.eof()) { //Reached end of file without finding proper closing tag for blueprint
//				throw 1;
//			}
//			stringbuffer = getTag(&armourBlueprints); //Get the tag
//			if (stringbuffer == "maxHealthModifier") { //Set the appropriate attribute, then ignore the rest of the line.
//				armourBlueprints >> maxHealthModifier;
//			}
//			else if (stringbuffer == "maxManaModifier") {
//				armourBlueprints >> maxManaModifier;
//			}
//			else if (stringbuffer == "turnManaRegenModifier") {
//				armourBlueprints >> turnManaRegenModifier;
//			}
//			else if (stringbuffer == "battleManaRegenModifier") {
//				armourBlueprints >> battleManaRegenModifier;
//			}
//			else if (stringbuffer == "constRegenModifier") {
//				armourBlueprints >> constRegenModifier;
//			}
//			else if (stringbuffer == "battleRegenModifier") {
//				armourBlueprints >> battleRegenModifier;
//			}
//			else if (stringbuffer == "flatArmourModifier") {
//				armourBlueprints >> flatArmourModifier;
//			}
//			else if (stringbuffer == "propArmourModifier") {
//				armourBlueprints >> propArmourModifier;
//				if (propArmourModifier < -1) {
//					propArmourModifier = -1;
//				}
//			}
//			else if (stringbuffer == "flatMagicArmourModifier") {
//				armourBlueprints >> flatMagicArmourModifier;
//			}
//			else if (stringbuffer == "propMagicArmourModifier") {
//				armourBlueprints >> propMagicArmourModifier;
//				if (propMagicArmourModifier < -1) {
//					propMagicArmourModifier = -1;
//				}
//			}
//			else if (stringbuffer == "flatDamageModifier") {
//				armourBlueprints >> flatDamageModifier;
//			}
//			else if (stringbuffer == "propDamageModifier") {
//				armourBlueprints >> propDamageModifier;
//				if (propDamageModifier < -1) {
//					propDamageModifier = -1;
//				}
//			}
//			else if (stringbuffer == "evadeChanceModifier") {
//				armourBlueprints >> evadeChanceModifier;
//				if (evadeChanceModifier < -1) {
//					evadeChanceModifier = -1;
//				}
//			}
//			else if (stringbuffer == "poisonResistModifier") {
//				armourBlueprints >> poisonResistModifier;
//				if (poisonResistModifier < -1) {
//					poisonResistModifier = -1;
//				}
//			}
//			else if (stringbuffer == "bleedResistModifier") {
//				armourBlueprints >> bleedResistModifier;
//				if (bleedResistModifier < -1) {
//					bleedResistModifier = -1;
//				}
//			}
//			else if (stringbuffer == "flatMagicDamageModifier") {
//				armourBlueprints >> flatMagicDamageModifier;
//			}
//			else if (stringbuffer == "propMagicDamageModifier") {
//				armourBlueprints >> propMagicDamageModifier;
//				if (propMagicDamageModifier < -1) {
//					propMagicDamageModifier = -1;
//				}
//			}
//			else if (stringbuffer == "flatArmourPiercingDamageModifier") {
//				armourBlueprints >> flatArmourPiercingDamageModifier;
//			}
//			else if (stringbuffer == "propArmourPiercingDamageModifier") {
//				armourBlueprints >> propArmourPiercingDamageModifier;
//				if (propMagicDamageModifier < -1) {
//					propMagicDamageModifier = -1;
//				}
//			}
//			else if (stringbuffer == "name") {
//				getline(armourBlueprints, name, '<');
//				armourBlueprints.seekg(-1, ios_base::cur);
//			}
//			else if (stringbuffer == "description") {
//				getline(armourBlueprints, description, '<');
//				armourBlueprints.seekg(-1, ios_base::cur);
//			}
//			if (getTag(&armourBlueprints) != '/' + stringbuffer) {
//				throw 1;
//			}
//			ignoreLine(&armourBlueprints);
//			if (!armourBlueprints) {
//				throw 1;
//			}
//		}
//		armourBlueprints.close();
//	}
//	catch (int err) { //Default values for an empty slot
//		armourBlueprints.close();
//		real = false;
//		name = "";
//		description = "";
//		switch (err) {
//		case 1:
//			cout << "Unable to parse blueprint or blueprintList " << blueprint << ". Using default armour.\n";
//			break;
//		case 2:
//			cout << "No blueprint or blueprintList found with name " << blueprint << ". Using default armour.\n";
//			break;
//		case 4:
//			cout << "Could not open armourBlueprints.xml, using default armour.\n";
//			break;
//		case 5:
//			cout << "blueprintList " << blueprint << " contains no entries, using default armour.\n";
//			break;
//		}
//	}
//}
//
//void armourFeet::loadFromFile(string blueprint) {
//	real = true;
//	ifstream armourBlueprints;
//	string stringbuffer = "";
//	try { //Will throw an exception if it fails to find a properly formed blueprint
//		if (blueprint == "EMPTY") { //Refers to an empty armour slot
//			throw 3;
//		}
//		//Open blueprint file
//		armourBlueprints.open("data\\armourBlueprints.xml");
//		if (!armourBlueprints.is_open()) { //Could not open file
//			throw 4;
//		}
//		//Check for a blueprint list
//		{
//			bool noList = false; //Set when end of file is reached without finding a list
//			streampos filePos = 0; //Position in file
//			short listCount = -1; //Number of items in a list, initialising to -1 to streamline later code, also using to store which entry we have chosen
//			while (stringbuffer != "armourFeetBlueprintList name=\"" + blueprint + "\"") { //Haven't found a list
//				stringbuffer = getTag(&armourBlueprints);
//				ignoreLine(&armourBlueprints);
//				if (armourBlueprints.eof()) { //Reached end of file without finding list. If the last line of the file is the opening tag of a matching list, it would be bad XML so that case can be ignored.
//					armourBlueprints.clear(); //Reset eofbit and (if necessary) failbit
//					noList = true;
//					break;
//				}
//			}
//			if (!noList) { //Found a list, position in the file is start of line containing first item in list
//				filePos = armourBlueprints.tellg(); //Store file position
//				do {
//					if (armourBlueprints.eof()) { //File ends before list terminates
//						throw 1;
//					}
//					listCount++;
//					stringbuffer = getTag(&armourBlueprints);
//					ignoreLine(&armourBlueprints);
//				} while (stringbuffer != "/armourFeetBlueprintList");
//				armourBlueprints.clear(); //In case the closing tag of the list was the end of the file
//				if (listCount == 0) { //Empty list
//					throw 5;
//				}
//				listCount = rng(1, listCount); //Pick a random entry in the list
//				armourBlueprints.seekg(filePos); //Go back to start of first entry
//				for (int i = 1; i < listCount; i++) { //Ignore lines up to the one we picked
//					ignoreLine(&armourBlueprints);
//				}
//				if (getTag(&armourBlueprints) != "name") { //It should be this
//					throw 1;
//				}
//				getline(armourBlueprints, blueprint, '<'); //Get the blueprint name
//				if (blueprint == "EMPTY") {
//					throw 3;
//				}
//				getline(armourBlueprints, stringbuffer, '>');
//				if (stringbuffer != "/name") {
//					throw 1;
//				}
//			}
//			armourBlueprints.seekg(0); //Go back to beginning of file
//			stringbuffer = "";
//		}
//		//Find and read actual blueprint
//		while (stringbuffer != "armourFeetBlueprint name=\"" + blueprint + "\"") { //Haven't found a blueprint
//			stringbuffer = getTag(&armourBlueprints);
//			ignoreLine(&armourBlueprints);
//			if (armourBlueprints.eof()) { //Reached end of file without finding blueprint. If the last line of the file is the opening tag of a matching blueprint, it would be bad XML so that case can be ignored.
//				throw 2;
//			}
//		}
//		while (stringbuffer != "/armourFeetBlueprint") { //Keep reading data until we reach the end of the blueprint, this is what getTag will return
//			if (armourBlueprints.eof()) { //Reached end of file without finding proper closing tag for blueprint
//				throw 1;
//			}
//			stringbuffer = getTag(&armourBlueprints); //Get the tag
//			if (stringbuffer == "maxHealthModifier") { //Set the appropriate attribute, then ignore the rest of the line.
//				armourBlueprints >> maxHealthModifier;
//			}
//			else if (stringbuffer == "maxManaModifier") {
//				armourBlueprints >> maxManaModifier;
//			}
//			else if (stringbuffer == "turnManaRegenModifier") {
//				armourBlueprints >> turnManaRegenModifier;
//			}
//			else if (stringbuffer == "battleManaRegenModifier") {
//				armourBlueprints >> battleManaRegenModifier;
//			}
//			else if (stringbuffer == "constRegenModifier") {
//				armourBlueprints >> constRegenModifier;
//			}
//			else if (stringbuffer == "battleRegenModifier") {
//				armourBlueprints >> battleRegenModifier;
//			}
//			else if (stringbuffer == "flatArmourModifier") {
//				armourBlueprints >> flatArmourModifier;
//			}
//			else if (stringbuffer == "propArmourModifier") {
//				armourBlueprints >> propArmourModifier;
//				if (propArmourModifier < -1) {
//					propArmourModifier = -1;
//				}
//			}
//			else if (stringbuffer == "flatMagicArmourModifier") {
//				armourBlueprints >> flatMagicArmourModifier;
//			}
//			else if (stringbuffer == "propMagicArmourModifier") {
//				armourBlueprints >> propMagicArmourModifier;
//				if (propMagicArmourModifier < -1) {
//					propMagicArmourModifier = -1;
//				}
//			}
//			else if (stringbuffer == "flatDamageModifier") {
//				armourBlueprints >> flatDamageModifier;
//			}
//			else if (stringbuffer == "propDamageModifier") {
//				armourBlueprints >> propDamageModifier;
//				if (propDamageModifier < -1) {
//					propDamageModifier = -1;
//				}
//			}
//			else if (stringbuffer == "evadeChanceModifier") {
//				armourBlueprints >> evadeChanceModifier;
//				if (evadeChanceModifier < -1) {
//					evadeChanceModifier = -1;
//				}
//			}
//			else if (stringbuffer == "poisonResistModifier") {
//				armourBlueprints >> poisonResistModifier;
//				if (poisonResistModifier < -1) {
//					poisonResistModifier = -1;
//				}
//			}
//			else if (stringbuffer == "bleedResistModifier") {
//				armourBlueprints >> bleedResistModifier;
//				if (bleedResistModifier < -1) {
//					bleedResistModifier = -1;
//				}
//			}
//			else if (stringbuffer == "flatMagicDamageModifier") {
//				armourBlueprints >> flatMagicDamageModifier;
//			}
//			else if (stringbuffer == "propMagicDamageModifier") {
//				armourBlueprints >> propMagicDamageModifier;
//				if (propMagicDamageModifier < -1) {
//					propMagicDamageModifier = -1;
//				}
//			}
//			else if (stringbuffer == "flatArmourPiercingDamageModifier") {
//				armourBlueprints >> flatArmourPiercingDamageModifier;
//			}
//			else if (stringbuffer == "propArmourPiercingDamageModifier") {
//				armourBlueprints >> propArmourPiercingDamageModifier;
//				if (propMagicDamageModifier < -1) {
//					propMagicDamageModifier = -1;
//				}
//			}
//			else if (stringbuffer == "name") {
//				getline(armourBlueprints, name, '<');
//				armourBlueprints.seekg(-1, ios_base::cur);
//			}
//			else if (stringbuffer == "description") {
//				getline(armourBlueprints, description, '<');
//				armourBlueprints.seekg(-1, ios_base::cur);
//			}
//			if (getTag(&armourBlueprints) != '/' + stringbuffer) {
//				throw 1;
//			}
//			ignoreLine(&armourBlueprints);
//			if (!armourBlueprints) {
//				throw 1;
//			}
//		}
//		armourBlueprints.close();
//	}
//	catch (int err) { //Default values for an empty slot
//		armourBlueprints.close();
//		real = false;
//		name = "";
//		description = "";
//		switch (err) {
//		case 1:
//			cout << "Unable to parse blueprint or blueprintList " << blueprint << ". Using default armour.\n";
//			break;
//		case 2:
//			cout << "No blueprint or blueprintList found with name " << blueprint << ". Using default armour.\n";
//			break;
//		case 4:
//			cout << "Could not open armourBlueprints.xml, using default armour.\n";
//			break;
//		case 5:
//			cout << "blueprintList " << blueprint << " contains no entries, using default armour.\n";
//			break;
//		}
//	}
//}

void armour::displayStats() {
	if (!real) {
		cout << "None\n";
		return;
	}
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
	if (maxHealthModifier > 0) {
		cout << '+' << maxHealthModifier << " maximum health\n";
	}
	else if (maxHealthModifier < 0) {
		cout << maxHealthModifier << " maximum health\n";
	}
	//Health regen per turn
	if (constRegenModifier > 0) {
		cout << '+' << constRegenModifier << " health per turn\n";
	}
	else if (constRegenModifier < 0) {
		cout << constRegenModifier << " health per turn\n";
	}
	//Health regen at end of battle
	if (battleRegenModifier > 0) {
		cout << '+' << battleRegenModifier << " healing after battle\n";
	}
	else if (battleRegenModifier < 0) {
		cout << battleRegenModifier << " healing after battle\n";
	}
	//Max mana
	if (maxManaModifier > 0) {
		cout << '+' << maxManaModifier << " maximum " << g_manaName.plural() << '\n';
	}
	else if (maxManaModifier < 0) {
		cout << maxManaModifier << " maximum " << g_manaName.plural() << '\n';
	}
	//Mana regen per turn
	if (turnManaRegenModifier == 1) {
		cout << "+1 " << g_manaName.singular() << " per turn\n";
	}
	else if (turnManaRegenModifier > 1) {
		cout << '+' << turnManaRegenModifier << ' ' << g_manaName.plural() << " per turn\n";
	}
	else if (turnManaRegenModifier == -1) {
		cout << "-1 " << g_manaName.singular() << " per turn\n";
	}
	else if (turnManaRegenModifier < -1) {
		cout << turnManaRegenModifier << ' ' << g_manaName.plural() << " per turn\n";
	}
	//Post battle mana regeneration
	if (battleManaRegenModifier == 1) {
		cout << "+1 " << g_manaName.singular() << " recovered after battle\n";
	}
	else if (battleManaRegenModifier > 1) {
		cout << '+' << battleManaRegenModifier << ' ' << g_manaName.plural() << " recovered after battle\n";
	}
	else if (battleManaRegenModifier == -1) {
		cout << "-1 " << g_manaName.singular() << " recovered after battle\n";
	}
	else if (battleManaRegenModifier < -1) {
		cout << battleManaRegenModifier << ' ' << g_manaName.plural() << " recovered after battle\n";
	}
	//Flat armour
	if (flatArmourModifier > 0) {
		cout << '+' << flatArmourModifier << " physical armour rating\n";
	}
	else if (flatArmourModifier < 0) {
		cout << flatArmourModifier << " physical armour rating\n";
	}
	//Proportional armour
	if (propArmourModifier > 0) {
		cout << '+' << 100 * propArmourModifier << "% physical damage received\n";
	}
	else if (propArmourModifier < 0) {
		cout << 100 * propArmourModifier << "% physical damage received\n";
	}
	//Flat magic armour
	if (flatMagicArmourModifier > 0) {
		cout << '+' << flatMagicArmourModifier << " magic armour rating\n";
	}
	else if (flatMagicArmourModifier < 0) {
		cout << flatMagicArmourModifier << " magic armour rating\n";
	}
	//Proportional magic armour
	if (propMagicArmourModifier > 0) {
		cout << '+' << 100 * propMagicArmourModifier << "% magic damage received\n";
	}
	else if (propMagicArmourModifier < 0) {
		cout << 100 * propMagicArmourModifier << "% magic damage received\n";
	}
	//Flat damage modifier
	if (flatDamageModifier > 0) {
		cout << '+' << flatDamageModifier << " physical damage\n";
	}
	else if (flatDamageModifier < 0) {
		cout << flatDamageModifier << " physical damage\n";
	}
	//Prop damage modifier
	if (propDamageModifier > 0) {
		cout << '+' << 100 * propDamageModifier << "% physical damage\n";
	}
	else if (propDamageModifier < 0) {
		cout << 100 * propDamageModifier << "% physical damage\n";
	}
	//Flat magic damage
	if (flatMagicDamageModifier > 0) {
		cout << '+' << flatMagicDamageModifier << " magic damage\n";
	}
	else if (flatMagicDamageModifier < 0) {
		cout << flatMagicDamageModifier << " magic damage\n";
	}
	//Prop magic damage
	if (propMagicDamageModifier > 0) {
		cout << '+' << 100 * propMagicDamageModifier << "% magic damage\n";
	}
	else if (propMagicDamageModifier < 0) {
		cout << 100 * propMagicDamageModifier << "% magic damage\n";
	}
	//Flat AP damage
	if (flatArmourPiercingDamageModifier > 0) {
		cout << '+' << flatArmourPiercingDamageModifier << " armour piercing damage\n";
	}
	else if (flatArmourPiercingDamageModifier < 0) {
		cout << flatArmourPiercingDamageModifier << " armour piercing damage\n";
	}
	//Prop AP damage
	if (propArmourPiercingDamageModifier > 0) {
		cout << '+' << 100 * propArmourPiercingDamageModifier << "% armour piercing damage\n";
	}
	else if (propArmourPiercingDamageModifier < 0) {
		cout << 100 * propArmourPiercingDamageModifier << "% armour piercing damage\n";
	}
	//Evade chance
	if (evadeChanceModifier > 0) {
		cout << '+' << 100 * evadeChanceModifier << "% evade chance\n";
	}
	else if (evadeChanceModifier < 0) {
		cout << 100 * evadeChanceModifier << "% evade chance\n";
	}
	//Counter attack chance
	if (counterAttackChanceModifier > 0) {
		cout << '+' << 100 * counterAttackChanceModifier << "% chance to counter attack\n";
	}
	else if (counterAttackChanceModifier < 0) {
		cout << 100 * counterAttackChanceModifier << "% chance to counter attack\n";
	}
	//Poison resist
	if (poisonResistModifier > 0) {
		cout << '+' << 100 * poisonResistModifier << "% poison resistance\n";
	}
	else if (poisonResistModifier < 0) {
		cout << 100 * poisonResistModifier << "% poison resistance\n";
	}
	//Bleed resist
	if (bleedResistModifier > 0) {
		cout << '+' << 100 * bleedResistModifier << "% bleed resistance\n";
	}
	else if (bleedResistModifier < 0) {
		cout << 100 * bleedResistModifier << "% bleed resist\n";
	}
	//Bonus actions
	if (bonusActionsModifier == 1) {
		cout << "+1 bonus action\n";
	}
	else if (bonusActionsModifier > 0) {
		cout << '+' << bonusActionsModifier << " bonus actions\n";
	}
	else if (bonusActionsModifier == -1) {
		cout << "-1 bonus action\n";
	}
	else if (bonusActionsModifier < -1) {
		cout << bonusActionsModifier << " bonus actions\n";
	}
}