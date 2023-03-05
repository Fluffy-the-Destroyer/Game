#pragma once
#include <string>
#include "rng.h"
#define SPELL_TYPES_NO 5 //Number of types of spell, excluding the no type set type (type 0)
#define ATTACK_SPELL_FLAT_CUTOFF 10 //Amount of flat damage a spell must do to be considered an attack spell
#define ATTACK_SPELL_PROP_CUTOFF 0.2f //Amount of prop damage a spell must do to be considered an attack spell
#define HEALING_SPELL_FLAT_CUTOFF 10 //Amount of flat healing a spell must do to be considered a healing spell
#define HEALING_SPELL_PROP_CUTOFF 0.2f //Amount of prop healing a spell must do to be considered a healing spell

//Spell types:
// 0: No type set
// 1: Attack spell
// 2: Healing spell
// 3: Support spell - not attack or healing
// 4: Attack and healing, e.g. a life drain spell
// 5: Counter only - spell does nothing except counter actions

extern bool g_useCustomData;

class spell {
private:
	bool real; //Is it a real spell
	std::string spellName;
	std::string name; //Spell name
	std::string description; //Spell description
	short flatDamageMin;
	short flatDamageMax;
	short flatMagicDamageMin;
	short flatMagicDamageMax;
	short flatArmourPiercingDamageMin;
	short flatArmourPiercingDamageMax;
	float propDamage;
	short flatSelfDamageMin;
	short flatSelfDamageMax;
	short flatSelfMagicDamageMin;
	short flatSelfMagicDamageMax;
	short flatSelfArmourPiercingDamageMin;
	short flatSelfArmourPiercingDamageMax;
	float propSelfDamage;
	unsigned char hitCount;
	unsigned char counterHits;
	bool noEvade;
	bool canCounterAttack; //Allows counter attacks
	bool noCounter; //Spell cannot be countered
	short manaChangeEnemy; //Reduces enemy's mana by this much
	short manaChange;
	short projectileChange;
	short poison; //Negatives remove poison
	short selfPoison;
	short bleed;
	short selfBleed;
	short maxHealthModifierEnemy; //Modifies enemy's maximum health
	short maxHealthModifier; //Modifies player's max health
	short maxManaModifierEnemy; //Modifies enemy's max mana
	short maxManaModifier; //Modifies player's max mana
	short turnManaRegenModifierEnemy; //Modifies enemy's turn mana regen
	short turnManaRegenModifier; //Modifies player's turn mana regen
	short battleManaRegenModifierEnemy;
	short battleManaRegenModifier; //Modifies player's battle mana regen
	float poisonResistModifierEnemy; //Modifies enemy's poison resist
	float poisonResistModifier; //Modifies player's poison resist
	float bleedResistModifierEnemy; //Modifies enemy's bleed resist
	float bleedResistModifier; //Modifies player's bleed resist
	short tempRegen; //Modifies enemy's tempRegen
	short tempRegenSelf; //Modifies player's tempRegen
	short constRegenModifierEnemy; //Modifies enemy's const regen
	short constRegenModifier; //Modifies player's const regen
	short battleRegenModifierEnemy;
	short battleRegenModifier; //Modifies player's battle regen
	short flatArmourModifierEnemy; //Modifies enemy's flat armour
	short flatArmourModifier; //Modifies player's flat armour
	float propArmourModifierEnemy; //Modifies enemy's prop armour
	float propArmourModifier; //Modifies player's prop armour
	short flatMagicArmourModifierEnemy; //Modifies enemy's flat magic armour
	short flatMagicArmourModifier; //Modifies player's flat magic armour
	float propMagicArmourModifierEnemy; //Modifies enemy's prop magic armour
	float propMagicArmourModifier; //Modifies player's prop magic armour
	short flatDamageModifierEnemy; //Modifies enemy's flat damage modifier
	short flatDamageModifier;
	float propDamageModifierEnemy;
	float propDamageModifier;
	short flatMagicDamageModifierEnemy;
	short flatMagicDamageModifier;
	float propMagicDamageModifierEnemy;
	float propMagicDamageModifier;
	short flatArmourPiercingDamageModifierEnemy;
	short flatArmourPiercingDamageModifier;
	float propArmourPiercingDamageModifierEnemy;
	float propArmourPiercingDamageModifier;
	float evadeChanceModifierEnemy;
	float evadeChanceModifier;
	unsigned char cooldown; //Cooldown between uses, a value of 1 allows 1 use every turn, value of 0 is unlimited use
	unsigned char currentCooldown; //Current cooldown. Gets set to cooldown on cast, decrements every turn, cannot cast if non-zero
	unsigned char spellType; //Type of spell, see top of file
	unsigned char timing; //When can it be cast, 0 is only during own turn, 1 is can also be cast in response to enemy action, 2 is can only be cast in response to enemy action
	unsigned char counterSpell; //0 is no countering, 1 is can counter spells, 2 is can counter weapons, 3 is can counter both
	short bonusActionsModifierEnemy;
	short bonusActionsModifier;
	bool lifelink;
	short healthChange;
	bool selfOverheal;
	bool targetOverheal;
	unsigned char effectType; //Whom it can affect. 0 is no effect, 1 is only caster, 2 is also target, 3 is only target. 10s digit is for flat damage, units for all other effects
	std::string upgrade;
public:
	//Loads spell from file
	void loadFromFile(std::string blueprint = "EMPTY", bool custom = g_useCustomData);
	//Constructor, just calls function to load from file
	spell(std::string blueprint = "EMPTY") { loadFromFile(blueprint); }
	void startCooldown() { currentCooldown = cooldown; }
	void resetCooldown() { currentCooldown = 0; }
	void decCooldown();
	std::string getSpellName() { return spellName; }
	std::string getName();
	std::string getDescription() { return description; }
	short getFlatDamage() { return rng(flatDamageMin, flatDamageMax); }
	short getFlatDamageMin() { return flatDamageMin; }
	short getFlatDamageMax() { return flatDamageMax; }
	short getFlatMagicDamage() { return rng(flatMagicDamageMin, flatMagicDamageMax); }
	short getFlatMagicDamageMin() { return flatMagicDamageMin; }
	short getFlatMagicDamageMax() { return flatMagicDamageMax; }
	short getFlatArmourPiercingDamage() { return rng(flatArmourPiercingDamageMin, flatArmourPiercingDamageMax); }
	short getFlatArmourPiercingDamageMin() { return flatArmourPiercingDamageMin; }
	short getFlatArmourPiercingDamageMax() { return flatArmourPiercingDamageMax; }
	float getPropDamage() { return propDamage; }
	short getFlatSelfDamage() { return rng(flatSelfDamageMin, flatSelfDamageMax); }
	short getFlatSelfMagicDamage() { return rng(flatSelfMagicDamageMin, flatSelfMagicDamageMax); }
	short getFlatSelfArmourPiercingDamage() { return rng(flatSelfArmourPiercingDamageMin, flatSelfArmourPiercingDamageMax); }
	short getFlatSelfDamageMin() { return flatSelfDamageMin; }
	short getFlatSelfMagicDamageMin() { return flatSelfMagicDamageMin; }
	short getFlatSelfArmourPiercingDamageMin() { return flatSelfArmourPiercingDamageMin; }
	short getFlatSelfDamageMax() { return flatSelfDamageMax; }
	short getFlatSelfMagicDamageMax() { return flatSelfMagicDamageMax; }
	short getFlatSelfArmourPiercingDamageMax() { return flatSelfArmourPiercingDamageMax; }
	float getPropSelfDamage() { return propSelfDamage; }
	unsigned char getHitCount() { return hitCount; }
	bool getNoEvade() { return noEvade; }
	short getManaChangeEnemy() { return manaChangeEnemy; }
	short getManaChange() { return manaChange; }
	short getProjectileChange() { return projectileChange; }
	short getPoison() { return poison; }
	short getSelfPoison() { return selfPoison; }
	short getBleed() { return bleed; }
	short getSelfBleed() { return selfBleed; }
	short getMaxHealthModifierEnemy() { return maxHealthModifierEnemy; }
	short getMaxHealthModifier() { return maxHealthModifier; }
	short getMaxManaModifierEnemy() { return maxManaModifierEnemy; }
	short getMaxManaModifier() { return maxManaModifier; }
	short getTurnManaRegenModifierEnemy() { return turnManaRegenModifierEnemy; }
	short getTurnManaRegenModifier() { return turnManaRegenModifier; }
	short getBattleManaRegenModifierEnemy() { return battleManaRegenModifierEnemy; }
	short getBattleManaRegenModifier() { return battleManaRegenModifier; }
	float getPoisonResistModifierEnemy() { return poisonResistModifierEnemy; }
	float getPoisonResistModifier() { return poisonResistModifier; }
	float getBleedResistModifierEnemy() { return bleedResistModifierEnemy; }
	float getBleedResistModifier() { return bleedResistModifier; }
	short getTempRegen() { return tempRegen; }
	short getTempRegenSelf() { return tempRegenSelf; }
	short getConstRegenModifierEnemy() { return constRegenModifierEnemy; }
	short getConstRegenModifier() { return constRegenModifier; }
	short getBattleRegenModifierEnemy() { return battleRegenModifierEnemy; }
	short getBattleRegenModifier() { return battleRegenModifier; }
	short getFlatArmourModifierEnemy() { return flatArmourModifierEnemy; }
	short getFlatArmourModifier() { return flatArmourModifier; }
	float getPropArmourModifierEnemy() { return propArmourModifierEnemy; }
	float getPropArmourModifier() { return propArmourModifier; }
	short getFlatMagicArmourModifierEnemy() { return flatMagicArmourModifierEnemy; }
	short getFlatMagicArmourModifier() { return flatMagicArmourModifier; }
	float getPropMagicArmourModifierEnemy() { return propMagicArmourModifierEnemy; }
	float getPropMagicArmourModifier() { return propMagicArmourModifier; }
	short getFlatDamageModifierEnemy() { return flatDamageModifierEnemy; }
	short getFlatDamageModifier() { return flatDamageModifier; }
	float getPropDamageModifierEnemy() { return propDamageModifierEnemy; }
	float getPropDamageModifier() { return propDamageModifier; }
	short getFlatMagicDamageModifierEnemy() { return flatMagicDamageModifierEnemy; }
	short getFlatMagicDamageModifier() { return flatMagicDamageModifier; }
	float getPropMagicDamageModifierEnemy() { return propMagicDamageModifierEnemy; }
	float getPropMagicDamageModifier() { return propMagicDamageModifier; }
	short getFlatArmourPiercingDamageModifierEnemy() { return flatArmourPiercingDamageModifierEnemy; }
	short getFlatArmourPiercingDamageModifier() { return flatArmourPiercingDamageModifier; }
	float getPropArmourPiercingDamageModifierEnemy() { return propArmourPiercingDamageModifierEnemy; }
	float getPropArmourPiercingDamageModifier() { return propArmourPiercingDamageModifier; }
	float getEvadeChanceModifierEnemy() { return evadeChanceModifierEnemy; }
	float getEvadeChanceModifier() { return evadeChanceModifier; }
	unsigned char getCooldown() { return cooldown; }
	unsigned char getCurrentCooldown() { return currentCooldown; }
	unsigned char getSpellType() { return spellType; }
	unsigned char getCounterHits() { return counterHits; }
	bool getCanCounterAttack() { return canCounterAttack; }
	unsigned char getTiming() { return timing; }
	unsigned char getCounterSpell() { return counterSpell; }
	short getBonusActionsModifierEnemy() { return bonusActionsModifierEnemy; }
	short getBonusActionsModifier() { return bonusActionsModifier; }
	bool getNoCounter() { return noCounter; }
	bool getReal() { return real; }
	bool getLifelink() { return lifelink; }
	short getHealthChange() { return healthChange; }
	unsigned char getEffectType() { return effectType; }
	bool getSelfOverheal() { return selfOverheal; }
	bool getTargetOverheal() { return targetOverheal; }
	//Displays stats
	void displayStats();
	//Sets spell type based on stats, pretty rudimentary, ideally the type should be specified in the blueprint. For example, a spell which does nothing would be classified as support, and a support focused enemy would use it
	void setSpellType();
	//Displays name and cost, for use in battle
	void displayName();
	//Checks if the spell is the given type, type 4 spells count as type 1 and type 2
	bool checkSpellType(unsigned char type);
	//Checks if the spell has any effect on caster
	bool checkSelfEffect();
	//Checks if affects target
	bool checkTargetEffect();
	//Sets effectType
	void setEffectType();
	bool checkSelfDamage();
	bool checkTargetDamage();
	//Upgrades the item, returns false if it wasn't upgraded
	bool upgradeItem();
};