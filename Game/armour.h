#pragma once
#include <string>

extern bool g_useCustomData;

class armour {
protected:
	bool real; //Actual piece of armour. If it is false, it is an empty slot
	short maxHealthModifier;
	short maxManaModifier;
	short turnManaRegenModifier;
	short battleManaRegenModifier;
	short constRegenModifier;
	short battleRegenModifier;
	short flatArmourModifier;
	float propArmourModifier;
	short flatMagicArmourModifier;
	float propMagicArmourModifier;
	short flatDamageModifier;
	float propDamageModifier;
	short flatMagicDamageModifier;
	float propMagicDamageModifier;
	short flatArmourPiercingDamageModifier;
	float propArmourPiercingDamageModifier;
	float evadeChanceModifier;
	float poisonResistModifier;
	float bleedResistModifier;
	float counterAttackChanceModifier;
	short bonusActionsModifier;
	std::string armourName;
	std::string name; //Armour name
	std::string description; //Description
public:
	//Get attributes
	std::string getArmourName() { return armourName; }
	short getMaxHealthModifier() { return maxHealthModifier; }
	short getMaxManaModifier() { return maxManaModifier; }
	short getTurnManaRegenModifier() { return turnManaRegenModifier; }
	short getBattleManaRegenModifier() { return battleManaRegenModifier; }
	short getConstRegenModifier() { return constRegenModifier; }
	short getBattleRegenModifier() { return battleRegenModifier; }
	short getFlatArmourModifier() { return flatArmourModifier; }
	float getPropArmourModifier() { return propArmourModifier; }
	short getFlatMagicArmourModifier() { return flatMagicArmourModifier; }
	float getPropMagicArmourModifier() { return propMagicArmourModifier; }
	short getFlatDamageModifier() { return flatDamageModifier; }
	float getPropDamageModifier() { return propDamageModifier; }
	float getEvadeChanceModifier() { return evadeChanceModifier; }
	float getPoisonResistModifier() { return poisonResistModifier; }
	float getBleedResistModifier() { return bleedResistModifier; }
	std::string getName();
	std::string getDescription() { return description; }
	short getFlatMagicDamageModifier() { return flatMagicDamageModifier; }
	float getPropMagicDamageModifier() { return propMagicDamageModifier; }
	short getFlatArmourPiercingDamageModifier() { return flatArmourPiercingDamageModifier; }
	float getPropArmourPiercingDamageModifier() { return propArmourPiercingDamageModifier; }
	float getCounterAttackChanceModifier() { return counterAttackChanceModifier; }
	short getBonusActionsModifier() { return bonusActionsModifier; }
	bool getReal() { return real; }
	//Displays an armour piece's attributes, intended for use in an inventory or similar
	void displayStats();
	//Returns type of armour, 1 is head, 2 is torso, 3 is legs, 4 is feet
	virtual char armourType() { return 0; }
	//Loads armour from file
	void loadFromFile(std::string blueprint = "EMPTY", bool custom = g_useCustomData);
	armour(std::string blueprint = "EMPTY") { loadFromFile(blueprint); }
};

class armourHead :public armour {
public:
	//Constructor to load from file, blueprint "EMPTY" refers to a non existent piece of armour
	armourHead(std::string blueprint = "EMPTY") { loadFromFile(blueprint); }
	//Loads data from file onto already existing armour, defaults to replacing with non existent armour
	//void loadFromFile(std::string blueprint = "EMPTY");
	//Returns type of armour
	char armourType() { return 1; }
};

class armourTorso :public armour {
public:
	//Constructor to load from file, blueprint "EMPTY" refers to a non existent piece of armour
	armourTorso(std::string blueprint = "EMPTY") { loadFromFile(blueprint); }
	//Loads data from file onto already existing armour, defaults to replacing with non existent armour
	//void loadFromFile(std::string blueprint = "EMPTY");
	//Returns type of armour
	char armourType() { return 2; }
};

class armourLegs :public armour {
public:
	//Constructor to load from file, blueprint "EMPTY" refers to a non existent piece of armour
	armourLegs(std::string blueprint = "EMPTY") { loadFromFile(blueprint); }
	//Loads data from file onto already existing armour, defaults to replacing with non existent armour
	//void loadFromFile(std::string blueprint = "EMPTY");
	//Returns type of armour
	char armourType() { return 3; }
};

class armourFeet :public armour {
public:
	//Constructor to load from file, blueprint "EMPTY" refers to a non existent piece of armour
	armourFeet(std::string blueprint = "EMPTY") { loadFromFile(blueprint); }
	//Loads data from file onto already existing armour, defaults to replacing with non existent armour
	//void loadFromFile(std::string blueprint = "EMPTY");
	//Returns type of armour
	char armourType() { return 4; }
};