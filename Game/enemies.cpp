#include "enemies.h"
#include <fstream>
#include "inputs.h"
#include "battle.h"
using namespace std;

//Error codes:
// 1: Bad XML
// 2: Blueprint or list not found
// 3: Loading empty enemy
// 4: Unable to open file
// 5: Empty list
// 6: Attempting to access slot out of range

short enemy::flatDamage(short p, short m, short a, bool overHeal) {
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

void enemy::propDamage(float d) {
	if (d > 0) {
		health = static_cast<short>(health * (1 - d));
	}
	else if (d < 0) {
		if (health + static_cast<short>(maxHealth * (-d)) > maxHealth || SHRT_MAX - static_cast<short>(maxHealth * (-d)) < health) {
			health = maxHealth;
		}
		else {
			health += static_cast<short>(maxHealth * (-d));
		}
	}
}

void enemy::modifyHealth(short h) {
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

void enemy::modifyMaxHealth(short m) {
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

void enemy::modifyProjectiles(short p) {
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

void enemy::modifyMana(short m) {
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

void enemy::modifyMaxMana(short m) {
	if (m > 0) { //Increase
		if (SHRT_MAX - m < maxMana) { //Overflow
			maxMana = SHRT_MAX;
		}
		else {
			maxMana += m;
		}
		modifyMana(m); //Add mana accordingly
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

void enemy::modifyTurnManaRegen(short m) {
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

bool enemy::modifyPoison(short p, bool resist) {
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

bool enemy::modifyBleed(short b, bool resist) {
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

void enemy::modifyTempRegen(short r) {
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

void enemy::modifyTurnRegen(short c) {
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

void enemy::modifyFlatArmour(short f) {
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

void enemy::modifyPropArmour(float p) {
	if (p < -1) { //Lower values not allowed
		p = -1;
	}
	propArmour = ((propArmour + 1) * (p + 1)) - 1;
}

void enemy::modifyFlatMagicArmour(short f) {
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

void enemy::modifyPropMagicArmour(float p) {
	if (p < -1) {
		p = -1;
	}
	propMagicArmour = ((propMagicArmour + 1) * (p + 1)) - 1;
}

void enemy::modifyFlatDamageModifier(short f) {
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

void enemy::modifyPropDamageModifier(float p) {
	if (p < -1) {
		p = -1;
	}
	propDamageModifier = ((propDamageModifier + 1) * (p + 1)) - 1;
}

void enemy::modifyEvadeChance(float e) {
	if (e < -1) {
		e = -1;
	}
	evadeChance *= (e + 1);
}

void enemy::modifyPoisonResist(float p) {
	if (p < -1) {
		p = -1;
	}
	poisonResist *= (p + 1);
}

void enemy::modifyBleedResist(float b) {
	if (b < -1) {
		b = -1;
	}
	bleedResist *= (b + 1);
}

void enemy::modifyFlatMagicDamageModifier(short f) {
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

void enemy::modifyPropMagicDamageModifier(float p) {
	if (p < -1) {
		p = -1;
	}
	propMagicDamageModifier = ((propMagicDamageModifier + 1) * (p + 1)) - 1;
}

void enemy::modifyFlatArmourPiercingDamageModifier(short f) {
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

void enemy::modifyPropArmourPiercingDamageModifier(float p) {
	if (p < -1) {
		p = -1;
	}
	propArmourPiercingDamageModifier = ((propArmourPiercingDamageModifier + 1) * (p + 1)) - 1;
}

weapon* enemy::getWeapon(uint8_t i) {
	if (i >= weapons.size()) {
		throw 6;
	}
	return &(weapons[i]);
}

spell* enemy::getSpell(uint8_t i) {
	if (i >= spells.size()) {
		throw 6;
	}
	return &(spells[i]);
}

void enemy::modifyCounterAttackChance(float c) {
	if (c < -1) {
		c = -1;
	}
	counterAttackChance *= (c + 1);
}

void enemy::modifyBonusActions(short b) {
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

void enemy::loadFromFile(string blueprint, bool custom) {
	ifstream enemyBlueprints;
	string buffer = "";
	short charBuf = 0;
	try {
		if (blueprint == "EMPTY") {
			throw 3;
		}
		if (custom) {
			enemyBlueprints.open("custom\\enemyBlueprints.xml");
		}
		else {
			enemyBlueprints.open("data\\enemyBlueprints.xml");
		}
		if (!enemyBlueprints.is_open()) {
			throw 4;
		}
		ignoreLine(&enemyBlueprints, '<');
		if (custom && enemyBlueprints.eof()) {
			throw 4;
		}
		enemyBlueprints.seekg(-1, ios_base::cur);
		string blueprintName = "enemyBlueprintList name=\"" + blueprint + '\"';
		bool customFile = custom;
		{
			bool noList = false;
			streampos filePos = 0;
			short listCount = -1;
			while (true) {
				while (buffer != blueprintName) {
					buffer = getTag(&enemyBlueprints);
					ignoreLine(&enemyBlueprints);
					if (enemyBlueprints.eof()) {
						enemyBlueprints.clear();
						noList = true;
						break;
					}
				}
				if (!noList) {
					filePos = enemyBlueprints.tellg();
					do {
						if (enemyBlueprints.eof()) {
							throw 1;
						}
						listCount++;
						buffer = getTag(&enemyBlueprints);
						ignoreLine(&enemyBlueprints);
					} while (buffer != "/enemyBlueprintList");
					enemyBlueprints.clear();
					if (listCount == 0) {
						throw 5;
					}
					listCount = rng(1, listCount);
					enemyBlueprints.seekg(filePos);
					for (int i = 1; i < listCount; i++) {
						ignoreLine(&enemyBlueprints);
					}
					if (getTag(&enemyBlueprints) != "name") {
						throw 1;
					}
					blueprint = stringFromFile(&enemyBlueprints);
					if (getTag(&enemyBlueprints) != "/name") {
						throw 1;
					}
					if (blueprint == "EMPTY") {
						throw 3;
					}
				}
				else if (customFile) {
					enemyBlueprints.close();
					enemyBlueprints.open("data\\enemyBlueprints.xml");
					if (!enemyBlueprints.is_open()) {
						enemyBlueprints.open("custom\\enemyBlueprints.xml");
						if (!enemyBlueprints.is_open()) {
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
			enemyBlueprints.close();
			enemyBlueprints.open("custom\\enemyBlueprints.xml");
			if (!enemyBlueprints.is_open()) {
				enemyBlueprints.open("data\\enemyBlueprints.xml");
				if (!enemyBlueprints.is_open()) {
					custom = false;
					throw 4;
				}
			}
			else {
				customFile = true;
			}
		}
		enemyBlueprints.seekg(0);
		buffer = "";
		real = true;
		name = introduction = "";
		maxHealth = projectiles = maxMana = turnManaRegen = turnRegen = flatArmour = flatMagicArmour = flatDamageModifier = flatMagicDamageModifier = flatArmourPiercingDamageModifier = 0;
		poison = bleed = tempRegen = 0;
		AIType = 2;
		poisonResist = bleedResist = evadeChance = counterAttackChance = 0.1f;
		weapons.resize(0);
		spells.resize(0);
		initialSpell = -1;
		propArmour = propMagicArmour = propDamageModifier = propMagicDamageModifier = propArmourPiercingDamageModifier = 0;
		bonusActions = 1;
		initiative = 10;
		noCounterWeapons.resize(0);
		noCounterSpells.resize(0);
		blueprintName = "enemyBlueprint name=\"" + blueprint + '\"';
		while (true) {
			while (buffer != blueprintName) {
				buffer = getTag(&enemyBlueprints);
				ignoreLine(&enemyBlueprints);
				if (enemyBlueprints.eof()) {
					break;
				}
			}
			if (enemyBlueprints.eof()) {
				if (customFile) {
					enemyBlueprints.close();
					enemyBlueprints.open("data\\enemyBlueprints.xml");
					if (!enemyBlueprints.is_open()) {
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
		buffer = getTag(&enemyBlueprints);
		while (buffer != "/enemyBlueprint") {
			if (buffer == "name") {
				name = stringFromFile(&enemyBlueprints);
			}
			else if (buffer == "introduction") {
				introduction = stringFromFile(&enemyBlueprints);
			}
			else if (buffer == "maxHealth") {
				maxHealth = numFromFile(&enemyBlueprints);
				if (maxHealth < 0) {
					maxHealth = 0;
				}
			}
			else if (buffer == "maxMana") {
				maxMana = numFromFile(&enemyBlueprints);
				if (maxMana < 0) {
					maxMana = 0;
				}
			}
			else if (buffer == "turnManaRegen") {
				turnManaRegen = numFromFile(&enemyBlueprints);
			}
			else if (buffer == "poisonResist") {
				poisonResist = floatFromFile(&enemyBlueprints);
				if (poisonResist < 0) {
					poisonResist = 0;
				}
			}
			else if (buffer == "bleedResist") {
				bleedResist = floatFromFile(&enemyBlueprints);
				if (bleedResist < 0) {
					bleedResist = 0;
				}
			}
			else if (buffer == "turnRegen") {
				turnRegen = numFromFile(&enemyBlueprints);
			}
			else if (buffer.substr(0, 21) == "weapons projectiles=\"") {
				buffer.erase(0, 21);
				projectiles = static_cast<short>(numFromString(&buffer));
				if (projectiles < 0) {
					projectiles = 0;
				}
				if (buffer != "\"") {
					throw 1;
				}
				ignoreLine(&enemyBlueprints);
				for (short i = 0; i <= 255; i++) {
					buffer = getTag(&enemyBlueprints);
					if (buffer == "weapon") {
						buffer = stringFromFile(&enemyBlueprints);
						if (enemyBlueprints.eof()) {
							throw 1;
						}
						weapons.emplace_back(buffer);
						if (getTag(&enemyBlueprints) != "/weapon") {
							throw 1;
						}
						ignoreLine(&enemyBlueprints);
					}
					else if (buffer == "/weapons") {
						ignoreLine(&enemyBlueprints);
						break;
					}
					else {
						throw 1;
					}
				}
				while (buffer != "/weapons") {
					buffer = getTag(&enemyBlueprints);
					ignoreLine(&enemyBlueprints);
				}
				buffer = getTag(&enemyBlueprints);
				continue;
			}
			else if (buffer == "spells") {
				ignoreLine(&enemyBlueprints);
				for (short i = 0; i <= 255; i++) {
					buffer = getTag(&enemyBlueprints);
					if (buffer.substr(0, 5) == "spell") {
						buffer.erase(0, 5);
						if (buffer == " initial=\"true\"") { //Indicates the enemy should use this spell on turn 1
							initialSpell = i; //If multiple spells have the initial flag, only the last one with it will be the initial spell
						}
						buffer = stringFromFile(&enemyBlueprints);
						if (enemyBlueprints.eof()) {
							throw 1;
						}
						spells.emplace_back(buffer);
						if (getTag(&enemyBlueprints) != "/spell") {
							throw 1;
						}
						ignoreLine(&enemyBlueprints);
					}
					else if (buffer == "/spells") {
						ignoreLine(&enemyBlueprints);
						break;
					}
					else {
						throw 1;
					}
				}
				while (buffer != "/spells") {
					buffer = getTag(&enemyBlueprints);
					ignoreLine(&enemyBlueprints);
				}
				buffer = getTag(&enemyBlueprints);
				continue;
			}
			else if (buffer == "deathSpell") {
				buffer = stringFromFile(&enemyBlueprints);
				deathSpell.loadFromFile(buffer);
				if (getTag(&enemyBlueprints) != "/deathSpell") {
					throw 1;
				}
				ignoreLine(&enemyBlueprints);
				buffer = getTag(&enemyBlueprints);
				continue;
			}
			else if (buffer == "flatArmour") {
				flatArmour = numFromFile(&enemyBlueprints);
			}
			else if (buffer == "propArmour") {
				propArmour = floatFromFile(&enemyBlueprints);
				if (propArmour < -1) {
					propArmour = -1;
				}
			}
			else if (buffer == "flatMagicArmour") {
				flatMagicArmour = numFromFile(&enemyBlueprints);
			}
			else if (buffer == "propMagicArmour") {
				propMagicArmour = floatFromFile(&enemyBlueprints);
				if (propMagicArmour < -1) {
					propMagicArmour = -1;
				}
			}
			else if (buffer == "flatDamageModifier") {
				flatDamageModifier = numFromFile(&enemyBlueprints);
			}
			else if (buffer == "propDamageModifier") {
				propDamageModifier = floatFromFile(&enemyBlueprints);
				if (propDamageModifier < -1) {
					propDamageModifier = -1;
				}
			}
			else if (buffer == "flatMagicDamageModifier") {
				flatMagicDamageModifier = numFromFile(&enemyBlueprints);
			}
			else if (buffer == "propMagicDamageModifier") {
				propMagicDamageModifier = floatFromFile(&enemyBlueprints);
				if (propMagicDamageModifier < -1) {
					propMagicDamageModifier = -1;
				}
			}
			else if (buffer == "flatArmourPiercingDamageModifier") {
				flatArmourPiercingDamageModifier = numFromFile(&enemyBlueprints);
			}
			else if (buffer == "propArmourPiercingDamageModifier") {
				propArmourPiercingDamageModifier = floatFromFile(&enemyBlueprints);
				if (propArmourPiercingDamageModifier < -1) {
					propArmourPiercingDamageModifier = -1;
				}
			}
			else if (buffer == "evadeChance") {
				evadeChance = floatFromFile(&enemyBlueprints);
				if (evadeChance < 0) {
					evadeChance = 0;
				}
			}
			else if (buffer == "counterAttackChance") {
				counterAttackChance = floatFromFile(&enemyBlueprints);
				if (counterAttackChance < 0) {
					counterAttackChance = 0;
				}
			}
			else if (buffer == "bonusActions") {
				charBuf = numFromFile(&enemyBlueprints);
				if (charBuf > 127) {
					charBuf = 127;
				}
				else if (charBuf < -127) {
					charBuf = -127;
				}
				bonusActions = static_cast<int8_t>(charBuf);
			}
			else if (buffer == "AIType") {
				charBuf = numFromFile(&enemyBlueprints);
				if (charBuf < 0 || charBuf > AI_TYPES_NO) {
					charBuf = 2;
				}
				AIType = static_cast<uint8_t>(charBuf);
			}
			else if (buffer == "initiative") {
				initiative = numFromFile(&enemyBlueprints);
				if (initiative < 0) {
					initiative = 0;
				}
			}
			else if (buffer == "xp") {
				xp = numFromFile(&enemyBlueprints);
			}
			else {
				throw 1;
			}
			if (getTag(&enemyBlueprints) != '/' + buffer) {
				throw 1;
			}
			ignoreLine(&enemyBlueprints);
			buffer = getTag(&enemyBlueprints);
		}
		enemyBlueprints.close();
		health = maxHealth;
		mana = maxMana;
		currentBonusActions = bonusActions;
	}
	catch (int err) {
		enemyBlueprints.close();
		real = false;
		name = introduction = "";
		switch (err) {
		case 1:
			cout << "Unable to parse enemy or list " << blueprint << '\n';
			break;
		case 2:
			cout << "Unable to find blueprint or list " << blueprint << '\n';
			break;
		case 4:
			if (custom) {
				loadFromFile(blueprint, false);
				return;
			}
			cout << "Unable to open file enemyBlueprints.xml\n";
			break;
		case 5:
			cout << "blueprintList " << blueprint << " is empty\n";
			break;
		}
	}
}

void enemy::turnStart() {
	modifyHealth(-(POISON_MULTIPLIER * poison + BLEED_MULTIPLIER * bleed));
	modifyHealth(turnRegen + REGEN_MULTIPLIER * tempRegen);
	if (health > maxHealth) {
		health = max(static_cast<int>(maxHealth), health - ENEMY_OVERHEAL_DECAY);
	}
	modifyMana(turnManaRegen);
	if (mana > maxMana) {
		mana = max(static_cast<int>(maxMana), mana - ENEMY_MANA_DECAY);
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
	uint8_t enemySpellSlots = static_cast<uint8_t>(spells.size());
	for (uint8_t i = 0; i < enemySpellSlots; i++) {
		spells[i].decCooldown();
	}
	if (bonusActions < 0) {
		currentBonusActions = 0;
	}
	else {
		currentBonusActions = bonusActions;
	}
}

//Enemy AI

uint8_t enemy::chooseAction(uint8_t* slot1, uint8_t* slot2, uint8_t timing, string itemName1, string itemName2, bool firstTurn) {
	uint8_t selection = 0; //For holding slot selection
	if (firstTurn && timing == 0) {
		if (initialSpell >= 0) { //If an initial spell is set, cast if possible
			selection = static_cast<uint8_t>(initialSpell);
			if (check(true, selection, 0, true)) { //Check if can cast initial spell
				*slot1 = selection;
				return 2;
			}
		}
	}
	switch (AIType) {
		//Combined types
	case 1:
	case 2:
	case 3:
		switch (timing) {
		case 0:
			if (healingCheck()) { //Healing check
				if (chooseSpell(2, slot1)) { //Choose healing spell
					return 2;
				}
				if (attackCheck()) { //Attack check
					switch (chooseAttack(slot1, slot2)) {
					case 1:
						return 1;
					case 2:
						return 2;
					case 3:
						currentBonusActions--;
						return 3;
					}
					//Cast utility spell
					if (chooseSpell(3, slot1)) {
						return 2;
					}
					//No survivable actions found, will have to suicide
					return chooseSuicide(slot1, slot2);
				}
				//Cast utility spell
				if (chooseSpell(3, slot1)) {
					return 2;
				}
				//Now try attack
				switch (chooseAttack(slot1, slot2)) {
				case 1:
					return 1;
				case 2:
					return 2;
				case 3:
					currentBonusActions--;
					return 3;
				}
				//No survivable actions found
				return chooseSuicide(slot1, slot2);
			}
			if (attackCheck()) { //Attack check
				switch (chooseAttack(slot1, slot2)) {
				case 1:
					return 1;
				case 2:
					return 2;
				case 3:
					currentBonusActions--;
					return 3;
				}
				//Couldn't find attack, make another healing check
				if (healingCheck()) {
					if (chooseSpell(2, slot1) || chooseSpell(3, slot1)) { //Look for healing spell, then utility
						return 2;
					}
					//Must suicide
					return chooseSuicide(slot1, slot2);
				}
				//Try utility, then healing
				if (chooseSpell(3, slot1) || chooseSpell(2, slot1)) {
					return 2;
				}
				//Suicide
				return chooseSuicide(slot1, slot2);
			}
			//Cast utility spell
			if (chooseSpell(3, slot1)) {
				return 2;
			}
			//Healing check
			if (healingCheck()) {
				if (chooseSpell(2, slot1)) {
					return 2;
				}
				//Look for attack
				switch (chooseAttack(slot1, slot2)) {
				case 1:
					return 1;
				case 2:
					return 2;
				case 3:
					currentBonusActions--;
					return 3;
				}
				return chooseSuicide(slot1, slot2);
			}
			//Look for attack
			switch (chooseAttack(slot1, slot2)) {
			case 1:
				return 1;
			case 2:
				return 2;
			case 3:
				currentBonusActions--;
				return 3;
			}
			if (chooseSpell(2, slot1)) {
				return 2;
			}
			//Suicide
			return chooseSuicide(slot1, slot2);
		case 4:
		case 1: //Responding to weapon attack
			if (currentBonusActions <= 0) {
				return 0;
			}
			if (checkCounter(1, itemName1) || (timing == 4 && checkCounter(1, itemName2))) { //Check if can be countered
				if (chooseWeaponCounterSpell(slot1, firstTurn)) { //Check for a spell to counter attack
					currentBonusActions--;
					return 2;
				}
			}
			//Healing check
			if (healingCheck()) {
				if (chooseSpell(2, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				//Make attack check
				if (attackCheck()) {
					if (chooseSpell(1, slot1, 1, false, firstTurn) || chooseSpell(3, slot1, 1, false, firstTurn)) {
						currentBonusActions--;
						return 2;
					}
					return 0;
				}
				if (chooseSpell(3, slot1, 1, false, firstTurn) || chooseSpell(1, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				return 0;
			}
			//Attack check
			if (attackCheck()) {
				if (chooseSpell(1, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				if (chooseSpell(3, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				return 0;
			}
			if (chooseSpell(3, slot1, 1, false, firstTurn)) {
				currentBonusActions--;
				return 2;
			}
			return 0;
		case 2: //Responding to spell
			if (currentBonusActions <= 0) {
				return 0;
			}
			if (checkCounter(2, itemName1)) {
				if (chooseSpellCounterSpell(slot1, firstTurn)) { //Check for a spell to counter attack
					currentBonusActions--;
					return 2;
				}
			}
			//Healing check
			if (healingCheck()) {
				if (chooseSpell(2, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				//Make attack check
				if (attackCheck()) {
					if (chooseSpell(1, slot1, 1, false, firstTurn) || chooseSpell(3, slot1, 1, false, firstTurn)) {
						currentBonusActions--;
						return 2;
					}
					return 0;
				}
				if (chooseSpell(3, slot1, 1, false, firstTurn) || chooseSpell(1, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				return 0;
			}
			//Attack check
			if (attackCheck()) {
				if (chooseSpell(1, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				if (chooseSpell(3, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				return 0;
			}
			if (chooseSpell(3, slot1, 1, false, firstTurn)) {
				currentBonusActions--;
				return 2;
			}
			return 0;
		case 3: //Counter attack
			if (currentBonusActions <= 0) {
				return 0;
			}
			if (healingCheck()) {
				if (chooseSpell(2, slot1, 3, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
			}
			if (attackCheck()) {
				switch (chooseAttack(slot1, slot2, 3, false, firstTurn)) {
				case 1:
					currentBonusActions--;
					return 1;
				case 2:
					currentBonusActions--;
					return 2;
				case 3:
					currentBonusActions -= 2;
					return 3;
				}
				if (chooseSpell(3, slot1, 3, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				return 0;
			}
			if (chooseSpell(3, slot1, 3, false, firstTurn)) {
				currentBonusActions--;
				return 2;
			}
			switch (chooseAttack(slot1, slot2, 3, false, firstTurn)) {
			case 1:
				currentBonusActions--;
				return 1;
			case 2:
				currentBonusActions--;
				return 2;
			case 3:
				currentBonusActions -= 2;
				return 3;
			}
			return 0;
		}
		break;
	case 4:
	case 5:
	case 6:
		switch (timing) {
		case 0:
			if (healingCheck()) {
				if (chooseSpell(2, slot1)) {
					return 2;
				}
				if (attackCheck()) {
					if (chooseSpell(1, slot1) || chooseSpell(3, slot1)) {
						return 2;
					}
				}
				else {
					if (chooseSpell(3, slot1) || chooseSpell(1, slot1)) {
						return 2;
					}
				}
				switch (chooseWeapon(slot1, slot2)) {
				case 1:
					return 1;
				case 3:
					currentBonusActions--;
					return 3;
				}
				return chooseSuicide(slot1, slot2);
			}
			if (attackCheck()) {
				if (chooseSpell(1, slot1)) {
					return 2;
				}
				if (healingCheck()) {
					if (chooseSpell(2, slot1) || chooseSpell(3, slot1)) {
						return 2;
					}
				}
				else {
					if (chooseSpell(3, slot1) || chooseSpell(2, slot1)) {
						return 2;
					}
				}
				switch (chooseWeapon(slot1, slot2)) {
				case 1:
					return 1;
				case 3:
					currentBonusActions--;
					return 3;
				}
				return chooseSuicide(slot1, slot2);
			}
			if (chooseSpell(3, slot1)) {
				return 2;
			}
			if (healingCheck()) {
				if (chooseSpell(2, slot1) || chooseSpell(1, slot1)) {
					return 2;
				}
			}
			else {
				if (chooseSpell(1, slot1) || chooseSpell(2, slot1)) {
					return 2;
				}
			}
			switch (chooseWeapon(slot1, slot2)) {
			case 1:
				return 1;
			case 3:
				currentBonusActions--;
				return 3;
			}
			return chooseSuicide(slot1, slot2);
		case 4:
		case 1: //Responding to weapon attack
			if (currentBonusActions <= 0) {
				return 0;
			}
			if (checkCounter(1, itemName1) || (timing == 4 && checkCounter(1, itemName2))) {
				if (chooseWeaponCounterSpell(slot1, firstTurn)) { //Check for a spell to counter attack
					currentBonusActions--;
					return 2;
				}
			}
			//Healing check
			if (healingCheck()) {
				if (chooseSpell(2, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				//Make attack check
				if (attackCheck()) {
					if (chooseSpell(1, slot1, 1, false, firstTurn) || chooseSpell(3, slot1, 1, false, firstTurn)) {
						currentBonusActions--;
						return 2;
					}
					return 0;
				}
				if (chooseSpell(3, slot1, 1, false, firstTurn) || chooseSpell(1, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				return 0;
			}
			//Attack check
			if (attackCheck()) {
				if (chooseSpell(1, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				if (chooseSpell(3, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				return 0;
			}
			if (chooseSpell(3, slot1, 1, false, firstTurn)) {
				currentBonusActions--;
				return 2;
			}
			return 0;
		case 2: //Responding to spell
			if (currentBonusActions <= 0) {
				return 0;
			}
			if (checkCounter(2, itemName1)) {
				if (chooseSpellCounterSpell(slot1, firstTurn)) { //Check for a spell to counter attack
					currentBonusActions--;
					return 2;
				}
			}
			//Healing check
			if (healingCheck()) {
				if (chooseSpell(2, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				//Make attack check
				if (attackCheck()) {
					if (chooseSpell(1, slot1, 1, false, firstTurn) || chooseSpell(3, slot1, 1, false, firstTurn)) {
						currentBonusActions--;
						return 2;
					}
					return 0;
				}
				if (chooseSpell(3, slot1, 1, false, firstTurn) || chooseSpell(1, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				return 0;
			}
			//Attack check
			if (attackCheck()) {
				if (chooseSpell(1, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				if (chooseSpell(3, slot1, 1, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
				return 0;
			}
			if (chooseSpell(3, slot1, 1, false, firstTurn)) {
				currentBonusActions--;
				return 2;
			}
			return 0;
		case 3: //Counter attack
			if (currentBonusActions <= 0) {
				return 0;
			}
			if (healingCheck()) {
				if (chooseSpell(2, slot1, 3, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
			}
			if (attackCheck()) {
				if (chooseSpell(1, slot1, 3, false, firstTurn) || chooseSpell(3, slot1, 3, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
			}
			else {
				if (chooseSpell(3, slot1, 3, false, firstTurn) || chooseSpell(1, slot1, 3, false, firstTurn)) {
					currentBonusActions--;
					return 2;
				}
			}
			switch (chooseWeapon(slot1, slot2, 3, false, firstTurn)) {
			case 1:
				currentBonusActions--;
				return 1;
			case 3:
				currentBonusActions -= 2;
				return 3;
			}
			return 0;
		}
		break;
	case 7: //Melee berserker
		switch (timing) {
		case 0:
			switch (chooseWeapon(slot1, slot2)) {
			case 1:
				return 1;
			case 3:
				currentBonusActions--;
				return 3;
			}
			return chooseSuicide(slot1, slot2);
		case 3:
			if (currentBonusActions <= 0) {
				return 0;
			}
			switch (chooseWeapon(slot1, slot2, 3, false, firstTurn)) {
			case 1:
				currentBonusActions--;
				return 1;
			case 3:
				currentBonusActions -= 2;
				return 3;
			}
			return 0;
		}
		break;
	}
	return 0;
}

bool enemy::check(bool type, uint8_t slot, uint8_t timing, bool kamikaze, bool firstTurn) {
	short currentHealth = health, currentMana = mana, currentConstRegen = turnRegen, currentTurnManaRegen = turnManaRegen, currentMaxMana = maxMana, currentMaxHealth = maxHealth, currentProjectiles = projectiles; //For holding current values while actions are simulated
	uint8_t currentBleed = bleed, currentPoison = poison, currentTempRegen = tempRegen; //For holding current values while actions are simulated
	switch (type) {
	case false: //Weapon
		//Check weapon exists
		if (slot >= weapons.size() || !weapons[slot].getReal()) { //Referenced weapon is out of range or not real
			return false;
		}
		switch (timing) { //Check weapon can be used at the current time
		case 0:
			if (weapons[slot].getHitCount() == 0) { //No regular attacks
				return false;
			}
			break;
		case 1: //Responding to weapon
		case 2: //Responding to spell
			return false;
		case 3: //Counter attack
			if (weapons[slot].getCounterHits() == 0) {
				return false;
			}
			break;
		default: //Nonsense parameter value
			return false;
		}
		//Health cost
		if (weapons[slot].getHealthChange() < 0 && (health + weapons[slot].getHealthChange() < 0 || !kamikaze && health + weapons[slot].getHealthChange() == 0)) { //Cannot afford cost, or would die and kamikaze is false
			return false;
		}
		//Mana cost
		if (weapons[slot].getManaChange() < 0 && mana + weapons[slot].getManaChange() < 0) { //Cannot afford mana cost
			return false;
		}
		//Projectile cost
		if (weapons[slot].getProjectileChange() < 0 && projectiles + weapons[slot].getProjectileChange() < 0) { //Cannot afford projectile cost
			return false;
		}
		//Can afford costs and is allowed to use at this timing
		if (kamikaze) { //If unconcerned with death, fine to use
			return true;
		}
		//Apply costs and maximum possible self damage
		modifyHealth(weapons[slot].getHealthChange());
		propDamage(weapons[slot].getPropSelfDamage());
		flatDamage(weapons[slot].getFlatSelfDamageMax(), weapons[slot].getFlatSelfMagicDamageMax(), weapons[slot].getFlatSelfArmourPiercingDamageMax(), weapons[slot].getSelfOverHeal());
		modifyMana(weapons[slot].getManaChange());
		modifyProjectiles(weapons[slot].getProjectileChange());
		if (health <= 0) { //Would die
			health = currentHealth;
			mana = currentMana;
			projectiles = currentProjectiles;
			return false;
		}
		//Check if would die next turn to poison/bleed, accounting for regeneration and assuming no resisting
		modifyPoison(weapons[slot].getSelfPoison(), false);
		modifyBleed(weapons[slot].getSelfBleed(), false);
		simulateTurn();
		if (health <= 0) { //Would die next turn, revert health and status effects
			health = currentHealth;
			poison = currentPoison;
			bleed = currentBleed;
			mana = currentMana;
			projectiles = currentProjectiles;
			return false;
		}
		if (firstTurn && initialSpell >= 0) {
			if (!check(true, static_cast<uint8_t>(initialSpell), 0, true)) { //Check whether will be able to cast initial spell next turn
				//Revert
				health = currentHealth;
				poison = currentPoison;
				bleed = currentBleed;
				mana = currentMana;
				projectiles = currentProjectiles;
				return false;
			}
		}
		health = currentHealth;
		poison = currentPoison;
		bleed = currentBleed;
		mana = currentMana;
		projectiles = currentProjectiles;
		return true;
		break;
	case true: //Spell
		//Check spell exists
		if (slot >= spells.size() || !spells[slot].getReal()) {
			return false;
		}
		//Check cooldown
		if (spells[slot].getCurrentCooldown() > 0) {
			return false;
		}
		//Check timing
		switch (timing) {
		case 0:
			if (spells[slot].getTiming() == 2) {
				return false;
			}
			break;
		case 1:
		case 2:
			if (spells[slot].getTiming() == 0) {
				return false;
			}
			break;
		case 3:
			if (spells[slot].getCounterHits() == 0) {
				return false;
			}
			break;
		default:
			return false;
		}
		//Check costs
		if (spells[slot].getHealthChange() < 0 && (health + spells[slot].getHealthChange() < 0 || !kamikaze && health + spells[slot].getHealthChange() == 0)) {
			return false;
		}
		if (spells[slot].getManaChange() < 0 && mana + spells[slot].getManaChange() < 0) {
			return false;
		}
		if (spells[slot].getProjectileChange() < 0 && projectiles + spells[slot].getProjectileChange() < 0) {
			return false;
		}
		//Kamikaze check
		if (kamikaze) {
			return true;
		}
		if (firstTurn && slot == initialSpell) {
			return true;
		}
		//Apply self damage/health cost
		modifyHealth(spells[slot].getHealthChange());
		propDamage(spells[slot].getPropSelfDamage());
		flatDamage(spells[slot].getFlatSelfDamageMax(), spells[slot].getFlatSelfMagicDamageMax(), spells[slot].getFlatSelfArmourPiercingDamageMax(), spells[slot].getSelfOverHeal());
		modifyMaxHealth(spells[slot].getMaxHealthModifier());
		if (health <= 0) { //Would die
			health = currentHealth;
			maxHealth = currentMaxHealth;
			return false;
		}
		//Apply poison/bleed/regen and simulate a turn
		modifyPoison(spells[slot].getSelfPoison(), false);
		modifyBleed(spells[slot].getSelfBleed(), false);
		modifyTempRegen(spells[slot].getTempRegenSelf());
		modifyTurnRegen(spells[slot].getTurnRegenModifier());
		modifyTurnManaRegen(spells[slot].getTurnManaRegenModifier());
		modifyMana(spells[slot].getManaChange());
		modifyMaxMana(spells[slot].getMaxManaModifier());
		modifyProjectiles(spells[slot].getProjectileChange());
		simulateTurn();
		if (health <= 0) { //Would die next turn
			health = currentHealth;
			maxHealth = currentMaxHealth;
			poison = currentPoison;
			bleed = currentBleed;
			tempRegen = currentTempRegen;
			turnRegen = currentConstRegen;
			turnManaRegen = currentTurnManaRegen;
			mana = currentMana;
			maxMana = currentMaxMana;
			projectiles = currentProjectiles;
			return false;
		}
		if (firstTurn && initialSpell >= 0) {
			if (!check(true, static_cast<uint8_t>(initialSpell), 0, true)) {
				health = currentHealth;
				maxHealth = currentMaxHealth;
				poison = currentPoison;
				bleed = currentBleed;
				tempRegen = currentTempRegen;
				turnRegen = currentConstRegen;
				turnManaRegen = currentTurnManaRegen;
				mana = currentMana;
				maxMana = currentMaxMana;
				projectiles = currentProjectiles;
				return false;
			}
		}
		health = currentHealth;
		maxHealth = currentMaxHealth;
		poison = currentPoison;
		bleed = currentBleed;
		tempRegen = currentTempRegen;
		turnRegen = currentConstRegen;
		turnManaRegen = currentTurnManaRegen;
		mana = currentMana;
		maxMana = currentMaxMana;
		projectiles = currentProjectiles;
		return true;
		break;
	}
	return false;
}

void enemy::simulateTurn() {
	modifyHealth(-(POISON_MULTIPLIER * poison + BLEED_MULTIPLIER * bleed));
	modifyHealth(turnRegen + REGEN_MULTIPLIER * tempRegen);
	if (health > maxHealth) {
		health = max(static_cast<int>(maxHealth), health - ENEMY_OVERHEAL_DECAY);
	}
	modifyMana(turnManaRegen);
	if (mana > maxMana) {
		mana = max(static_cast<int>(maxMana), mana - ENEMY_MANA_DECAY);
	}
}

bool enemy::chooseSpell(uint8_t type, uint8_t* selection, uint8_t timing, bool kamikaze, bool firstTurn) {
	uint8_t spellSlots = static_cast<uint8_t>(spells.size());
	if (spellSlots == 0) {
		return false;
	}
	vector<uint8_t> possibleSpells; //Holds slot numbers of spells which could be chosen
	for (uint8_t i = 0; i < spellSlots; i++) {
		if (spells[i].checkSpellType(type) && check(true, i, timing, kamikaze, firstTurn)) { //If spell in slot i is a spell of correct type which could be cast, put that slot in possibleSpells
			possibleSpells.push_back(i);
		}
	}
	if (possibleSpells.empty()) { //No possible spells
		return false;
	}
	uint8_t possibleNumber = static_cast<uint8_t>(possibleSpells.size() - 1);
	*selection = possibleSpells[rng(0, possibleNumber)]; //Pick a random possible spell
	return true;
}

uint8_t enemy::chooseWeapon(uint8_t* selection1, uint8_t* selection2, uint8_t timing, bool kamikaze, bool firstTurn) {
	uint8_t weaponSlots = static_cast<uint8_t>(weapons.size());
	if (weaponSlots == 0) {
		return 0;
	}
	vector<uint8_t> possibleWeapons;
	for (uint8_t i = 0; i < weaponSlots; i++) {
		if (check(false, i, timing, kamikaze, firstTurn)) {
			possibleWeapons.push_back(i);
			if (!weapons[i].getDualWield()) { //If cannot dual wield, add it twice, as dual weapons can be picked as the first or second in a pair
				possibleWeapons.push_back(i);
			}
		}
	}
	if (possibleWeapons.empty()) {
		return 0;
	}
	short possibleNumber = static_cast<short>(possibleWeapons.size() - 1);
	*selection1 = possibleWeapons[rng(0, possibleNumber)];
	if (weapons[*selection1].getDualWield()) { //If the weapon allows dual wielding, look for a second weapon to use with it
		possibleWeapons.resize(0);
		for (uint8_t i = 0; i < weaponSlots; i++) {
			if (check(*selection1, i, timing, kamikaze, firstTurn)) {
				possibleWeapons.push_back(i);
			}
		}
		if (possibleWeapons.empty()) { //Nothing to dual wield with it
			return 1;
		}
		possibleNumber = static_cast<short>(possibleWeapons.size() - 1);
		*selection2 = possibleWeapons[rng(0, possibleNumber)];
		return 3;
	}
	return 1;
}

bool enemy::chooseWeaponCounterSpell(uint8_t* selection, bool firstTurn) {
	uint8_t spellSlots = static_cast<uint8_t>(spells.size());
	if (spellSlots == 0) {
		return false;
	}
	vector<uint8_t> possibleSpells;
	for (uint8_t i = 0; i < spellSlots; i++) {
		if ((spells[i].getCounterSpell() == 2 || spells[i].getCounterSpell() == 3) && check(true, i, 1, false, firstTurn)) {
			possibleSpells.push_back(i);
		}
	}
	if (possibleSpells.empty()) {
		return false;
	}
	short possibleNumber = static_cast<short>(possibleSpells.size() - 1);
	*selection = possibleSpells[rng(0, possibleNumber)];
	return true;
}

bool enemy::chooseSpellCounterSpell(uint8_t* selection, bool firstTurn) {
	uint8_t spellSlots = static_cast<uint8_t>(spells.size());
	if (spellSlots == 0) {
		return false;
	}
	vector<uint8_t> possibleSpells;
	for (uint8_t i = 0; i < spellSlots; i++) {
		if ((spells[i].getCounterSpell() == 1 || spells[i].getCounterSpell() == 3) && check(true, i, 2, false, firstTurn)) {
			possibleSpells.push_back(i);
		}
	}
	if (possibleSpells.empty()) {
		return false;
	}
	short possibleNumber = static_cast<short>(possibleSpells.size() - 1);
	*selection = possibleSpells[rng(0, possibleNumber)];
	return true;
}

uint8_t enemy::chooseAttack(uint8_t* selection1, uint8_t* selection2, uint8_t timing, bool kamikaze, bool firstTurn) {
	uint8_t spellSlots = static_cast<uint8_t>(spells.size()), weaponSlots = static_cast<uint8_t>(weapons.size());
	short slot;
	vector<short> possibleAttacks; //Holds possible slots, indexed from 1. Negatives are spells, positives are weapons, so a value of -2 is the spell in slot 1, a value of 1 is the weapon in slot 0
	for (uint8_t i = 0; i < weaponSlots; i++) {
		if (check(false, i, timing, kamikaze, firstTurn)) {
			possibleAttacks.push_back(i + 1);
			if (!weapons[i].getDualWield()) {
				possibleAttacks.push_back(i + 1);
			}
		}
	}
	for (uint8_t i = 0; i < spellSlots; i++) {
		if (spells[i].checkSpellType(1) && check(true, i, timing, kamikaze, firstTurn)) {
			possibleAttacks.push_back(-(i + 1));
			possibleAttacks.push_back(-(i + 1));
		}
	}
	if (possibleAttacks.empty()) {
		return 0;
	}
	short possibleNumber = static_cast<short>(possibleAttacks.size() - 1);
	slot = possibleAttacks[rng(0, possibleNumber)];
	if (slot > 0) { //Weapon
		*selection1 = static_cast<uint8_t>(slot - 1);
		if (weapons[*selection1].getDualWield()) {
			possibleAttacks.resize(0);
			for (uint8_t i = 0; i < weaponSlots; i++) {
				if (check(*selection1, i, timing, kamikaze, firstTurn)) {
					possibleAttacks.push_back(i);
				}
			}
			if (possibleAttacks.empty()) { //Nothing to dual wield with it
				return 1;
			}
			possibleNumber = static_cast<short>(possibleAttacks.size() - 1);
			*selection2 = static_cast<uint8_t>(possibleAttacks[rng(0, possibleNumber)]);
			return 3;
		}
		return 1;
	}
	else { //Spell
		*selection1 = static_cast<uint8_t>(-(slot + 1));
		return 2;
	}
}

uint8_t enemy::chooseSuicide(uint8_t* selection1, uint8_t* selection2) {
	uint8_t choice;
	switch (AIType) {
	case 1: //Combined types
	case 2:
	case 3:
		choice = chooseAttack(selection1, selection2, 0, true); //Look for an attack
		if (choice != 0) {
			return choice;
		}
		if (chooseSpell(3, selection1, 0, true) || chooseSpell(2, selection1, 0, true)) { //Look for utility, then healing
			return 2;
		}
		return 0;
	case 4: //Mage types
	case 5:
	case 6:
		choice = chooseSpell(1, selection1, 0, true); //Look for attack spell
		if (choice != 0) {
			return choice;
		}
		//Look for weapon
		switch (chooseWeapon(selection1, selection2, 0, true)) {
		case 1:
			return 1;
		case 3:
			currentBonusActions--;
			return 3;
		}
		//Look for utility, then healing
		if (chooseSpell(3, selection1, 0, true) || chooseSpell(2, selection1, 0, true)) {
			return 2;
		}
		return 0;
	case 7: //Melee berserker
		switch (chooseWeapon(selection1, selection2, 0, true)) {
		case 1:
			return 1;
		case 3:
			currentBonusActions--;
			return 3;
		}
		return 0;
	}
	return 0;
}

bool enemy::healingCheck() {
	float healthProp = health;
	healthProp /= maxHealth;
	if (healthProp >= AI_HEALING_THRESHOLD) {
		return false;
	}
	switch (AIType) {
	case 1:
	case 4:
		return (rng(0.f, 1.f) < powf(healthProp, 4));
	case 2:
	case 5:
		return (rng(0.f, 1.f) < powf(healthProp, 3));
	case 3:
	case 6:
		return (rng(0.f, 1.f) < powf(healthProp, 2));
	}
	return false;
}

bool enemy::attackCheck() {
	switch (AIType) {
	case 1:
	case 4:
		return (rng(0.f, 1.f) < 0.75f);
	case 2:
	case 5:
		return (rng(0.f, 1.f) < 0.5f);
	case 3:
	case 6:
		return (rng(0.f, 1.f) < 0.25f);
	}
	return true;
}

bool enemy::check(uint8_t weapon1, uint8_t weapon2, uint8_t timing, bool kamikaze, bool firstTurn) {
	short currentHealth = health, currentProjectiles = projectiles, currentMana = mana;
	uint8_t currentPoison = poison, currentBleed = bleed;
	if (weapon1 == weapon2) {
		return false;
	}
	//Check both weapons can dual wield
	if (!weapons[weapon2].getDualWield() || !weapons[weapon1].getDualWield()) {
		return false;
	}
	//This function will only be called when weapon1 has already been checked, so don't need to check it again
	if (max(weapon1, weapon2) >= weapons.size() || !weapons[weapon2].getReal()) {
		return false;
	}
	switch (timing) {
	case 0:
		if (currentBonusActions <= 0 || weapons[weapon2].getHitCount() == 0) {
			return false;
		}
		break;
	case 3:
		if (currentBonusActions <= 1 || weapons[weapon2].getCounterHits() == 0) {
			return false;
		}
		break;
	default:
		return false;
	}
	//Costs
	//Apply cost of weapon1
	modifyHealth(weapons[weapon1].getHealthChange());
	modifyMana(weapons[weapon1].getManaChange());
	modifyProjectiles(weapons[weapon1].getProjectileChange());
	//Check costs of weapon2
	if (weapons[weapon2].getHealthChange() < 0 && (health + weapons[weapon2].getHealthChange() < 0 || !kamikaze && health + weapons[weapon2].getHealthChange() == 0)) { //Cannot afford cost, or would die and kamikaze is false
		health = currentHealth;
		mana = currentMana;
		projectiles = currentProjectiles;
		return false;
	}
	//Mana cost
	if (weapons[weapon2].getManaChange() < 0 && mana + weapons[weapon2].getManaChange() < 0) { //Cannot afford mana cost
		health = currentHealth;
		mana = currentMana;
		projectiles = currentProjectiles;
		return false;
	}
	//Projectile cost
	if (weapons[weapon2].getProjectileChange() < 0 && projectiles + weapons[weapon2].getProjectileChange() < 0) { //Cannot afford projectile cost
		health = currentHealth;
		mana = currentMana;
		projectiles = currentProjectiles;
		return false;
	}
	//Can afford costs
	if (kamikaze) {
		health = currentHealth;
		mana = currentMana;
		projectiles = currentProjectiles;
		return true;
	}
	//Apply costs of weapon2
	modifyHealth(weapons[weapon2].getHealthChange());
	modifyMana(weapons[weapon2].getManaChange());
	modifyProjectiles(weapons[weapon2].getProjectileChange());
	//Apply max self damage
	if (weapons[weapon1].getPropSelfDamage() > 0) {
		propDamage(weapons[weapon2].getPropSelfDamage());
		propDamage(weapons[weapon1].getPropSelfDamage());
	}
	else {
		propDamage(weapons[weapon1].getPropSelfDamage());
		propDamage(weapons[weapon2].getPropSelfDamage());
	}
	if (weapons[weapon1].getSelfOverHeal()) {
		flatDamage(weapons[weapon2].getFlatSelfDamageMax(), weapons[weapon2].getFlatSelfMagicDamageMax(), weapons[weapon2].getFlatSelfArmourPiercingDamageMax(), weapons[weapon2].getSelfOverHeal());
		flatDamage(weapons[weapon1].getFlatSelfDamageMax(), weapons[weapon1].getFlatSelfMagicDamageMax(), weapons[weapon1].getFlatSelfArmourPiercingDamageMax(), true);
	}
	else {
		flatDamage(weapons[weapon1].getFlatSelfDamageMax(), weapons[weapon1].getFlatSelfMagicDamageMax(), weapons[weapon1].getFlatSelfArmourPiercingDamageMax());
		flatDamage(weapons[weapon2].getFlatSelfDamageMax(), weapons[weapon2].getFlatSelfMagicDamageMax(), weapons[weapon2].getFlatSelfArmourPiercingDamageMax(), weapons[weapon2].getSelfOverHeal());
	}
	if (health <= 0) {
		health = currentHealth;
		mana = currentMana;
		projectiles = currentProjectiles;
		return false;
	}
	//Apply poison/bleed and simulate a turn
	modifyPoison(weapons[weapon1].getSelfPoison(), false);
	modifyPoison(weapons[weapon2].getSelfPoison(), false);
	modifyBleed(weapons[weapon1].getSelfBleed(), false);
	modifyBleed(weapons[weapon2].getSelfBleed(), false);
	simulateTurn();
	if (health <= 0) {
		health = currentHealth;
		mana = currentMana;
		projectiles = currentProjectiles;
		poison = currentPoison;
		bleed = currentBleed;
		return false;
	}
	if (firstTurn && initialSpell >= 0) {
		if (!check(true, static_cast<uint8_t>(initialSpell), 0, true)) { //Check if will be able to cast initial spell next turn
			health = currentHealth;
			mana = currentMana;
			projectiles = currentProjectiles;
			poison = currentPoison;
			bleed = currentBleed;
			return false;
		}
	}
	health = currentHealth;
	mana = currentMana;
	projectiles = currentProjectiles;
	poison = currentPoison;
	bleed = currentBleed;
	return true;
}

void enemy::addNoCounter(uint8_t type, string itemName) {
	switch (type) {
	case 1:
		for (short i = 0; i < noCounterWeapons.size(); i++) {
			if (itemName == noCounterWeapons[i]) {
				return;
			}
		}
		noCounterWeapons.push_back(itemName);
		return;
	case 2:
		for (short i = 0; i < noCounterSpells.size(); i++) {
			if (itemName == noCounterSpells[i]) {
				return;
			}
		}
		noCounterSpells.push_back(itemName);
		return;
	}
}

bool enemy::checkCounter(uint8_t type, string itemName) {
	switch (type) {
	case 1:
		for (short i = 0; i < noCounterWeapons.size(); i++) {
			if (itemName == noCounterWeapons[i]) {
				return false;
			}
		}
		break;
	case 2:
		for (short i = 0; i < noCounterSpells.size(); i++) {
			if (itemName == noCounterSpells[i]) {
				return false;
			}
		}
		break;
	}
	return true;
}

void enemy::applyDamageModifiers(short* p, short* m, short* a) {
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

void enemy::resetBonusActions() {
	if (bonusActions < 0) {
		currentBonusActions = 0;
	}
	else {
		currentBonusActions = bonusActions;
	}
}