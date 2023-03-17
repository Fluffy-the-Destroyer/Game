#include "adventures.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include "inputs.h"
#include "rng.h"
#include "resources.h"
#include "player.h"
#include "language.h"
#include <thread>
using namespace std;

extern bool g_useCustomData;
extern resource g_manaName, g_projName;
extern variables g_customVars;

/*Error codes:
	0: Unspecified, no further error messages should be shown
	1: Bad XML
	2: Unable to find specified item
	3: Loading empty slot
	4: Could not open file
	5: Empty list
	6: Accessing slot out of range
	7: Attempting to choose from empty set
	8: Variable limit reached
	9: Global class list missing
*/

unsigned char adventureMode() {
	unsigned char saveSlot = '0';
	//Allow player to load a save
	{
		bool existingSaves = false; //Are there existing save games
		for (unsigned char i = 48; i < 58; i++) {
			existingSaves |= checkSaveSlot(i);
		}
		if (existingSaves) {
			//Add functionality here to load saved games
		}
	}
	string adventureBlueprint; //Blueprint of the adventure we are running
	bool custom; //Whether it is a custom adventure
	{
		//Load list of adventures
		vector<dataRef> vanillaAdventures, customAdventures;
		int adventureCount = 0;
		ifstream adventures("data\\adventures.xml");
		{
			try {
				if (!adventures.is_open()) {
					throw 4;
				}
				string buffer;
				while (buffer != "adventureList") {
					buffer = getTag(&adventures);
					ignoreLine(&adventures);
					if (adventures.eof()) {
						throw 2;
					}
				}
				buffer = getTag(&adventures);
				while (buffer != "/adventureList") {
					if (adventureCount == INT_MAX) {
						break;
					}
					vanillaAdventures.emplace_back("adventure", buffer, &adventures);
					adventureCount++;
					buffer = getTag(&adventures);
				}
				adventures.close();
				adventures.open("custom\\adventures.xml");
				if (adventures.is_open()) {
					ignoreLine(&adventures, '<');
					if (!adventures.eof()) {
						while (buffer != "adventureList" && !adventures.eof()) {
							buffer = getTag(&adventures);
							ignoreLine(&adventures);
						}
						if (!adventures.eof()) {
							buffer = getTag(&adventures);
							while (buffer != "/adventureList") {
								if (adventureCount == INT_MAX) {
									break;
								}
								customAdventures.emplace_back("adventure", buffer, &adventures);
								adventureCount++;
								buffer = getTag(&adventures);
							}
						}
					}
					adventures.close();
				}
			}
			catch (int err) {
				adventures.close();
				switch (err) {
				case 1:
					cout << "Unable to parse adventureList, returning to menu\n";
					break;
				case 2:
					cout << "Unable to find list of adventures, returning to menu\n";
					break;
				case 4:
					cout << "Unable to open adventures.xml, returning to menu\n";
					break;
				}
				return 1;
			}
		}
		if (adventureCount <= 0) {
			cout << "Error: no adventures found! Returning to menu\n";
			return 1;
		}
		cout << "The following adventures are available, enter the number of the adventure you wish to play.\nTo go back to the main menu, enter 0.\n";
		{
			unsigned char numWidth = static_cast<unsigned char>(floor(log10(adventureCount))) + 1;
			if (!vanillaAdventures.empty()) {
				cout << "Vanilla adventures:\n\n";
				for (int i = 0; i < vanillaAdventures.size(); i++) {
					cout << setw(numWidth) << i + 1 << ": " << vanillaAdventures[i].getName() << '\n';
					cout << vanillaAdventures[i].getDescription() << "\n\n";
				}
			}
			if (!customAdventures.empty()) {
				cout << "Custom adventures:\n\n";
				for (int i = static_cast<int>(vanillaAdventures.size()); i < adventureCount; i++) {
					cout << setw(numWidth) << i + 1 << ": " << customAdventures[i - vanillaAdventures.size()].getName() << '\n';
					cout << customAdventures[i - vanillaAdventures.size()].getDescription() << "\n\n";
				}
			}
		}
		int adventureChoice = userChoice(0, adventureCount);
		if (adventureChoice == 0) {
			return 1;
		}
		adventureChoice--;
		if (adventureChoice < vanillaAdventures.size()) {
			adventureBlueprint = vanillaAdventures[adventureChoice].getBlueprintName();
			custom = false;
		}
		else {
			adventureChoice -= static_cast<int>(vanillaAdventures.size());
			adventureBlueprint = customAdventures[adventureChoice].getBlueprintName();
			custom = true;
		}
	}
	try {
		adventure chosenAdventure(adventureBlueprint, custom, saveSlot);
		return chosenAdventure.adventureInitialiser();
	}
	catch (int err) {
		switch (err) {
		case 1:
			cout << "Unable to parse adventureBlueprint " << adventureBlueprint << ", returning to menu\n";
			return 1;
		case 2:
			cout << "Unable to find adventureBlueprint " << adventureBlueprint << ", returning to menu\n";
			return 1;
		case 3:
			cout << "EMPTY is not an allowed blueprint name, returning to menu\n";
			return 1;
		case 4:
			if (custom) {
				cout << "Could not open custom/adventures.xml, returning to menu\n";
				return 1;
			}
			cout << "Could not open data/adventures.xml, returning to menu\n";
			return 1;
		case 5:
			cout << "adventureBlueprintList " << adventureBlueprint << ", contains no entries, returning to menu\n";
			return 1;
		default:
			return 1;
		}
	}
}

