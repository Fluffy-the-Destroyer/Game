#include "battle.h"
#include "rng.h"
#include <iostream>
#include "resources.h"
#include "inputs.h"
using namespace std;

extern resource g_manaName, g_projName;

//Error codes:
// 6: Trying to access slot which is out of range

unsigned char battleHandler(player* playerCharacter, enemy* opponent) {
	//Check if enemy dies immediately
	if (opponent->getHealth() <= 0) {
		cout << opponent->getName() << " immediately dies.\n";
		if (opponent->getDeathSpell()->getReal()) { //Check if enemy has a death spell and if so cast it
			cout << "On death, it casts " << opponent->getDeathSpell()->getName() << '\n';
			if (spellCast(opponent->getDeathSpell(), playerCharacter) == 1) {
				return 2;
			}
		}
		return 1;
	}
	bool done; //Is the current turn done
	unsigned char p_selection1 = 0, p_selection2 = 0, e_selection1 = 0, e_selection2 = 0; //For holding weapon/spell selection
	bool firstTurn = true; //Is it the first turn
	while (true) { //Turn cycle loop
		done = false;
		playerCharacter->turnStart();
		if (playerCharacter->getHealth() <= 0) {
			return 2;
		}
		while (!done) { //Player turn
			cout << "Health: " << playerCharacter->getHealth() << '/' << playerCharacter->getMaxHealth() << ' ' << g_manaName.Plural() << ": " << playerCharacter->getMana() << '/' << playerCharacter->getMaxMana() << ' ' << g_projName.Plural() << ": " << playerCharacter->getProjectiles() << ' ';
			if (playerCharacter->getPoison() > 0) {
				cout << "Poison: " << +playerCharacter->getPoison() << ' ';
			}
			if (playerCharacter->getBleed() > 0) {
				cout << "Bleed: " << +playerCharacter->getBleed() << ' ';
			}
			if (playerCharacter->getRegen() > 0) {
				cout << "Regeneration: " << +playerCharacter->getRegen();
			}
			cout << "\nTo view your inventory, enter 1.\nTo attack with a weapon, enter 2.\nTo cast a spell, enter 3.\n";
			switch (userChoice(1, 3)) {
			case 1:
				playerCharacter->showInventory();
				break;
			case 2:
				cout << "Enter the number of the weapon you wish to attack with.\nTo go back, enter 0.\n";
				try {
					p_selection1 = 0;
					for (unsigned char i = 0; i < playerCharacter->getWeaponSlots(); i++) {
						cout << i + 1 << ' ';
						playerCharacter->getWeapon(i)->displayName();
						cout << '\n';
						p_selection1 = i + 1;
					}
				}
				catch (int) {
					cout << "\nAn internal error occured, stored number of weapon slots does not match size of weapons vector\n";
				}
				p_selection1 = userChoice(0, p_selection1);
				if (p_selection1 == 0) {
					break;
				}
				if (!playerCharacter->getWeapon(p_selection1)->getReal()) {
					cout << "Cannot attack with empty slot!\n";
					break;
				}
				if (playerCharacter->getWeapon(p_selection1)->getHealthChange() > playerCharacter->getHealth()) {
					if (playerCharacter->getWeapon(p_selection1)->getManaChange() < -playerCharacter->getMana()) {
						if (playerCharacter->getWeapon(p_selection1)->getProjectileChange() < -playerCharacter->getProjectiles()) {
							cout << "Not enough health, " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
							break;
						}
						cout << "Not enough health or " << g_manaName.plural() << "!\n";
						break;
					}
					if (playerCharacter->getWeapon(p_selection1)->getProjectileChange() < -playerCharacter->getProjectiles()) {
						cout << "Not enough health or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough health!\n";
					break;
				}
				if (playerCharacter->getWeapon(p_selection1)->getManaChange() < -playerCharacter->getMana()) {
					if (playerCharacter->getWeapon(p_selection1)->getProjectileChange() < -playerCharacter->getProjectiles()) {
						cout << "Not enough " << g_manaName.plural() << " or " << g_projName.plural() << "!\n";
						break;
					}
					cout << "Not enough " << g_manaName.plural() << "!\n";
					break;
				}
				if (playerCharacter->getWeapon(p_selection1)->getProjectileChange() < -playerCharacter->getProjectiles()) {
					cout << "Not enough " << g_projName.plural() << "!\n";
					break;
				}
				weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter); //Apply attack costs
				if (playerCharacter->getHealth() <= 0) { //Check if player is dead
					return 2;
				}
				if (true) { //Ask enemy what it is doing, 2 means casting a spell
					spellDeclare(opponent->getSpell(e_selection1), opponent); //Enemy casts spell
					switch (spellCast(opponent->getSpell(e_selection1), opponent, playerCharacter)) {
					case 1:
						return 2; //It killed the player
					case 2:
						return 1;
					}
					if ((opponent->getSpell(e_selection1)->getCounterSpell() == 2 || opponent->getSpell(e_selection1)->getCounterSpell() == 3) && !playerCharacter->getWeapon(p_selection1)->getNoCounter()) { //Check if the cast spell counters the attack, if it does, player's turn is done
						done = true;
						break;
					}
				}
				switch (weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter, opponent)) {
				case 1:
					return 1;
				case 2:
					return 2;
				}
				if (!playerCharacter->getWeapon(p_selection1)->getNoCounterAttack()) {
					switch (4) {
					case 1:
						weaponDeclare(opponent->getWeapon(e_selection1), opponent);
						switch (weaponAttack(opponent->getWeapon(e_selection1), opponent, playerCharacter, true)) {
						case 1:
							return 2;
						case 2:
							return 1;
						}
						break;
					case 2:
						spellDeclare(opponent->getSpell(e_selection1), opponent);
						switch (spellCast(opponent->getSpell(e_selection1), opponent, playerCharacter, true)) {
						case 1:
							return 2;
						case 2:
							return 1;
						}
						break;
					}
				}
				done = true;
				break;
			}
		}
	}
	return 1;
}

