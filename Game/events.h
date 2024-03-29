#pragma once
#include <string>
#include <vector>
#include "language.h"

extern bool g_useCustomData;

class choice {
private:
	std::string text = "Continue"; //What the text of the choice is, e.g. 'Attack the monster'
	short healthChange = 0; //Damage dealt to player when picking the choice, can be picked even if it would kill the player
	short manaChange = 0; //Change in mana when picking the choice, if negative, usually cannot be chosen if the player does not have enough
	short projectileChange = 0; //The change in projectiles when picking, if negative, usually cannot be chosen if the player does not have enough
	std::string req = "true"; //Requirement on variables for the choice to appear
	bool hidden = false; //If true, player cannot see any costs/benefits from picking it. Also, can be chosen when unaffordable, and will simply take all of the resource
	std::string eventName = "EMPTY"; //The event which the choice loads, if it's EMPTY, the choice does not load an event
	friend class Event;
public:
	//Displays the choice
	void display();
};

class Event {
private:
	bool real;
	std::string preBattleText; //Text displayed at start of event
	std::string preBattleSpell; //Spell to be cast on player on event start, for environmental effects or similar. EMPTY is no spell
	std::string enemyBlueprint; //Enemy to fight, EMPTY is no fight
	int8_t firstGo; //Who goes first in the battle, -1 is enemy, 1 is player, 0 is use initiative
	std::string postBattleText; //Text displayed after battle
	short statChanges[18];
	int xpChange;
	std::string reward; //Item to be granted as a reward, prefix with a letter to specify what type
	variables varChanges; //Changes to be made to variables
	std::vector<choice> choices; //Choices for the player to make
public:
	bool getReal() { return real; }
	void loadFromFile(std::string blueprint = "EMPTY", bool custom = g_useCustomData);
	Event(std::string blueprint = "EMPTY") { loadFromFile(blueprint); }
	//Returns 1 if chaining into a new event, returns 0 if not. Returns 2 if player died
	uint8_t eventHandler(player* playerCharacter);
};