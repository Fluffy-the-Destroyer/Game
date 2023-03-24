#include "player.h"
#include <fstream>
#include "rng.h"
#include <iostream>
#include "inputs.h"
#include "resources.h"
#include "battle.h"
#include <thread>
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

short player::flatDamage(short p, short m, short a, bool overHeal) {
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
		if (overHeal) { //May over heal
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
			if (health >= maxHealth) { //Already over healed
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
	modifyMaxHealth(chestPlate.getMaxHealthModifier());
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
	modifyMaxMana(chestPlate.getMaxManaModifier());
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
	modifyTurnManaRegen(chestPlate.getTurnManaRegenModifier());
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
	modifyBattleManaRegen(chestPlate.getBattleManaRegenModifier());
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

void player::modifyTurnRegen(short c) {
	if (c > 0) {
		if (SHRT_MAX - c < turnRegen) { //Overflow
			turnRegen = SHRT_MAX;
		}
		else {
			turnRegen += c;
		}
	}
	else if (c < 0) {
		if (SHRT_MIN + (-c) > turnRegen) { //Underflow
			turnRegen = SHRT_MIN;
		}
		else {
			turnRegen += c;
		}
	}
}

void player::calculateTurnRegen() {
	turnRegen = turnRegenBase;
	modifyTurnRegen(helmet.getTurnRegenModifier());
	modifyTurnRegen(chestPlate.getTurnRegenModifier());
	modifyTurnRegen(greaves.getTurnRegenModifier());
	modifyTurnRegen(boots.getTurnRegenModifier());
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
	modifyBattleRegen(chestPlate.getBattleRegenModifier());
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
	modifyFlatArmour(chestPlate.getFlatArmourModifier());
	modifyFlatArmour(greaves.getFlatArmourModifier());
	modifyFlatArmour(boots.getFlatArmourModifier());
	//Prop armour
	propArmour = propArmourBase;
	modifyPropArmour(helmet.getPropArmourModifier());
	modifyPropArmour(chestPlate.getPropArmourModifier());
	modifyPropArmour(greaves.getPropArmourModifier());
	modifyPropArmour(boots.getPropArmourModifier());
	//Flat magic armour
	flatMagicArmour = flatMagicArmourBase;
	modifyFlatMagicArmour(helmet.getFlatMagicArmourModifier());
	modifyFlatMagicArmour(chestPlate.getFlatMagicArmourModifier());
	modifyFlatMagicArmour(greaves.getFlatMagicArmourModifier());
	modifyFlatMagicArmour(boots.getFlatMagicArmourModifier());
	//Prop magic armour
	propMagicArmour = propMagicArmourBase;
	modifyPropMagicArmour(helmet.getPropMagicArmourModifier());
	modifyPropMagicArmour(chestPlate.getPropMagicArmourModifier());
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
	modifyFlatDamageModifier(chestPlate.getFlatDamageModifier());
	modifyFlatDamageModifier(greaves.getFlatDamageModifier());
	modifyFlatDamageModifier(boots.getFlatDamageModifier());
	//Prop
	propDamageModifier = propDamageModifierBase;
	modifyPropDamageModifier(helmet.getPropDamageModifier());
	modifyPropDamageModifier(chestPlate.getPropDamageModifier());
	modifyPropDamageModifier(greaves.getPropDamageModifier());
	modifyPropDamageModifier(boots.getPropDamageModifier());
	//Flat magic
	flatMagicDamageModifier = flatMagicDamageModifierBase;
	modifyFlatMagicDamageModifier(helmet.getFlatMagicDamageModifier());
	modifyFlatMagicDamageModifier(chestPlate.getFlatMagicDamageModifier());
	modifyFlatMagicDamageModifier(greaves.getFlatMagicDamageModifier());
	modifyFlatMagicDamageModifier(boots.getFlatMagicDamageModifier());
	for (uint8_t i = 0; i < weapons.size(); i++) {
		if (weapons[i].getReal()) {
			modifyFlatMagicDamageModifier(weapons[i].getFlatMagicDamageModifier());
		}
	}
	//Prop magic
	propMagicDamageModifier = propMagicDamageModifierBase;
	modifyPropMagicDamageModifier(helmet.getPropMagicDamageModifier());
	modifyPropMagicDamageModifier(chestPlate.getPropMagicDamageModifier());
	modifyPropMagicDamageModifier(greaves.getPropMagicDamageModifier());
	modifyPropMagicDamageModifier(boots.getPropMagicDamageModifier());
	//Flat AP
	flatArmourPiercingDamageModifier = flatArmourPiercingDamageModifierBase;
	modifyFlatArmourPiercingDamageModifier(helmet.getFlatArmourPiercingDamageModifier());
	modifyFlatArmourPiercingDamageModifier(chestPlate.getFlatArmourPiercingDamageModifier());
	modifyFlatArmourPiercingDamageModifier(greaves.getFlatArmourPiercingDamageModifier());
	modifyFlatArmourPiercingDamageModifier(boots.getFlatArmourPiercingDamageModifier());
	//Prop AP
	propArmourPiercingDamageModifier = propArmourPiercingDamageModifierBase;
	modifyPropArmourPiercingDamageModifier(helmet.getPropArmourPiercingDamageModifier());
	modifyPropArmourPiercingDamageModifier(chestPlate.getPropArmourPiercingDamageModifier());
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
	modifyPoisonResist(chestPlate.getPoisonResistModifier());
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
	modifyBleedResist(chestPlate.getBleedResistModifier());
	modifyBleedResist(greaves.getBleedResistModifier());
	modifyBleedResist(boots.getBleedResistModifier());
}

void player::calculateEvadeChance() {
	evadeChance = evadeChanceBase;
	modifyEvadeChance(helmet.getEvadeChanceModifier());
	modifyEvadeChance(chestPlate.getEvadeChanceModifier());
	modifyEvadeChance(greaves.getEvadeChanceModifier());
	modifyEvadeChance(boots.getEvadeChanceModifier());
}

void player::calculateModifiers() {
	calculateMaxHealth();
	calculateMaxMana();
	calculateTurnManaRegen();
	calculateBattleManaRegen();
	calculateTurnRegen();
	calculateBattleRegen();
	calculateArmour();
	calculateDamageModifiers();
	calculateEvadeChance();
	calculatePoisonResist();
	calculateBleedResist();
	calculateCounterAttackChance();
	calculateBonusActions();
	calculateInitiative();
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
		cout << i + 1 << ": " << getWeapon(static_cast<uint8_t>(i))->getName() << '\n';
	}
	while (true) {
		cout << "Enter the number of the slot in which to equip the weapon.\nTo discard the weapon, enter 0.\n";
		try {
			uint8_t slot = userChoice(0, static_cast<int>(weapons.size()));
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
		cout << i + 1 << ": " << getSpell(static_cast<uint8_t>(i))->getName() << '\n';
	}
	while (true) {
		cout << "Enter the number of the slot in which to equip the spell.\nTo discard the spell, enter 0.\n";
		try {
			uint8_t slot = userChoice(0, static_cast<int>(spells.size()));
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
	chestPlate.displayStats();
	cout << "New armour:\n";
	c->displayStats();
	cout << "To equip the new armour, enter 1.\nTo discard it, enter 2.\n";
	if (userChoice(1, 2) == 1) {
		chestPlate = *c;
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

weapon* player::getWeapon(uint8_t i) {
	if (i >= weapons.size()) {
		throw 6;
	}
	return &(weapons[i]);
}

spell* player::getSpell(uint8_t i) {
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
	modifyCounterAttackChance(chestPlate.getCounterAttackChanceModifier());
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
	modifyBonusActions(chestPlate.getBonusActionsModifier());
	modifyBonusActions(greaves.getBonusActionsModifier());
	modifyBonusActions(boots.getBonusActionsModifier());
}

void player::loadClass(string playerClass, bool custom) {
	ifstream classBlueprints;
	string buffer = "";
	short charBuf;
	//Open blueprint file
	try {
		if (playerClass == "EMPTY") {
			throw 3;
		}
		if (custom) {
			classBlueprints.open("custom\\classBlueprints.xml");
		}
		else {
			classBlueprints.open("data\\classBlueprints.xml");
		}
		if (!classBlueprints.is_open()) {
			throw 4;
		}
		ignoreLine(&classBlueprints, '<');
		if (custom && classBlueprints.eof()) {
			throw 4;
		}
		classBlueprints.seekg(-1, ios_base::cur);
		string blueprintName = "classBlueprintList name=\"" + playerClass + '\"';
		bool customFile = custom;
		//Check for a list
		{
			bool noList = false; //Tracking if we found a list
			streampos filePos = 0; //Position in file
			short listCount = 0; //NUmber of items in a list, also tracking which item we have chosen
			while (true) {
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
					playerClass = stringFromFile(&classBlueprints);
					if (getTag(&classBlueprints) != "/name") {
						throw 1;
					}
					if (playerClass == "EMPTY") {
						throw 3;
					}
				}
				else if (customFile) {
					classBlueprints.close();
					classBlueprints.open("data\\classBlueprints.xml");
					if (!classBlueprints.is_open()) {
						classBlueprints.open("custom\\classBlueprints.xml");
						if (!classBlueprints.is_open()) {
							custom = false;
							throw 4;
						}
						break;
					}
					customFile = noList = false;
					filePos = 0;
					listCount = -1;
					continue;
				}
				break;
			}
		}
		if (customFile != custom) {
			classBlueprints.close();
			classBlueprints.open("custom\\classBlueprints.xml");
			if (!classBlueprints.is_open()) {
				classBlueprints.open("data\\classBlueprints.xml");
				if (!classBlueprints.is_open()) {
					custom = false;
					throw 4;
				}
			}
			else {
				customFile = true;
			}
		}
		classBlueprints.seekg(0);
		buffer = "";
		blueprintName = "classBlueprint name=\"" + playerClass + '\"';
		//Find and read blueprint
		while (true) {
			while (buffer != blueprintName) {
				buffer = getTag(&classBlueprints);
				ignoreLine(&classBlueprints);
				if (classBlueprints.eof()) {
					break;
				}
			}
			if (classBlueprints.eof()) {
				if (customFile) {
					classBlueprints.close();
					classBlueprints.open("data\\classBlueprints.xml");
					if (!classBlueprints.is_open()) {
						custom = false;
						throw 4;
					}
					customFile = false;
					continue;
				}
				throw 2;
			}
			break;
		}
		buffer = getTag(&classBlueprints);
		while (buffer != "/classBlueprint") {
			if (buffer == "maxHealth") {
				maxHealthBase = numFromFile(&classBlueprints);
			}
			else if (buffer == "maxMana") {
				maxManaBase = numFromFile(&classBlueprints);
			}
			else if (buffer == "turnManaRegen") {
				turnManaRegenBase = numFromFile(&classBlueprints);
			}
			else if (buffer == "battleManaRegen") {
				battleManaRegenBase = numFromFile(&classBlueprints);
			}
			else if (buffer == "poisonResist") {
				poisonResistBase = floatFromFile(&classBlueprints);
				if (poisonResistBase < 0) {
					poisonResistBase = 0;
				}
			}
			else if (buffer == "bleedResist") {
				bleedResistBase = floatFromFile(&classBlueprints);
				if (bleedResistBase < 0) {
					bleedResistBase = 0;
				}
			}
			else if (buffer == "turnRegen") {
				turnRegenBase = numFromFile(&classBlueprints);
			}
			else if (buffer == "battleRegen") {
				battleRegenBase = numFromFile(&classBlueprints);
			}
			else if (buffer.substr(0, 15) == "weapons count=\"") { //It's the weapons tag, count is how many slots there are, which varies, so only checking the beginning of the tag
				buffer.erase(0, 15); //Get rid of the stuff that has been checked
				weapons.resize(static_cast<uint8_t>(numFromString(&buffer))); //If the number is negative or bigger than 255, it will overflow or underflow, but the documentation will say it must be in this range so it is an error by whomever made the class
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
						buffer = stringFromFile(&classBlueprints); //Get the blueprint of the weapon (which could be a list)
						if (classBlueprints.eof()) {
							throw 1;
						}
						weapons[i].loadFromFile(buffer); //Load that weapon from files
						if (getTag(&classBlueprints) != "/weapon") {
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
				buffer = getTag(&classBlueprints);
				continue;
			}
			else if (buffer.substr(0, 14) == "spells count=\"") { //Similar to weapon setting
				buffer.erase(0, 14);
				spells.resize(static_cast<uint8_t>(numFromString(&buffer)));
				if (buffer != "\"") {
					throw 1;
				}
				ignoreLine(&classBlueprints);
				for (int i = 0; i < spells.size(); i++) {
					buffer = getTag(&classBlueprints);
					if (buffer == "spell") {
						buffer = stringFromFile(&classBlueprints);
						if (classBlueprints.eof()) {
							throw 1;
						}
						spells[i].loadFromFile(buffer);
						if (getTag(&classBlueprints) != "/spell") {
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
				buffer = getTag(&classBlueprints);
				continue;
			}
			else if (buffer == "flatArmour") {
				flatArmourBase = numFromFile(&classBlueprints);
			}
			else if (buffer == "propArmour") {
				propArmourBase = floatFromFile(&classBlueprints);
				if (propArmourBase < -1) {
					propArmourBase = -1;
				}
			}
			else if (buffer == "flatMagicArmour") {
				flatMagicArmourBase = numFromFile(&classBlueprints);
			}
			else if (buffer == "propMagicArmour") {
				propMagicArmourBase = floatFromFile(&classBlueprints);
				if (propMagicArmourBase < -1) {
					propMagicArmourBase = -1;
				}
			}
			else if (buffer == "helmet") {
				buffer = stringFromFile(&classBlueprints);
				if (classBlueprints.eof()) {
					throw 1;
				}
				helmet.loadFromFile(buffer);
				if (getTag(&classBlueprints) != "/helmet") {
					throw 1;
				}
				ignoreLine(&classBlueprints);
				buffer = getTag(&classBlueprints);
				continue;
			}
			else if (buffer == "chestPlate") {
				buffer = stringFromFile(&classBlueprints);
				if (classBlueprints.eof()) {
					throw 1;
				}
				chestPlate.loadFromFile(buffer);
				if (getTag(&classBlueprints) != "/chestPlate") {
					throw 1;
				}
				ignoreLine(&classBlueprints);
				buffer = getTag(&classBlueprints);
				continue;
			}
			else if (buffer == "greaves") {
				buffer = stringFromFile(&classBlueprints);
				if (classBlueprints.eof()) {
					throw 1;
				}
				greaves.loadFromFile(buffer);
				if (getTag(&classBlueprints) != "/greaves") {
					throw 1;
				}
				ignoreLine(&classBlueprints);
				buffer = getTag(&classBlueprints);
				continue;
			}
			else if (buffer == "boots") {
				buffer = stringFromFile(&classBlueprints);
				if (classBlueprints.eof()) {
					throw 1;
				}
				boots.loadFromFile(buffer);
				if (getTag(&classBlueprints) != "/boots") {
					throw 1;
				}
				ignoreLine(&classBlueprints);
				buffer = getTag(&classBlueprints);
				continue;
			}
			else if (buffer == "flatDamageModifier") {
				flatDamageModifierBase = numFromFile(&classBlueprints);
			}
			else if (buffer == "propDamageModifier") {
				propDamageModifierBase = floatFromFile(&classBlueprints);
				if (propDamageModifierBase < -1) {
					propDamageModifierBase = -1;
				}
			}
			else if (buffer == "flatMagicDamageModifier") {
				flatMagicDamageModifierBase = numFromFile(&classBlueprints);
			}
			else if (buffer == "propMagicDamageModifier") {
				propMagicDamageModifierBase = floatFromFile(&classBlueprints);
				if (propMagicDamageModifierBase < -1) {
					propMagicDamageModifierBase = -1;
				}
			}
			else if (buffer == "flatArmourPiercingDamageModifier") {
				flatArmourPiercingDamageModifierBase = numFromFile(&classBlueprints);
			}
			else if (buffer == "propArmourPiercingDamageModifier") {
				propArmourPiercingDamageModifierBase = floatFromFile(&classBlueprints);
				if (propArmourPiercingDamageModifierBase < -1) {
					propArmourPiercingDamageModifierBase = -1;
				}
			}
			else if (buffer == "evadeChance") {
				evadeChanceBase = floatFromFile(&classBlueprints);
				if (evadeChanceBase < 0) {
					evadeChanceBase = 0;
				}
			}
			else if (buffer == "counterAttackChance") {
				counterAttackChanceBase = floatFromFile(&classBlueprints);
				if (counterAttackChanceBase < 0) {
					counterAttackChanceBase = 0;
				}
			}
			else if (buffer == "bonusActions") {
				charBuf = numFromFile(&classBlueprints);
				if (charBuf < -127) {
					charBuf = -127;
				}
				else if (charBuf > 127) {
					charBuf = 127;
				}
				bonusActionsBase = static_cast<int8_t>(charBuf);
			}
			else if (buffer == "className") {
				className = stringFromFile(&classBlueprints);
			}
			else if (buffer == "initiative") {
				initiative = numFromFile(&classBlueprints);
			}
			else if (buffer == "maxXp") {
				maxXp = numFromFile(&classBlueprints);
				if (maxXp < 0) {
					maxXp = 0;
				}
			}
			else if (buffer == "nextLevel") {
				nextLevel = stringFromFile(&classBlueprints);
			}
			else {
				throw 1;
			}
			if (getTag(&classBlueprints) != '/' + buffer) {
				throw 1;
			}
			ignoreLine(&classBlueprints);
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
		if (err == 4) {
			if (custom) {
				loadClass(playerClass, false);
				return;
			}
		}
		throw err;
	}
}

void player::displayStats() {
	cout << "Level " << level << ' ' << className << '\n';
	if (maxXp != 0) {
		cout << xp << '/' << maxXp << " experience\n";
	}
	else {
		cout << "Max level\n";
	}
	//Health
	cout << "Health: " << health << '/' << maxHealth << '\n';
	if (turnRegen > 0) {
		cout << '+' << turnRegen << " health per turn\n";
	}
	else if (turnRegen < 0) {
		cout << turnRegen << " health per turn\n";
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
	cout << "Speed: " << +initiative << '\n';
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
		cout << "Torso: " << chestPlate.getName() << '\n';
		cout << "Legs: " << greaves.getName() << '\n';
		cout << "Feet: " << boots.getName() << '\n';
		cout << "Weapons:\n";
		for (short i = 0; i < weapons.size(); i++) {
			cout << i + 1 << ": " << getWeapon(static_cast<uint8_t>(i))->getName() << '\n';
		}
		cout << "Spells:\n";
		for (short i = 0; i < spells.size(); i++) {
			cout << i + 1 << ": " << getSpell(static_cast<uint8_t>(i))->getName() << '\n';
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
				chestPlate.displayStats();
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
				getWeapon(static_cast<uint8_t>(userChoice(1, static_cast<int>(weapons.size())) - 1))->displayStats(); //Using getWeapon as it has protection against attempts to access outside the weapons vector
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
				getSpell(static_cast<uint8_t>(userChoice(1, static_cast<int>(spells.size())) - 1))->displayStats();
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
	modifyHealth(turnRegen + REGEN_MULTIPLIER * tempRegen);
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
	for (uint8_t i = 0; i < spells.size(); i++) {
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

void player::resetBonusActions() {
	if (bonusActions < 0) {
		currentBonusActions = 0;
	}
	else {
		currentBonusActions = bonusActions;
	}
}

void player::reset() {
	modifyHealth(battleRegen);
	modifyMana(battleManaRegen);
	for (uint8_t i = 0; i < spells.size(); i++) {
		spells[i].resetCooldown();
	}
	calculateModifiers();
	cureBleed();
	curePoison();
	removeRegen();
}

uint8_t player::chooseAction(uint8_t* slot1, uint8_t* slot2, string enemyName, const uint8_t timing, string itemName1, string itemName2) {
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
		for (uint8_t i = 0; i < spells.size(); i++) {
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
		for (uint8_t i = 0; i < weapons.size(); i++) {
			if (weapons[i].getReal() && weapons[i].getCounterHits() > 0) {
				choiceCounter++;
			}
		}
		for (uint8_t i = 0; i < spells.size(); i++) {
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
				for (uint8_t i = 0; i < weapons.size(); i++) {
					cout << i + 1 << ": ";
					weapons[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<uint8_t>(userChoice(0, static_cast<int>(weapons.size())));
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
				for (uint8_t i = 0; i < weapons.size(); i++) {
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
							weapons[static_cast<uint8_t>(choices[i] - 1)].displayName();
							cout << '\n';
						}
						*slot2 = static_cast<uint8_t>(userChoice(choices));
						if (*slot2 == 0) {
							break;
						}
						(*slot2)--;
						if (!weapons[*slot2].getReal()) {
							cout << "Cannot attack with empty slot!\n";
							break;
						}
						if (weapons[*slot2].getHitCount() == 0) {
							cout << "Selected weapon cannot be used at this time!\n";
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
				for (uint8_t i = 0; i < spells.size(); i++) {
					cout << i + 1 << ": ";
					spells[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<uint8_t>(userChoice(0, static_cast<int>(spells.size())));
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
				for (uint8_t i = 0; i < spells.size(); i++) {
					cout << i + 1 << ": ";
					spells[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<uint8_t>(userChoice(0, static_cast<int>(spells.size())));
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
				for (uint8_t i = 0; i < spells.size(); i++) {
					cout << i + 1 << ": ";
					spells[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<uint8_t>(userChoice(0, static_cast<int>(spells.size())));
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
				for (uint8_t i = 0; i < spells.size(); i++) {
					cout << i + 1 << ": ";
					spells[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<uint8_t>(userChoice(0, static_cast<int>(spells.size())));
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
				for (uint8_t i = 0; i < weapons.size(); i++) {
					cout << i + 1 << ": ";
					weapons[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<uint8_t>(userChoice(0, static_cast<int>(weapons.size())));
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
				for (uint8_t i = 0; i < weapons.size(); i++) {
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
							weapons[static_cast<uint8_t>(choices[i] - 1)].displayName();
							cout << '\n';
						}
						*slot2 = static_cast<uint8_t>(userChoice(choices));
						if (*slot2 == 0) {
							break;
						}
						(*slot2)--;
						if (!weapons[*slot2].getReal()) {
							cout << "Cannot attack with empty slot!\n";
							break;
						}
						if (weapons[*slot2].getCounterHits() == 0) {
							cout << "Selected weapon cannot be used at this time!\n";
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
				for (uint8_t i = 0; i < spells.size(); i++) {
					cout << i + 1 << ": ";
					spells[i].displayName();
					cout << '\n';
				}
				*slot1 = static_cast<uint8_t>(userChoice(0, static_cast<int>(spells.size())));
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
	if (upgradeNum <= 0) {
		return;
	}
	cout << upgradeNum << " equipment upgrade points acquired\n";
	bool done;
	for (short i = upgradeNum; i > 0; i--) {
		done = false;
		while (!done) {
			cout << "Armour:\n";
			cout << "Head: " << helmet.getName() << '\n';
			cout << "Torso: " << chestPlate.getName() << '\n';
			cout << "Legs: " << greaves.getName() << '\n';
			cout << "Feet: " << boots.getName() << '\n';
			cout << "Weapons:\n";
			for (short j = 0; j < weapons.size(); j++) {
				cout << j + 1 << ": " << getWeapon(static_cast<uint8_t>(j))->getName() << '\n';
			}
			cout << "Spells:\n";
			for (short j = 0; j < spells.size(); j++) {
				cout << j + 1 << ": " << getSpell(static_cast<uint8_t>(j))->getName() << '\n';
			}
			cout << i << " upgrade points remaining\n";
			cout << "To upgrade an armour piece, enter 1.\nTo upgrade a weapon, enter 2.\nTo upgrade a spell, enter 3.\nTo upgrade nothing, enter 0.\n";
			uint8_t j;
			switch (userChoice(0, 3)) {
			case 1:
				cout << "To upgrade your head armour, enter 1.\nTo upgrade your torso armour, enter 2.\nTo upgrade your leg armour, enter 3.\nTo upgrade your foot armour, enter 4.\nTo go back, enter 0.\n";
				switch (userChoice(0, 4)) {
				case 1:
					done = helmet.upgradeItem();
					break;
				case 2:
					done = chestPlate.upgradeItem();
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
	this_thread::sleep_for(chrono::milliseconds(100));
}

void player::giveXp(int xpGain) {
	if (maxXp == 0) {
		return;
	}
	if (xpGain > 0) {
		if (xpGain > INT_MAX - xp) { //Overflow protection
			xp = INT_MAX;
		}
		else {
			xp += xpGain;
		}
	}
	else if (xpGain < 0) {
		xp = max(0, xp + xpGain);
		return;
	}
	else {
		return;
	}
	while (xp > maxXp && maxXp > 0) {
		xp -= maxXp;
		levelUp();
	}
}

void player::levelUp() {
	if (maxXp == 0) {
		return;
	}
	try {
		playerLevel newLevel(nextLevel);
		level++;
		cout << "Level up! You are now level " << level << '\n';
		if (newLevel.maxXp == 0) {
			cout << "Maximum level reached!\n";
			xp = 0;
		}
		cout << showpos;
		this_thread::sleep_for(chrono::milliseconds(100));
		if (!newLevel.fullHeal && newLevel.heal < 0) {
			cout << newLevel.heal << " health\n";
			modifyHealth(newLevel.heal);
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (!newLevel.fullMana && newLevel.mana < 0) {
			if (newLevel.mana == -1) {
				cout << "-1 " << g_manaName.singular() << '\n';
			}
			else {
				cout << newLevel.mana << ' ' << g_manaName.plural() << '\n';
			}
			modifyMana(newLevel.mana);
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.projectiles == 1 || newLevel.projectiles == -1) {
			cout << newLevel.projectiles << ' ' << g_projName.singular() << '\n';
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.projectiles != 0) {
			cout << newLevel.projectiles << ' ' << g_projName.plural() << '\n';
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		modifyProjectiles(newLevel.projectiles);
		if (newLevel.maxHealth > 0) {
			cout << newLevel.maxHealth << " max health\n";
			if (maxHealthBase > SHRT_MAX - newLevel.maxHealth) { //Overflow
				maxHealthBase = SHRT_MAX;
			}
			else {
				maxHealthBase += newLevel.maxHealth;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.maxHealth < 0) {
			cout << newLevel.maxHealth << " max health\n";
			if (maxHealthBase < SHRT_MIN - newLevel.maxHealth) { //Underflow
				maxHealthBase = SHRT_MIN;
			}
			else {
				maxHealthBase += newLevel.maxHealth;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.maxMana > 0) {
			cout << newLevel.maxMana << " max " << g_manaName.plural() << '\n';
			if (maxManaBase > SHRT_MAX - newLevel.maxMana) {
				maxManaBase = SHRT_MAX;
			}
			else {
				maxManaBase += newLevel.maxMana;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.maxMana < 0) {
			cout << newLevel.maxMana << " max " << g_manaName.plural() << '\n';
			if (maxManaBase < SHRT_MIN - newLevel.maxMana) {
				maxManaBase = SHRT_MIN;
			}
			else {
				maxManaBase += newLevel.maxMana;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.turnManaRegen > 0) {
			if (newLevel.turnManaRegen == 1) {
				cout << "+1 " << g_manaName.singular() << " per turn\n";
			}
			else {
				cout << newLevel.turnManaRegen << ' ' << g_manaName.plural() << " per turn\n";
			}
			if (turnManaRegenBase > SHRT_MAX - newLevel.turnManaRegen) {
				turnManaRegenBase = SHRT_MAX;
			}
			else {
				turnManaRegenBase += newLevel.turnManaRegen;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.turnManaRegen < 0) {
			if (newLevel.turnManaRegen == 1) {
				cout << "-1 " << g_manaName.singular() << " per turn\n";
			}
			else {
				cout << newLevel.turnManaRegen << ' ' << g_manaName.plural() << " per turn\n";
			}
			if (turnManaRegenBase < SHRT_MIN - newLevel.turnManaRegen) {
				turnManaRegenBase = SHRT_MIN;
			}
			else {
				turnManaRegenBase += newLevel.turnManaRegen;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.battleManaRegen > 0) {
			if (newLevel.battleManaRegen == 1) {
				cout << "+1 " << g_manaName.singular() << " at end of battle\n";
			}
			else {
				cout << newLevel.battleManaRegen << ' ' << g_manaName.plural() << " at end of battle\n";
			}
			if (battleManaRegenBase > SHRT_MAX - newLevel.battleManaRegen) {
				battleManaRegenBase = SHRT_MAX;
			}
			else {
				battleManaRegenBase += newLevel.battleManaRegen;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.battleManaRegen < 0) {
			if (newLevel.battleManaRegen == 1) {
				cout << "-1 " << g_manaName.singular() << " at end of battle\n";
			}
			else {
				cout << newLevel.battleManaRegen << ' ' << g_manaName.plural() << " at end of battle\n";
			}
			if (battleManaRegenBase < SHRT_MIN - newLevel.battleManaRegen) {
				battleManaRegenBase = SHRT_MIN;
			}
			else {
				battleManaRegenBase += newLevel.battleManaRegen;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.poisonResist != -2) {
			cout << noshowpos << "Base poison resist chance is now " << 100 * newLevel.poisonResist << "%\n" << showpos;
			poisonResistBase = newLevel.poisonResist;
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.bleedResist != -2) {
			cout << noshowpos << "Base bleed resist chance is now " << 100 * newLevel.bleedResist << "%\n" << showpos;
			bleedResistBase = newLevel.bleedResist;
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.turnRegen > 0) {
			cout << newLevel.turnRegen << " health per turn\n";
			if (turnRegenBase > SHRT_MAX - newLevel.turnRegen) {
				turnRegenBase = SHRT_MAX;
			}
			else {
				turnRegenBase += newLevel.turnRegen;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.turnRegen < 0) {
			cout << newLevel.turnRegen << " health per turn\n";
			if (turnRegenBase < SHRT_MIN - newLevel.turnRegen) {
				turnRegenBase = SHRT_MIN;
			}
			else {
				turnRegenBase += newLevel.turnRegen;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.battleRegen > 0) {
			cout << newLevel.battleRegen << " health at end of battle\n";
			if (battleRegenBase > SHRT_MAX - newLevel.battleRegen) {
				battleRegenBase = SHRT_MAX;
			}
			else {
				battleRegenBase += newLevel.battleRegen;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.battleRegen < 0) {
			cout << newLevel.battleRegen << " health at end of battle\n";
			if (battleRegenBase < SHRT_MIN - newLevel.battleRegen) {
				battleRegenBase = SHRT_MIN;
			}
			else {
				battleRegenBase += newLevel.battleRegen;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.flatArmour > 0) {
			cout << newLevel.flatArmour << " physical armour\n";
			if (flatArmourBase > SHRT_MAX - newLevel.flatArmour) {
				flatArmourBase = SHRT_MAX;
			}
			else {
				flatArmourBase += newLevel.flatArmour;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.flatArmour < 0) {
			cout << newLevel.flatArmour << " physical armour\n";
			if (flatArmourBase < SHRT_MIN - newLevel.flatArmour) {
				flatArmourBase = SHRT_MIN;
			}
			else {
				flatArmourBase += newLevel.flatArmour;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.propArmour != -2) {
			propArmourBase = newLevel.propArmour;
		}
		if (newLevel.flatMagicArmour > 0) {
			cout << newLevel.flatMagicArmour << " magic armour\n";
			if (flatMagicArmourBase > SHRT_MAX - newLevel.flatMagicArmour) {
				flatMagicArmourBase = SHRT_MAX;
			}
			else {
				flatMagicArmourBase += newLevel.flatMagicArmour;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.flatMagicArmour < 0) {
			cout << newLevel.flatMagicArmour << " magic armour\n";
			if (flatMagicArmourBase < SHRT_MIN - newLevel.flatMagicArmour) {
				flatMagicArmourBase = SHRT_MIN;
			}
			else {
				flatMagicArmourBase += newLevel.flatMagicArmour;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.propMagicArmour != -2) {
			propMagicArmourBase = newLevel.propMagicArmour;
		}
		if (newLevel.flatDamageModifier > 0) {
			cout << newLevel.flatDamageModifier << " physical damage\n";
			if (flatDamageModifierBase > SHRT_MAX - newLevel.flatDamageModifier) {
				flatDamageModifierBase = SHRT_MAX;
			}
			else {
				flatDamageModifierBase += newLevel.flatDamageModifier;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.flatDamageModifier < 0) {
			cout << newLevel.flatDamageModifier << " physical damage\n";
			if (flatDamageModifierBase < SHRT_MIN - newLevel.flatDamageModifier) {
				flatDamageModifierBase = SHRT_MIN;
			}
			else {
				flatDamageModifierBase += newLevel.flatDamageModifier;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.propDamageModifier != -2) {
			propDamageModifierBase = newLevel.propDamageModifier;
		}
		if (newLevel.flatMagicDamageModifier > 0) {
			cout << newLevel.flatMagicDamageModifier << " magic damage\n";
			if (flatMagicDamageModifierBase > SHRT_MAX - newLevel.flatMagicDamageModifier) {
				flatMagicDamageModifierBase = SHRT_MAX;
			}
			else {
				flatMagicDamageModifierBase += newLevel.flatMagicDamageModifier;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.flatMagicDamageModifier < 0) {
			cout << newLevel.flatMagicDamageModifier << " magic damage\n";
			if (flatMagicDamageModifierBase < SHRT_MIN - newLevel.flatMagicDamageModifier) {
				flatMagicDamageModifierBase = SHRT_MIN;
			}
			else {
				flatMagicDamageModifierBase += newLevel.flatMagicDamageModifier;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.propMagicDamageModifier != -2) {
			propMagicDamageModifierBase = newLevel.propMagicDamageModifier;
		}
		if (newLevel.flatArmourPiercingDamageModifier > 0) {
			cout << newLevel.flatArmourPiercingDamageModifier << " armour piercing damage\n";
			if (flatArmourPiercingDamageModifierBase > SHRT_MAX - newLevel.flatArmourPiercingDamageModifier) {
				flatArmourPiercingDamageModifierBase = SHRT_MAX;
			}
			else {
				flatArmourPiercingDamageModifierBase += newLevel.flatArmourPiercingDamageModifier;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.flatArmourPiercingDamageModifier < 0) {
			cout << newLevel.flatArmourPiercingDamageModifier << " armour piercing damage\n";
			if (flatArmourPiercingDamageModifierBase < SHRT_MIN - newLevel.flatArmourPiercingDamageModifier) {
				flatArmourPiercingDamageModifierBase = SHRT_MIN;
			}
			else {
				flatArmourPiercingDamageModifierBase += newLevel.flatArmourPiercingDamageModifier;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.propArmourPiercingDamageModifier != -2) {
			propArmourPiercingDamageModifierBase = newLevel.propArmourPiercingDamageModifier;
		}
		if (newLevel.evadeChance != -2) {
			cout << noshowpos << "Base evade chance is now " << 100 * newLevel.evadeChance << "%\n" << showpos;
			evadeChanceBase = newLevel.evadeChance;
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.counterAttackChance != -2) {
			cout << noshowpos << "Base counter attack chance is now " << 100 * newLevel.counterAttackChance << "%\n" << showpos;
			counterAttackChanceBase = newLevel.counterAttackChance;
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.bonusActions != 0) {
			cout << newLevel.bonusActions << " bonus actions\n";
			if (bonusActionsBase + newLevel.bonusActions > 127) {
				bonusActionsBase = 127;
			}
			else if (bonusActionsBase + newLevel.bonusActions < -127) {
				bonusActionsBase = -127;
			}
			else {
				bonusActionsBase += newLevel.bonusActions;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.initiative > 0) {
			cout << newLevel.initiative << " speed\n";
			if (initiativeBase > SHRT_MAX - newLevel.initiative) {
				initiativeBase = SHRT_MAX;
			}
			else {
				initiativeBase += newLevel.initiative;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.initiative < 0) {
			cout << newLevel.initiative << " speed\n";
			if (initiativeBase < SHRT_MIN - newLevel.initiative) {
				initiativeBase = SHRT_MIN;
			}
			else {
				initiativeBase += newLevel.initiative;
			}
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		cout << noshowpos;
		maxXp = newLevel.maxXp;
		nextLevel = newLevel.nextLevel;
		upgradeStats(newLevel.statPoints);
		upgradeItems(newLevel.upgradePoints);
		calculateModifiers();
		cout << showpos;
		if (newLevel.fullHeal) {
			cout << "Full health\n";
			health = maxHealth;
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.heal > 0) {
			cout << newLevel.heal << " health\n";
			modifyHealth(newLevel.heal);
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		if (newLevel.fullMana) {
			cout << "Full " << g_manaName.plural() << '\n';
			mana = maxMana;
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.mana == 1) {
			cout << "+1 " << g_manaName.singular() << '\n';
			modifyMana(1);
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		else if (newLevel.mana > 0) {
			cout << newLevel.mana << ' ' << g_manaName.plural() << '\n';
			this_thread::sleep_for(chrono::milliseconds(100));
		}
		cout << noshowpos;
	}
	catch (int err) {
		switch (err) {
		case 1:
			cout << "Unable to parse levelUp blueprint " << nextLevel << '\n';
			break;
		case 2:
			cout << "Unable to find levelUp blueprint " << nextLevel << '\n';
			break;
		case 3:
			cout << "Already at max level\n";
			break;
		case 4:
			cout << "Unable to open classBlueprints.xml\n";
			break;
		case 5:
			cout << "LevelUp blueprintList " << nextLevel << " contains no entries\n";
			break;
		}
		maxXp = 0;
		nextLevel = "";
	}
}

void playerLevel::loadFromFile(string blueprint, bool custom) {
	ifstream classBlueprints;
	string buffer, valBuffer;
	try {
		if (blueprint == "EMPTY") {
			throw 3;
		}
		if (custom) {
			classBlueprints.open("custom\\classBlueprints.xml");
		}
		else {
			classBlueprints.open("data\\classBlueprints.xml");
		}
		if (!classBlueprints.is_open()) {
			throw 4;
		}
		ignoreLine(&classBlueprints, '<');
		if (custom && classBlueprints.eof()) {
			throw 4;
		}
		classBlueprints.seekg(-1, ios_base::cur);
		string blueprintName = "levelBlueprintList name=\"" + blueprint + '\"';
		bool customFile = custom;
		{
			bool noList = false;
			streampos filePos = 0;
			short listCount = -1;
			while (true) {
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
					blueprintName = "/levelBlueprintList";
					do {
						listCount++;
						buffer = getTag(&classBlueprints);
						ignoreLine(&classBlueprints);
					} while (buffer != blueprintName);
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
					blueprint = stringFromFile(&classBlueprints);
					if (blueprint == "EMPTY") {
						throw 3;
					}
					if (getTag(&classBlueprints) != "/name") {
						throw 1;
					}
				}
				else if (customFile) {
					classBlueprints.close();
					classBlueprints.open("data\\classBlueprints.xml");
					if (!classBlueprints.is_open()) {
						classBlueprints.open("custom\\classBlueprints.xml");
						if (!classBlueprints.is_open()) {
							custom = false;
							throw 4;
						}
						break;
					}
					customFile = noList = false;
					filePos = 0;
					listCount = -1;
					continue;
				}
			}
		}
		if (customFile != custom) {
			classBlueprints.close();
			classBlueprints.open("custom\\classBlueprints.xml");
			if (!classBlueprints.is_open()) {
				classBlueprints.open("data\\classBlueprints.xml");
				if (!classBlueprints.is_open()) {
					custom = false;
					throw 4;
				}
			}
			else {
				customFile = true;
			}
		}
		classBlueprints.seekg(0);
		buffer = "";
		blueprintName = "levelBlueprint name=\"" + blueprint + '\"';
		while (true) {
			while (buffer != blueprintName) {
				ignoreLine(&classBlueprints);
				if (classBlueprints.eof()) {
					break;
				}
			}
			if (classBlueprints.eof()) {
				if (customFile) {
					classBlueprints.close();
					classBlueprints.open("data\\classBlueprints.xml");
					if (!classBlueprints.is_open()) {
						custom = false;
						throw 4;
					}
					customFile = false;
					continue;
				}
				throw 2;
			}
			break;
		}
		blueprintName = "/levelBlueprint";
		buffer = getTag(&classBlueprints);
		while (buffer != blueprintName) {
			if (buffer == "heal") {
				valBuffer = stringFromFile(&classBlueprints);
				if (valBuffer == "FULL") {
					fullHeal = true;
				}
				else {
					heal = numFromString(&valBuffer);
				}
			}
			else if (buffer == "mana") {
				valBuffer = stringFromFile(&classBlueprints);
				if (valBuffer == "FULL") {
					fullMana = true;
				}
				else {
					mana = numFromString(&valBuffer);
				}
			}
			else if (buffer == "maxHealth") {
				maxHealth = numFromFile(&classBlueprints);
			}
			else if (buffer == "projectiles") {
				projectiles = numFromFile(&classBlueprints);
			}
			else if (buffer == "maxMana") {
				maxMana = numFromFile(&classBlueprints);
			}
			else if (buffer == "turnManaRegen") {
				turnManaRegen = numFromFile(&classBlueprints);
			}
			else if (buffer == "battleManaRegen") {
				battleManaRegen = numFromFile(&classBlueprints);
			}
			else if (buffer == "poisonResist") {
				poisonResist = floatFromFile(&classBlueprints);
				if (poisonResist < 0) {
					poisonResist = -2;
				}
			}
			else if (buffer == "bleedResist") {
				bleedResist = floatFromFile(&classBlueprints);
				if (bleedResist < 0) {
					bleedResist = -2;
				}
			}
			else if (buffer == "turnRegen") {
				turnRegen = numFromFile(&classBlueprints);
			}
			else if (buffer == "battleRegen") {
				battleRegen = numFromFile(&classBlueprints);
			}
			else if (buffer == "flatArmour") {
				flatArmour = numFromFile(&classBlueprints);
			}
			else if (buffer == "propArmour") {
				propArmour = floatFromFile(&classBlueprints);
				if (propArmour < -1) {
					propArmour = -2;
				}
			}
			else if (buffer == "flatMagicArmour") {
				flatMagicArmour = numFromFile(&classBlueprints);
			}
			else if (buffer == "propMagicArmour") {
				propMagicArmour = floatFromFile(&classBlueprints);
				if (propArmour < -1) {
					propArmour = -2;
				}
			}
			else if (buffer == "flatDamageModifier") {
				flatDamageModifier = numFromFile(&classBlueprints);
			}
			else if (buffer == "propDamageModifier") {
				propDamageModifier = floatFromFile(&classBlueprints);
				if (propDamageModifier < -1) {
					propDamageModifier = -2;
				}
			}
			else if (buffer == "flatMagicDamageModifier") {
				flatMagicDamageModifier = numFromFile(&classBlueprints);
			}
			else if (buffer == "propMagicDamageModifier") {
				propMagicDamageModifier = floatFromFile(&classBlueprints);
				if (propMagicDamageModifier < -1) {
					propMagicDamageModifier = -2;
				}
			}
			else if (buffer == "flatArmourPiercingDamageModifier") {
				flatArmourPiercingDamageModifier = numFromFile(&classBlueprints);
			}
			else if (buffer == "propArmourPiercingDamageModifier") {
				propArmourPiercingDamageModifier = floatFromFile(&classBlueprints);
				if (propArmourPiercingDamageModifier < -1) {
					propArmourPiercingDamageModifier = -2;
				}
			}
			else if (buffer == "evadeChance") {
				evadeChance = floatFromFile(&classBlueprints);
				if (evadeChance < 0) {
					evadeChance = -2;
				}
			}
			else if (buffer == "counterAttackChance") {
				counterAttackChance = floatFromFile(&classBlueprints);
				if (counterAttackChance < 0) {
					counterAttackChance = -2;
				}
			}
			else if (buffer == "bonusActions") {
				bonusActions = numFromFile(&classBlueprints);
			}
			else if (buffer == "initiative") {
				initiative = numFromFile(&classBlueprints);
			}
			else if (buffer == "maxXp") {
				maxXp = numFromFile(&classBlueprints);
			}
			else if (buffer == "nextLevel") {
				nextLevel = stringFromFile(&classBlueprints);
			}
			else if (buffer == "statPoints") {
				statPoints = numFromFile(&classBlueprints);
			}
			else if (buffer == "upgradePoints") {
				upgradePoints = numFromFile(&classBlueprints);
			}
			else {
				throw 1;
			}
			if (getTag(&classBlueprints) != '/' + buffer) {
				throw 1;
			}
			ignoreLine(&classBlueprints);
			buffer = getTag(&classBlueprints);
		}
		classBlueprints.close();
		if (maxXp == 0 || nextLevel == "" || nextLevel == "EMPTY") {
			maxXp = 0;
			nextLevel = "";
		}
	}
	catch (int err) {
		classBlueprints.close();
		switch (err) {
		case 4:
			if (custom) {
				loadFromFile(blueprint, false);
				return;
			}
		default:
			throw err;
		}
	}
}

void player::modifyInitiative(short i) {
	if (i > 0) {
		if (initiative > SHRT_MAX - i) {
			initiative = SHRT_MAX;
		}
		else {
			initiative += i;
		}
	}
	else if (i < 0) {
		if (initiative < SHRT_MIN - i) {
			initiative = SHRT_MIN;
		}
		else {
			initiative += i;
		}
	}
}

void player::calculateInitiative() {
	initiative = initiativeBase;
	modifyInitiative(helmet.getInitiativeModifier());
	modifyInitiative(chestPlate.getInitiativeModifier());
	modifyInitiative(greaves.getInitiativeModifier());
	modifyInitiative(boots.getInitiativeModifier());
}

void player::upgradeStats(short upgradeNum) {
	if (upgradeNum == 0) {
		return;
	}
	if (upgradeNum > 0) {
		cout << upgradeNum << " stat upgrade points acquired\n";
	}
	else {
		cout << -upgradeNum << " stat downgrade points acquired\n";
	}
	vector<short> upgradeChoices(1, 0);
	for (short i = upgradeNum; i > 0; i--) {
		upgradeChoices.resize(1);
		cout << upgradeNum << " stat points remaining\n";
		if (maxHealthBase < SHRT_MAX) {
			cout << "To upgrade maximum health, enter 1. (+10 maximum health)\n";
			upgradeChoices.push_back(1);
		}
		if (maxManaBase < SHRT_MAX) {
			cout << "To upgrade maximum " << g_manaName.plural() << ", enter 2. (+10 maximum " << g_manaName.plural() << ")\n";
			upgradeChoices.push_back(2);
		}
		if (turnManaRegenBase < SHRT_MAX || battleManaRegenBase < SHRT_MAX) {
			cout << "To upgrade " << g_manaName.plural() << " regeneration, enter 3. (+5 " << g_manaName.plural() << " per turn and +10 " << g_manaName.plural() << " at end of battle)\n";
			upgradeChoices.push_back(3);
		}
		if (flatArmourBase < SHRT_MAX || flatMagicArmourBase < SHRT_MAX) {
			cout << "To upgrade armour, enter 4. (+1 physical and magic armour)\n";
			upgradeChoices.push_back(4);
		}
		if (flatDamageModifierBase < SHRT_MAX || flatMagicDamageModifierBase < SHRT_MAX || flatArmourPiercingDamageModifierBase < SHRT_MAX) {
			cout << "To upgrade damage, enter 5. (+1 physical, magic and armour piercing damage)\n";
			upgradeChoices.push_back(5);
		}
		if (initiativeBase < SHRT_MAX) {
			cout << "To upgrade speed, enter 6. (+1 speed)\n";
			upgradeChoices.push_back(6);
		}
		if (upgradeChoices.size() == 1) {
			cout << "All upgradeable stats are already at maximum!\n";
			return;
		}
		cout << "To upgrade nothing, enter 0.\n";
		switch (userChoice(upgradeChoices)) {
		case 1: //Max health
			if (maxHealthBase > SHRT_MAX - 10) {
				maxHealthBase = SHRT_MAX;
			}
			else {
				maxHealthBase += 10;
			}
			break;
		case 2: //Max mana
			if (maxManaBase > SHRT_MAX - 10) {
				maxManaBase = SHRT_MAX;
			}
			else {
				maxManaBase += 10;
			}
			break;
		case 3: //Mana regen
			if (turnManaRegenBase > SHRT_MAX - 5) {
				turnManaRegenBase = SHRT_MAX;
			}
			else {
				turnManaRegenBase += 5;
			}
			if (battleManaRegenBase > SHRT_MAX - 10) {
				battleManaRegenBase = SHRT_MAX;
			}
			else {
				battleManaRegenBase += 10;
			}
			break;
		case 4: //Armour
			if (flatArmourBase < SHRT_MAX) {
				flatArmourBase++;
			}
			if (flatMagicArmourBase < SHRT_MAX) {
				flatMagicArmourBase++;
			}
			break;
		case 5: //Damage
			if (flatDamageModifierBase < SHRT_MAX) {
				flatDamageModifierBase++;
			}
			if (flatMagicDamageModifierBase < SHRT_MAX) {
				flatMagicDamageModifierBase++;
			}
			if (flatArmourPiercingDamageModifierBase < SHRT_MAX) {
				flatArmourPiercingDamageModifierBase++;
			}
			break;
		case 6: //Speed
			if (initiativeBase < SHRT_MAX) {
				initiativeBase++;
			}
			break;
		}
	}
	for (short i = upgradeNum; i < 0; i++) {
		upgradeChoices.resize(0);
		cout << upgradeNum << " stat downgrade points remaining\n";
		if (maxHealthBase > SHRT_MIN) {
			cout << "To downgrade maximum health, enter 1. (-10 maximum health)\n";
			upgradeChoices.push_back(1);
		}
		if (maxManaBase > SHRT_MIN) {
			cout << "To downgrade maximum " << g_manaName.plural() << ", enter 2. (-10 maximum " << g_manaName.plural() << ")\n";
			upgradeChoices.push_back(2);
		}
		if (turnManaRegenBase > SHRT_MIN || battleManaRegenBase > SHRT_MIN) {
			cout << "To downgrade " << g_manaName.plural() << " regeneration, enter 3. (-5 " << g_manaName.plural() << " per turn and -10 " << g_manaName.plural() << " at end of battle)\n";
			upgradeChoices.push_back(3);
		}
		if (flatArmourBase > SHRT_MIN || flatMagicArmourBase > SHRT_MIN) {
			cout << "To downgrade armour, enter 4. (-1 physical and magic armour)\n";
			upgradeChoices.push_back(4);
		}
		if (flatDamageModifierBase > SHRT_MIN || flatMagicDamageModifierBase > SHRT_MIN || flatArmourPiercingDamageModifierBase > SHRT_MIN) {
			cout << "To downgrade damage, enter 5. (-1 physical, magic and armour piercing damage)\n";
			upgradeChoices.push_back(5);
		}
		if (initiativeBase > SHRT_MIN) {
			cout << "To downgrade speed, enter 6. (-1 speed)\n";
			upgradeChoices.push_back(6);
		}
		if (upgradeChoices.empty()) {
			cout << "All upgradeable stats are already at minimum!\n";
			return;
		}
		switch (userChoice(upgradeChoices)) {
		case 1: //Max health
			if (maxHealthBase < SHRT_MIN + 10) {
				maxHealthBase = SHRT_MIN;
			}
			else {
				maxHealthBase -= 10;
			}
			break;
		case 2: //Max mana
			if (maxManaBase < SHRT_MIN + 10) {
				maxManaBase = SHRT_MIN;
			}
			else {
				maxManaBase -= 10;
			}
			break;
		case 3: //Mana regen
			if (turnManaRegenBase < SHRT_MIN + 5) {
				turnManaRegenBase = SHRT_MIN;
			}
			else {
				turnManaRegenBase -= 5;
			}
			if (battleManaRegenBase < SHRT_MIN + 10) {
				battleManaRegenBase = SHRT_MIN;
			}
			else {
				battleManaRegenBase -= 10;
			}
			break;
		case 4: //Armour
			if (flatArmourBase > SHRT_MIN) {
				flatArmourBase--;
			}
			if (flatMagicArmourBase > SHRT_MIN) {
				flatMagicArmourBase--;
			}
			break;
		case 5: //Damage
			if (flatDamageModifierBase > SHRT_MIN) {
				flatDamageModifierBase--;
			}
			if (flatMagicDamageModifierBase > SHRT_MIN) {
				flatMagicDamageModifierBase--;
			}
			if (flatArmourPiercingDamageModifierBase > SHRT_MIN) {
				flatArmourPiercingDamageModifierBase--;
			}
			break;
		case 6: //Speed
			if (initiativeBase > SHRT_MIN) {
				initiativeBase--;
			}
			break;
		}
	}
	this_thread::sleep_for(chrono::milliseconds(100));
}

void player::save(ofstream* saveFile) {
	*saveFile << "<player>\n";
		*saveFile << "\t<level>" << level << "</level>\n";
		*saveFile << "\t<className>" << addEscapes(className) << "</className>\n";
		*saveFile << "\t<maxHealth>" << maxHealthBase << "</maxHealth>\n";
		*saveFile << "\t<maxMana>" << maxManaBase << "</maxMana>\n";
		if (turnManaRegenBase != 5) {
			*saveFile << "\t<turnManaRegen>" << turnManaRegenBase << "</turnManaRegen>\n";
		}
		if (battleManaRegenBase != 10) {
			*saveFile << "\t<battleManaRegen>" << battleManaRegenBase << "</battleManaRegen>\n";
		}
		if (poisonResistBase != 0.1f) {
			*saveFile << "\t<poisonResist>" << poisonResistBase << "</poisonResist>\n";
		}
		if (bleedResistBase != 0.1f) {
			*saveFile << "\t<bleedResist>" << bleedResistBase << "</bleedResist>\n";
		}
		if (turnRegenBase != 0) {
			*saveFile << "\t<turnRegen>" << turnRegenBase << "</turnRegen>\n";
		}
		if (battleRegenBase != 0) {
			*saveFile << "\t<battleRegen>" << battleRegenBase << "</battleRegen>\n";
		}
		*saveFile << "\t<weapons count=\"" << weapons.size() << "\" projectiles=\"" << projectiles << "\">\n";
		for (uint8_t i = 0; i < weapons.size(); i++) {
			weapons[i].save(saveFile);
		}
		*saveFile << "\t</weapons>\n";
		*saveFile << "\t<spells count=\"" << spells.size() << "\">\n";
		for (uint8_t i = 0; i < spells.size(); i++) {
			spells[i].save(saveFile);
		}
		*saveFile << "\t</spells>\n";
		if (flatArmourBase != 0) {
			*saveFile << "\t<flatArmour>" << flatArmourBase << "</flatArmour>\n";
		}
		if (propArmourBase != 0) {
			*saveFile << "\t<propArmour>" << propArmourBase << "</propArmour>\n";
		}
		if (flatMagicArmourBase != 0) {
			*saveFile << "\t<flatMagicArmour>" << flatMagicArmourBase << "</flatMagicArmour>\n";
		}
		if (propMagicArmourBase != 0) {
			*saveFile << "\t<propMagicArmour>" << propMagicArmourBase << "</propMagicArmour>\n";
		}
		*saveFile << "\t<armour>\n";
		helmet.save(saveFile);
		chestPlate.save(saveFile);
		greaves.save(saveFile);
		boots.save(saveFile);
		*saveFile << "\t</armour>\n";
		if (flatDamageModifierBase != 0) {
			*saveFile << "\t<flatDamageModifier>" << flatDamageModifierBase << "</flatDamageModifier>\n";
		}
		if (propDamageModifierBase != 0) {
			*saveFile << "\t<propDamageModifier>" << propDamageModifierBase << "</propDamageModifier>\n";
		}
		if (flatMagicDamageModifierBase != 0) {
			*saveFile << "\t<flatMagicDamageModifier>" << flatMagicDamageModifierBase << "</flatMagicDamageModifier>\n";
		}
		if (propMagicDamageModifierBase != 0) {
			*saveFile << "\t<propMagicDamageModifier>" << propMagicDamageModifierBase << "</propMagicDamageModifier>\n";
		}
		if (flatArmourPiercingDamageModifierBase != 0) {
			*saveFile << "\t<flatArmourPiercingDamageModifier>" << flatArmourPiercingDamageModifierBase << "</flatArmourPiercingDamageModifier>\n";
		}
		if (propArmourPiercingDamageModifierBase != 0) {
			*saveFile << "\t<propArmourPiercingDamageModifier>" << propArmourPiercingDamageModifierBase << "</propArmourPiercingDamageModifier>\n";
		}
		if (evadeChanceBase != 0.1f) {
			*saveFile << "\t<evadeChance>" << evadeChanceBase << "</evadeChance>\n";
		}
		if (counterAttackChanceBase != 0.1f) {
			*saveFile << "\t<counterAttackChance>" << counterAttackChanceBase << "</counterAttackChance>\n";
		}
		if (bonusActionsBase != 1) {
			*saveFile << "\t<bonusActions>" << +bonusActionsBase << "</bonusActions>\n";
		}
		if (initiativeBase != 10) {
			*saveFile << "\t<initiative>" << initiativeBase << "</initiative>\n";
		}
		*saveFile << "\t<xp>" << xp << "</xp>\n";
		*saveFile << "\t<maxXp>" << maxXp << "</maxXp>\n";
		*saveFile << "\t<nextLevel>" << addEscapes(nextLevel) << "</nextLevel>\n";
		*saveFile << "\t<health>" << health << "</health>\n";
		*saveFile << "\t<mana>" << mana << "</mana>\n";
	*saveFile << "</player>\n\n";
}

void player::loadSave(ifstream* saveFile) {
	string buffer = getTag(saveFile);
	if (buffer != "player") {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer != "level") {
		throw 1;
	}
	*saveFile >> level;
	if (getTag(saveFile) != '/' + buffer) {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer != "className") {
		throw 1;
	}
	className = stringFromFile(saveFile);
	if (getTag(saveFile) != '/' + buffer) {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer != "maxHealth") {
		throw 1;
	}
	*saveFile >> maxHealthBase;
	if (getTag(saveFile) != '/' + buffer) {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer != "maxMana") {
		throw 1;
	}
	*saveFile >> maxManaBase;
	if (getTag(saveFile) != '/' + buffer) {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer == "turnManaRegen") {
		*saveFile >> turnManaRegenBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		turnManaRegenBase = 5;
	}
	if (buffer == "battleManaRegen") {
		*saveFile >> battleManaRegenBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		battleManaRegenBase = 10;
	}
	if (buffer == "poisonResist") {
		*saveFile >> poisonResistBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		poisonResistBase = 0.1f;
	}
	if (buffer == "bleedResist") {
		*saveFile >> bleedResistBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		bleedResistBase = 0.1f;
	}
	if (buffer == "turnRegen") {
		*saveFile >> turnRegenBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		turnRegenBase = 0;
	}
	if (buffer == "battleRegen") {
		*saveFile >> battleRegenBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		battleRegenBase = 0;
	}
	if (buffer.substr(0, 15) != "weapons count=\"") {
		throw 1;
	}
	buffer.erase(0, 15);
	short numBuf = numFromString(&buffer);
	if (numBuf > 255) {
		throw 1;
	}
	if (buffer.substr(0, 15) != "\" projectiles=\"") {
		throw 1;
	}
	buffer.erase(0, 15);
	projectiles = numFromString(&buffer);
	if (buffer != "\"") {
		throw 1;
	}
	ignoreLine(saveFile);
	for (uint8_t i = 0; i < numBuf; i++) {
		weapons.emplace_back(saveFile);
	}
	if (getTag(saveFile) != "/weapons") {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer.substr(0, 14) != "spells count=\"") {
		throw 1;
	}
	buffer.erase(0, 14);
	numBuf = numFromString(&buffer);
	if (numBuf > 255) {
		throw 1;
	}
	if (buffer != "\"") {
		throw 1;
	}
	ignoreLine(saveFile);
	for (uint8_t i = 0; i < numBuf; i++) {
		spells.emplace_back(saveFile);
	}
	if (getTag(saveFile) != "/spells") {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer == "flatArmour") {
		*saveFile >> flatArmourBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatArmourBase = 0;
	}
	if (buffer == "propArmour") {
		*saveFile >> propArmourBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		propArmourBase = 0;
	}
	if (buffer == "flatMagicArmour") {
		*saveFile >> flatMagicArmourBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatMagicArmourBase = 0;
	}
	if (buffer == "propMagicArmour") {
		*saveFile >> propMagicArmourBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		propMagicArmourBase = 0;
	}
	if (buffer != "armour") {
		throw 1;
	}
	ignoreLine(saveFile);
	helmet.loadSave(saveFile);
	chestPlate.loadSave(saveFile);
	greaves.loadSave(saveFile);
	boots.loadSave(saveFile);
	if (getTag(saveFile) != "/armour") {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer == "flatDamageModifier") {
		*saveFile >> flatDamageModifierBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatDamageModifierBase = 0;
	}
	if (buffer == "propDamageModifier") {
		*saveFile >> propDamageModifierBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		propDamageModifierBase = 0;
	}
	if (buffer == "flatMagicDamageModifier") {
		*saveFile >> flatMagicDamageModifierBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatMagicDamageModifierBase = 0;
	}
	if (buffer == "propMagicDamageModifier") {
		*saveFile >> propMagicDamageModifierBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		propMagicDamageModifierBase = 0;
	}
	if (buffer == "flatArmourPiercingDamageModifier") {
		*saveFile >> flatArmourPiercingDamageModifierBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		flatArmourPiercingDamageModifierBase = 0;
	}
	if (buffer == "propArmourPiercingDamageModifier") {
		*saveFile >> propArmourPiercingDamageModifierBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		propArmourPiercingDamageModifierBase = 0;
	}
	if (buffer == "evadeChance") {
		*saveFile >> evadeChanceBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		evadeChanceBase = 0.1f;
	}
	if (buffer == "counterAttackChance") {
		*saveFile >> counterAttackChanceBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		counterAttackChanceBase = 0.1f;
	}
	if (buffer == "bonusActions") {
		*saveFile >> numBuf;
		bonusActionsBase = static_cast<int8_t>(numBuf);
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		bonusActionsBase = 1;
	}
	if (buffer == "initiative") {
		*saveFile >> initiativeBase;
		if (getTag(saveFile) != '/' + buffer) {
			throw 1;
		}
		ignoreLine(saveFile);
		buffer = getTag(saveFile);
	}
	else {
		initiativeBase = 10;
	}
	if (buffer != "xp") {
		throw 1;
	}
	*saveFile >> xp;
	if (getTag(saveFile) != '/' + buffer) {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer != "maxXp") {
		throw 1;
	}
	*saveFile >> maxXp;
	if (getTag(saveFile) != '/' + buffer) {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer != "nextLevel") {
		throw 1;
	}
	nextLevel = stringFromFile(saveFile);
	if (getTag(saveFile) != '/' + buffer) {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	calculateModifiers();
	if (buffer != "health") {
		throw 1;
	}
	*saveFile >> health;
	if (getTag(saveFile) != '/' + buffer) {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer != "mana") {
		throw 1;
	}
	*saveFile >> mana;
	if (getTag(saveFile) != '/' + buffer) {
		throw 1;
	}
	ignoreLine(saveFile);
	buffer = getTag(saveFile);
	if (buffer != "/player") {
		throw 1;
	}
	ignoreLine(saveFile);
}