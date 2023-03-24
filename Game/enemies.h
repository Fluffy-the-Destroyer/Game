#pragma once
#include "spells.h"
#include <vector>
#include "weapons.h"
#define AI_TYPES_NO 7 //Number of AI types (excluding the default random type)
#define AI_HEALING_THRESHOLD 0.8f //AI will not heal if remaining health proportion is this or more
#define ENEMY_OVERHEAL_DECAY 5 //Enemy over heal decay rate
#define ENEMY_MANA_DECAY 5 //Enemy excess mana decay rate

//Enemy AI types:
// All weapons are considered attacking, also enemies will always use specified initial spell (as long as they can afford it, and even if it kills them)
// 0: Does nothing, should not happen
// Types which use weapons and spells
// 1: Aggressive, rarely heals, attacks 75% of the time
// 2: Balanced, sometimes heals, attacks half the time
// 3: Defensive, heals more often, attacks 25% of the time
// Mage classes, will only use weapons if they have no available spells, or if all available spells would kill them, if all available actions would kill them, prefers spells
// 4: Aggressive
// 5: Balanced
// 6: Defensive
// 
// 7: Berserker, never casts spells

extern bool g_useCustomData;

class enemy {
private:
	bool real = false;
	std::string name = "";
	std::string introduction = "";
	short health = 0;
	short maxHealth = 0;
	short projectiles = 0;
	short mana = 0;
	short maxMana = 0;
	short turnManaRegen = 0;
	uint8_t poison = 0;
	float poisonResist = 0.1f;
	uint8_t bleed = 0;
	float bleedResist = 0.1f;
	uint8_t tempRegen = 0;
	short turnRegen = 0;
	std::vector<weapon> weapons;
	std::vector<spell> spells;
	short initialSpell = -1; //The enemy will use the spell in this slot on turn 1. If it's -1, there is no initial spell
	spell deathSpell; //This spell is cast on death, ignoring costs. Cannot revive enemy and cannot be cast at any other time
	short flatArmour = 0;
	float propArmour = 0;
	short flatMagicArmour = 0;
	float propMagicArmour = 0;
	short flatDamageModifier = 0;
	float propDamageModifier = 0;
	short flatMagicDamageModifier = 0;
	float propMagicDamageModifier = 0;
	short flatArmourPiercingDamageModifier = 0;
	float propArmourPiercingDamageModifier = 0;
	float evadeChance = 0.1f;
	float counterAttackChance = 0.1f;
	int8_t bonusActions = 1;
	int8_t currentBonusActions = 0;
	uint8_t AIType = 2; //What sort of AI it has. See top of file
	std::vector<std::string> noCounterWeapons; //Weapons the enemy knows cannot be countered
	std::vector<std::string> noCounterSpells; //Spells the enemy knows cannot be countered
	short initiative = 10;
	int xp = 0; //How much xp is gained by defeating it
public:
	void removeAllHealth() { health = 0; }
	short flatDamage(short p, short m = 0, short a = 0, bool overheal = false);
	void propDamage(float d);
	void modifyHealth(short h);
	void modifyMaxHealth(short m);
	void modifyProjectiles(short p);
	void modifyMana(short m);
	void modifyMaxMana(short m);
	void modifyTurnManaRegen(short t);
	bool modifyPoison(short p, bool resist = true);
	bool modifyBleed(short b, bool resist = true);
	void modifyTempRegen(short r);
	void modifyTurnRegen(short c);
	void modifyFlatArmour(short f);
	void modifyPropArmour(float p);
	void modifyFlatMagicArmour(short f);
	void modifyPropMagicArmour(float p);
	void modifyFlatDamageModifier(short f);
	void modifyPropDamageModifier(float p);
	void modifyFlatMagicDamageModifier(short f);
	void modifyPropMagicDamageModifier(float p);
	void modifyFlatArmourPiercingDamageModifier(short f);
	void modifyPropArmourPiercingDamageModifier(float p);
	void modifyEvadeChance(float e);
	void modifyPoisonResist(float p);
	void modifyBleedResist(float b);
	void modifyCounterAttackChance(float c);
	void modifyBonusActions(short b);
	void resetBonusActions();
	//Start of turn, decrements cooldowns, applies dot/regen and decrements them
	void turnStart();
	//Get attributes
	short getHealth() { return health; }
	short getMaxHealth() { return maxHealth; }
	short getProjectiles() { return projectiles; }
	short getMana() { return mana; }
	short getMaxMana() { return maxMana; }
	short getTurnManaRegen() { return turnManaRegen; }
	uint8_t getPoison() { return poison; }
	float getPoisonResist() { return poisonResist; }
	uint8_t getBleed() { return bleed; }
	float getBleedResist() { return bleedResist; }
	uint8_t getTempRegen() { return tempRegen; }
	short getTurnRegen() { return turnRegen; }
	uint8_t getWeapons() { return static_cast<uint8_t>(weapons.size()); }
	weapon* getWeapon(uint8_t i);
	uint8_t getSpells() { return static_cast<uint8_t>(spells.size()); }
	spell* getSpell(uint8_t i);
	short getInitialSpell() { return initialSpell; }
	spell* getDeathSpell() { return &deathSpell; }
	short getFlatArmour() { return flatArmour; }
	float getPropArmour() { return propArmour; }
	short getFlatMagicArmour() { return flatMagicArmour; }
	float getPropMagicArmour() { return propMagicArmour; }
	short getFlatDamageModifier() { return flatDamageModifier; }
	float getPropDamageModifier() { return propDamageModifier; }
	short getFlatMagicDamageModifier() { return flatMagicDamageModifier; }
	bool getReal() { return real; }
	float getPropMagicDamageModifier() { return propMagicDamageModifier; }
	short getFlatArmourPiercingDamageModifier() { return flatArmourPiercingDamageModifier; }
	float getPropArmourPiercingDamageModifier() { return propArmourPiercingDamageModifier; }
	float getEvadeChance() { return evadeChance; }
	float getCounterAttackChance() { return counterAttackChance; }
	int8_t getBonusActions() { return bonusActions; }
	int8_t getCurrentBonusActions() { return currentBonusActions; }
	std::string getName() { return name; }
	std::string getIntroduction() { return introduction; }
	short rollInitiative() { return rng(0, initiative); }
	int getXp() { return xp; }
	//Load from file
	void loadFromFile(std::string blueprint, bool custom = g_useCustomData);
	//Decides what action the enemy should take. Returns 0 for no action, 1 for a weapon, 2 for a spell. Returns 3 for dual weapon attack. Sets the value of 'slot' to the slot it has chosen. Timing 0 is normal, 1 is in response to weapon attack, 2 is in response to spell cast, 3 is counter attack, 4 is in response to dual weapon attack. itemName is the name of the weapon/spell being responded to
	uint8_t chooseAction(uint8_t* slot1, uint8_t* slot2, uint8_t timing = 0, std::string itemName1 = "", std::string itemName2 = "", bool firstTurn = false);
	//Checks if enemy can afford to use weapon or spell, then if kamikaze is false, checks it would not lead to suicide. Type true indicates spell, false indicates weapon. If specified weapon or spell is out of range or not real, returns false. Timing specifies the spell cast timing
	bool check(bool type, uint8_t slot, uint8_t timing = 0, bool kamikaze = false, bool firstTurn = false);
	//Simulates a turn of poison/bleed/regeneration
	void simulateTurn();
	//Chooses a spell, returns false if it didn't pick one, otherwise stores slot in selection. type is the spell type to look for
	bool chooseSpell(uint8_t type, uint8_t* selection, uint8_t timing = 0, bool kamikaze = false, bool firstTurn = false);
	//Chooses a weapon, returns 0 if it picked nothing, 1 if it picked a weapon, 3 if it picked dual weapons
	uint8_t chooseWeapon(uint8_t* selection1, uint8_t* selection2, uint8_t timing = 0, bool kamikaze = false, bool firstTurn = false);
	//Chooses a spell to counter a weapon, returns false if it didn't find one, otherwise stores slot in selection
	bool chooseWeaponCounterSpell(uint8_t* selection, bool firstTurn = false);
	//Chooses a spell to counter a spell, returns false if it didn't find one, otherwise stores slot in selection
	bool chooseSpellCounterSpell(uint8_t* selection, bool firstTurn = false);
	//Chooses a weapon or spell to use to attack, returns 0 if it didn't pick one, 1 if it picked a weapon and 2 if it picked a spell. Returns 3 if it picked dual weapons. If it picked one, stores slot in selection
	uint8_t chooseAttack(uint8_t* selection1, uint8_t* selection2, uint8_t timing = 0, bool kamikaze = false, bool firstTurn = false);
	//Chooses an action when it has no survivable actions
	uint8_t chooseSuicide(uint8_t* selection1, uint8_t* selection2);
	//Makes healing check
	bool healingCheck();
	//Makes attack check
	bool attackCheck();
	//Checks if two weapons can be dual wielded
	bool check(uint8_t weapon1, uint8_t weapon2, uint8_t timing = 0, bool kamikaze = false, bool firstTurn = false);
	//Adds weapon/spell name to the list of weapons/spells which cannot be countered. Type 1 is a weapon, 2 a spell
	void addNoCounter(uint8_t type, std::string itemName);
	//Checks if the weapon/spell can be countered (by the enemy's knowledge)
	bool checkCounter(uint8_t type, std::string itemName);
	//Applies modifiers to damage
	void applyDamageModifiers(short* p, short* m, short* a);
};