unsigned char spellCast(spell* magic, player* caster, enemy* target, bool counter) {
	//Self damage
	caster->propDamage(magic->getPropSelfDamage());
	caster->flatDamage(magic->getFlatSelfDamage(), 1);
	caster->flatDamage(magic->getFlatSelfMagicDamage(), 2);
	caster->flatDamage(magic->getFlatSelfArmourPiercingDamage(), 3);
	//Poison
	caster->modifyPoison(magic->getSelfPoison());
	//Bleed
	caster->modifyBleed(magic->getSelfBleed());
	//Turn mana regen
	caster->modifyTurnManaRegen(magic->getTurnManaRegenModifier());
	//Battle mana regen
	caster->modifyBattleManaRegen(magic->getBattleManaRegenModifier());
	//Temp regen
	caster->modifyTempRegen(magic->getTempRegenSelf());
	//Const regen
	caster->modifyConstRegen(magic->getConstRegenModifier());
	//Battle regen
	caster->modifyBattleRegen(magic->getBattleRegenModifier());
	//Do hits
	unsigned char hits;
	if (counter) {
		hits = magic->getCounterHits();
	}
	else {
		hits = magic->getHitCount();
	}
	for (unsigned char i = 0; i < hits; i++) {
		spellHit(magic, caster, target);
	}
	//Caster modifiers
	caster->modifyMaxHealth(magic->getMaxHealthModifier());
	caster->modifyMaxMana(magic->getMaxManaModifier());
	caster->modifyPoisonResist(magic->getPoisonResistModifier());
	caster->modifyBleedResist(magic->getBleedResistModifier());
	caster->modifyFlatArmour(magic->getFlatArmourModifier());
	caster->modifyPropArmour(magic->getPropArmourModifier());
	caster->modifyFlatMagicArmour(magic->getFlatMagicArmourModifier());
	caster->modifyPropMagicArmour(magic->getPropMagicArmourModifier());
	caster->modifyFlatDamageModifier(magic->getFlatDamageModifier());
	caster->modifyPropDamageModifier(magic->getPropDamageModifier());
	caster->modifyFlatMagicDamageModifier(magic->getFlatMagicDamageModifier());
	caster->modifyPropMagicDamageModifier(magic->getPropMagicDamageModifier());
	caster->modifyFlatArmourPiercingDamageModifier(magic->getFlatArmourPiercingDamageModifier());
	caster->modifyPropArmourPiercingDamageModifier(magic->getPropArmourPiercingDamageModifier());
	caster->modifyEvadeChance(magic->getEvadeChanceModifier());
	caster->modifyBonusActions(magic->getBonusActionsModifier());
	if (target->getHealth() <= 0 && target->getDeathSpell()->getReal()) { //Enemy is dead and has death spell
		cout << target->getName() << " is dead. On death, it casts " << target->getDeathSpell()->getName() << '\n';
		if (spellCast(target->getDeathSpell(), caster) == 1) {
			return 2;
		}
		return 1;
	}
	if (caster->getHealth() <= 0) { //Player is dead
		return 2;
	}
	if (target->getHealth() <= 0) { //Enemy is dead
		cout << target->getName() << " is dead.\n";
		return 1;
	}
	return 0;
}

