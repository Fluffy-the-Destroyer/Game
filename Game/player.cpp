#include "player.h"
#include <fstream>
#include "rng.h"
#include <iostream>
#include "inputs.h"
#include "resources.h"
#include "battle.h"
using namespace std;

//Error codes:
// 1: Bad XML, including premature end of file or nonsense values
// 2: Specified blueprint or list not found
// 4: Unable to open blueprint file
// 5: Empty blueprint list
// 6: Trying to access slot which is out of range
// 7: Trying to choose a number from an empty set

extern resource g_manaName;
extern resource g_projName;

short player::flatDamage(short p, short m, short a, bool overheal) {
	if (p > 0) { //Apply physical armour
		p = max(0, p - flatArmour);
		p = static_cast<short>(p * (1 + propArmour));
	}
	if (m > 0) { //Apply magic armour
		m = max(0, m - flatMagicArmour);
		m = static_cast<short>(m * (1 + propMagicArmour));
	}
	long totDamage = p + m + a; //Calculate total damage
	if (totDamage > 0) {
		if (SHRT_MIN + totDamage > health) { //Check for underflow
			health = SHRT_MIN;
		}
		else {
			health = static_cast<short>(health - totDamage);
		}
		if (totDamage > SHRT_MAX) {
			return SHRT_MAX;
		}
		return static_cast<short>(totDamage);
	}
	else if (totDamage < 0) {
		if (overheal) { //May overheal
			if (SHRT_MAX + totDamage < health) { //Check for overflow
				health = SHRT_MAX;
			}
			else {
				health = static_cast<short>(health - totDamage);
			}
			if (totDamage < SHRT_MIN) {
				return SHRT_MIN;
			}
			return static_cast<short>(totDamage);
		}
		else {
			if (health >= maxHealth) { //Already overhealed
				return 0;
			}
			if (health - totDamage > maxHealth || SHRT_MAX + totDamage < health) { //Overflow or exceed max
				totDamage = health;
				totDamage -= maxHealth;
				health = maxHealth;
				if (totDamage < SHRT_MIN) {
					return SHRT_MIN;
				}
				return static_cast<short>(totDamage);
			}
			health = static_cast<short>(health - totDamage);
			if (totDamage < SHRT_MIN) {
				return SHRT_MIN;
			}
			return static_cast<short>(totDamage);
		}
	}
	return 0;
}

void player::propDamage(float d) { //By construction, d will always be between -1 and 1
	if (d > 0) { //Health reduction
		health = static_cast<short>(health * (1 - d)); //Apply the damage
	}
	else if (d < 0) { //Healing, this heals by a proportion of max health
		if (health + static_cast<short>(maxHealth * (-d)) > maxHealth || SHRT_MAX - static_cast<short>(maxHealth * (-d)) < health) { //Health would exceed max or overflow
			health = maxHealth;
		}
		else {
			health += static_cast<short>(maxHealth * (-d));
		}
	}
}

void player::modifyHealth(short h) {
	if (h > 0) {
		if (health >= maxHealth) {
			return;
		}
		if (health + h > maxHealth || SHRT_MAX - h < health) { //Overflow or exceed max
			health = maxHealth;
		}
		else {
			health += h;
		}
	}
	else if (h < 0) {
		if (health < SHRT_MIN - h) {
			health = SHRT_MIN;
		}
		else {
			health += h;
		}
	}
}

void player::modifyMaxHealth(short m) {
	if (m > 0) { //Increase
		if (SHRT_MAX - m < maxHealth) { //Overflow
			maxHealth = SHRT_MAX; //Set to max short value
		}
		else {
			maxHealth += m;
		}
	}
	else if (m < 0) {
		if (maxHealth < SHRT_MIN - m) {
			maxHealth = SHRT_MIN;
		}
		else {
			maxHealth += m;
		}
	}
}

void player::calculateMaxHealth() {
	maxHealth = maxHealthBase;
	modifyMaxHealth(helmet.getMaxHealthModifier());
	modifyMaxHealth(chestplate.getMaxHealthModifier());
	modifyMaxHealth(greaves.getMaxHealthModifier());
	modifyMaxHealth(boots.getMaxHealthModifier());
	if (health > maxHealth) {
		health = maxHealth;
	}
}

void player::modifyProjectiles(short p) {
	if (p > 0) {
		if (SHRT_MAX - p < projectiles) { //Overflow
			projectiles = SHRT_MAX;
		}
		else {
			projectiles += p;
		}
	}
	else if (p < 0) {
		projectiles = max(0, projectiles + p);
	}
}

void player::modifyMana(short m) {
	if (m > 0) {
		if (mana + m > maxMana || SHRT_MAX - m < mana) { //Overflow or exceed max
			mana = maxMana;
		}
		else {
			mana += m;
		}
	}
	else if (m < 0) {
		mana = max(0, mana + m);
	}
}

void player::modifyMaxMana(short m) {
	if (m > 0) { //Increase
		if (SHRT_MAX - m < maxMana) { //Overflow
			maxMana = SHRT_MAX;
		}
		else {
			maxMana += m;
		}
	}
	else if (m < 0) {
		if (maxMana < SHRT_MIN - m) {
			maxMana = SHRT_MIN;
		}
		else {
			maxMana += m;
		}
	}
}

void player::calculateMaxMana() {
	maxMana = maxManaBase;
	modifyMaxMana(helmet.getMaxManaModifier());
	modifyMaxMana(chestplate.getMaxManaModifier());
	modifyMaxMana(greaves.getMaxManaModifier());
	modifyMaxMana(boots.getMaxManaModifier());
	if (mana > maxMana) {
		mana = maxMana;
	}
}

void player::modifyTurnManaRegen(short m) {
	if (m > 0) {
		if (SHRT_MAX - m < turnManaRegen) { //Overflow
			turnManaRegen = SHRT_MAX;
		}
		else {
			turnManaRegen += m;
		}
	}
	else if (m < 0) {
		if (SHRT_MIN + (-m) > turnManaRegen) { //Underflow
			turnManaRegen = SHRT_MIN;
		}
		else {
			turnManaRegen += m;
		}
	}
}

void player::calculateTurnManaRegen() {
	turnManaRegen = turnManaRegenBase;
	modifyTurnManaRegen(helmet.getTurnManaRegenModifier());
	modifyTurnManaRegen(chestplate.getTurnManaRegenModifier());
	modifyTurnManaRegen(greaves.getTurnManaRegenModifier());
	modifyTurnManaRegen(boots.getTurnManaRegenModifier());
}

void player::modifyBattleManaRegen(short b) {
	if (b > 0) {
		if (SHRT_MAX - b < battleManaRegen) { //Overflow
			battleManaRegen = SHRT_MAX;
		}
		else {
			battleManaRegen += b;
		}
	}
	else if (b < 0) {
		if (SHRT_MIN + (-b) > battleManaRegen) { //Underflow
			battleManaRegen = SHRT_MIN;
		}
		else {
			battleManaRegen += b;
		}
	}
}

void player::calculateBattleManaRegen() {
	battleManaRegen = battleManaRegenBase;
	modifyBattleManaRegen(helmet.getBattleManaRegenModifier());
	modifyBattleManaRegen(chestplate.getBattleManaRegenModifier());
	modifyBattleManaRegen(greaves.getBattleManaRegenModifier());
	modifyBattleManaRegen(boots.getBattleManaRegenModifier());
}

bool player::modifyPoison(short p, bool resist) {
	if (p > 255) {
		p = 255;
	}
	if (p > 0) {
		if (resist) {
			if (rng(0.f, 1.f) < poisonResist) {
				return false;
			}
		}
		poison = min(255, poison + p);
	}
	else if (p < 0) {
		poison = max(0, poison + p);
	}
	return true;
}

bool player::modifyBleed(short b, bool resist) {
	if (b > 255) {
		b = 255;
	}
	if (b > 0) {
		if (resist) {
			if (rng(0.f, 1.f) < bleedResist) {
				return false;
			}
		}
		bleed = min(255, bleed + b);
	}
	else if (b < 0) {
		bleed = max(0, bleed + b);
	}
	return true;
}

void player::modifyTempRegen(short r) {
	if (r > 255) {
		r = 255;
	}
	if (r > 0) {
		tempRegen = min(255, tempRegen + r);
	}
	else if (r < 0) {
		tempRegen = max(0, tempRegen + r);
	}
}

void player::modifyConstRegen(short c) {
	if (c > 0) {
		if (SHRT_MAX - c < constRegen) { //Overflow
			constRegen = SHRT_MAX;
		}
		else {
			constRegen += c;
		}
	}
	else if (c < 0) {
		if (SHRT_MIN + (-c) > constRegen) { //Underflow
			constRegen = SHRT_MIN;
		}
		else {
			constRegen += c;
		}
	}
}

void player::calculateConstRegen() {
	constRegen = constRegenBase;
	modifyConstRegen(helmet.getConstRegenModifier());
	modifyConstRegen(chestplate.getConstRegenModifier());
	modifyConstRegen(greaves.getConstRegenModifier());
	modifyConstRegen(boots.getConstRegenModifier());
}

