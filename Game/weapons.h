#pragma once
#include <string>
#include "rng.h"

extern bool g_useCustomData;

class weapon {
private:
	bool real; //Is it a real weapon
	std::string weaponName;
	std::string name; //Weapon name
	std::string description; //Description
	short flatDamageMin; //Minimum flat damage, negatives will heal
	short flatDamageMax; //Maximum flat damage
	short flatMagicDamageMin; //Minimum flat magic damage
	short flatMagicDamageMax; //Maximum flat magic damage
	short flatArmourPiercingDamageMin; //Minimum armour piercing damage
	short flatArmourPiercingDamageMax; //Maximum armour piercing damage
	float propDamage; //Proportional damage
	short flatSelfDamageMin; //Minimum flat self damage
	short flatSelfDamageMax;
	short flatSelfMagicDamageMin;
	short flatSelfMagicDamageMax;
	short flatSelfArmourPiercingDamageMin;
	short flatSelfArmourPiercingDamageMax;
	float propSelfDamage;
	short healthChange; //Health change on self as attack cost, applied even if attack is countered
	unsigned char hitCount; //Number of hits when attacking, will deal damage for each, value of 0 means cannot attack (may still counter attack)
	unsigned char counterHits; //Number of hits when counter attacking, value of 0 means it cannot counter attack
	bool noEvade; //Is the attack impossible to dodge
	bool noCounter; //Can the attack be countered
	bool noCounterAttack; //Are counter attacks allowed
	short manaChange; //Mana cost to use the weapon
	short projectileChange; //Projectile cost to attack
	unsigned char poison; //Poison duration applied
	unsigned char selfPoison; //Poison applied to self
	unsigned char bleed; //Bleed duration applied
	unsigned char selfBleed; //Bleed applied to self
	bool lifelink; //Heals user equal to damage done
	bool dualWield; //If two weapons with this are equipped, they can both be used in a single attack
	unsigned char effectType; //Whom it can affect. 0 is no effect, 1 is only attacker, 2 is also target, 3 is only target. 10s digit is for flat damage, units for all other effects
	bool selfOverheal; //Can it overheal wielder
	bool targetOverheal;
	std::string upgrade; //The blueprint name of the upgraded version
public:
	//Loads weapon from file
	void loadFromFile(std::string blueprint = "EMPTY", bool custom = g_useCustomData);
	//Constructor, calls function to load from file
	weapon(std::string blueprint = "EMPTY") { loadFromFile(blueprint); }
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
	short getFlatSelfDamageMin() { return flatSelfDamageMin; }
	short getFlatSelfDamageMax() { return flatSelfDamageMax; }
	short getFlatSelfMagicDamage() { return rng(flatSelfMagicDamageMin, flatSelfMagicDamageMax); }
	short getFlatSelfMagicDamageMin() { return flatSelfMagicDamageMin; }
	short getFlatSelfMagicDamageMax() { return flatSelfMagicDamageMax; }
	short getFlatSelfArmourPiercingDamage() { return rng(flatSelfArmourPiercingDamageMin, flatSelfArmourPiercingDamageMax); }
	short getFlatSelfArmourPiercingDamageMin() { return flatSelfArmourPiercingDamageMin; }
	short getFlatSelfArmourPiercingDamageMax() { return flatSelfArmourPiercingDamageMax; }
	float getPropSelfDamage() { return propSelfDamage; }
	unsigned char getHitCount() { return hitCount; }
	bool getNoEvade() { return noEvade; }
	bool getNoCounter() { return noCounter; }
	short getManaChange() { return manaChange; }
	short getProjectileChange() { return projectileChange; }
	unsigned char getPoison() { return poison; }
	unsigned char getBleed(){return bleed;}
	unsigned char getSelfPoison() { return selfPoison; }
	unsigned char getSelfBleed() { return selfBleed; }
	unsigned char getCounterHits() { return counterHits; }
	bool getNoCounterAttack() { return noCounterAttack; }
	bool getReal() { return real; }
	bool getLifelink() { return lifelink; }
	short getHealthChange() { return healthChange; }
	std::string getWeaponName() { return weaponName; }
	bool getDualWield() { return dualWield; }
	unsigned char getEffectType() { return effectType; }
	bool getSelfOverheal() { return selfOverheal; }
	bool getTargetOverheal() { return targetOverheal; }
	//Displays weapon stats
	void displayStats();
	//Displays name and cost, for use in battle
	void displayName();
	bool checkSelfEffect();
	bool checkTargetEffect();
	void setEffectType();
	bool checkSelfDamage();
	bool checkTargetDamage();
	//Upgrades the item, returns false if it wasn't upgraded
	bool upgradeItem();
};