adventure::adventure(string blueprint, bool custom, unsigned char slot) :saveSlot(slot) {
	ifstream adventures;
	string buffer;
	string buffer3[3];
	size_t posBuffer1 = 0, posBuffer2 = 0;
	try {
		if (custom) {
			adventures.open("custom\\adventures.xml");
		}
		else {
			adventures.open("data\\adventures.xml");
		}
		if (!adventures.is_open()) {
			throw 4;
		}
		string blueprintName = "adventureBlueprintList name=\"" + blueprint + '\"';
		{
			bool noList = false;
			streampos filePos = 0;
			short listCount = -1;
			while (buffer != blueprintName) {
				buffer = getTag(&adventures);
				ignoreLine(&adventures);
				if (adventures.eof()) {
					adventures.clear();
					noList = true;
					break;
				}
			}
			if (!noList) {
				filePos = adventures.tellg();
				do {
					listCount++;
					buffer = getTag(&adventures);
					ignoreLine(&adventures);
				} while (buffer != "/adventureBlueprintList");
				adventures.clear();
				if (listCount == 0) {
					throw 5;
				}
				listCount = rng(1, listCount);
				adventures.seekg(filePos);
				for (int i = 1; i < listCount; i++) {
					ignoreLine(&adventures);
				}
				if (getTag(&adventures) != "name") {
					throw 1;
				}
				blueprint = stringFromFile(&adventures);
				if (blueprint == "EMPTY") {
					throw 3;
				}
				if (getTag(&adventures) != "/name") {
					throw 1;
				}
			}
		}
		adventures.seekg(0);
		buffer = "";
		blueprintName = "adventureBlueprint name=\"" + blueprint + '\"';
		while (buffer != blueprintName) {
			buffer = getTag(&adventures);
			ignoreLine(&adventures);
			if (adventures.eof()) {
				throw 2;
			}
		}
		buffer = getTag(&adventures);
		while (buffer != "/adventureBlueprint") {
			if (adventures.eof()) {
				throw 1;
			}
			if (buffer == "name") {
				name = stringFromFile(&adventures);
			}
			else if (buffer == "description") {
				description = stringFromFile(&adventures);
			}
			else if (buffer == "forceCustom/") {
				forceCustom = true;
				ignoreLine(&adventures);
				if (!adventures) {
					throw 1;
				}
				buffer = getTag(&adventures);
				continue;
			}
			else if (buffer == "file") {
				file = stringFromFile(&adventures);
			}
			else if (buffer == "classes") {
				ignoreLine(&adventures);
				buffer = getTag(&adventures);
				while (buffer != "/classes") {
					if (classes.size() == INT_MAX) {
						ignoreLine(&adventures);
					}
					else {
						classes.emplace_back("class", buffer, &adventures);
					}
					buffer = getTag(&adventures);
				}
				ignoreLine(&adventures);
				if (!adventures) {
					throw 1;
				}
				buffer = getTag(&adventures);
				continue;
			}
			else {
				throw 1;
			}
			if (getTag(&adventures) != '/' + buffer) {
				throw 1;
			}
			ignoreLine(&adventures);
			if (!adventures) {
				throw 1;
			}
			buffer = getTag(&adventures);
		}
		adventures.close();
	}
	catch (int err) {
		adventures.close();
		throw err;
	}
}

