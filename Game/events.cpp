#include "events.h"
#include <fstream>
#include <iostream>
#include "inputs.h"
#include "rng.h"
#include "enemies.h"
#include "spells.h"
#include "battle.h"
#include "blueprints.h"
#include "armour.h"
#include "resources.h"
using namespace std;

extern variables g_customVars;
extern resource g_manaName, g_projName;


//Error codes:
// 1: Bad XML, including premature end of file or nonsense values
// 2: Specified blueprint or list not found
// 3: Loading empty slot, not technically an error
// 4: Unable to open blueprint file
// 5: Empty blueprint list
// 8: Exceeded limit for custom variables

void Event::loadFromFile(string blueprint, bool custom) {
	//Reset attributes to default values
	eventName = preBattleSpell = enemyBlueprint = postBattleSpell = reward = "EMPTY";
	preBattleText = postBattleText = "";
	choices.resize(0);
	varChanges.reset();
	ifstream events;
	string buffer = "", buffer2="";
	try {
		if (blueprint == "EMPTY") {
			throw 3;
		}
		if (custom) {
			events.open("custom\\events.xml");
		}
		else {
			events.open("data\\events.xml");
		}
		if (!events.is_open()) {
			throw 4;
		}
		string blueprintName = "eventList name=\"" + blueprint + '\"';
		{
			bool noList = false;
			streampos filePos = 0;
			short listCount = -1;
			while (buffer != blueprintName) {
				buffer = getTag(&events);
				ignoreLine(&events);
				if (events.eof()) {
					events.clear();
					noList = true;
					break;
				}
			}
			if (!noList) {
				filePos = events.tellg();
				blueprintName = "/eventList";
				do {
					listCount++;
					buffer = getTag(&events);
					ignoreLine(&events);
				} while (buffer != blueprintName);
				events.clear();
				if (listCount == 0) {
					throw 5;
				}
				listCount = rng(1, listCount);
				events.seekg(filePos);
				for (int i = 1; i < listCount; i++) {
					ignoreLine(&events);
				}
				if (getTag(&events) != "name") {
					throw 1;
				}
				getline(events, blueprint, '<');
				if (blueprint == "EMPTY") {
					throw 3;
				}
				getline(events, buffer, '>');
				if (buffer != "/name") {
					throw 1;
				}
			}
			events.seekg(0);
			buffer = "";
		}
		blueprintName = "event name=\"" + blueprint + '\"';
		while (buffer != blueprintName) {
			buffer = getTag(&events);
			ignoreLine(&events);
			if (events.eof()) {
				throw 2;
			}
		}
		blueprintName = "/event";
		buffer = getTag(&events);
		short varBuffer = 0;
		while (buffer != blueprintName) {
			if (events.eof()) {
				throw 1;
			}
			if (buffer == "preBattleText") {
				getline(events, preBattleText, '<');
			}
			else if (buffer == "preBattleSpell") {
				getline(events, preBattleSpell, '<');
			}
			else if (buffer == "enemyBlueprint") {
				getline(events, enemyBlueprint, '<');
			}
			else if (buffer == "postBattleText") {
				getline(events, postBattleText, '<');
			}
			else if (buffer == "postBattleSpell") {
				getline(events, postBattleSpell, '<');
			}
			else if (buffer == "reward") {
				getline(events, reward, '<');
			}
			else if (buffer == "varChanges") {
				ignoreLine(&events);
				if (!events) {
					throw 1;
				}
				buffer = getTag(&events);
				while (buffer != "/varChanges") {
					if (events.eof()) {
						throw 1;
					}
					if (buffer.substr(0, 10) == "var name=\"") {
						buffer.erase(0, 10);
						if (buffer.empty() || buffer.back() != '\"') {
							throw 1;
						}
						buffer.pop_back();
						if (buffer.empty()) {
							throw 1;
						}
						clearSpace(&events);
						getline(events, buffer2, '<');
						events.seekg(-1, ios_base::cur);
						varBuffer = numFromString(&buffer2);
						*varChanges.value(buffer) += varBuffer;
						if (getTag(&events) != "/var") {
							throw 1;
						}
						ignoreLine(&events);
						if (!events) {
							throw 1;
						}
						buffer = getTag(&events);
					}
					else {
						throw 1;
					}
				}
				ignoreLine(&events);
				if (!events) {
					throw 1;
				}
				buffer = getTag(&events);
				continue;
			}
			else if (buffer == "choice") {
				ignoreLine(&events);
				if (choices.size() < SHRT_MAX) {
					choices.emplace_back();
				}
				buffer = getTag(&events);
				while (buffer != "/choice") {
					if (events.eof()) {
						throw 1;
					}
					if (buffer == "text") {
						getline(events, choices.back().text, '<');
						events.seekg(-1, ios_base::cur);
					}
					else if (buffer == "healthChange") {
						events >> choices.back().healthChange;
					}
					else if (buffer == "manaChange") {
						events >> choices.back().manaChange;
					}
					else if (buffer == "projectileChange") {
						events >> choices.back().projectileChange;
					}
					else if (buffer == "req") {
						getline(events, choices.back().req, '<');
						events.seekg(-1, ios_base::cur);
					}
					else if (buffer == "hidden/") {
						choices.back().hidden = true;
						ignoreLine(&events);
						if (!events) {
							throw 1;
						}
						buffer = getTag(&events);
						continue;
					}
					else if (buffer == "eventName") {
						getline(events, choices.back().eventName, '<');
						events.seekg(-1, ios_base::cur);
					}
					else {
						throw 1;
					}
					if (getTag(&events) != '/' + buffer) {
						throw 1;
					}
					ignoreLine(&events);
					if (!events) {
						throw 1;
					}
					buffer = getTag(&events);
				}
				ignoreLine(&events);
				if (!events) {
					throw 1;
				}
				buffer = getTag(&events);
				continue;
			}
			else {
				throw 1;
			}
			events.seekg(-1, ios_base::cur);
			if (getTag(&events) != '/' + buffer) {
				throw 1;
			}
			ignoreLine(&events);
			if (!events) {
				throw 1;
			}
			buffer = getTag(&events);
		}
		events.close();
		eventName = blueprint;
	}
	catch (int err) {
		events.close();
		eventName = preBattleSpell = enemyBlueprint = postBattleSpell = reward = "EMPTY";
		preBattleText = postBattleText = "";
		choices.resize(0);
		varChanges.reset();
		switch (err) {
		case 1:
			cout << "Unable to parse event or eventList " << blueprint << '\n';
			break;
		case 2:
			if (custom) {
				loadFromFile(blueprint, false);
				return;
			}
			cout << "No event or eventList found with name " << blueprint << '\n';
			break;
		case 4:
			if (custom) {
				loadFromFile(blueprint, false);
				return;
			}
			cout << "Could not open events.xml\n";
			break;
		case 5:
			cout << "eventList " << blueprint << " contains no entries\n";
			break;
		case 8:
			cout << "Maximum number of variables reached (32767)\n";
			break;
		}
	}
}

