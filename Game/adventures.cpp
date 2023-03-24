#include "adventures.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include "inputs.h"
#include "resources.h"
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

uint8_t adventureMode() {
	signed char saveSlot = '0';
	//Allow player to load a save
	{
		bool existingSaves = false; //Are there existing save games
		for (signed char i = '0'; i <= '9' && !existingSaves; i++) {
			existingSaves |= checkSaveSlot(i);
		}
		if (existingSaves) {
			bool done = false;
			while (!done) {
				cout << "To start a new game, enter 1.\nTo load a saved game, enter 2.\nTo return to the menu, enter 0.\n";
				switch (userChoice(0, 2)) {
				case 0:
					return 1;
				case 1:
					done = true;
					break;
				case 2: //Loading a save
					cout << "Enter the number of the saved game you wish to load.\nTo go back, enter 0.\n";
					cout << "Existing saved games:\n";
					{
						vector<short> saveSlots(1, 0);
						for (signed char slot = '0'; slot <= '9'; slot++) {
							if (checkSaveSlot(slot)) {
								saveSlots.push_back(slot - 47);
								displaySaveData(slot);
							}
							else {
								cout << "Save slot " << slot - 47 << ": Empty\n\n";
							}
						}
						short selectedSlot = userChoice(saveSlots);
						if (selectedSlot == 0) {
							break;
						}
						try {
							signed char slot = static_cast<signed char>(selectedSlot + 47);
							adventure chosenAdventure(slot);
							return chosenAdventure.loadFromSave();
						}
						catch (int) {}
					}
					break;
				}
			}
		}
		else {
			cout << "No existing save data found, starting a new game\n";
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
						adventures.seekg(-1, ios_base::cur);
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
			uint8_t numWidth = static_cast<uint8_t>(floor(log10(adventureCount))) + 1;
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
		while (true) {
			cout << "Enter the save slot you wish to use. (A number from 1 to 10)\n";
			saveSlot = static_cast<uint8_t>(userChoice(1, 10) + 47);
			if (checkSaveSlot(saveSlot)) {
				cout << "Selected slot contains existing data:\n";
				displaySaveData(saveSlot);
				cout << "To overwrite this saved game, enter 1.\nTo choose a different slot, enter 2.\nTo return to the main menu, enter 0.\n";
				switch (userChoice(0, 2)) {
				case 0:
					return 1;
				case 2:
					continue;
				}
			}
			break;
		}
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

adventure::adventure(string blueprint, bool custom, signed char slot) :saveSlot(slot) {
	ifstream adventures;
	string buffer;
	string buffer3[3];
	size_t posBuffer1 = 0, posBuffer2 = 0;
	deleteSave();
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
			if (buffer == "name") {
				name = stringFromFile(&adventures);
			}
			else if (buffer == "description") {
				description = stringFromFile(&adventures);
			}
			else if (buffer == "forceCustom/") {
				forceCustom = true;
				ignoreLine(&adventures);
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

uint8_t adventure::adventureInitialiser() {
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
						dataFile.seekg(-1, ios_base::cur);
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
			g_customVars.reset(); //Reset values
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
			uint8_t numWidth = static_cast<uint8_t>(floor(log10(classes.size())) + 1);
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

uint8_t adventure::loadFromSave() {
	if (saveSlot < '0' || saveSlot>'9') {
		cout << "Invalid save slot\n";
		return 1;
	}
	ifstream dataFile;
	try {
		player playerCharacter;
		{
			string savePath = "saves\\sav";
			savePath += saveSlot;
			savePath += ".xml";
			dataFile.open(savePath);
		}
		if (!dataFile.is_open()) {
			cout << "Error: Unable to open save file\n";
			throw 0;
		}
		streampos filePos;
		try {
			if (getTag(&dataFile) != "saveGame/") {
				cout << "Error: Save file contains no data\n";
				throw 0;
			}
			ignoreLine(&dataFile);
			if (getTag(&dataFile) != "adventure") {
				throw 1;
			}
			ignoreLine(&dataFile);
			if (getTag(&dataFile) != "name") {
				throw 1;
			}
			name = stringFromFile(&dataFile);
			if (getTag(&dataFile) != "/name") {
				throw 1;
			}
			ignoreLine(&dataFile);
			if (getTag(&dataFile) != "description") {
				throw 1;
			}
			description = stringFromFile(&dataFile);
			if (getTag(&dataFile) != "/description") {
				throw 1;
			}
			ignoreLine(&dataFile);
			if (getTag(&dataFile) != "file") {
				throw 1;
			}
			file = stringFromFile(&dataFile);
			if (getTag(&dataFile) != "/file") {
				throw 1;
			}
			ignoreLine(&dataFile);
			if (getTag(&dataFile) != "victory") {
				throw 1;
			}
			victory = stringFromFile(&dataFile);
			if (getTag(&dataFile) != "/victory") {
				throw 1;
			}
			ignoreLine(&dataFile);
			if (getTag(&dataFile) != "defeat") {
				throw 1;
			}
			defeat = stringFromFile(&dataFile);
			if (getTag(&dataFile) != "/defeat") {
				throw 1;
			}
			ignoreLine(&dataFile);
			if (getTag(&dataFile) != "/adventure") {
				throw 1;
			}
			ignoreLine(&dataFile);
			string buffer = getTag(&dataFile);
			if (buffer == "custom/") {
				g_useCustomData = true;
				ignoreLine(&dataFile);
			}
			else if (buffer == "noCustom/") {
				g_useCustomData = false;
				ignoreLine(&dataFile);
			}
			else {
				throw 1;
			}
			playerCharacter.loadSave(&dataFile);
			if (getTag(&dataFile) != "filePos") {
				throw 1;
			}
			{
				long long posBuffer;
				dataFile >> posBuffer;
				filePos = posBuffer;
			}
			if (getTag(&dataFile) != "/filePos") {
				throw 1;
			}
			ignoreLine(&dataFile);
			if (getTag(&dataFile) != "variables") {
				throw 1;
			}
			ignoreLine(&dataFile);
			buffer = getTag(&dataFile);
			{
				g_customVars.vars.resize(0); //Reset variables
				size_t posBuffer;
				short value;
				while (buffer != "/variables") {
					if (g_customVars.vars.size() == USHRT_MAX) {
						throw 8;
					}
					if (buffer.substr(0, 10) != "var name=\"") {
						throw 1;
					}
					buffer.erase(0, 10);
					posBuffer = buffer.find('\"');
					if (posBuffer == string::npos) {
						throw 1;
					}
					if (buffer.substr(posBuffer) != "\"") {
						throw 1;
					}
					buffer.pop_back();
					dataFile >> value;
					g_customVars.vars.emplace_back(buffer, value);
					if (getTag(&dataFile) != "/var") {
						throw 1;
					}
					ignoreLine(&dataFile);
					buffer = getTag(&dataFile);
				}
				ignoreLine(&dataFile);
			}
			if (getTag(&dataFile) != "resources") {
				throw 1;
			}
			ignoreLine(&dataFile);
			if (getTag(&dataFile) != "projectiles") {
				throw 1;
			}
			ignoreLine(&dataFile);
			g_projName.loadSave(&dataFile);
			if (getTag(&dataFile) != "/projectiles") {
				throw 1;
			}
			ignoreLine(&dataFile);
			if (getTag(&dataFile) != "mana") {
				throw 1;
			}
			ignoreLine(&dataFile);
			g_manaName.loadSave(&dataFile);
			if (getTag(&dataFile) != "/mana") {
				throw 1;
			}
			ignoreLine(&dataFile);
			if (getTag(&dataFile) != "/resources") {
				throw 1;
			}
			ignoreLine(&dataFile);
			dataFile.close();
		}
		catch (int err) {
			if (err == 1) {
				cout << "Error: Unable to parse save file\n";
				throw 0;
			}
			else {
				throw err;
			}
		}
		//Open actual adventure file
		dataFile.open("adventures\\" + file);
		if (!dataFile.is_open()) {
			cout << "Error: Unable to open adventure file\n";
			throw 0;
		}
		dataFile.seekg(filePos); //Move to saved position in file
		return adventureHandler(&playerCharacter, &dataFile);
	}
	catch (int err) {
		dataFile.close();
		switch (err) {
		case 1:
			cout << "Error: Unable to parse adventure file\n";
			break;
		case 2:
			cout << "Error: Specified item not found\n";
			break;
		case 3:
			cout << "Error: Attempted to load empty slot\n";
			break;
		case 4:
			cout << "Error: Unable to open file\n";
			break;
		case 5:
			cout << "Error: Attempting to choose entry from an empty list\n";
			break;
		case 6:
			cout << "Error: Attempting to access slot out of range\n";
			break;
		case 7:
			cout << "Error: Attempting to choose number from empty set\n";
			break;
		case 8:
			cout << "Error: Variable limit reached\n";
			break;
		case 9:
			cout << "Error: Global class list missing\n";
			break;
		}
	}
	cout << "To return to the main menu, enter 1.\nTo quit, enter 0.\n";
	return userChoice(0, 1);
}



uint8_t adventure::adventureHandler(player* playerCharacter, ifstream* file) {
	uint8_t i = 0;
	while (i == 0) {
		i = doLine(file, playerCharacter);
		switch (i) {
		case 1:
			cout << victory << '\n';
			deleteSave();
			break;
		case 2:
			cout << defeat << '\n';
			//deleteSave();
			//Currently save is not deleted on defeat, to allow retrying, may be changed later
			break;
		case 3:
		case 4:
			throw 1;
		case 5:
			save(playerCharacter, file->tellg());
			i = 0;
			break;
		}
	}
	this_thread::sleep_for(chrono::milliseconds(500));
	cout << "To return to the main menu, enter 1.\nTo quit, enter 0.\n";
	return userChoice(0, 1);
}

void adventure::save(player* playerCharacter, streampos filePos) {
	if (saveSlot < 48 || saveSlot > 57) {
		cout << "Ivalid save slot\n";
		return;
	}
	ofstream saveFile;
	{
		string savePath = "saves\\sav";
		savePath += saveSlot;
		savePath += ".xml";
		saveFile.open(savePath, ofstream::trunc);
	}
	if (!saveFile.is_open()) {
		cout << "Unable to open file for saving\n";
		return;
	}
	saveFile << "<saveGame/>\n\n"; //Indicates there is save data
	//Adventure data
	saveFile << "<adventure>\n";
		saveFile << "\t<name>" << addEscapes(name) << "</name>\n";
		saveFile << "\t<description>" << addEscapes(description) << "</description>\n";
		saveFile << "\t<file>" << addEscapes(file) << "</file>\n";
		saveFile << "\t<victory>" << addEscapes(victory) << "</victory>\n";
		saveFile << "\t<defeat>" << addEscapes(defeat) << "</defeat>\n";
	saveFile << "</adventure>\n\n";
	//Is custom data enabled
	if (g_useCustomData) {
		saveFile << "<custom/>\n\n";
	}
	else {
		saveFile << "<noCustom/>\n\n";
	}
	//Player stats, stats which have default value will not be saved, only base values will be saved
	playerCharacter->save(&saveFile);
	//Position in adventure file
	saveFile << "<filePos>" << filePos << "</filePos>\n\n";
	//Variables
	saveFile << "<variables>\n";
	for (unsigned short i = 0; i < g_customVars.vars.size(); i++) {
		saveFile << "\t<var name=\"" << g_customVars.vars[i].name << "\">" << g_customVars.vars[i].value << "</var>\n";
	}
	saveFile << "</variables>\n\n";
	//Name of mana and projectiles
	saveFile << "<resources>\n";
		saveFile << "\t<projectiles>\n";
			saveFile << "\t\t<singular>" << addEscapes(g_projName.singular()) << "</singular>\n";
			saveFile << "\t\t<plural>" << addEscapes(g_projName.plural()) << "</plural>\n";
		saveFile << "\t</projectiles>\n";
		saveFile << "\t<mana>\n";
			saveFile << "\t\t<singular>" << addEscapes(g_manaName.singular()) << "</singular>\n";
			saveFile << "\t\t<plural>" << addEscapes(g_manaName.plural()) << "</plural>\n";
		saveFile << "\t</mana>\n";
	saveFile << "</resources>\n\n";
	saveFile.close();
}

void displaySaveData(signed char saveSlot) {
	if (saveSlot < 48 || saveSlot > 57) {
		cout << "Invalid save slot\n";
		return;
	}
	string savePath = "saves\\sav";
	savePath += saveSlot;
	savePath += ".xml";
	saveSlot++;
	cout << "Save slot " << saveSlot << ": ";
	ifstream saveFile(savePath);
	if (!saveFile.is_open()) {
		cout << "Empty\n\n";
		return;
	}
	ignoreLine(&saveFile, '<');
	if (saveFile.eof()) {
		cout << "Empty\n\n";
		saveFile.close();
		return;
	}
	saveFile.seekg(-1, ios_base::cur);
	if (getTag(&saveFile) != "saveGame/") {
		cout << "Empty\n\n";
		saveFile.close();
		return;
	}
	try {
		string name, className, buffer;
		short level;
		bool custom;
		ignoreLine(&saveFile);
		if (getTag(&saveFile) != "adventure") {
			throw 1;
		}
		ignoreLine(&saveFile);
		if (getTag(&saveFile) != "name") {
			throw 1;
		}
		name = stringFromFile(&saveFile);
		if (getTag(&saveFile) != "/name") {
			throw 1;
		}
		do {
			ignoreLine(&saveFile);
		} while (getTag(&saveFile) != "/adventure");
		ignoreLine(&saveFile);
		buffer = getTag(&saveFile);
		if (buffer == "custom/") {
			custom = true;
			ignoreLine(&saveFile);
			buffer = getTag(&saveFile);
		}
		else if (buffer == "noCustom/") {
			custom = false;
			ignoreLine(&saveFile);
			buffer = getTag(&saveFile);
		}
		else {
			throw 1;
		}
		if (buffer != "player") {
			throw 1;
		}
		ignoreLine(&saveFile);
		if (getTag(&saveFile) != "level") {
			throw 1;
		}
		saveFile >> level;
		if (getTag(&saveFile) != "/level") {
			throw 1;
		}
		ignoreLine(&saveFile);
		if (getTag(&saveFile) != "className") {
			throw 1;
		}
		className = stringFromFile(&saveFile);
		if (getTag(&saveFile) != "/className") {
			throw 1;
		}
		saveFile.close();
		cout << name << '\n';
		cout << "Level " << level << ' ' << className;
		if (custom) {
			cout << "; custom data enabled";
		}
		cout << "\n\n";
	}
	catch (int) {
		saveFile.close();
		cout << "Error parsing save file\n\n";
	}
}

void adventure::deleteSave() {
	string savePath = "saves\\sav";
	savePath += saveSlot;
	savePath += ".xml";
	ofstream saveFile(savePath, ofstream::trunc);
	saveFile.close();
}