dataRef::dataRef(string type, string openTag, ifstream* file) {
	size_t posBuffer1 = 0, posBuffer2 = 0;
	if (openTag.substr(0, type.size() + 7) != type + " name=\"") {
		throw 1;
	}
	posBuffer1 = openTag.find('\"', type.size() + 7);
	if (posBuffer1 == string::npos) {
		throw 1;
	}
	if (openTag.substr(posBuffer1, 13) != "\" blueprint=\"") {
		throw 1;
	}
	posBuffer2 = openTag.find('\"', posBuffer1 + 13);
	if (posBuffer2 == string::npos) {
		throw 1;
	}
	if (!openTag.substr(posBuffer2 + 1).empty()) {
		throw 1;
	}
	openTag.pop_back();
	name = removeEscapes(openTag.substr(type.size() + 7, posBuffer1 - type.size() - 7));
	blueprintName = removeEscapes(openTag.substr(posBuffer1 + 13));
	description = stringFromFile(file);
	if (getTag(file) != '/' + type) {
		throw 1;
	}
	ignoreLine(file);
	if (!(*file)) {
		throw 1;
	}
}

unsigned char adventure::adventureInitialiser() {
	cout << name << "\n\n" << description << "\n\n";
	if (forceCustom) { //Enable custom data if required, otherwise allow player to choose
		cout << "This adventure requires custom data to be enabled\n";
		g_useCustomData = true;
	}
	else {
		cout << "To enable custom data, enter 1.\nTo play without custom data, enter 0.\n";
		switch (userChoice(0, 1)) {
		case 0:
			g_useCustomData = false;
			break;
		case 1:
			g_useCustomData = true;
			break;
		}
	}
	g_manaName.loadFromFile("MANA"); //Load resource names
	g_projName.loadFromFile("PROJECTILE");
	ifstream dataFile;
	try {
		if (classes.empty()) { //If classes were not specified, load in global lists
			string buffer;
			if (g_useCustomData) {
				dataFile.open("custom\\classBlueprints.xml");
				if (dataFile.is_open()) {
					ignoreLine(&dataFile, '<');
					if (!dataFile.eof()) {
						while (buffer != "classList name=\"GLOBAL\"") { //Find global class list
							buffer = getTag(&dataFile);
							ignoreLine(&dataFile);
							if (dataFile.eof()) {
								dataFile.close();
								break;
							}
						}
						if (dataFile.is_open()) {
							buffer = getTag(&dataFile);
							while (buffer != "/classList") {
								if (classes.size() == INT_MAX) {
									ignoreLine(&dataFile);
								}
								else {
									classes.emplace_back("class", buffer, &dataFile);
								}
								buffer = getTag(&dataFile);
							}
							dataFile.close();
						}
					}
					else {
						dataFile.close();
					}
				}
				buffer = "";
			}
			{
				int customClasses = static_cast<int>(classes.size());
				dataFile.open("data\\classBlueprints.xml");
				if (!dataFile.is_open()) {
					throw 4;
				}
				while (buffer != "classList name=\"GLOBAL\"") {
					buffer = getTag(&dataFile);
					ignoreLine(&dataFile);
					if (dataFile.eof()) {
						throw 9;
					}
				}
				buffer = getTag(&dataFile);
				while (buffer != "/classList") {
					if (classes.size() == INT_MAX) {
						ignoreLine(&dataFile);
					}
					else {
						classes.emplace_back("class", buffer, &dataFile);
						for (int i = 0; i < customClasses; i++) { //Check against custom classes to see if any of them overwrites this vanilla class we have just found
							if (classes[i].getBlueprintName() == classes.back().getBlueprintName()) {
								classes.pop_back();
								break;
							}
						}
						buffer = getTag(&dataFile);
					}
				}
				dataFile.close();
			}
		}
		if (classes.empty()) {
			cout << "Error: Global class list is empty!\n";
			throw 0;
		}
		//Load victory and defeat text
		dataFile.open("adventures\\" + file);
		if (!dataFile.is_open()) {
			cout << "Unable to open adventure file " << file << '\n';
			throw 0;
		}
		if (getTag(&dataFile) != "victoryText") {
			throw 1;
		}
		victory = stringFromFile(&dataFile);
		if (getTag(&dataFile) != "/victoryText") {
			throw 1;
		}
		ignoreLine(&dataFile);
		if (getTag(&dataFile) != "defeatText") {
			throw 1;
		}
		defeat = stringFromFile(&dataFile);
		if (getTag(&dataFile) != "/defeatText") {
			throw 1;
		}
		ignoreLine(&dataFile);
		//Variable initialisation
		{
			streampos filePos = dataFile.tellg();
			string buffer1, buffer2;
			if (getTag(&dataFile) == "initialVars") { //Check if there is a variable initialiser
				ignoreLine(&dataFile);
				buffer1 = getTag(&dataFile);
				while (buffer1 != "/initialVars") {
					if (buffer1.substr(0, 10) != "var name=\"") {
						throw 1;
					}
					buffer1.erase(0, 10);
					if (buffer1.empty() || buffer1.back() != '\"') {
						throw 1;
					}
					buffer1.pop_back();
					buffer2 = stringFromFile(&dataFile);
					if (getTag(&dataFile) != "/var") {
						throw 1;
					}
					ignoreLine(&dataFile);
					if (!dataFile) {
						throw 1;
					}
					modifyVar(buffer1, buffer2);
					buffer1 = getTag(&dataFile);
				}
				ignoreLine(&dataFile);
			}
			else {
				dataFile.seekg(filePos);
			}
		}
		//Select class
		player playerCharacter; //Player
		cout << "Enter the number of the class you wish to play as:\n\n";
		{
			unsigned char numWidth = static_cast<unsigned char>(floor(log10(classes.size())) + 1);
			for (int i = 0; i < classes.size(); i++) {
				cout << setw(numWidth) << i + 1 << ": " << classes[i].getName() << '\n' << classes[i].getDescription() << "\n\n";
			}
			int chosenClass = userChoice(1, static_cast<int>(classes.size())); //Choose a class
			chosenClass--;
			try {
				playerCharacter.loadClass(classes[chosenClass].getBlueprintName()); //Load in player class
			}
			catch (int err) {
				switch (err) {
				case 1:
					cout << "Unable to parse classBlueprint " << classes[chosenClass].getBlueprintName() << '\n';
					break;
				case 2:
					cout << "Unable to find classBlueprint " << classes[chosenClass].getBlueprintName() << '\n';
					break;
				case 3:
					cout << "Trying to load empty class template\n";
					break;
				case 4:
					cout << "Unable to open classBlueprints.xml\n";
					break;
				case 5:
					cout << "classBlueprintList " << classes[chosenClass].getBlueprintName() << " contains no classes\n";
					break;
				}
				throw 0;
			}
		}
		return adventureHandler(&playerCharacter, &dataFile);
	}
	catch (int err) {
		dataFile.close();
		switch (err) {
		case 1: //Bad XML
			cout << "Error: Unable to parse adventure file\n";
			break;
		case 2:
			cout << "Error: Unable to find specified item\n";
			break;
		case 3:
			cout << "Error: Cannot load empty slot\n";
			break;
		case 4:
			cout << "Error: Unable to open file\n";
			break;
		case 5:
			cout << "Error: Attempting to select item from empty list\n";
			break;
		case 6:
			cout << "Error: Attempting to access weapon/spell slot out of range\n";
			break;
		case 7:
			cout << "Error: Attempting to choose number from the empty set\n";
			break;
		case 8:
			cout << "Error: Variable limit reached\n";
			break;
		case 9:
			cout << "Error: Global class list not found\n";
			break;
		}
	}
	cout << "To return to the main menu, enter 1.\nTo quit, enter 0.\n";
	return userChoice(0, 1);
}

adventure::adventure(unsigned char slot) :saveSlot(slot) {

}

unsigned char adventure::adventureInitialiser(unsigned char slot) {
	return 0;
}

unsigned char adventure::adventureHandler(player* playerCharacter, ifstream* file) {
	unsigned char i = 0;
	while (i == 0) {
		i = doLine(file, playerCharacter);
		switch (i) {
		case 1:
			cout << victory << '\n';
			break;
		case 2:
			cout << defeat << '\n';
			break;
		case 3:
		case 4:
			throw 1;
		case 5:
			save();
			i = 0;
			break;
		}
	}
	this_thread::sleep_for(chrono::milliseconds(500));
	cout << "To return to the main menu, enter 1.\nTo quit, enter 0.\n";
	return userChoice(0, 1);
}

void adventure::save() {

}