unsigned char Event::eventHandler(player* playerCharacter) {
	static spell eventSpell;
	static enemy opponent;
	if (!preBattleText.empty()) {
		cout << preBattleText << '\n';
	}
	g_customVars += varChanges;
	eventSpell.loadFromFile(preBattleSpell);
	opponent.loadFromFile(enemyBlueprint);
	if (eventSpell.getReal()) {
		if (spellCast(&eventSpell, playerCharacter) == 1) {
			return 2;
		}
	}
	if (opponent.getReal()) { //REVIEW WORDING
		cout << opponent.getIntroduction() << '\n' << "To start the battle, enter 1.\n";
		userChoice(1, 1);
		if (battleHandler(playerCharacter, &opponent) == 2) {
			return 2;
		}
		playerCharacter->reset();
		if (playerCharacter->getHealth() <= 0) {
			return 2;
		}
	}
	if (!postBattleText.empty()) {
		cout << postBattleText << '\n';
	}
	eventSpell.loadFromFile(postBattleSpell);
	if (eventSpell.getReal()) {
		if (spellCast(&eventSpell, playerCharacter) == 1) {
			return 2;
		}
	}
	switch (blueprintListSelector(&reward)) { //Find type of equipment and its blueprint name
	case 1: //Helmet
	{
		armourHead newHelmet(reward);
		cout << "New headwear: " << newHelmet.getName() << '\n';
		cout << "To view stats, enter 1.\nTo discard, enter 2.\n";
		if (userChoice(1, 2) == 1) {
			playerCharacter->equip(&newHelmet);
		}
		break;
	}
	case 2: //Chestplate
	{
		armourTorso newChestplate(reward);
		cout << "New chestwear: " << newChestplate.getName() << '\n';
		cout << "To view stats, enter 1.\nTo discard, enter 2.\n";
		if (userChoice(1, 2) == 1) {
			playerCharacter->equip(&newChestplate);
		}
		break;
	}
	case 3: //Legs
	{
		armourLegs newGreaves(reward);
		cout << "New legwear: " << newGreaves.getName() << '\n';
		cout << "To view stats, enter 1.\nTo discard, enter 2.\n";
		if (userChoice(1, 2) == 1) {
			playerCharacter->equip(&newGreaves);
		}
		break;
	}
	case 4: //Boots
	{
		armourFeet newBoots(reward);
		cout << "New footwear: " << newBoots.getName() << '\n';
		cout << "To view stats, enter 1.\nTo discard, enter 2.\n";
		if (userChoice(1, 2) == 1) {
			playerCharacter->equip(&newBoots);
		}
		break;
	}
	case 5: //Weapon
	{
		weapon newWeapon(reward);
		cout << "New weapon: " << newWeapon.getName() << '\n';
		cout << "To view stats, enter 1.\nTo discard, enter 2.\n";
		if (userChoice(1, 2) == 1) {
			playerCharacter->equip(&newWeapon);
		}
		break;
	}
	case 6: //Spell
		eventSpell.loadFromFile(reward);
		cout << "New spell: " << eventSpell.getName() << '\n';
		cout << "To view stats, enter 1.\nTo discard, enter 2.\n";
		if (userChoice(1, 2) == 1) {
			playerCharacter->equip(&eventSpell);
		}
		break;
	}
	//Recalculate modifiers, as new equipment may have been equipped, then check player is not dead
	playerCharacter->calculateModifiers();
	if (playerCharacter->getHealth() <= 0) {
		return 2;
	}
	vector<choice> possibleChoices; //Choices which are allowed
	vector<Event> newEvents; //Events which the choices will load
	for (short i = 0; i < choices.size(); i++) {
		if (evalCond(choices[i].req) && (choices[i].hidden || (choices[i].manaChange >= -playerCharacter->getMana() && choices[i].projectileChange >= -playerCharacter->getProjectiles() && choices[i].healthChange >= -playerCharacter->getHealth()))) { //Are requirements met for the choice
			possibleChoices.push_back(choices[i]); //Add choice to list
			newEvents.emplace_back(choices[i].eventName); //Load its event
		}
	}
	if (possibleChoices.empty()) { //If no choices allowed, add trivial one
		possibleChoices.emplace_back();
		newEvents.emplace_back();
	}
	cout << "Enter the number of the desired choice\n";
	for (short i = 0; i < possibleChoices.size(); i++) {
		cout << i + 1 << ". ";
		possibleChoices[i].display();
	}
	short chosen = userChoice(1, static_cast<int>(possibleChoices.size())) - 1; //Choose a choice
	//Apply stat changes
	playerCharacter->modifyHealth(possibleChoices[chosen].healthChange);
	if (playerCharacter->getHealth() <= 0) { //Check in case this killed the player
		return 2;
	}
	playerCharacter->modifyProjectiles(possibleChoices[chosen].projectileChange);
	playerCharacter->modifyMana(possibleChoices[chosen].manaChange);
	//Set next event
	if (newEvents[chosen].eventName == "EMPTY") {
		return 0;
	}
	*this = newEvents[chosen];
	return 1;
}