void player::modifyBattleRegen(short b) {
	if (b > 0) {
		if (SHRT_MAX - b < battleRegen) { //Overflow
			battleRegen = SHRT_MAX;
		}
		else {
			battleRegen += b;
		}
	}
	else if (b < 0) {
		if (SHRT_MIN + (-b) > battleRegen) { //Underflow
			battleRegen = SHRT_MIN;
		}
		else {
			battleRegen += b;
		}
	}
}

void player::calculateBattleRegen() {
	battleRegen = battleRegenBase;
	modifyBattleRegen(helmet.getBattleRegenModifier());
	modifyBattleRegen(chestplate.getBattleRegenModifier());
	modifyBattleRegen(greaves.getBattleRegenModifier());
	modifyBattleRegen(boots.getBattleRegenModifier());
}

void player::modifyFlatArmour(short f) {
	if (f > 0) {
		if (SHRT_MAX - f < flatArmour) { //Overflow
			flatArmour = SHRT_MAX;
		}
		else {
			flatArmour += f;
		}
	}
	else if (f < 0) {
		if (flatArmour < SHRT_MIN - f) {
			flatArmour = SHRT_MIN;
		}
		else {
			flatArmour += f;
		}
	}
}

void player::modifyPropArmour(float p) {
	if (p < -1) { //Lower values not allowed
		p = -1;
	}
	propArmour = ((propArmour + 1) * (p + 1)) - 1;
}

void player::modifyFlatMagicArmour(short f) {
	if (f > 0) {
		if (SHRT_MAX - f < flatMagicArmour) { //Overflow
			flatMagicArmour = SHRT_MAX;
		}
		else {
			flatMagicArmour += f;
		}
	}
	else if (f < 0) {
		if (flatMagicArmour < SHRT_MIN - f) {
			flatMagicArmour = SHRT_MIN;
		}
		else {
			flatMagicArmour += f;
		}
	}
}

void player::modifyPropMagicArmour(float p) {
	if (p < -1) {
		p = -1;
	}
	propMagicArmour = ((propMagicArmour + 1) * (p + 1)) - 1;
}

void player::calculateArmour() {
	//Flat armour
	flatArmour = flatArmourBase;
	modifyFlatArmour(helmet.getFlatArmourModifier());
	modifyFlatArmour(chestplate.getFlatArmourModifier());
	modifyFlatArmour(greaves.getFlatArmourModifier());
	modifyFlatArmour(boots.getFlatArmourModifier());
	//Prop armour
	propArmour = propArmourBase;
	modifyPropArmour(helmet.getPropArmourModifier());
	modifyPropArmour(chestplate.getPropArmourModifier());
	modifyPropArmour(greaves.getPropArmourModifier());
	modifyPropArmour(boots.getPropArmourModifier());
	//Flat magic armour
	flatMagicArmour = flatMagicArmourBase;
	modifyFlatMagicArmour(helmet.getFlatMagicArmourModifier());
	modifyFlatMagicArmour(chestplate.getFlatMagicArmourModifier());
	modifyFlatMagicArmour(greaves.getFlatMagicArmourModifier());
	modifyFlatMagicArmour(boots.getFlatMagicArmourModifier());
	//Prop magic armour
	propMagicArmour = propMagicArmourBase;
	modifyPropMagicArmour(helmet.getPropMagicArmourModifier());
	modifyPropMagicArmour(chestplate.getPropMagicArmourModifier());
	modifyPropMagicArmour(greaves.getPropMagicArmourModifier());
	modifyPropMagicArmour(boots.getPropMagicArmourModifier());
}

void player::modifyFlatDamageModifier(short f) {
	if (f > 0) {
		if (SHRT_MAX - f < flatDamageModifier) { //Overflow
			flatDamageModifier = SHRT_MAX;
		}
		else {
			flatDamageModifier += f;
		}
	}
	else if (f < 0) {
		if (SHRT_MIN + (-f) > flatDamageModifier) {
			flatDamageModifier = SHRT_MIN;
		}
		else {
			flatDamageModifier += f;
		}
	}
}

void player::modifyPropDamageModifier(float p) {
	if (p < -1) {
		p = -1;
	}
	propDamageModifier = ((propDamageModifier + 1) * (p + 1)) - 1;
}

void player::calculateDamageModifiers() {
	//Flat
	flatDamageModifier = flatDamageModifierBase;
	modifyFlatDamageModifier(helmet.getFlatDamageModifier());
	modifyFlatDamageModifier(chestplate.getFlatDamageModifier());
	modifyFlatDamageModifier(greaves.getFlatDamageModifier());
	modifyFlatDamageModifier(boots.getFlatDamageModifier());
	//Prop
	propDamageModifier = propDamageModifierBase;
	modifyPropDamageModifier(helmet.getPropDamageModifier());
	modifyPropDamageModifier(chestplate.getPropDamageModifier());
	modifyPropDamageModifier(greaves.getPropDamageModifier());
	modifyPropDamageModifier(boots.getPropDamageModifier());
	//Flat magic
	flatMagicDamageModifier = flatMagicDamageModifierBase;
	modifyFlatMagicDamageModifier(helmet.getFlatMagicDamageModifier());
	modifyFlatMagicDamageModifier(chestplate.getFlatMagicDamageModifier());
	modifyFlatMagicDamageModifier(greaves.getFlatMagicDamageModifier());
	modifyFlatMagicDamageModifier(boots.getFlatMagicDamageModifier());
	//Prop magic
	propMagicDamageModifier = propMagicDamageModifierBase;
	modifyPropMagicDamageModifier(helmet.getPropMagicDamageModifier());
	modifyPropMagicDamageModifier(chestplate.getPropMagicDamageModifier());
	modifyPropMagicDamageModifier(greaves.getPropMagicDamageModifier());
	modifyPropMagicDamageModifier(boots.getPropMagicDamageModifier());
	//Flat AP
	flatArmourPiercingDamageModifier = flatArmourPiercingDamageModifierBase;
	modifyFlatArmourPiercingDamageModifier(helmet.getFlatArmourPiercingDamageModifier());
	modifyFlatArmourPiercingDamageModifier(chestplate.getFlatArmourPiercingDamageModifier());
	modifyFlatArmourPiercingDamageModifier(greaves.getFlatArmourPiercingDamageModifier());
	modifyFlatArmourPiercingDamageModifier(boots.getFlatArmourPiercingDamageModifier());
	//Prop AP
	propArmourPiercingDamageModifier = propArmourPiercingDamageModifierBase;
	modifyPropArmourPiercingDamageModifier(helmet.getPropArmourPiercingDamageModifier());
	modifyPropArmourPiercingDamageModifier(chestplate.getPropArmourPiercingDamageModifier());
	modifyPropArmourPiercingDamageModifier(greaves.getPropArmourPiercingDamageModifier());
	modifyPropArmourPiercingDamageModifier(boots.getPropArmourPiercingDamageModifier());
}

void player::modifyEvadeChance(float e) {
	if (e < -1) {
		e = -1;
	}
	evadeChance *= (e + 1);
}

void player::modifyPoisonResist(float p) {
	if (p < -1) {
		p = -1;
	}
	poisonResist *= (p + 1);
}

void player::calculatePoisonResist() {
	poisonResist = poisonResistBase;
	modifyPoisonResist(helmet.getPoisonResistModifier());
	modifyPoisonResist(chestplate.getPoisonResistModifier());
	modifyPoisonResist(greaves.getPoisonResistModifier());
	modifyPoisonResist(boots.getPoisonResistModifier());
}

void player::modifyBleedResist(float b) {
	if (b < -1) {
		b = -1;
	}
	bleedResist *= (b + 1);
}

void player::calculateBleedResist() {
	bleedResist = bleedResistBase;
	modifyBleedResist(helmet.getBleedResistModifier());
	modifyBleedResist(chestplate.getBleedResistModifier());
	modifyBleedResist(greaves.getBleedResistModifier());
	modifyBleedResist(boots.getBleedResistModifier());
}

void player::calculateEvadeChance() {
	evadeChance = evadeChanceBase;
	modifyEvadeChance(helmet.getEvadeChanceModifier());
	modifyEvadeChance(chestplate.getEvadeChanceModifier());
	modifyEvadeChance(greaves.getEvadeChanceModifier());
	modifyEvadeChance(boots.getEvadeChanceModifier());
}

void player::calculateModifiers() {
	calculateMaxHealth();
	calculateMaxMana();
	calculateTurnManaRegen();
	calculateBattleManaRegen();
	calculateConstRegen();
	calculateBattleRegen();
	calculateArmour();
	calculateDamageModifiers();
	calculateEvadeChance();
	calculatePoisonResist();
	calculateBleedResist();
	calculateCounterAttackChance();
	calculateBonusActions();
}

