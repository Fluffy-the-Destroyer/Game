#pragma once
#include <string>
#include <vector>
#include "player.h"

class dataRef {
private:
	std::string name;
	std::string blueprintName;
	std::string description;
public:
	dataRef(std::string values[3]) :name(values[0]), blueprintName(values[1]), description(values[2]) {}
	dataRef(std::string type, std::string openTag, std::ifstream* file);
	std::string getName() { return name; }
	std::string getBlueprintName() { return blueprintName; }
	std::string getDescription() { return description; }
};

class adventure {
private:
	std::string name;
	std::string description;
	bool forceCustom = false;
	std::string file;
	std::vector<dataRef> classes;
	std::string victory; //What to say on victory
	std::string defeat; //What to say on defeat
	signed char saveSlot; //What save slot to use
public:
	adventure(std::string blueprint, bool custom, signed char slot);
	//Constructor for loading save data, just initialises save slot
	adventure(signed char slot) :saveSlot(slot) {}
	//Initialises a new adventure, creates player and opens file, then runs adventure handler
	uint8_t adventureInitialiser();
	//Loads an adventure from save slot, loads player data, opens file in saved position then runs handler
	uint8_t loadFromSave();
	//Runs adventure
	uint8_t adventureHandler(player* playerCharacter, std::ifstream* file);
	//Saves the game
	void save(player* playerCharacter, std::streampos filePos);
	//Deletes save slot contents
	void deleteSave();
};

//Runs adventure mode. Returns 0 if quitting, 1 if returning to menu
uint8_t adventureMode();
//Displays data on the selected save slot
void displaySaveData(signed char saveSlot);