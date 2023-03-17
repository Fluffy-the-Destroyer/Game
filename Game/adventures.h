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
	dataRef(std::string vals[3]) :name(vals[0]), blueprintName(vals[1]), description(vals[2]) {}
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
	unsigned char saveSlot; //What save slot to use
public:
	adventure(std::string blueprint, bool custom, unsigned char slot);
	//Loads adventure data from save slot
	adventure(unsigned char slot);
	//Initialises a new adventure, creates player and opens file, then runs adventure handler
	unsigned char adventureInitialiser();
	//Initialises an adventure from save slot, loads player data, opens file in saved position then runs handler
	unsigned char adventureInitialiser(unsigned char slot);
	//Runs adventure
	unsigned char adventureHandler(player* playerCharacter, std::ifstream* file);
	//Saves the game
	void save();
};

//Runs adventure mode. Returns 0 if quitting, 1 if returning to menu
unsigned char adventureMode();