void player::modifyFlatMagicDamageModifier(short f) {
	if (f > 0) {
		if (SHRT_MAX - f < flatMagicDamageModifier) { //Overflow
			flatMagicDamageModifier = SHRT_MAX;
		}
		else {
			flatMagicDamageModifier += f;
		}
	}
	else if (f < 0) {
		if (SHRT_MIN + (-f) > flatMagicDamageModifier) {
			flatMagicDamageModifier = SHRT_MIN;
		}
		else {
			flatMagicDamageModifier += f;
		}
	}
}

void player::modifyPropMagicDamageModifier(float p) {
	if (p < -1) {
		p = -1;
	}
	propMagicDamageModifier = ((propMagicDamageModifier + 1) * (p + 1)) - 1;
}

void player::modifyFlatArmourPiercingDamageModifier(short f) {
	if (f > 0) {
		if (SHRT_MAX - f < flatArmourPiercingDamageModifier) { //Overflow
			flatArmourPiercingDamageModifier = SHRT_MAX;
		}
		else {
			flatArmourPiercingDamageModifier += f;
		}
	}
	else if (f < 0) {
		if (SHRT_MIN + (-f) > flatArmourPiercingDamageModifier) {
			flatArmourPiercingDamageModifier = SHRT_MIN;
		}
		else {
			flatArmourPiercingDamageModifier += f;
		}
	}
}

void player::modifyPropArmourPiercingDamageModifier(float p) {
	if (p < -1) {
		p = -1;
	}
	propArmourPiercingDamageModifier = ((propArmourPiercingDamageModifier + 1) * (p + 1)) - 1;
}

void player::equip(weapon* w) {
	if (weapons.size() == 0) {
		cout << "Cannot equip weapon, no weapon slots\n";
		return;
	}
	cout << "Currently equipped weapons:\n";
	for (short i = 0; i < weapons.size(); i++) {
		cout << i + 1 << ": " << getWeapon(static_cast<unsigned char>(i))->getName() << '\n';
	}
	while (true) {
		cout << "Enter the number of the slot in which to equip the weapon.\nTo discard the weapon, enter 0.\n";
		try {
			unsigned char slot = userChoice(0, static_cast<int>(weapons.size()));
			if (slot == 0) {
				return;
			}
			slot--;
			cout << "Currently equipped:\n";
			getWeapon(slot)->displayStats();
			cout << "New weapon:\n";
			w->displayStats();
			cout << "To equip the weapon in this slot, enter 1.\nTo choose a different slot, enter 2.\n";
			if (userChoice(1, 2) == 1) {
				weapons[slot] = *w;
				return;
			}
		}
		catch (int err) {
			switch (err) {
			case 7:
				cout << "An internal error occurred\n";
				break;
			}
		}
	}
}

void player::equip(spell* s) {
	if (spells.size() == 0) {
		cout << "Cannot equip spell, no spell slots\n";
		return;
	}
	cout << "Currently equipped spells:\n";
	for (short i = 0; i < spells.size(); i++) {
		cout << i + 1 << ": " << getSpell(static_cast<unsigned char>(i))->getName() << '\n';
	}
	while (true) {
		cout << "Enter the number of the slot in which to equip the spell.\nTo discard the spell, enter 0.\n";
		try {
			unsigned char slot = userChoice(0, static_cast<int>(spells.size()));
			if (slot == 0) {
				return;
			}
			slot--;
			cout << "Currently equipped:\n";
			getSpell(slot)->displayStats();
			cout << "New spell:\n";
			s->displayStats();
			cout << "To equip the spell in this slot, enter 1.\nTo choose a different slot, enter 2.\n";
			if (userChoice(1, 2) == 1) {
				spells[slot] = *s;
				return;
			}
		}
		catch (int err) {
			switch (err) {
			case 7:
				cout << "An internal error occurred\n";
				break;
			}
		}
	}
}

void player::equip(armourHead* h) {
	cout << "Currently equipped:\n";
	helmet.displayStats();
	cout << "New armour:\n";
	h->displayStats();
	cout << "To equip the new armour, enter 1.\nTo discard it, enter 2.\n";
	if (userChoice(1, 2) == 1) {
		helmet = *h;
	}
}

void player::equip(armourTorso* c) {
	cout << "Currently equipped:\n";
	chestplate.displayStats();
	cout << "New armour:\n";
	c->displayStats();
	cout << "To equip the new armour, enter 1.\nTo discard it, enter 2.\n";
	if (userChoice(1, 2) == 1) {
		chestplate = *c;
	}
}

void player::equip(armourLegs* g) {
	cout << "Currently equipped:\n";
	greaves.displayStats();
	cout << "New armour:\n";
	g->displayStats();
	cout << "To equip the new armour, enter 1.\nTo discard it, enter 2.\n";
	if (userChoice(1, 2) == 1) {
		greaves = *g;
	}
}

void player::equip(armourFeet* b) {
	cout << "Currently equipped:\n";
	boots.displayStats();
	cout << "New armour:\n";
	b->displayStats();
	cout << "To equip the new armour, enter 1.\nTo discard it, enter 2.\n";
	if (userChoice(1, 2) == 1) {
		boots = *b;
	}
}

weapon* player::getWeapon(unsigned char i) {
	if (i >= weapons.size()) {
		throw 6;
	}
	return &(weapons[i]);
}

spell* player::getSpell(unsigned char i) {
	if (i >= spells.size()) {
		throw 6;
	}
	return &(spells[i]);
}

void player::modifyCounterAttackChance(float c) {
	if (c < -1) {
		c = -1;
	}
	counterAttackChance *= (c + 1);
}

void player::calculateCounterAttackChance() {
	counterAttackChance = counterAttackChanceBase;
	modifyCounterAttackChance(helmet.getCounterAttackChanceModifier());
	modifyCounterAttackChance(chestplate.getCounterAttackChanceModifier());
	modifyCounterAttackChance(greaves.getCounterAttackChanceModifier());
	modifyCounterAttackChance(boots.getCounterAttackChanceModifier());
}

void player::modifyBonusActions(short b) {
	if (b > 255) {
		bonusActions = 127;
	}
	else if (b < -255) {
		bonusActions = -127;
	}
	else if (bonusActions + b > 127) {
		bonusActions = 127;
	}
	else if (bonusActions + b < -127) {
		bonusActions = -127;
	}
	else {
		bonusActions += b;
	}
}

void player::calculateBonusActions() {
	bonusActions = bonusActionsBase;
	modifyBonusActions(helmet.getBonusActionsModifier());
	modifyBonusActions(chestplate.getBonusActionsModifier());
	modifyBonusActions(greaves.getBonusActionsModifier());
	modifyBonusActions(boots.getBonusActionsModifier());
}

