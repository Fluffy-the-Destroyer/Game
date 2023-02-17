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

short player::flatDamage(short d, char t) {
	if (d > 0) { //Dealing damage
		if (t == 1) { //Normal damage
			d = max(0, d - flatArmour); //Apply flat damage reduction, prevent going negative
			d = static_cast<short>(d * (1 + propArmour)); //Apply damage multiplier
		}
		else if (t == 2) {
			d = max(0, d - flatMagicArmour);
			d = static_cast<short>(d * (1 + propMagicArmour));
		}
		short healthLoss = min(d, health); //Actual amount of health lost
		if (SHRT_MIN + d > health) { //Happens exactly if health would underflow
			health = SHRT_MIN; //Set to minimum value
		}
		else {
			health -= d;
		}
		return max((short)0, healthLoss);
	}
	else if (d < 0) { //Healing
		if (health - d > maxHealth || SHRT_MAX + d < health) { //Health would overflow or exceed max
			health = maxHealth;
		}
		else {
			health -= d;
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

void player::modifyPoison(short p, bool resist) {
	if (p > 255) {
		p = 255;
	}
	if (p > 0) {
		if (resist) {
			if (rng(0.f, 1.f) < poisonResist) {
				return;
			}
		}
		poison = min(255, poison + p);
	}
	else if (p < 0) {
		poison = max(0, poison + p);
	}
}

void player::modifyBleed(short b, bool resist) {
	if (b > 255) {
		b = 255;
	}
	if (b > 0) {
		if (resist) {
			if (rng(0.f, 1.f) < bleedResist) {
				return;
			}
		}
		bleed = min(255, bleed + b);
	}
	else if (b < 0) {
		bleed = max(0, bleed + b);
	}
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
	if (weaponSlots == 0) {
		cout << "Cannot equip weapon, no weapon slots\n";
		return;
	}
	cout << "Currently equipped weapons:\n";
	try {
		for (short i = 0; i < weaponSlots; i++) {
			cout << i + 1 << ": " << getWeapon(static_cast<unsigned char>(i))->getName() << '\n';
		}
	}
	catch (int) {
		cout << "An internal error occurred, stored number of weapon slots does not match size of weapons vector, resizing vector\n";
		weapons.resize(weaponSlots);
	}
	while (true) {
		cout << "Enter the number of the slot in which to equip the weapon.\nTo discard the weapon, enter 0.\n";
		try {
			unsigned char slot = userChoice(0, weaponSlots);
			if (slot == 0) {
				return;
			}
			cout << "Currently equipped:\n";
			getWeapon(slot - 1)->displayStats();
			cout << "New weapon:\n";
			w->displayStats();
			cout << "To equip the weapon in this slot, enter 1.\nTo choose a different slot, enter 2.\n";
			if (userChoice(1, 2) == 1) {
				weapons[slot - 1] = *w;
				return;
			}
		}
		catch (int err) {
			switch (err) {
			case 6:
				cout << "An internal error occurred, stored number of weapon slots does not match size of weapons vector, resizing vector\n";
				weapons.resize(weaponSlots);
				break;
			case 7:
				cout << "An internal error occurred\n";
				break;
			}
		}
	}
}

void player::equip(spell* s) {
	if (spellSlots == 0) {
		cout << "Cannot equip spell, no spell slots\n";
		return;
	}
	cout << "Currently equipped spells:\n";
	try {
		for (short i = 0; i < spellSlots; i++) {
			cout << i + 1 << ": " << getSpell(static_cast<unsigned char>(i))->getName() << '\n';
		}
	}
	catch (int) {
		cout << "An internal error occurred, stored number of spell slots does not match size of spells vector, resizing vector\n";
		spells.resize(spellSlots);
	}
	while (true) {
		cout << "Enter the number of the slot in which to equip the spell.\nTo discard the spell, enter 0.\n";
		try {
			unsigned char slot = userChoice(0, spellSlots);
			if (slot == 0) {
				return;
			}
			cout << "Currently equipped:\n";
			getSpell(slot - 1)->displayStats();
			cout << "New spell:\n";
			s->displayStats();
			cout << "To equip the spell in this slot, enter 1.\nTo choose a different slot, enter 2.\n";
			if (userChoice(1, 2) == 1) {
				spells[slot - 1] = *s;
				return;
			}
		}
		catch (int err) {
			switch (err) {
			case 6:
				cout << "An internal error occurred, stored number of spell slots does not match size of spells vector, resizing vector\n";
				spells.resize(spellSlots);
				break;
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
	if (i >= weaponSlots || i >= weapons.size()) {
		throw 6;
	}
	return &(weapons[i]);
}

spell* player::getSpell(unsigned char i) {
	if (i >= spellSlots || i >= spells.size()) {
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
				classBlueprints >> maxHealthBase;
			}
			else if (buffer == "maxMana") {
				classBlueprints >> maxManaBase;
			}
			else if (buffer == "turnManaRegen") {
				classBlueprints >> turnManaRegenBase;
			}
			else if (buffer == "battleManaRegen") {
				classBlueprints >> battleManaRegenBase;
			}
			else if (buffer == "poisonResist") {
				classBlueprints >> poisonResistBase;
				if (poisonResistBase < 0) {
					poisonResistBase = 0;
				}
			}
			else if (buffer == "bleedResist") {
				classBlueprints >> bleedResistBase;
				if (bleedResistBase < 0) {
					bleedResistBase = 0;
				}
			}
			else if (buffer == "constRegen") {
				classBlueprints >> constRegenBase;
			}
			else if (buffer == "battleRegen") {
				classBlueprints >> battleRegenBase;
			}
			else if (buffer.substr(0, 15) == "weapons count=\"") { //It's the weapons tag, count is how many slots there are, which varies, so only checking the beginning of the tag
				buffer.erase(0, 15); //Get rid of the stuff that has been checked
				weaponSlots = static_cast<unsigned char>(numFromString(&buffer)); //If the number is negative or bigger than 255, it will overflow or underflow, but the documentation will say it must be in this range so it is an error by whoever made the class
				weapons.resize(weaponSlots); //Set the number of slots and resize the vector appropriately, weaponSlots cannot go negative, so this will be fine
				if (weapons.size() != weaponSlots) { //Failed to resize weapons
					throw 1;
				}
				if (buffer.substr(0, 15) != "\" projectiles=\"") { //This should be next
					throw 1;
				}
				buffer.erase(0, 15);
				projectiles = static_cast<short>(numFromString(&buffer));
				if (buffer != "\"") {
					throw 1;
				}
				ignoreLine(&classBlueprints); //Go to next line
				for (int i = 0; i < weaponSlots; i++) {
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
						for (int j = i; j < weaponSlots; j++) { //Iterate over the current slot and all later ones
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
				spellSlots = static_cast<unsigned char>(numFromString(&buffer));
				spells.resize(spellSlots);
				if (spells.size() != spellSlots) {
					throw 1;
				}
				if (buffer != "\"") {
					throw 1;
				}
				ignoreLine(&classBlueprints);
				for (int i = 0; i < spellSlots; i++) {
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
						for (int j = i; j < spellSlots; j++) {
							spells[j].loadFromFile("EMPTY");
						}
						break;
					}
					else {
						throw 1;
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
			}
			else if (buffer == "flatArmour") {
				classBlueprints >> flatArmourBase;
			}
			else if (buffer == "propArmour") {
				classBlueprints >> propArmourBase;
				if (propArmourBase < -1) {
					propArmourBase = -1;
				}
			}
			else if (buffer == "flatMagicArmour") {
				classBlueprints >> flatMagicArmourBase;
			}
			else if (buffer == "propMagicArmour") {
				classBlueprints >> propMagicArmourBase;
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
				classBlueprints >> flatDamageModifierBase;
			}
			else if (buffer == "propDamageModifier") {
				classBlueprints >> propDamageModifierBase;
				if (propDamageModifierBase < -1) {
					propDamageModifierBase = -1;
				}
			}
			else if (buffer == "flatMagicDamageModifier") {
				classBlueprints >> flatMagicDamageModifierBase;
			}
			else if (buffer == "propMagicDamageModifier") {
				classBlueprints >> propMagicDamageModifierBase;
				if (propMagicDamageModifierBase < -1) {
					propMagicDamageModifierBase = -1;
				}
			}
			else if (buffer == "flatArmourPiercingDamageModifier") {
				classBlueprints >> flatArmourPiercingDamageModifierBase;
			}
			else if (buffer == "propArmourPiercingDamageModifier") {
				classBlueprints >> propArmourPiercingDamageModifierBase;
				if (propArmourPiercingDamageModifierBase < -1) {
					propArmourPiercingDamageModifierBase = -1;
				}
			}
			else if (buffer == "evadeChance") {
				classBlueprints >> evadeChanceBase;
				if (evadeChanceBase < 0) {
					evadeChanceBase = 0;
				}
			}
			else {
				throw 1;
			}
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
		try {
			for (short i = 0; i < weaponSlots; i++) {
				cout << i + 1 << ": " << getWeapon(static_cast<unsigned char>(i))->getName() << '\n';
			}
		}
		catch (int) {
			cout << "An internal error occurred, stored number of weapon slots does not match size of weapons vector, resizing vector\n";
			weapons.resize(weaponSlots);
		}
		cout << "Spells:\n";
		try {
			for (short i = 0; i < spellSlots; i++) {
				cout << i + 1 << ": " << getSpell(static_cast<unsigned char>(i))->getName() << '\n';
			}
		}
		catch (int) {
			cout << "An internal error occurred, stored number of spell slots does not match size of spells vector, resizing vector\n";
			spells.resize(spellSlots);
		}
		cout << "To view armour stats, enter 1.\nTo view weapon stats, enter 2.\nTo view spell stats, enter 3.\nTo view player stats, enter 4.\n";
		switch (userChoice(1, 3)) {
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
			if (weaponSlots == 0) {
				cout << "No weapon slots\n";
				break;
			}
			cout << "Enter the number of the weapon slot you wish to view\n";
			try {
				getWeapon(static_cast<unsigned char>(userChoice(1, weaponSlots) - 1))->displayStats(); //Using getWeapon as it has protection against attempts to access outside the weapons vector
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
			if (spellSlots == 0) {
				cout << "No spell slots\n";
				break;
			}
			cout << "Enter the number of the spell slot you wish to view\n";
			try {
				getSpell(static_cast<unsigned char>(userChoice(1, spellSlots) - 1))->displayStats();
			}
			catch (int err) {
				switch (err) {
				case 6:
					cout << "An internal error occurred, specified spell slot does not exist\n";
					break;
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
	for (unsigned char i = 0; i < spellSlots; i++) {
		spells[i].decCooldown();
	}
	currentBonusActions = bonusActions;
}

void player::decBonusActions() {
	if (currentBonusActions > 0) {
		currentBonusActions--;
	}
}

void player::reset() {
	modifyHealth(battleRegen);
	modifyMana(battleManaRegen);
	for (unsigned char i = 0; i < spellSlots; i++) {
		spells[i].resetCooldown();
	}
	calculateModifiers();
	cureBleed();
	curePoison();
	removeRegen();
}