unsigned char spellCast(spell* magic, enemy* caster, player* target, bool counter) {
	//Self damage
	caster->propDamage(magic->getPropSelfDamage());
	caster->flatDamage(magic->getFlatSelfDamage(), 1);
	caster->flatDamage(magic->getFlatSelfMagicDamage(), 2);
	caster->flatDamage(magic->getFlatSelfArmourPiercingDamage(), 3);
	//Poison
	caster->modifyPoison(magic->getSelfPoison());
	//Bleed
	caster->modifyBleed(magic->getSelfBleed());
	//Turn mana regen
	caster->modifyTurnManaRegen(magic->getTurnManaRegenModifier());
	//Temp regen
	caster->modifyTempRegen(magic->getTempRegenSelf());
	//Const regen
	caster->modifyConstRegen(magic->getConstRegenModifier());
	//Do hits
	unsigned char hits;
	if (counter) {
		hits = magic->getCounterHits();
	}
	else {
		hits = magic->getHitCount();
	}
	for (unsigned char i = 0; i < hits; i++) {
		spellHit(magic, caster, target);
	}
	//Caster modifiers
	caster->modifyMaxHealth(magic->getMaxHealthModifier());
	caster->modifyMaxMana(magic->getMaxManaModifier());
	caster->modifyPoisonResist(magic->getPoisonResistModifier());
	caster->modifyBleedResist(magic->getBleedResistModifier());
	caster->modifyFlatArmour(magic->getFlatArmourModifier());
	caster->modifyPropArmour(magic->getPropArmourModifier());
	caster->modifyFlatMagicArmour(magic->getFlatMagicArmourModifier());
	caster->modifyPropMagicArmour(magic->getPropMagicArmourModifier());
	caster->modifyFlatDamageModifier(magic->getFlatDamageModifier());
	caster->modifyPropDamageModifier(magic->getPropDamageModifier());
	caster->modifyFlatMagicDamageModifier(magic->getFlatMagicDamageModifier());
	caster->modifyPropMagicDamageModifier(magic->getPropMagicDamageModifier());
	caster->modifyFlatArmourPiercingDamageModifier(magic->getFlatArmourPiercingDamageModifier());
	caster->modifyPropArmourPiercingDamageModifier(magic->getPropArmourPiercingDamageModifier());
	caster->modifyEvadeChance(magic->getEvadeChanceModifier());
	caster->modifyBonusActions(magic->getBonusActionsModifier());
	if (caster->getHealth() <= 0 && caster->getDeathSpell()->getReal()) { //Enemy is dead and has death spell
		cout << caster->getName() << " is dead. On death, it casts " << caster->getDeathSpell()->getName() << '\n';
		if (spellCast(caster->getDeathSpell(), target) == 1) {
			return 1;
		}
		return 2;
	}
	if (target->getHealth() <= 0) {
		return 1;
	}
	if (caster->getHealth() <= 0) {
		cout << caster->getName() << " is dead.\n";
		return 2;
	}
	return 0;
}

unsigned char spellCast(spell* magic, player* target) {
	unsigned char hits = magic->getHitCount();
	for (unsigned char i = 0; i < hits; i++) {
		spellHit(magic, target);
	}
	if (target->getHealth() <= 0) {
		return 1;
	}
	return 0;
}