void player::loadClass(string playerClass, bool custom) {
	ifstream classBlueprints;
	string buffer = "";
	string valBuffer;
	//Open blueprint file
	try {
		if (custom) {
			classBlueprints.open("custom\\classBlueprints.xml");
		}
		else {
			classBlueprints.open("data\\classBlueprints.xml");
		}
		if (!classBlueprints.is_open()) {
			throw 4;
		}
		string blueprintName = "classBlueprintList name=\"" + playerClass + '\"';
		//Check for a list
		{
			bool noList = false; //Tracking if we found a list
			streampos filePos = 0; //Position in file
			short listCount = 0; //NUmber of items in a list, also tracking which item we have chosen
			while (buffer != blueprintName) {
				buffer = getTag(&classBlueprints);
				ignoreLine(&classBlueprints);
				if (classBlueprints.eof()) {
					classBlueprints.clear();
					noList = true;
					break;
				}
			}
			if (!noList) {
				filePos = classBlueprints.tellg();
				do {
					if (classBlueprints.eof()) {
						throw 1;
					}
					listCount++;
					buffer = getTag(&classBlueprints);
					ignoreLine(&classBlueprints);
				} while (buffer != "/classBlueprintList");
				classBlueprints.clear();
				if (listCount == 0) {
					throw 5;
				}
				listCount = rng(1, listCount);
				classBlueprints.seekg(filePos);
				for (int i = 1; i < listCount; i++) {
					ignoreLine(&classBlueprints);
				}
				if (getTag(&classBlueprints) != "name") {
					throw 1;
				}
				getline(classBlueprints, playerClass, '<');
				getline(classBlueprints, buffer, '>');
				if (buffer != "/name") {
					throw 1;
				}
			}
			classBlueprints.seekg(0);
			buffer = "";
		}
		blueprintName = "classBlueprint name=\"" + playerClass + '\"';
		//Find and read blueprint
		while (buffer != blueprintName) {
			buffer = getTag(&classBlueprints);
			ignoreLine(&classBlueprints);
			if (classBlueprints.eof()) {
				throw 2;
			}
		}
		buffer = getTag(&classBlueprints);
		while (buffer != "/classBlueprint") {
			if (classBlueprints.eof()) {
				throw 1;
			}
			if (buffer == "maxHealth") {
				getline(classBlueprints, valBuffer, '<');
				maxHealthBase = numFromString(&valBuffer);
			}
			else if (buffer == "maxMana") {
				getline(classBlueprints, valBuffer, '<');
				maxManaBase = numFromString(&valBuffer);
			}
			else if (buffer == "turnManaRegen") {
				getline(classBlueprints, valBuffer, '<');
				turnManaRegenBase = numFromString(&valBuffer);
			}
			else if (buffer == "battleManaRegen") {
				getline(classBlueprints, valBuffer, '<');
				battleManaRegenBase = numFromString(&valBuffer);
			}
			else if (buffer == "poisonResist") {
				getline(classBlueprints, valBuffer, '<');
				poisonResistBase = floatFromString(&valBuffer);
				if (poisonResistBase < 0) {
					poisonResistBase = 0;
				}
			}
			else if (buffer == "bleedResist") {
				getline(classBlueprints, valBuffer, '<');
				bleedResistBase = floatFromString(&valBuffer);
				if (bleedResistBase < 0) {
					bleedResistBase = 0;
				}
			}
			else if (buffer == "constRegen") {
				getline(classBlueprints, valBuffer, '<');
				constRegenBase = numFromString(&valBuffer);
			}
			else if (buffer == "battleRegen") {
				getline(classBlueprints, valBuffer, '<');
				battleRegenBase = numFromString(&valBuffer);
			}
			else if (buffer.substr(0, 15) == "weapons count=\"") { //It's the weapons tag, count is how many slots there are, which varies, so only checking the beginning of the tag
				buffer.erase(0, 15); //Get rid of the stuff that has been checked
				weapons.resize(static_cast<unsigned char>(numFromString(&buffer))); //If the number is negative or bigger than 255, it will overflow or underflow, but the documentation will say it must be in this range so it is an error by whoever made the class
				if (buffer.substr(0, 15) != "\" projectiles=\"") { //This should be next
					throw 1;
				}
				buffer.erase(0, 15);
				projectiles = static_cast<short>(numFromString(&buffer));
				if (buffer != "\"") {
					throw 1;
				}
				ignoreLine(&classBlueprints); //Go to next line
				for (int i = 0; i < weapons.size(); i++) {
					buffer = getTag(&classBlueprints);
					if (buffer == "weapon") { //It's a weapon
						getline(classBlueprints, buffer, '<'); //Get the blueprint of the weapon (which could be a list)
						if (classBlueprints.eof()) {
							throw 1;
						}
						weapons[i].loadFromFile(buffer); //Load that weapon from files
						getline(classBlueprints, buffer, '>');
						if (buffer != "/weapon") {
							throw 1;
						}
						ignoreLine(&classBlueprints);
					}
					else if (buffer == "/weapons") { //Reached end of list of starting weapons, this would occur if the number of listed weapons is lower than the number of slots, which is fine, the player starts with excess slots empty
						ignoreLine(&classBlueprints);
						for (int j = i; j < weapons.size(); j++) { //Iterate over the current slot and all later ones
							weapons[j].loadFromFile("EMPTY"); //Load empty slots
						}
						break;
					}
					else { //Anything else is bad XML
						throw 1;
					}
				}
				while (buffer != "/weapons") { //Not yet reached end of weapon list, if the list ended early, this will already be true. Otherwise, we skip over extra declared weapons which exceed the number of slots
					buffer = getTag(&classBlueprints);
					ignoreLine(&classBlueprints);
				}
				if (!classBlueprints) {
					throw 1;
				}
				buffer = getTag(&classBlueprints);
				continue;
			}
			else if (buffer.substr(0, 14) == "spells count=\"") { //Similar to weapon setting
				buffer.erase(0, 14);
				spells.resize(static_cast<unsigned char>(numFromString(&buffer)));
				if (buffer != "\"") {
					throw 1;
				}
				ignoreLine(&classBlueprints);
				for (int i = 0; i < spells.size(); i++) {
					buffer = getTag(&classBlueprints);
					if (buffer == "spell") {
						getline(classBlueprints, buffer, '<');
						if (classBlueprints.eof()) {
							throw 1;
						}
						spells[i].loadFromFile(buffer);
						getline(classBlueprints, buffer, '>');
						if (buffer != "/spell") {
							throw 1;
						}
						ignoreLine(&classBlueprints);
					}
					else if (buffer == "/spells") {
						ignoreLine(&classBlueprints);
						for (int j = i; j < spells.size(); j++) {
							spells[j].loadFromFile("EMPTY");
						}
						break;
					}
					else {
						throw 1;
					}
				}
				while (buffer != "/spells") {
					buffer = getTag(&classBlueprints);
					ignoreLine(&classBlueprints);
				}
				if (!classBlueprints) {
					throw 1;
				}
				buffer = getTag(&classBlueprints);
				continue;
			}
			else if (buffer == "flatArmour") {
				getline(classBlueprints, valBuffer, '<');
				flatArmourBase = numFromString(&valBuffer);
			}
			else if (buffer == "propArmour") {
				getline(classBlueprints, valBuffer, '<');
				propArmourBase = floatFromString(&valBuffer);
				if (propArmourBase < -1) {
					propArmourBase = -1;
				}
			}
			else if (buffer == "flatMagicArmour") {
				getline(classBlueprints, valBuffer, '<');
				flatMagicArmourBase = numFromString(&valBuffer);
			}
			else if (buffer == "propMagicArmour") {
				getline(classBlueprints, valBuffer, '<');
				propMagicArmourBase = floatFromString(&valBuffer);
				if (propMagicArmourBase < -1) {
					propMagicArmourBase = -1;
				}
			}
			else if (buffer == "helmet") {
				getline(classBlueprints, buffer, '<');
				if (classBlueprints.eof()) {
					throw 1;
				}
				helmet.loadFromFile(buffer);
				getline(classBlueprints, buffer, '>');
				if (buffer != "/helmet") {
					throw 1;
				}
				ignoreLine(&classBlueprints);
				buffer = getTag(&classBlueprints);
				continue;
			}
			else if (buffer == "chestplate") {
				getline(classBlueprints, buffer, '<');
				if (classBlueprints.eof()) {
					throw 1;
				}
				chestplate.loadFromFile(buffer);
				getline(classBlueprints, buffer, '>');
				if (buffer != "/chestplate") {
					throw 1;
				}
				ignoreLine(&classBlueprints);
				buffer = getTag(&classBlueprints);
				continue;
			}
			else if (buffer == "greaves") {
				getline(classBlueprints, buffer, '<');
				if (classBlueprints.eof()) {
					throw 1;
				}
				greaves.loadFromFile(buffer);
				getline(classBlueprints, buffer, '>');
				if (buffer != "/greaves") {
					throw 1;
				}
				ignoreLine(&classBlueprints);
				buffer = getTag(&classBlueprints);
				continue;
			}
			else if (buffer == "boots") {
				getline(classBlueprints, buffer, '<');
				if (classBlueprints.eof()) {
					throw 1;
				}
				boots.loadFromFile(buffer);
				getline(classBlueprints, buffer, '>');
				if (buffer != "/boots") {
					throw 1;
				}
				ignoreLine(&classBlueprints);
				buffer = getTag(&classBlueprints);
				continue;
			}
			else if (buffer == "flatDamageModifier") {
				getline(classBlueprints, valBuffer, '<');
				flatDamageModifierBase = numFromString(&valBuffer);
			}
			else if (buffer == "propDamageModifier") {
				getline(classBlueprints, valBuffer, '<');
				propDamageModifierBase = floatFromString(&valBuffer);
				if (propDamageModifierBase < -1) {
					propDamageModifierBase = -1;
				}
			}
			else if (buffer == "flatMagicDamageModifier") {
				getline(classBlueprints, valBuffer, '<');
				flatMagicDamageModifierBase = numFromString(&valBuffer);
			}
			else if (buffer == "propMagicDamageModifier") {
				getline(classBlueprints, valBuffer, '<');
				propMagicDamageModifierBase = floatFromString(&valBuffer);
				if (propMagicDamageModifierBase < -1) {
					propMagicDamageModifierBase = -1;
				}
			}
			else if (buffer == "flatArmourPiercingDamageModifier") {
				getline(classBlueprints, valBuffer, '<');
				flatArmourPiercingDamageModifierBase = numFromString(&valBuffer);
			}
			else if (buffer == "propArmourPiercingDamageModifier") {
				getline(classBlueprints, valBuffer, '<');
				propArmourPiercingDamageModifierBase = floatFromString(&valBuffer);
				if (propArmourPiercingDamageModifierBase < -1) {
					propArmourPiercingDamageModifierBase = -1;
				}
			}
			else if (buffer == "evadeChance") {
				getline(classBlueprints, valBuffer, '<');
				evadeChanceBase = floatFromString(&valBuffer);
				if (evadeChanceBase < 0) {
					evadeChanceBase = 0;
				}
			}
			else {
				throw 1;
			}
			classBlueprints.seekg(-1, ios_base::cur);
			if (getTag(&classBlueprints) != '/' + buffer) {
				throw 1;
			}
			ignoreLine(&classBlueprints);
			if (!classBlueprints) {
				throw 1;
			}
			buffer = getTag(&classBlueprints);
		}
		classBlueprints.close();
		//Set all the other stats
		calculateModifiers();
		fullHeal();
		fullMana();
	}
	catch (int err) {
		classBlueprints.close();
		switch (err) {
		case 2:
			if (custom) {
				try {
					loadClass(playerClass, false);
					return;
				}
				catch (int err2) {
					throw err2;
				}
			}
		case 4:
			if (custom) {
				try {
					loadClass(playerClass, false);
					return;
				}
				catch (int err2) {
					throw err2;
				}
			}
		default:
			throw err;
		}
	}
}