void choice::display() {
	cout << text << '\n';
	if ((healthChange != 0 || manaChange != 0 || projectileChange != 0) && !hidden) {
		cout << '(';
		if (healthChange > 0) {
			cout << "Gain " << healthChange << " health; ";
		}
		else if (healthChange < 0) {
			cout << "Lose " << healthChange << " health; ";
		}
		if (manaChange == 1) {
			cout << "Gain 1 " << g_manaName.singular() << "; ";
		}
		else if (manaChange > 1) {
			cout << "Gain " << manaChange << ' ' << g_manaName.plural() << "; ";
		}
		else if (manaChange == -1) {
			cout << "Lose 1 " << g_manaName.singular() << "; ";
		}
		else if (manaChange < -1) {
			cout << "Lose " << -manaChange << ' ' << g_manaName.plural() << "; ";
		}
		if (projectileChange == 1) {
			cout << "Gain 1 " << g_projName.singular() << "; ";
		}
		else if (projectileChange > 1) {
			cout << "Gain " << projectileChange << ' ' << g_projName.plural() << "; ";
		}
		else if (projectileChange == -1) {
			cout << "Lose 1 " << g_projName.singular() << "; ";
		}
		else if (projectileChange < -1) {
			cout << "Lose " << -projectileChange << ' ' << g_projName.plural() << "; ";
		}
		cout << (char)8 << (char)8 << ")\n";
	}
}