void spellHit(spell* magic, player* caster, enemy* target) {
	if (target->getHealth() <= 0) {
		return;
	}
	if (!magic->getNoEvade()) { //If can be dodged, roll for evade
		if (rng(0.f, 1.f) < target->getEvadeChance()) {
			return;
		}
	}
	//Prop damage
	target->propDamage(magic->getPropDamage());
	//Flat damage
	short damageBuffer = magic->getFlatDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage(static_cast<short>((damageBuffer + caster->getFlatDamageModifier()) * (1 + caster->getPropDamageModifier())), 1);
		if (magic->getLifelink()) {
			caster->flatDamage(-damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	//Magic damage
	damageBuffer = magic->getFlatMagicDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage(static_cast<short>((damageBuffer + caster->getFlatMagicDamageModifier()) * (1 + caster->getPropMagicDamageModifier())), 2);
		if (magic->getLifelink()) {
			caster->flatDamage(-damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	//AP damage
	damageBuffer = magic->getFlatArmourPiercingDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage(static_cast<short>((damageBuffer + caster->getFlatArmourPiercingDamageModifier()) * (1 + caster->getPropArmourPiercingDamageModifier())), 3);
		if (magic->getLifelink()) {
			caster->flatDamage(-damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	//Enemy mana
	target->modifyMana(magic->getManaChangeEnemy());
	//Poison
	target->modifyPoison(magic->getPoison());
	//Bleed
	target->modifyBleed(magic->getBleed());
	//Max health
	target->modifyMaxHealth(magic->getMaxHealthModifierEnemy());
	//Max mana
	target->modifyMaxMana(magic->getMaxManaModifierEnemy());
	//Turn mana regen
	target->modifyTurnManaRegen(magic->getTurnManaRegenModifierEnemy());
	//Poison resist
	target->modifyPoisonResist(magic->getPoisonResistModifierEnemy());
	//Bleed resist
	target->modifyBleedResist(magic->getBleedResistModifierEnemy());
	//Temp regen
	target->modifyTempRegen(magic->getTempRegen());
	//Const regen
	target->modifyConstRegen(magic->getConstRegenModifierEnemy());
	//Flat armour
	target->modifyFlatArmour(magic->getFlatArmourModifierEnemy());
	//Flat magic armour
	target->modifyFlatMagicArmour(magic->getFlatMagicArmourModifierEnemy());
	//Prop armour
	target->modifyPropArmour(magic->getPropArmourModifierEnemy());
	//Prop magic armour
	target->modifyPropMagicArmour(magic->getPropMagicArmourModifierEnemy());
	//Damage modifiers
	target->modifyFlatDamageModifier(magic->getFlatDamageModifierEnemy());
	target->modifyPropDamageModifier(magic->getPropDamageModifierEnemy());
	target->modifyFlatMagicDamageModifier(magic->getFlatMagicDamageModifierEnemy());
	target->modifyPropMagicDamageModifier(magic->getPropMagicDamageModifierEnemy());
	target->modifyFlatArmourPiercingDamageModifier(magic->getFlatArmourPiercingDamageModifierEnemy());
	target->modifyPropArmourPiercingDamageModifier(magic->getPropArmourPiercingDamageModifierEnemy());
	//Evade chance
	target->modifyEvadeChance(magic->getEvadeChanceModifierEnemy());
	//Bonus actions
	target->modifyBonusActions(magic->getBonusActionsModifierEnemy());
}

void spellHit(spell* magic, enemy* caster, player* target) {
	if (target->getHealth() <= 0) {
		return;
	}
	if (!magic->getNoEvade()) { //If can be dodged, roll for evade
		if (rng(0.f, 1.f) < target->getEvadeChance()) {
			return;
		}
	}
	//Prop damage
	target->propDamage(magic->getPropDamage());
	//Flat damage
	short damageBuffer = magic->getFlatDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage(static_cast<short>((damageBuffer + caster->getFlatDamageModifier()) * (1 + caster->getPropDamageModifier())), 1);
		if (magic->getLifelink()) {
			caster->flatDamage(-damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	//Magic damage
	damageBuffer = magic->getFlatMagicDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage(static_cast<short>((damageBuffer + caster->getFlatMagicDamageModifier()) * (1 + caster->getPropMagicDamageModifier())), 2);
		if (magic->getLifelink()) {
			caster->flatDamage(-damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	//AP damage
	damageBuffer = magic->getFlatArmourPiercingDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage(static_cast<short>((damageBuffer + caster->getFlatArmourPiercingDamageModifier()) * (1 + caster->getPropArmourPiercingDamageModifier())), 3);
		if (magic->getLifelink()) {
			caster->flatDamage(-damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	//Enemy mana
	target->modifyMana(magic->getManaChangeEnemy());
	//Poison
	target->modifyPoison(magic->getPoison());
	//Bleed
	target->modifyBleed(magic->getBleed());
	//Max health
	target->modifyMaxHealth(magic->getMaxHealthModifierEnemy());
	//Max mana
	target->modifyMaxMana(magic->getMaxManaModifierEnemy());
	//Turn mana regen
	target->modifyTurnManaRegen(magic->getTurnManaRegenModifierEnemy());
	//Poison resist
	target->modifyPoisonResist(magic->getPoisonResistModifierEnemy());
	//Bleed resist
	target->modifyBleedResist(magic->getBleedResistModifierEnemy());
	//Temp regen
	target->modifyTempRegen(magic->getTempRegen());
	//Const regen
	target->modifyConstRegen(magic->getConstRegenModifierEnemy());
	//Flat armour
	target->modifyFlatArmour(magic->getFlatArmourModifierEnemy());
	//Flat magic armour
	target->modifyFlatMagicArmour(magic->getFlatMagicArmourModifierEnemy());
	//Prop armour
	target->modifyPropArmour(magic->getPropArmourModifierEnemy());
	//Prop magic armour
	target->modifyPropMagicArmour(magic->getPropMagicArmourModifierEnemy());
	//Damage modifiers
	target->modifyFlatDamageModifier(magic->getFlatDamageModifierEnemy());
	target->modifyPropDamageModifier(magic->getPropDamageModifierEnemy());
	target->modifyFlatMagicDamageModifier(magic->getFlatMagicDamageModifierEnemy());
	target->modifyPropMagicDamageModifier(magic->getPropMagicDamageModifierEnemy());
	target->modifyFlatArmourPiercingDamageModifier(magic->getFlatArmourPiercingDamageModifierEnemy());
	target->modifyPropArmourPiercingDamageModifier(magic->getPropArmourPiercingDamageModifierEnemy());
	//Evade chance
	target->modifyEvadeChance(magic->getEvadeChanceModifierEnemy());
	//Bonus actions
	target->modifyBonusActions(magic->getBonusActionsModifierEnemy());
	//Battle mana regen
	target->modifyBattleManaRegen(magic->getBattleManaRegenModifierEnemy());
	//Battle regen
	target->modifyBattleRegen(magic->getBattleRegenModifierEnemy());
}

void spellHit(spell* magic, player* target) {
	if (target->getHealth() <= 0) {
		return;
	}
	if (!magic->getNoEvade()) { //If can be dodged, roll for evade
		if (rng(0.f, 1.f) < target->getEvadeChance()) {
			return;
		}
	}
	//Prop damage
	target->propDamage(magic->getPropDamage());
	//Flat damage
	target->flatDamage(magic->getFlatDamage(), 1);
	//Magic damage
	target->flatDamage(magic->getFlatMagicDamage(), 2);
	//AP damage
	target->flatDamage(magic->getFlatArmourPiercingDamage(), 3);
	//Enemy mana
	target->modifyMana(magic->getManaChangeEnemy());
	//Poison
	target->modifyPoison(magic->getPoison());
	//Bleed
	target->modifyBleed(magic->getBleed());
	//Max health
	target->modifyMaxHealth(magic->getMaxHealthModifierEnemy());
	//Max mana
	target->modifyMaxMana(magic->getMaxManaModifierEnemy());
	//Turn mana regen
	target->modifyTurnManaRegen(magic->getTurnManaRegenModifierEnemy());
	//Poison resist
	target->modifyPoisonResist(magic->getPoisonResistModifierEnemy());
	//Bleed resist
	target->modifyBleedResist(magic->getBleedResistModifierEnemy());
	//Temp regen
	target->modifyTempRegen(magic->getTempRegen());
	//Const regen
	target->modifyConstRegen(magic->getConstRegenModifierEnemy());
	//Flat armour
	target->modifyFlatArmour(magic->getFlatArmourModifierEnemy());
	//Flat magic armour
	target->modifyFlatMagicArmour(magic->getFlatMagicArmourModifierEnemy());
	//Prop armour
	target->modifyPropArmour(magic->getPropArmourModifierEnemy());
	//Prop magic armour
	target->modifyPropMagicArmour(magic->getPropMagicArmourModifierEnemy());
	//Damage modifiers
	target->modifyFlatDamageModifier(magic->getFlatDamageModifierEnemy());
	target->modifyPropDamageModifier(magic->getPropDamageModifierEnemy());
	target->modifyFlatMagicDamageModifier(magic->getFlatMagicDamageModifierEnemy());
	target->modifyPropMagicDamageModifier(magic->getPropMagicDamageModifierEnemy());
	target->modifyFlatArmourPiercingDamageModifier(magic->getFlatArmourPiercingDamageModifierEnemy());
	target->modifyPropArmourPiercingDamageModifier(magic->getPropArmourPiercingDamageModifierEnemy());
	//Evade chance
	target->modifyEvadeChance(magic->getEvadeChanceModifierEnemy());
	//Bonus actions
	target->modifyBonusActions(magic->getBonusActionsModifierEnemy());
}

void spellDeclare(spell* magic, player* caster) {
	magic->startCooldown();
	caster->modifyMana(magic->getManaChange());
	caster->modifyProjectiles(magic->getProjectileChange());
	caster->modifyHealth(magic->getHealthChange());
}

void spellDeclare(spell* magic, enemy* caster) {
	magic->startCooldown();
	caster->modifyMana(magic->getManaChange());
	caster->modifyProjectiles(magic->getProjectileChange());
	caster->modifyHealth(magic->getHealthChange());
}

unsigned char weaponAttack(weapon* weaponry, player* attacker, enemy* target, bool counter) {
	//Self damage
	attacker->propDamage(weaponry->getPropSelfDamage());
	attacker->flatDamage(weaponry->getFlatSelfDamage(), 1);
	attacker->flatDamage(weaponry->getFlatSelfMagicDamage(), 2);
	attacker->flatDamage(weaponry->getFlatSelfArmourPiercingDamage(), 3);
	attacker->modifyPoison(weaponry->getSelfPoison());
	attacker->modifyBleed(weaponry->getSelfBleed());
	unsigned char hits;
	if (counter) {
		hits = weaponry->getCounterHits();
	}
	else {
		hits = weaponry->getHitCount();
	}
	for (unsigned char i = 0; i < hits; i++) {
		weaponHit(weaponry, attacker, target);
	}
	if (target->getHealth() <= 0 && target->getDeathSpell()->getReal()) { //Enemy is dead and has death spell
		cout << target->getName() << " is dead. On death, it casts " << target->getDeathSpell()->getName() << '\n';
		if (spellCast(target->getDeathSpell(), attacker) == 1) {
			return 2;
		}
		return 1;
	}
	if (attacker->getHealth() <= 0) {
		return 2;
	}
	if (target->getHealth() <= 0) {
		cout << target->getName() << " is dead.\n";
		return 1;
	}
	return 0;
}

unsigned char weaponAttack(weapon* weaponry, enemy* attacker, player* target, bool counter) {
	//Self damage
	attacker->propDamage(weaponry->getPropSelfDamage());
	attacker->flatDamage(weaponry->getFlatSelfDamage(), 1);
	attacker->flatDamage(weaponry->getFlatSelfMagicDamage(), 2);
	attacker->flatDamage(weaponry->getFlatSelfArmourPiercingDamage(), 3);
	attacker->modifyPoison(weaponry->getSelfPoison());
	attacker->modifyBleed(weaponry->getSelfBleed());
	unsigned char hits;
	if (counter) {
		hits = weaponry->getCounterHits();
	}
	else {
		hits = weaponry->getHitCount();
	}
	for (unsigned char i = 0; i < hits; i++) {
		weaponHit(weaponry, attacker, target);
	}
	if (attacker->getHealth() <= 0 && attacker->getDeathSpell()->getReal()) { //Enemy is dead and has death spell
		cout << attacker->getName() << " is dead. On death, it casts " << attacker->getDeathSpell()->getName() << '\n';
		if (spellCast(attacker->getDeathSpell(), target) == 1) {
			return 1;
		}
		return 2;
	}
	if (target->getHealth() <= 0) {
		return 1;
	}
	if (attacker->getHealth() <= 0) {
		cout << attacker->getName() << " is dead.\n";
		return 2;
	}
	return 0;
}

void weaponHit(weapon* weaponry, player* attacker, enemy* target) {
	if (target->getHealth() <= 0) {
		return;
	}
	//Check evasion
	if (!weaponry->getNoEvade()) {
		if (rng(0.f, 1.f) < target->getEvadeChance()) {
			return;
		}
	}
	//Damage
	target->propDamage(weaponry->getPropDamage());
	short damageBuffer = weaponry->getFlatDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage((static_cast<short>((damageBuffer + attacker->getFlatDamageModifier()) * (1 + attacker->getPropDamageModifier()))), 1);
		if (weaponry->getLifelink()) {
			attacker->flatDamage(-damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	damageBuffer = weaponry->getFlatMagicDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage((static_cast<short>((damageBuffer + attacker->getFlatMagicDamageModifier()) * (1 + attacker->getPropMagicDamageModifier()))), 2);
		if (weaponry->getLifelink()) {
			attacker->flatDamage(-damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	damageBuffer = weaponry->getFlatArmourPiercingDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage((static_cast<short>((damageBuffer + attacker->getFlatArmourPiercingDamageModifier()) * (1 + attacker->getPropArmourPiercingDamageModifier()))), 3);
		if (weaponry->getLifelink()) {
			attacker->flatDamage(-damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	//Poison
	target->modifyPoison(weaponry->getPoison());
	//Bleed
	target->modifyBleed(weaponry->getBleed());
}

void weaponHit(weapon* weaponry, enemy* attacker, player* target) {
	if (target->getHealth() <= 0) {
		return;
	}
	//Check evasion
	if (!weaponry->getNoEvade()) {
		if (rng(0.f, 1.f) < target->getEvadeChance()) {
			return;
		}
	}
	//Damage
	target->propDamage(weaponry->getPropDamage());
	short damageBuffer = weaponry->getFlatDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage((static_cast<short>((damageBuffer + attacker->getFlatDamageModifier()) * (1 + attacker->getPropDamageModifier()))), 1);
		if (weaponry->getLifelink()) {
			attacker->flatDamage(-damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	damageBuffer = weaponry->getFlatMagicDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage((static_cast<short>((damageBuffer + attacker->getFlatMagicDamageModifier()) * (1 + attacker->getPropMagicDamageModifier()))), 2);
		if (weaponry->getLifelink()) {
			attacker->flatDamage(-damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	damageBuffer = weaponry->getFlatArmourPiercingDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage((static_cast<short>((damageBuffer + attacker->getFlatArmourPiercingDamageModifier()) * (1 + attacker->getPropArmourPiercingDamageModifier()))), 3);
		if (weaponry->getLifelink()) {
			attacker->flatDamage(-damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	//Poison
	target->modifyPoison(weaponry->getPoison());
	//Bleed
	target->modifyBleed(weaponry->getBleed());
}

void weaponDeclare(weapon* weaponry, player* attacker) {
	attacker->modifyMana(weaponry->getManaChange());
	attacker->modifyProjectiles(weaponry->getProjectileChange());
	attacker->modifyHealth(weaponry->getHealthChange());
}

void weaponDeclare(weapon* weaponry, enemy* attacker) {
	attacker->modifyMana(weaponry->getManaChange());
	attacker->modifyProjectiles(weaponry->getProjectileChange());
	attacker->modifyHealth(weaponry->getHealthChange());
}

void resetPlayer(player* playerCharacter) {
	playerCharacter->calculateModifiers();
	playerCharacter->curePoison();
	playerCharacter->cureBleed();
	playerCharacter->removeRegen();
	unsigned char spellSlots = playerCharacter->getSpellSlots();
	try {
		for (unsigned char i = 0; i < spellSlots; i++) {
			playerCharacter->getSpell(i)->resetCooldown();
		}
	}
	catch (int) {}
}

unsigned char weaponAttack(weapon* weapon1, weapon* weapon2, player* attacker, enemy* target, bool counter) {
	if (counter) { //If weapon2 hits more times, run the function with them swapped
		if (weapon2->getCounterHits() > weapon1->getCounterHits()) {
			return weaponAttack(weapon2, weapon1, attacker, target, counter);
		}
	}
	else {
		if (weapon2->getHitCount() > weapon1->getHitCount()) {
			return weaponAttack(weapon2, weapon1, attacker, target, counter);
		}
	}
	//Now may assume that weapon1 hits at least as many times as weapon2
	//Self damage
	if (weapon1->getPropSelfDamage() > 0) { //Weapon1 gives prop self healing, do weapon2 first in case it does damage
		attacker->propDamage(weapon2->getPropSelfDamage());
		attacker->propDamage(weapon1->getPropSelfDamage());
	}
	else {
		attacker->propDamage(weapon1->getPropSelfDamage());
		attacker->propDamage(weapon2->getPropSelfDamage());
	}
	attacker->flatDamage(weapon1->getFlatSelfDamage());
	attacker->flatDamage(weapon2->getFlatSelfDamage());
	attacker->flatDamage(weapon1->getFlatSelfMagicDamage(), 2);
	attacker->flatDamage(weapon2->getFlatSelfMagicDamage(), 2);
	attacker->flatDamage(weapon1->getFlatSelfArmourPiercingDamage(), 3);
	attacker->flatDamage(weapon2->getFlatSelfArmourPiercingDamage(), 3);
	//Poison
	attacker->modifyPoison(weapon1->getSelfPoison());
	attacker->modifyPoison(weapon2->getSelfPoison());
	//Bleed
	attacker->modifyBleed(weapon1->getSelfBleed());
	attacker->modifyBleed(weapon2->getSelfBleed());
	unsigned char hits1; //Number of hits of weapon1, which is always at least the number of hits of weapon2
	unsigned char hits2; //Number of hits of weapon2
	if (counter) {
		hits1 = weapon1->getCounterHits();
		hits2 = weapon2->getCounterHits();
	}
	else {
		hits1 = weapon1->getHitCount();
		hits2 = weapon2->getHitCount();
	}
	for (unsigned char i = 0; i < hits1; i++) {
		weaponHit(weapon1, attacker, target);
		if (i < hits2) {
			weaponHit(weapon2, attacker, target);
		}
	}
	if (target->getHealth() <= 0 && target->getDeathSpell()->getReal()) { //Enemy is dead and has death spell
		cout << target->getName() << " is dead. On death, it casts " << target->getDeathSpell()->getName() << '\n';
		if (spellCast(target->getDeathSpell(), attacker) == 1) {
			return 2;
		}
		return 1;
	}
	if (attacker->getHealth() <= 0) {
		return 2;
	}
	if (target->getHealth() <= 0) {
		cout << target->getName() << " is dead.\n";
		return 1;
	}
	return 0;
}

unsigned char weaponAttack(weapon* weapon1, weapon* weapon2, enemy* attacker, player* target, bool counter) {
	if (counter) { //If weapon2 hits more times, run the function with them swapped
		if (weapon2->getCounterHits() > weapon1->getCounterHits()) {
			return weaponAttack(weapon2, weapon1, attacker, target, counter);
		}
	}
	else {
		if (weapon2->getHitCount() > weapon1->getHitCount()) {
			return weaponAttack(weapon2, weapon1, attacker, target, counter);
		}
	}
	//Now may assume that weapon1 hits at least as many times as weapon2
	//Self damage
	if (weapon1->getPropSelfDamage() > 0) { //Weapon1 gives prop self healing, do weapon2 first in case it does damage
		attacker->propDamage(weapon2->getPropSelfDamage());
		attacker->propDamage(weapon1->getPropSelfDamage());
	}
	else {
		attacker->propDamage(weapon1->getPropSelfDamage());
		attacker->propDamage(weapon2->getPropSelfDamage());
	}
	attacker->flatDamage(weapon1->getFlatSelfDamage());
	attacker->flatDamage(weapon2->getFlatSelfDamage());
	attacker->flatDamage(weapon1->getFlatSelfMagicDamage(), 2);
	attacker->flatDamage(weapon2->getFlatSelfMagicDamage(), 2);
	attacker->flatDamage(weapon1->getFlatSelfArmourPiercingDamage(), 3);
	attacker->flatDamage(weapon2->getFlatSelfArmourPiercingDamage(), 3);
	//Poison
	attacker->modifyPoison(weapon1->getSelfPoison());
	attacker->modifyPoison(weapon2->getSelfPoison());
	//Bleed
	attacker->modifyBleed(weapon1->getSelfBleed());
	attacker->modifyBleed(weapon2->getSelfBleed());
	unsigned char hits1; //Number of hits of weapon1, which is always at least the number of hits of weapon2
	unsigned char hits2; //Number of hits of weapon2
	if (counter) {
		hits1 = weapon1->getCounterHits();
		hits2 = weapon2->getCounterHits();
	}
	else {
		hits1 = weapon1->getHitCount();
		hits2 = weapon2->getHitCount();
	}
	for (unsigned char i = 0; i < hits1; i++) {
		weaponHit(weapon1, attacker, target);
		if (i < hits2) {
			weaponHit(weapon2, attacker, target);
		}
	}
	if (attacker->getHealth() <= 0 && attacker->getDeathSpell()->getReal()) { //Enemy is dead and has death spell
		cout << attacker->getName() << " is dead. On death, it casts " << attacker->getDeathSpell()->getName() << '\n';
		if (spellCast(attacker->getDeathSpell(), target) == 1) {
			return 1;
		}
		return 2;
	}
	if (target->getHealth() <= 0) {
		return 1;
	}
	if (attacker->getHealth() <= 0) {
		cout << attacker->getName() << " is dead.\n";
		return 2;
	}
	return 0;
}

void weaponDeclare(weapon* weapon1, weapon* weapon2, player* attacker) {
	attacker->modifyHealth(weapon1->getHealthChange());
	attacker->modifyHealth(weapon2->getHealthChange());
	attacker->modifyMana(weapon1->getManaChange());
	attacker->modifyMana(weapon2->getManaChange());
	attacker->modifyProjectiles(weapon1->getProjectileChange());
	attacker->modifyProjectiles(weapon2->getProjectileChange());
}

void weaponDeclare(weapon* weapon1, weapon* weapon2, enemy* attacker) {
	attacker->modifyHealth(weapon1->getHealthChange());
	attacker->modifyHealth(weapon2->getHealthChange());
	attacker->modifyMana(weapon1->getManaChange());
	attacker->modifyMana(weapon2->getManaChange());
	attacker->modifyProjectiles(weapon1->getProjectileChange());
	attacker->modifyProjectiles(weapon2->getProjectileChange());
}