void player::displayStats() {
	//Health
	cout << "Health: " << health << '/' << maxHealth << '\n';
	if (constRegen > 0) {
		cout << '+' << constRegen << " health per turn\n";
	}
	else if (constRegen < 0) {
		cout << constRegen << " health per turn\n";
	}
	if (battleRegen > 0) {
		cout << '+' << battleRegen << " health at end of battle\n";
	}
	else if (battleRegen < 0) {
		cout << battleRegen << " health at end of battle\n";
	}
	//Poison, bleed , regen
	if (poison > 0) {
		cout << "Current poison: " << +poison << '\n';
	}
	if (bleed > 0) {
		cout << "Current bleed: " << +bleed << '\n';
	}
	if (tempRegen > 0) {
		cout << "Current regeneration: " << +tempRegen << '\n';
	}
	//Mana
	cout << g_manaName.Plural() << ": " << mana << '/' << maxMana << '\n';
	if (turnManaRegen == 1) {
		cout << "+1 " << g_manaName.singular() << " per turn\n";
	}
	else if (turnManaRegen > 0) {
		cout << '+' << turnManaRegen << ' ' << g_manaName.plural() << " per turn\n";
	}
	else if (turnManaRegen == -1) {
		cout << "-1 " << g_manaName.singular() << " per turn\n";
	}
	else if (turnManaRegen < 0) {
		cout << turnManaRegen << ' ' << g_manaName.plural() << " per turn\n";
	}
	if (battleManaRegen == 1) {
		cout << "+1 " << g_manaName.singular() << " at end of battle\n";
	}
	else if (battleManaRegen > 0) {
		cout << '+' << battleManaRegen << ' ' << g_manaName.plural() << " at end of battle\n";
	}
	else if (battleManaRegen == -1) {
		cout << "-1 " << g_manaName.singular() << " at end of battle\n";
	}
	else if (battleManaRegen < 0) {
		cout << battleManaRegen << ' ' << g_manaName.plural() << " at end of battle\n";
	}
	//Projectiles. This is in the inventory for now
	cout << g_projName.Plural() << ": " << projectiles << '\n';
	//Poison resist
	cout << "Poison resistance: " << 100 * poisonResist << "%\n";
	//Bleed resist
	cout << "Bleed resistance: " << 100 * bleedResist << "%\n";
	//Armour
	cout << "Physical armour rating: " << flatArmour << '\n';
	if (propArmour > 0) {
		cout << "Incoming physical damage increased by " << 100 * propArmour << "%\n";
	}
	else if (propArmour < 0) {
		cout << "Incoming physical damage reduced by " << -100 * propArmour << "%\n";
	}
	cout << "Magic armour rating: " << flatMagicArmour << '\n';
	if (propMagicArmour > 0) {
		cout << "Incoming magic damage increased by " << 100 * propMagicArmour << "%\n";
	}
	else if (propMagicArmour < 0) {
		cout << "Incoming magic damage reduced by " << -100 * propMagicArmour << "%\n";
	}
	//Evade
	cout << "Evade chance: " << 100 * evadeChance << "%\n";
	//Counter attack chance
	cout << "Counter attack chance: " << 100 * counterAttackChance << "%\n";
	//Bonus actions
	cout << "Bonus actions: " << +bonusActions << '\n'; //Might change later to display effective bonus actions
	//Damage modifiers
	if (flatDamageModifier > 0) {
		cout << "Physical damage increased by " << flatDamageModifier << '\n';
	}
	else if (flatDamageModifier < 0) {
		cout << "Physical damage reduced by " << -flatDamageModifier << '\n';
	}
	if (propDamageModifier > 0) {
		cout << "Physical damage increased by " << 100 * propDamageModifier << "%\n";
	}
	else if (propDamageModifier < 0) {
		cout << "Physical damage reduced by " << -100 * propDamageModifier << "%\n";
	}
	if (flatMagicDamageModifier > 0) {
		cout << "Magic damage increased by " << flatMagicDamageModifier << '\n';
	}
	else if (flatMagicDamageModifier < 0) {
		cout << "Magic damage reduced by " << -flatMagicDamageModifier << '\n';
	}
	if (propMagicDamageModifier > 0) {
		cout << "Magic damage increased by " << 100 * propMagicDamageModifier << "%\n";
	}
	else if (propMagicDamageModifier < 0) {
		cout << "Magic damage reduced by " << -100 * propMagicDamageModifier << "%\n";
	}
	if (flatArmourPiercingDamageModifier > 0) {
		cout << "Armour piercing damage increased by " << flatArmourPiercingDamageModifier << '\n';
	}
	else if (flatArmourPiercingDamageModifier < 0) {
		cout << "Armour piercing damage reduced by " << -flatArmourPiercingDamageModifier << '\n';
	}
	if (propArmourPiercingDamageModifier > 0) {
		cout << "Armour piercing damage increased by " << 100 * propArmourPiercingDamageModifier << "%\n";
	}
	else if (propArmourPiercingDamageModifier < 0) {
		cout << "Armour piercing damage reduced by " << -100 * propArmourPiercingDamageModifier << "%\n";
	}
}

void player::showInventory() {
	while (true) {
		cout << "Armour:\n";
		cout << "Head: " << helmet.getName() << '\n';
		cout << "Torso: " << chestplate.getName() << '\n';
		cout << "Legs: " << greaves.getName() << '\n';
		cout << "Feet: " << boots.getName() << '\n';
		cout << "Weapons:\n";
		for (short i = 0; i < weapons.size(); i++) {
			cout << i + 1 << ": " << getWeapon(static_cast<unsigned char>(i))->getName() << '\n';
		}
		cout << "Spells:\n";
		for (short i = 0; i < spells.size(); i++) {
			cout << i + 1 << ": " << getSpell(static_cast<unsigned char>(i))->getName() << '\n';
		}
		cout << "To view armour stats, enter 1.\nTo view weapon stats, enter 2.\nTo view spell stats, enter 3.\nTo view player stats, enter 4.\n";
		switch (userChoice(1, 4)) {
		case 1:
			cout << "To view head armour stats, enter 1.\nTo view torso armour stats, enter 2.\nTo view leg armour stats, enter 3.\nTo view foot armour stats, enter 4.\n";
			switch (userChoice(1, 4)) {
			case 1:
				helmet.displayStats();
				break;
			case 2:
				chestplate.displayStats();
				break;
			case 3:
				greaves.displayStats();
				break;
			case 4:
				boots.displayStats();
				break;
			}
			break;
		case 2:
			if (weapons.size() == 0) {
				cout << "No weapon slots\n";
				break;
			}
			cout << "Enter the number of the weapon slot you wish to view\n";
			try {
				getWeapon(static_cast<unsigned char>(userChoice(1, static_cast<int>(weapons.size())) - 1))->displayStats(); //Using getWeapon as it has protection against attempts to access outside the weapons vector
			}
			catch (int err) {
				switch (err) {
				case 6:
					cout << "An internal error occurred, specified weapon slot does not exist\n";
					break;
				case 7:
					cout << "An internal error occurred, no weapon slots\n";
					break;
				}
			}
			break;
		case 3:
			if (spells.size() == 0) {
				cout << "No spell slots\n";
				break;
			}
			cout << "Enter the number of the spell slot you wish to view\n";
			try {
				getSpell(static_cast<unsigned char>(userChoice(1, static_cast<int>(spells.size())) - 1))->displayStats();
			}
			catch (int err) {
				switch (err) {
				case 7:
					cout << "An internal error occurred, no spell slots\n";
					break;
				}
			}
			break;
		case 4:
			displayStats();
			break;
		}
		cout << "To view different stats, enter 1.\nTo close the inventory, enter 2.\n";
		if (userChoice(1, 2) == 2) {
			break;
		}
	}
}

void player::turnStart() {
	modifyHealth(-(POISON_MULTIPLIER * poison + BLEED_MULTIPLIER * bleed));
	modifyHealth(constRegen + REGEN_MULTIPLIER * tempRegen);
	if (health > maxHealth) {
		health = max(static_cast<int>(maxHealth), health - PLAYER_OVERHEAL_DECAY);
	}
	modifyMana(turnManaRegen);
	if (mana > maxMana) {
		mana = max(static_cast<int>(maxMana), mana - MANA_DECAY);
	}
	if (poison > 0) {
		poison--;
	}
	if (bleed > 0) {
		bleed--;
	}
	if (tempRegen > 0) {
		tempRegen--;
	}
	for (unsigned char i = 0; i < spells.size(); i++) {
		spells[i].decCooldown();
	}
	if (bonusActions < 0) {
		currentBonusActions = 0;
	}
	else {
		currentBonusActions = bonusActions;
	}
}

