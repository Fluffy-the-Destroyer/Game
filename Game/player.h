#pragma once
#include "weapons.h"
#include "spells.h"
#include "armour.h"
#include <vector>
#include "rng.h"
#define PLAYER_OVERHEAL_DECAY 5 //Amount of overheal lost per turn
#define MANA_DECAY 5 //Excess mana lost per turn

extern bool g_useCustomData;

class player {
private:
	std::string className;
	short health = 100; //Current health
	short maxHealthBase = 100; //Base max health
	short maxHealth = 100; //Max health. Must remain non-negative. Value of zero would lead to death
	short projectiles = 0; //How many projectiles the player has
	short mana = 0; //Current mana
	short maxManaBase = 0; //Base max mana
	short maxMana = 0; //Max mana
	short turnManaRegenBase = 0; //Base mana regen per turn (may vary by class)
	short turnManaRegen = 0; //Actual mana regen per turn (after modifiers)
	short battleManaRegenBase = 0; //Base mana regen after battle
	short battleManaRegen = 0; //Actual mana regen after battle
	unsigned char poison = 0; //Current amount of poison. Every turn, will lose k times this value in health, if positive, then it is decremented. k a constant to be determined later.
	float poisonResistBase = 0.1f;
	float poisonResist = 0; //Chance to resist poison. Must be at least 0
	unsigned char bleed = 0; //Current amount of bleed. Works the same as poison, but they will both decrement
	float bleedResistBase = 0.1f;
	float bleedResist = 0; //Bleed resist chance
	unsigned char tempRegen = 0; //Current regen. Similar to poison but for healing.
	short constRegen = 0; //Regen amount which is independent of timer, probably added by armour. May allow negatives, for long term damage over time
	short constRegenBase = 0; //Base per turn regen
	short battleRegenBase = 0; //Base battle regen
	short battleRegen = 0; //Actual battle regen
	std::vector<weapon> weapons; //The player's weapons
	std::vector<spell> spells; //The player's spells
	short flatArmourBase = 0;
	short flatArmour = 0; //Flat damage reduction, sum of values on armour pieces, only allows non negative values
	float propArmourBase = 0;
	float propArmour = 0; //Proportional damage multiplier, cannot be less than -1. Positive means taking additional damage
	short flatMagicArmourBase = 0; //Armour against magic damage
	short flatMagicArmour = 0;
	float propMagicArmourBase = 0;
	float propMagicArmour = 0;
	armourHead helmet; //Currently equipped headwear
	armourTorso chestplate; //Currently equipped chestpiece
	armourLegs greaves; //Currently equipped legwear
	armourFeet boots; //Currently equipped footwear
	short flatDamageModifierBase = 0; //Base value
	short flatDamageModifier = 0; //Flat damage modifer, only modifies flat damage. Apllies to damage dealt by player
	float propDamageModifierBase = 0;
	float propDamageModifier = 0; //Proportional damage modifier. This is the increase. Must be at least -1, which would be no damage dealt
	short flatMagicDamageModifierBase = 0;
	short flatMagicDamageModifier = 0;
	float propMagicDamageModifierBase = 0;
	float propMagicDamageModifier = 0;
	short flatArmourPiercingDamageModifierBase = 0;
	short flatArmourPiercingDamageModifier = 0;
	float propArmourPiercingDamageModifierBase = 0;
	float propArmourPiercingDamageModifier = 0;
	float evadeChanceBase = 0.1f;
	float evadeChance = 0; //Chance to evade a hit. Only applies to flat damage, Must be at least 0. Values greater than 1 guarantee evasion
	float counterAttackChanceBase = 0.1f; //Chance to counter attack
	float counterAttackChance = 0;
	signed char bonusActionsBase = 1; //How many bonus actions can be taken in a turn, instant speed spells and counter attacks
	signed char currentBonusActions = 1; //How manybonus actions remain this turn
	signed char bonusActions = 1;
	short initiativeBase = 10;
	short initiative = 10;
	int xp = 0; //Current experience
	int maxXp = 0; //Experience needed to level
	std::string nextLevel; //The blueprint name fo the next level
	short level = 1;
public: //Not providing set functions, as these values should not usually be set. Providing functions for altering some attributes in certain ways
	//Sets health to 0
	void removeAllHealth() { health = 0; }
	//Heals to full
	void fullHeal() { health = maxHealth; }
	//Deals flat damage, applies armour. Negatives heal. p is physical damage, m is magic, a is armour piercing. Returns actual health loss, overheal=true allows overhealing
	short flatDamage(short p, short m = 0, short a = 0, bool overheal = false);
	//Damages player by a proportion of their health.
	void propDamage(float d);
	void modifyHealth(short h);
	//Modifies max health by specified amount, if increasing, heals by that amount. If decreasing, removes excess health if necessary
	void modifyMaxHealth(short m);
	//Recalculates max health
	void calculateMaxHealth();
	//Modifies projectiles by specified amount, prevents overflow
	void modifyProjectiles(short p);
	//Modifies mana by specified amount
	void modifyMana(short m);
	//Modifies maxMana by specified amount
	void modifyMaxMana(short m);
	//Recalculates max mana
	void calculateMaxMana();
	//Fully restores mana
	void fullMana() { mana = maxMana; }
	//Modifies turnManaRegen
	void modifyTurnManaRegen(short m);
	//Recalculates turnManaRegen
	void calculateTurnManaRegen();
	//Modifies battleManaRegen
	void modifyBattleManaRegen(short b);
	//Recalculates battleManaRegen
	void calculateBattleManaRegen();
	//Modifies poison level by specified amount, resist=true allows it to be resisted. Returns false if it was resisted
	bool modifyPoison(short p, bool resist = true);
	//Fully cures poison
	void curePoison() { poison = 0; }
	//Modifies bleed, resist=true allows it to be resisted
	bool modifyBleed(short b, bool resist = true);
	//Removes bleed
	void cureBleed() { bleed = 0; }
	//Modifies tempRegen
	void modifyTempRegen(short r);
	//Fully removes tempRegen
	void removeRegen() { tempRegen = 0; }
	//Modifies constRegen
	void modifyConstRegen(short c);
	//Recalculates constRegen to the value from armour
	void calculateConstRegen();
	//Modifies battleRegen
	void modifyBattleRegen(short b);
	//Recalculates battleRegen
	void calculateBattleRegen();
	//Equips the specified weapon into player chosen slot.
	void equip(weapon* w);
	//Equips specified spell into player chosen slot
	void equip(spell* s);
	//Modifies flat armour, will only last until recalculated
	void modifyFlatArmour(short f);
	//Modifies propArmour
	void modifyPropArmour(float p);
	//Modifies flatMagicArmour
	void modifyFlatMagicArmour(short f);
	//Modifies propMagicArmour
	void modifyPropMagicArmour(float p);
	//Recalculates armour. Sets it to the values from the equipped armour, intended to be used at end of fights
	void calculateArmour();
	//Equips specified headwear
	void equip(armourHead* h);
	//Equips chestpiece
	void equip(armourTorso* c);
	//Equips leggings
	void equip(armourLegs* g);
	//Equips shoes
	void equip(armourFeet* b);
	//Modifies flat damage modifier
	void modifyFlatDamageModifier(short f);
	//Modifies proportional damage modifier
	void modifyPropDamageModifier(float p);
	//Modifies flat magic damage modifier
	void modifyFlatMagicDamageModifier(short f);
	//Modifies prop magic damage modifier
	void modifyPropMagicDamageModifier(float p);
	//Modifies flat AP damage modifier
	void modifyFlatArmourPiercingDamageModifier(short f);
	//Modifies prop AP damage modifier
	void modifyPropArmourPiercingDamageModifier(float p);
	//Recalculates damage modifiers to those added by armour
	void calculateDamageModifiers();
	//Modifies evadeChance
	void modifyEvadeChance(float e);
	//Recalculates evadeChance
	void calculateEvadeChance();
	//Modifies poison resist
	void modifyPoisonResist(float p);
	//Recalculates poison resist
	void calculatePoisonResist();
	//Modify Bleed resist
	void modifyBleedResist(float b);
	//Calculate bleed resist
	void calculateBleedResist();
	//Modify Counter attack chance
	void modifyCounterAttackChance(float c);
	//Recalculates counter attack chance
	void calculateCounterAttackChance();
	//Modifies bonus actions
	void modifyBonusActions(short b);
	void resetBonusActions();
	void decBonusActions();
	//Recalculate bonus actions
	void calculateBonusActions();
	//Recalculates all modifiers
	void calculateModifiers();
	void modifyInitiative(short i);
	void calculateInitiative();
	//Start of turn, decrements cooldowns, applies dot/regen and decrements them
	void turnStart();
	//Get Attributes
	short getHealth() { return health; }
	short getMaxHealth() { return std::max(maxHealth, (short)0); }
	short getProjectiles() { return projectiles; }
	short getMana() { return mana; }
	short getMaxMana() { return std::max(maxMana, (short)0); }
	short getTurnManaRegenBase() { return turnManaRegenBase; }
	short getTurnManaRegen() { return turnManaRegen; }
	short getBattleManaRegenBase() { return battleManaRegenBase; }
	short getBattleManaRegen() { return battleManaRegen; }
	unsigned char getPoison() { return poison; }
	float getPoisonResist() { return poisonResist; }
	unsigned char getBleed() { return bleed; }
	float getBleedResist() { return bleedResist; }
	unsigned char getRegen() { return tempRegen; }
	short getConstRegen() { return constRegen; }
	short getConstRegenBase() { return constRegenBase; }
	short getBattleRegenBase() { return battleRegenBase; }
	short getBattleRegen() { return battleRegen; }
	unsigned char getWeaponSlots() { return static_cast<unsigned char>(weapons.size()); }
	//Will throw a 6 if looking at a slot out of range, must be in a try block
	weapon* getWeapon(unsigned char i);
	unsigned char getSpellSlots() { return static_cast<unsigned char>(spells.size()); }
	//Will throw a 6 if looking at a slot out of range, must be in a try block
	spell* getSpell(unsigned char i);
	short getFlatArmourBase() { return flatArmourBase; }
	short getFlatArmour() { return flatArmour; }
	float getPropArmourBase() { return propArmourBase; }
	float getPropArmour() { return propArmour; }
	armourHead* getHelmet() { return &helmet; }
	armourTorso* getChestplate() { return &chestplate; }
	armourLegs* getGreaves() { return &greaves; }
	armourFeet* getBoots() { return &boots; }
	short getFlatDamageModifierBase() { return flatDamageModifierBase; }
	short getFlatDamageModifier() { return flatDamageModifier; }
	float getPropDamageModifierBase() { return propDamageModifierBase; }
	float getPropDamageModifier() { return propDamageModifier; }
	short getFlatMagicDamageModifierBase() { return flatMagicDamageModifierBase; }
	short getFlatMagicDamageModifier() { return flatMagicDamageModifier; }
	float getPropMagicDamageModifierBase() { return propMagicDamageModifierBase; }
	float getPropMagicDamageModifier() { return propMagicDamageModifier; }
	short getFlatArmourPiercingDamageModifierBase() { return flatArmourPiercingDamageModifierBase; }
	short getFlatArmourPiercingDamageModifier() { return flatArmourPiercingDamageModifier; }
	float getPropArmourPiercingDamageModifierBase() { return propArmourPiercingDamageModifierBase; }
	float getPropArmourPiercingDamageModifier() { return propArmourPiercingDamageModifier; }
	short getFlatMagicArmourBase() { return flatMagicArmourBase; }
	short getFlatMagicArmour() { return flatMagicArmour; }
	float getPropMagicArmourBase() { return propMagicArmourBase; }
	float getPropMagicArmour() { return propMagicArmour; }
	float getEvadeChanceBase() { return evadeChanceBase; }
	float getEvadeChance() { return evadeChance; }
	float getCounterAttackChanceBase() { return counterAttackChanceBase; }
	float getCounterAttackChance() { return counterAttackChance; }
	signed char getBonusActionsBase() { return bonusActionsBase; }
	signed char getBonusActions() { return bonusActions; }
	signed char getCurrentBonusActions() { return currentBonusActions; }
	std::string getClassName() { return className; }
	short getInitiative() { return initiative; }
	short rollInitiative() { return rng(0, std::max((short)0, initiative)); }
	int getXp() { return xp; }
	int getMaxXp() { return maxXp; }
	short getLevel() { return level; }
	//Constructor, loads from file
	player(std::string playerClass) { loadClass(playerClass); }
	//Constructor with no class
	player(){}
	//Initialises the player from file as a specific class
	void loadClass(std::string playerClass, bool custom = g_useCustomData);
	//Displays player's stats, doesn't display equipment or temporary effects
	void displayStats();
	//Shows inventory
	void showInventory();
	//For end of battle, applies battle regens, removes status effects and recalculates modifiers
	void reset();
	friend void save(std::ifstream* file, player* playerCharacter, std::string filePath, unsigned char slot);
	//Gets the player to choose an action. Returns 0 for no action, 1 for a weapon, 2 for a spell, 3 for dual wield. Timing 0 is normal, 1 is responding to weapon, 2 responding to spell, 3 is counter attacking. Timing 4 is responding to dual attack. itemName holds the name(s) of weapon/spell being responded to. Stores slots of selected weapon/spell in slot1 and slot2
	unsigned char chooseAction(unsigned char* slot1, unsigned char* slot2, std::string enemyName, const unsigned char timing = 0, std::string itemName1 = "", std::string itemName2 = "");
	//Applies modifiers to damage
	void applyDamageModifiers(short* p, short* m, short* a);
	//Lets the player upgrade some items
	void upgradeItems(short upgradeNum = 1);
	//Gives the player xp, levels them as appropriate, negatives will drain xp, but it cannot go negative
	void giveXp(int xpGain);
	//Levels the player
	void levelUp();
	//Allows the player to use stat points, if upgradeNum is negative, player must downgrade that many stats
	void upgradeStats(short upgradeNum = 1);
	//Modifies the specified base stat
	friend class Event;
};

class playerLevel {
private:
	friend class player;
	bool fullHeal = true;
	short heal = 0;
	short maxHealth = 0;
	short projectiles = 0;
	bool fullMana = true;
	short mana = 0;
	short maxMana = 0;
	short turnManaRegen = 0;
	short battleManaRegen = 0;
	float poisonResist = -2; //Value of -2 is not allowed, so using it to indicate no change
	float bleedResist = -2;
	short constRegen = 0;
	short battleRegen = 0;
	short flatArmour = 0;
	float propArmour = -2;
	short flatMagicArmour = 0;
	float propMagicArmour = -2;
	short flatDamageModifier = 0;
	float propDamageModifier = -2;
	short flatMagicDamageModifier = 0;
	float propMagicDamageModifier = -2;
	short flatArmourPiercingDamageModifier = 0;
	float propArmourPiercingDamageModifier = -2;
	float evadeChance = -2;
	float counterAttackChance = -2;
	short bonusActions = 0;
	short initiative = 0;
	int maxXp = 0;
	std::string nextLevel;
	short statPoints = 0;
	short upgradePoints = 0;
	playerLevel(std::string blueprint) { loadFromFile(blueprint); }
	void loadFromFile(std::string blueprint, bool custom = g_useCustomData);
};