void player::decBonusActions() {
	if (currentBonusActions > 0) {
		currentBonusActions--;
	}
}

void player::reset() {
	modifyHealth(battleRegen);
	modifyMana(battleManaRegen);
	for (unsigned char i = 0; i < spells.size(); i++) {
		spells[i].resetCooldown();
	}
	calculateModifiers();
	cureBleed();
	curePoison();
	removeRegen();
}

unsigned char player::chooseAction(unsigned char* slot1, unsigned char* slot2, string enemyName, const unsigned char timing, string itemName1, string itemName2) {
	vector<short> choices(1, 0); //For holding a list of possible choices
	const short currentHealth = health, currentMana = mana, currentProjectiles = projectiles;
	short choiceCounter = 0;
	switch (timing) { //Check if there are no possible actions and return 0 if so
	case 0:
		break;
	case 1:
	case 2:
	case 4:
		if (currentBonusActions <= 0) {
			return 0;
		}
		for (unsigned char i = 0; i < spells.size(); i++) {
			if (spells[i].getReal() && spells[i].getHitCount() > 0 && spells[i].getTiming() != 0) {
				choiceCounter++;
			}
		}
		if (choiceCounter == 0) {
			return 0;
		}
		break;
	case 3:
		if (currentBonusActions <= 0) {
			return 0;
		}
		for (unsigned char i = 0; i < weapons.size(); i++) {
			if (weapons[i].getReal() && weapons[i].getCounterHits() > 0) {
				choiceCounter++;
			}
		}
		for (unsigned char i = 0; i < spells.size(); i++) {
			if (spells[i].getReal() && spells[i].getCounterHits() > 0) {
				choiceCounter++;
			}
		}
		if (choiceCounter == 0) {
			return 0;
		}
		break;
	}
	while (true) {
		cout << "Health: " << health << '/' << maxHealth << ' ' << g_manaName.Plural() << ": " << mana << '/' << maxMana << ' ' << g_projName.Plural() << ": " << projectiles << ' ' << "Bonus Actions: " << +currentBonusActions << ' ';
		if (poison > 0) {
			cout << "Poison: " << +poison << ' ';
		}
		if (bleed > 0) {
			cout << "Bleed: " << +bleed << ' ';
		}
		if (tempRegen > 0) {
			cout << "Regeneration: " << +tempRegen;
		}
		cout << '\n';
		switch (timing) {
		case 0:
			cout << "To view your inventory, enter 0.\nTo attack with a weapon, enter 1.\nTo cast a spell, enter 2.\nTo do nothing, enter -1.\n";
			switch (userChoice(-1, 2)) {
			case -1:
				return 0;
			case 0:
				showInventory();
				break;
			case 1:
				if (weapons.size() == 0) {
					cout << "No weapon slots!\n";
					break;
				}
				cout << "Enter the number of the weapon you wish to attack with.\nTo go back, enter 0.\n";
				for (unsigned char i = 0; i < weapons.size(); i++) {
					cout << i + 1 << ": ";
					weapons[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<unsigned char>(userChoice(0, static_cast<int>(weapons.size())));
				if (*slot1 == 0) {
					break;
				}
				(*slot1)--;
				if (!weapons[*slot1].getReal()) {
					cout << "Cannot attack with empty slot!\n";
					break;
				}
				if (weapons[*slot1].getHitCount() == 0) {
					cout << "Selected weapon cannot be used at this time!\n";
					break;
				}
				if (weapons[*slot1].getHealthChange() < -health) { //Check can afford
					if (weapons[*slot1].getManaChange() < -mana) {
						if (weapons[*slot1].getProjectileChange() < -projectiles) {
							cout << "Not enough health, " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
							break;
						}
						cout << "Not enough health or " << g_manaName.plural() << "!\n";
						break;
					}
					if (weapons[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough health or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough health!\n";
					break;
				}
				if (weapons[*slot1].getManaChange() < -mana) {
					if (weapons[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough " << g_manaName.plural() << "!\n";
					break;
				}
				if (weapons[*slot1].getProjectileChange() < -projectiles) {
					cout << "Not enough " << g_projName.plural() << "!\n";
					break;
				}
				if (!weapons[*slot1].getDualWield() || currentBonusActions <= 0) { //If cannot dual wield, done with selection
					return 1;
				}
				//Can dual wield, choose another weapon
				//Find possible weapons
				choices.resize(1);
				for (unsigned char i = 0; i < weapons.size(); i++) {
					if (i == *slot1) { //Can't dual wield the same weapon twice
						continue;
					}
					if (weapons[i].getDualWield() && weapons[i].getReal() && weapons[i].getHitCount() != 0) { //If the weapon can dual wield, add it to list
						choices.push_back(i + 1);
					}
				}
				if (choices.size() == 1) { //Found nothing
					return 1;
				}
				modifyHealth(weapons[*slot1].getHealthChange());
				modifyMana(weapons[*slot1].getManaChange());
				modifyProjectiles(weapons[*slot1].getProjectileChange());
				while (true) {
					cout << "The selected weapon can be dual wielded\n";
					cout << "(After applying costs for selected weapon)\n";
					cout << "Health: " << health << '/' << maxHealth << ' ' << g_manaName.Plural() << ": " << mana << '/' << maxMana << ' ' << g_projName.Plural() << ": " << projectiles << ' ' << "Bonus Actions: " << +currentBonusActions << ' ';
					if (poison > 0) {
						cout << "Poison: " << +poison << ' ';
					}
					if (bleed > 0) {
						cout << "Bleed: " << +bleed << ' ';
					}
					if (tempRegen > 0) {
						cout << "Regeneration: " << +tempRegen;
					}
					cout << '\n';
					cout << "To view your inventory, enter 0.\nTo select a second weapon, enter 1.\nTo proceed without dual wielding, enter -1.\n";
					switch (userChoice(-1, 1)) {
					case -1:
						health = currentHealth;
						mana = currentMana;
						projectiles = currentProjectiles;
						return 1;
					case 0:
						showInventory();
						break;
					case 1:
						cout << "Enter the number of the additional weapon you wish to attack with.\nTo go back, enter 0.\n";
						for (short i = 1; i < choices.size(); i++) {
							cout << choices[i] << ": ";
							weapons[static_cast<unsigned char>(choices[i] - 1)].displayName();
						}
						*slot2 = static_cast<unsigned char>(userChoice(choices));
						if (*slot2 == 0) {
							break;
						}
						(*slot2)--;
						if (!weapons[*slot2].getReal()) {
							cout << "Cannot attack with empty slot!\n";
							break;
						}
						if (weapons[*slot2].getHitCount() == 0) {
							cout << "Selected weapon cnnot be used at this time!\n";
							break;
						}
						if (weapons[*slot2].getHealthChange() < -health) { //Check can afford
							if (weapons[*slot2].getManaChange() < -mana) {
								if (weapons[*slot2].getProjectileChange() < -projectiles) {
									cout << "Not enough health, " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
									break;
								}
								cout << "Not enough health or " << g_manaName.plural() << "!\n";
								break;
							}
							if (weapons[*slot2].getProjectileChange() < -projectiles) {
								cout << "Not enough health or " << g_projName.plural() << "!\n";
								break;
							}
							cout << "Not enough health!\n";
							break;
						}
						if (weapons[*slot2].getManaChange() < -mana) {
							if (weapons[*slot2].getProjectileChange() < -projectiles) {
								cout << "Not enough " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
								break;
							}
							cout << "Not enough " << g_manaName.plural() << "!\n";
							break;
						}
						if (weapons[*slot2].getProjectileChange() < -projectiles) {
							cout << "Not enough " << g_projName.plural() << "!\n";
							break;
						}
						//Can afford, so done selecting
						health = currentHealth;
						mana = currentMana;
						projectiles = currentProjectiles;
						currentBonusActions--;
						return 3;
					}
				}
				break;
			case 2: //Casting a spell
				if (spells.size() == 0) {
					cout << "No spell slots!\n";
					break;
				}
				cout << "Enter the number of the spell you wish to cast.\nTo go back, enter 0.\n";
				for (unsigned char i = 0; i < spells.size(); i++) {
					cout << i + 1 << ": ";
					spells[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<unsigned char>(userChoice(0, static_cast<int>(spells.size())));
				if (*slot1 == 0) {
					break;
				}
				(*slot1)--;
				if (!spells[*slot1].getReal()) {
					cout << "Selected spell slot is empty!\n";
					break;
				}
				if (spells[*slot1].getHitCount() == 0 || spells[*slot1].getTiming() == 2) {
					cout << "Selected spell cannot be cast at this time!\n";
					break;
				}
				if (spells[*slot1].getCurrentCooldown() > 0) {
					cout << "Selected spell is on cooldown!\n";
					break;
				}
				if (spells[*slot1].getHealthChange() < -health) { //Check can afford
					if (spells[*slot1].getManaChange() < -mana) {
						if (spells[*slot1].getProjectileChange() < -projectiles) {
							cout << "Not enough health, " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
							break;
						}
						cout << "Not enough health or " << g_manaName.plural() << "!\n";
						break;
					}
					if (spells[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough health or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough health!\n";
					break;
				}
				if (spells[*slot1].getManaChange() < -mana) {
					if (spells[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough " << g_manaName.plural() << "!\n";
					break;
				}
				if (spells[*slot1].getProjectileChange() < -projectiles) {
					cout << "Not enough " << g_projName.plural() << "!\n";
					break;
				}
				return 2;
			}
			break;
		case 1: //Responding to weapon attack (no dual wield)
			cout << enemyName << " is attacking with a weapon: " << itemName1 << '\n';
			cout << "To view your inventory, enter 0.\nTo cast a spell, enter 2.\nTo do nothing, enter -1.\n";
			choices.resize(1);
			choices.push_back(2);
			choices.push_back(-1);
			switch (userChoice(choices)) {
			case -1:
				return 0;
			case 0:
				showInventory();
				break;
			case 2:
				cout << "Enter the number of the spell you wish to cast.\nTo go back, enter 0.\n";
				for (unsigned char i = 0; i < spells.size(); i++) {
					cout << i + 1 << ": ";
					spells[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<unsigned char>(userChoice(0, static_cast<int>(spells.size())));
				if (*slot1 == 0) {
					break;
				}
				(*slot1)--;
				if (!spells[*slot1].getReal()) {
					cout << "Selected spell slot is empty!\n";
					break;
				}
				if (spells[*slot1].getHitCount() == 0 || spells[*slot1].getTiming() == 0) {
					cout << "Selected spell cannot be cast at this time!\n";
					break;
				}
				if (spells[*slot1].getCurrentCooldown() > 0) {
					cout << "Selected spell is on cooldown!\n";
					break;
				}
				if (spells[*slot1].getHealthChange() < -health) { //Check can afford
					if (spells[*slot1].getManaChange() < -mana) {
						if (spells[*slot1].getProjectileChange() < -projectiles) {
							cout << "Not enough health, " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
							break;
						}
						cout << "Not enough health or " << g_manaName.plural() << "!\n";
						break;
					}
					if (spells[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough health or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough health!\n";
					break;
				}
				if (spells[*slot1].getManaChange() < -mana) {
					if (spells[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough " << g_manaName.plural() << "!\n";
					break;
				}
				if (spells[*slot1].getProjectileChange() < -projectiles) {
					cout << "Not enough " << g_projName.plural() << "!\n";
					break;
				}
				currentBonusActions--;
				return 2;
			}
			break;
		case 2: //Responding to spell
			cout << enemyName << " is casting a spell: " << itemName1 << '\n';
			cout << "To view your inventory, enter 0.\nTo cast a spell, enter 2.\nTo do nothing, enter -1.\n";
			choices.resize(1);
			choices.push_back(2);
			choices.push_back(-1);
			switch (userChoice(choices)) {
			case -1:
				return 0;
			case 0:
				showInventory();
				break;
			case 2:
				cout << "Enter the number of the spell you wish to cast.\nTo go back, enter 0.\n";
				for (unsigned char i = 0; i < spells.size(); i++) {
					cout << i + 1 << ": ";
					spells[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<unsigned char>(userChoice(0, static_cast<int>(spells.size())));
				if (*slot1 == 0) {
					break;
				}
				(*slot1)--;
				if (!spells[*slot1].getReal()) {
					cout << "Selected spell slot is empty!\n";
					break;
				}
				if (spells[*slot1].getHitCount() == 0 || spells[*slot1].getTiming() == 0) {
					cout << "Selected spell cannot be cast at this time!\n";
					break;
				}
				if (spells[*slot1].getCurrentCooldown() > 0) {
					cout << "Selected spell is on cooldown!\n";
					break;
				}
				if (spells[*slot1].getHealthChange() < -health) { //Check can afford
					if (spells[*slot1].getManaChange() < -mana) {
						if (spells[*slot1].getProjectileChange() < -projectiles) {
							cout << "Not enough health, " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
							break;
						}
						cout << "Not enough health or " << g_manaName.plural() << "!\n";
						break;
					}
					if (spells[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough health or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough health!\n";
					break;
				}
				if (spells[*slot1].getManaChange() < -mana) {
					if (spells[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough " << g_manaName.plural() << "!\n";
					break;
				}
				if (spells[*slot1].getProjectileChange() < -projectiles) {
					cout << "Not enough " << g_projName.plural() << "!\n";
					break;
				}
				currentBonusActions--;
				return 2;
			}
			break;
		case 4:
			cout << enemyName << " is attacking with two weapons: " << itemName1 << " and " << itemName2 << '\n';
			cout << "To view your inventory, enter 0.\nTo cast a spell, enter 2.\nTo do nothing, enter -1.\n";
			choices.resize(1);
			choices.push_back(2);
			choices.push_back(-1);
			switch (userChoice(choices)) {
			case -1:
				return 0;
			case 0:
				showInventory();
				break;
			case 2:
				cout << "Enter the number of the spell you wish to cast.\nTo go back, enter 0.\n";
				for (unsigned char i = 0; i < spells.size(); i++) {
					cout << i + 1 << ": ";
					spells[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<unsigned char>(userChoice(0, static_cast<int>(spells.size())));
				if (*slot1 == 0) {
					break;
				}
				(*slot1)--;
				if (!spells[*slot1].getReal()) {
					cout << "Selected spell slot is empty!\n";
					break;
				}
				if (spells[*slot1].getHitCount() == 0 || spells[*slot1].getTiming() == 0) {
					cout << "Selected spell cannot be cast at this time!\n";
					break;
				}
				if (spells[*slot1].getCurrentCooldown() > 0) {
					cout << "Selected spell is on cooldown!\n";
					break;
				}
				if (spells[*slot1].getHealthChange() < -health) { //Check can afford
					if (spells[*slot1].getManaChange() < -mana) {
						if (spells[*slot1].getProjectileChange() < -projectiles) {
							cout << "Not enough health, " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
							break;
						}
						cout << "Not enough health or " << g_manaName.plural() << "!\n";
						break;
					}
					if (spells[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough health or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough health!\n";
					break;
				}
				if (spells[*slot1].getManaChange() < -mana) {
					if (spells[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough " << g_manaName.plural() << "!\n";
					break;
				}
				if (spells[*slot1].getProjectileChange() < -projectiles) {
					cout << "Not enough " << g_projName.plural() << "!\n";
					break;
				}
				currentBonusActions--;
				return 2;
			}
			break;
		case 3: //Counter attacking
			cout << "Counter attack opportunity.\nTo view your inventory, enter 0.\nTo counter attack with a weapon, enter 1.\nTo counter attack with a spell, enter 2.\nTo do nothing, enter -1.\n";
			switch (userChoice(-1, 2)) {
			case -1:
				return 0;
			case 0:
				showInventory();
				break;
			case 1:
				if (weapons.size() == 0) {
					cout << "No weapon slots!\n";
					break;
				}
				cout << "Enter the number of the weapon you wish to attack with.\nTo go back, enter 0.\n";
				for (unsigned char i = 0; i < weapons.size(); i++) {
					cout << i + 1 << ": ";
					weapons[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<unsigned char>(userChoice(0, static_cast<int>(weapons.size())));
				if (*slot1 == 0) {
					break;
				}
				(*slot1)--;
				if (!weapons[*slot1].getReal()) {
					cout << "Cannot attack with empty slot!\n";
					break;
				}
				if (weapons[*slot1].getCounterHits() == 0) {
					cout << "Selected weapon cannot be used at this time!\n";
					break;
				}
				if (weapons[*slot1].getHealthChange() < -health) { //Check can afford
					if (weapons[*slot1].getManaChange() < -mana) {
						if (weapons[*slot1].getProjectileChange() < -projectiles) {
							cout << "Not enough health, " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
							break;
						}
						cout << "Not enough health or " << g_manaName.plural() << "!\n";
						break;
					}
					if (weapons[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough health or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough health!\n";
					break;
				}
				if (weapons[*slot1].getManaChange() < -mana) {
					if (weapons[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough " << g_manaName.plural() << "!\n";
					break;
				}
				if (weapons[*slot1].getProjectileChange() < -projectiles) {
					cout << "Not enough " << g_projName.plural() << "!\n";
					break;
				}
				currentBonusActions--;
				if (!weapons[*slot1].getDualWield() || currentBonusActions <= 0) { //If cannot dual wield, done with selection
					return 1;
				}
				//Can dual wield, choose another weapon
				//Find possible weapons
				choices.resize(1);
				for (unsigned char i = 0; i < weapons.size(); i++) {
					if (i == *slot1) { //Can't dual wield the same weapon twice
						continue;
					}
					if (weapons[i].getDualWield() && weapons[i].getReal() && weapons[i].getCounterHits() != 0) { //If the weapon can dual wield, add it to list
						choices.push_back(i + 1);
					}
				}
				if (choices.size() == 1) { //Found nothing
					return 1;
				}
				modifyHealth(weapons[*slot1].getHealthChange());
				modifyMana(weapons[*slot1].getManaChange());
				modifyProjectiles(weapons[*slot1].getProjectileChange());
				while (true) {
					cout << "The selected weapon can be dual wielded\n";
					cout << "(After applying costs for selected weapon)\n";
					cout << "Health: " << health << '/' << maxHealth << ' ' << g_manaName.Plural() << ": " << mana << '/' << maxMana << ' ' << g_projName.Plural() << ": " << projectiles << ' ' << "Bonus Actions: " << +currentBonusActions << ' ';
					if (poison > 0) {
						cout << "Poison: " << +poison << ' ';
					}
					if (bleed > 0) {
						cout << "Bleed: " << +bleed << ' ';
					}
					if (tempRegen > 0) {
						cout << "Regeneration: " << +tempRegen;
					}
					cout << '\n';
					cout << "To view your inventory, enter 0.\nTo select a second weapon, enter 1.\nTo proceed without dual wielding, enter -1.\n";
					switch (userChoice(-1, 1)) {
					case -1:
						health = currentHealth;
						mana = currentMana;
						projectiles = currentProjectiles;
						return 1;
					case 0:
						showInventory();
						break;
					case 1:
						cout << "Enter the number of the additional weapon you wish to attack with.\nTo go back, enter 0.\n";
						for (short i = 1; i < choices.size(); i++) {
							cout << choices[i] << ": ";
							weapons[static_cast<unsigned char>(choices[i] - 1)].displayName();
						}
						*slot2 = static_cast<unsigned char>(userChoice(choices));
						if (*slot2 == 0) {
							break;
						}
						(*slot2)--;
						if (!weapons[*slot2].getReal()) {
							cout << "Cannot attack with empty slot!\n";
							break;
						}
						if (weapons[*slot2].getCounterHits() == 0) {
							cout << "Selected weapon cnnot be used at this time!\n";
							break;
						}
						if (weapons[*slot2].getHealthChange() < -health) { //Check can afford
							if (weapons[*slot2].getManaChange() < -mana) {
								if (weapons[*slot2].getProjectileChange() < -projectiles) {
									cout << "Not enough health, " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
									break;
								}
								cout << "Not enough health or " << g_manaName.plural() << "!\n";
								break;
							}
							if (weapons[*slot2].getProjectileChange() < -projectiles) {
								cout << "Not enough health or " << g_projName.plural() << "!\n";
								break;
							}
							cout << "Not enough health!\n";
							break;
						}
						if (weapons[*slot2].getManaChange() < -mana) {
							if (weapons[*slot2].getProjectileChange() < -projectiles) {
								cout << "Not enough " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
								break;
							}
							cout << "Not enough " << g_manaName.plural() << "!\n";
							break;
						}
						if (weapons[*slot2].getProjectileChange() < -projectiles) {
							cout << "Not enough " << g_projName.plural() << "!\n";
							break;
						}
						//Can afford, so done selecting
						health = currentHealth;
						mana = currentMana;
						projectiles = currentProjectiles;
						currentBonusActions--;
						return 3;
					}
				}
				break;
			case 2: //Casting a spell
				if (spells.size() == 0) {
					cout << "No spell slots!\n";
					break;
				}
				cout << "Enter the number of the spell you wish to cast.\nTo go back, enter 0.\n";
				for (unsigned char i = 0; i < spells.size(); i++) {
					cout << i + 1 << ": ";
					spells[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<unsigned char>(userChoice(0, static_cast<int>(spells.size())));
				if (*slot1 == 0) {
					break;
				}
				(*slot1)--;
				if (!spells[*slot1].getReal()) {
					cout << "Selected spell slot is empty!\n";
					break;
				}
				if (spells[*slot1].getCounterHits() == 0) {
					cout << "Selected spell cannot be cast at this time!\n";
					break;
				}
				if (spells[*slot1].getCurrentCooldown() > 0) {
					cout << "Selected spell is on cooldown!\n";
					break;
				}
				if (spells[*slot1].getHealthChange() < -health) { //Check can afford
					if (spells[*slot1].getManaChange() < -mana) {
						if (spells[*slot1].getProjectileChange() < -projectiles) {
							cout << "Not enough health, " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
							break;
						}
						cout << "Not enough health or " << g_manaName.plural() << "!\n";
						break;
					}
					if (spells[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough health or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough health!\n";
					break;
				}
				if (spells[*slot1].getManaChange() < -mana) {
					if (spells[*slot1].getProjectileChange() < -projectiles) {
						cout << "Not enough " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough " << g_manaName.plural() << "!\n";
					break;
				}
				if (spells[*slot1].getProjectileChange() < -projectiles) {
					cout << "Not enough " << g_projName.plural() << "!\n";
					break;
				}
				currentBonusActions--;
				return 2;
			}
			break;
		}
	}
}

void player::applyDamageModifiers(short* p, short* m, short* a) {
	float damStorage;
	if (*p > 0) {
		if (SHRT_MAX - *p < flatDamageModifier) {
			*p = SHRT_MAX;
		}
		else {
			*p += flatDamageModifier;
			if (*p < 0) {
				*p = 0;
			}
		}
		damStorage = *p * (1 + propDamageModifier);
		if (damStorage > SHRT_MAX) {
			*p = SHRT_MAX;
		}
		else {
			*p = static_cast<short>(damStorage);
		}
	}
	if (*m > 0) {
		if (SHRT_MAX - *m < flatMagicDamageModifier) {
			*m = SHRT_MAX;
		}
		else {
			*m += flatMagicDamageModifier;
			if (*m < 0) {
				*m = 0;
			}
		}
		damStorage = *m * (1 + propMagicDamageModifier);
		if (damStorage > SHRT_MAX) {
			*m = SHRT_MAX;
		}
		else {
			*m = static_cast<short>(damStorage);
		}
	}
	if (*a > 0) {
		if (SHRT_MAX - *a < flatArmourPiercingDamageModifier) {
			*a = SHRT_MAX;
		}
		else {
			*a += flatArmourPiercingDamageModifier;
			if (*a < 0) {
				*a = 0;
			}
		}
		damStorage = *a * (1 + propArmourPiercingDamageModifier);
		if (damStorage > SHRT_MAX) {
			*a = SHRT_MAX;
		}
		else {
			*a = static_cast<short>(damStorage);
		}
	}
}

void player::upgradeItems(short upgradeNum) {
	bool done = false;
	for (short i = upgradeNum; i > 0; i--) {
		done = false;
		while (!done) {
			cout << "Armour:\n";
			cout << "Head: " << helmet.getName() << '\n';
			cout << "Torso: " << chestplate.getName() << '\n';
			cout << "Legs: " << greaves.getName() << '\n';
			cout << "Feet: " << boots.getName() << '\n';
			cout << "Weapons:\n";
			for (short j = 0; j < weapons.size(); j++) {
				cout << j + 1 << ": " << getWeapon(static_cast<unsigned char>(j))->getName() << '\n';
			}
			cout << "Spells:\n";
			for (short j = 0; j < spells.size(); j++) {
				cout << j + 1 << ": " << getSpell(static_cast<unsigned char>(j))->getName() << '\n';
			}
			cout << i << " upgrade points remaining\n";
			cout << "To upgrade an armour piece, enter 1.\nTo upgrade a weapon, enter 2.\nTo upgrade a spell, enter 3.\nTo upgrade nothing, enter 0.\n";
			unsigned char j;
			switch (userChoice(0, 3)) {
			case 1:
				cout << "To upgrade your head armour, enter 1.\nTo upgrade your torso armour, enter 2.\nTo upgrade your leg armour, enter 3.\nTo upgrade your foot armour, enter 4.\nTo go back, enter 0.\n";
				switch (userChoice(0, 4)) {
				case 1:
					done = helmet.upgradeItem();
					break;
				case 2:
					done = chestplate.upgradeItem();
					break;
				case 3:
					done = greaves.upgradeItem();
					break;
				case 4:
					done = boots.upgradeItem();
					break;
				}
				break;
			case 2:
				cout << "Enter the slot number of the weapon you wish to upgrade.\nTo go back, enter 0.\n";
				j = userChoice(0, static_cast<int>(weapons.size()));
				if (j == 0) {
					break;
				}
				j--;
				done = weapons[j].upgradeItem();
				break;
			case 3:
				cout << "Enter the slot number of the spell you wish to upgrade.\nTo go back, enter 0.\n";
				j = userChoice(0, static_cast<int>(spells.size()));
				if (j == 0) {
					break;
				}
				j--;
				done = spells[j].upgradeItem();
				break;
			default:
				done = true;
			}
		}
	}
}