#include "battle.h"
#include "rng.h"
#include <iostream>
#include "resources.h"
#include "inputs.h"
#include "armour.h"
#include "blueprints.h"
using namespace std;

extern resource g_manaName, g_projName;
extern bool g_useCustomData;

//Error codes:
// 6: Trying to access slot which is out of range

unsigned char battleHandler(player* playerCharacter, enemy* opponent) {
	//Check if enemy dies immediately
	switch (deathCheck(playerCharacter, opponent)) {
	case 1:
		return 1;
	case 2:
		return 2;
	}
	unsigned char p_action = 0, p_selection1 = 0, p_selection2 = 0, e_action = 0, e_selection1 = 0, e_selection2 = 0; //For holding weapon/spell selection and action choice
	short health; //For holding combatants' health
	bool firstTurn = true; //Is it the first turn
	while (true) { //Turn cycle loop
		//Player turn
		playerCharacter->turnStart();
		switch (deathCheck(playerCharacter, opponent)) {
		case 1:
			return 1;
		case 2:
			return 2;
		}
		switch (playerCharacter->chooseAction(&p_selection1, &p_selection2, opponent->getName())) { //Get player choice
		case 0: //Did nothing
			cout << "You do nothing\n";
			break;
		case 1: //Attacking with weapon
			cout << "You attack with " << playerCharacter->getWeapon(p_selection1)->getName() << '\n';
			weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter);
			health = playerCharacter->getHealth();
			if (opponent->chooseAction(&e_selection1, &e_selection2, 1, playerCharacter->getWeapon(p_selection1)->getName(), "", firstTurn) == 2) { //Get enemy action choice
				cout << opponent->getName() << " casts " << opponent->getSpell(e_selection1)->getName() << " in response\n";
				spellDeclare(opponent->getSpell(e_selection1), opponent);
				spellCast(opponent->getSpell(e_selection1), opponent, playerCharacter);
				if (opponent->getSpell(e_selection1)->getPropDamage() > 0 || playerCharacter->getHealth() < health) { //Only run death check if enemy's spell was damaging
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
				}
				if (opponent->getSpell(e_selection1)->getCounterSpell() >= 2) { //Check counterspell
					if (playerCharacter->getWeapon(p_selection1)->getNoCounter()) {
						cout << playerCharacter->getWeapon(p_selection1)->getName() << " cannot be countered!\n";
						opponent->addNoCounter(1, playerCharacter->getWeapon(p_selection1)->getName()); //Tell enemy it cannot be countered
					}
					else {
						cout << "The effects of " << playerCharacter->getWeapon(p_selection1) << " are countered\n";
						switch (deathCheck(playerCharacter, opponent)) {
						case 1:
							return 1;
						case 2:
							return 2;
						}
						break;
					}
				}
			}
			weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter, opponent);
			switch (deathCheck(playerCharacter, opponent)) {
			case 1:
				return 1;
			case 2:
				return 2;
			}
			if (playerCharacter->getWeapon(p_selection1)->getNoCounterAttack()) { //If doesn't trigger counter attack, done
				break;
			}
			if (rng(0.f, 1.f) < opponent->getCounterAttackChance()) { //Roll for counter attack
				switch (opponent->chooseAction(&e_selection1, &e_selection2, 3, "", "", firstTurn)) { //Get opponent's action
				case 1:
					cout << opponent->getName() << " counter attacks with " << opponent->getWeapon(e_selection1)->getName() << '\n';
					weaponDeclare(opponent->getWeapon(e_selection1), opponent);
					weaponAttack(opponent->getWeapon(e_selection1), opponent, playerCharacter, true);
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
					break;
				case 2:
					cout << opponent->getName() << " counter attacks by casting " << opponent->getSpell(e_selection1)->getName() << '\n';
					spellDeclare(opponent->getSpell(e_selection1), opponent);
					spellCast(opponent->getSpell(e_selection1), opponent, playerCharacter, true);
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
					break;
				case 3:
					cout << opponent->getName() << " counter attacks with " << opponent->getWeapon(e_selection1)->getName() << " and " << opponent->getWeapon(e_selection2)->getName() << '\n';
					weaponDeclare(opponent->getWeapon(e_selection1), opponent->getWeapon(e_selection2), opponent);
					weaponAttack(opponent->getWeapon(e_selection1), opponent->getWeapon(e_selection2), opponent, playerCharacter, true);
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
					break;
				}
			}
			break;
		case 2: //Casting a spell
			cout << "You cast " << playerCharacter->getSpell(p_selection1)->getName() << '\n';
			spellDeclare(playerCharacter->getSpell(p_selection1), playerCharacter);
			health = playerCharacter->getHealth();
			if (opponent->chooseAction(&e_selection1, &e_selection2, 2, playerCharacter->getSpell(p_selection1)->getName(), "", firstTurn) == 2) {
				cout << opponent->getName() << " casts " << opponent->getSpell(e_selection1)->getName() << " in response\n";
				spellDeclare(opponent->getSpell(e_selection1), opponent);
				spellCast(opponent->getSpell(e_selection1), opponent, playerCharacter);
				if (opponent->getSpell(e_selection1)->getPropDamage() > 0 || playerCharacter->getHealth() < health) {
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
				}
				if (opponent->getSpell(e_selection1)->getCounterSpell() == 1 || opponent->getSpell(e_selection1)->getCounterSpell() == 3) {
					if (playerCharacter->getSpell(p_selection1)->getNoCounter()) {
						cout << playerCharacter->getSpell(p_selection1)->getName() << " cannot be countered!\n";
						opponent->addNoCounter(2, playerCharacter->getSpell(p_selection1)->getName());
					}
					else {
						cout << "The effects of " << playerCharacter->getSpell(p_selection1)->getName() << " are countered\n";
						switch (deathCheck(playerCharacter, opponent)) {
						case 1:
							return 1;
						case 2:
							return 2;
						}
						break;
					}
				}
			}
			spellCast(playerCharacter->getSpell(p_selection1), playerCharacter, opponent);
			switch (deathCheck(playerCharacter, opponent)) {
			case 1:
				return 1;
			case 2:
				return 2;
			}
			if (playerCharacter->getSpell(p_selection1)->getCanCounterAttack()) {
				if (rng(0.f, 1.f) < opponent->getCounterAttackChance()) {
					switch (opponent->chooseAction(&e_selection1, &e_selection2, 3, "", "", firstTurn)) { //Get opponent's action
					case 1:
						cout << opponent->getName() << " counter attacks with " << opponent->getWeapon(e_selection1)->getName() << '\n';
						weaponDeclare(opponent->getWeapon(e_selection1), opponent);
						weaponAttack(opponent->getWeapon(e_selection1), opponent, playerCharacter, true);
						switch (deathCheck(playerCharacter, opponent)) {
						case 1:
							return 1;
						case 2:
							return 2;
						}
						break;
					case 2:
						cout << opponent->getName() << " counter attacks by casting " << opponent->getSpell(e_selection1)->getName() << '\n';
						spellDeclare(opponent->getSpell(e_selection1), opponent);
						spellCast(opponent->getSpell(e_selection1), opponent, playerCharacter, true);
						switch (deathCheck(playerCharacter, opponent)) {
						case 1:
							return 1;
						case 2:
							return 2;
						}
						break;
					case 3:
						cout << opponent->getName() << " counter attacks with " << opponent->getWeapon(e_selection1)->getName() << " and " << opponent->getWeapon(e_selection2)->getName() << '\n';
						weaponDeclare(opponent->getWeapon(e_selection1), opponent->getWeapon(e_selection2), opponent);
						weaponAttack(opponent->getWeapon(e_selection1), opponent->getWeapon(e_selection2), opponent, playerCharacter, true);
						switch (deathCheck(playerCharacter, opponent)) {
						case 1:
							return 1;
						case 2:
							return 2;
						}
						break;
					}
				}
			}
			break;
		case 3: //Dual weapon attack
			cout << "You attack with " << playerCharacter->getWeapon(p_selection1)->getName() << " and " << playerCharacter->getWeapon(p_selection2)->getName() << '\n';
			weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter->getWeapon(p_selection2), playerCharacter);
			health = playerCharacter->getHealth();
			if (opponent->chooseAction(&e_selection1, &e_selection2, 4, playerCharacter->getWeapon(p_selection1)->getName(), playerCharacter->getWeapon(p_selection2)->getName(), firstTurn) == 2) {
				cout << opponent->getName() << " casts " << opponent->getSpell(e_selection1)->getName() << " in response\n";
				spellDeclare(opponent->getSpell(e_selection1), opponent);
				spellCast(opponent->getSpell(e_selection1), opponent, playerCharacter);
				if (opponent->getSpell(e_selection1)->getPropDamage() > 0 || playerCharacter->getHealth() < health) {
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
				}
				if (opponent->getSpell(e_selection1)->getCounterSpell() >= 2) {
					if (playerCharacter->getWeapon(p_selection1)->getNoCounter()) {
						if (playerCharacter->getWeapon(p_selection2)->getNoCounter()) {
							cout << playerCharacter->getWeapon(p_selection1)->getName() << " and " << playerCharacter->getWeapon(p_selection2)->getName() << " cannot be countered!\n";
							opponent->addNoCounter(1, playerCharacter->getWeapon(p_selection1)->getName());
							opponent->addNoCounter(1, playerCharacter->getWeapon(p_selection2)->getName());
						}
						else {
							cout << "The effects of " << playerCharacter->getWeapon(p_selection2)->getName() << " are countered, but " << playerCharacter->getWeapon(p_selection1)->getName() << " cannot be countered!\n";
							opponent->addNoCounter(1, playerCharacter->getWeapon(p_selection1)->getName());
							weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter, opponent);
							switch (deathCheck(playerCharacter, opponent)) {
							case 1:
								return 1;
							case 2:
								return 2;
							}
							if (playerCharacter->getWeapon(p_selection1)->getNoCounterAttack()) {
								break;
							}
							if (rng(0.f, 1.f) < opponent->getCounterAttackChance()) {
								switch (opponent->chooseAction(&e_selection1, &e_selection2, 3, "", "", firstTurn)) {
								case 1:
									cout << opponent->getName() << " counter attacks with " << opponent->getWeapon(e_selection1)->getName() << '\n';
									weaponDeclare(opponent->getWeapon(e_selection1), opponent);
									weaponAttack(opponent->getWeapon(e_selection1), opponent, playerCharacter, true);
									switch (deathCheck(playerCharacter, opponent)) {
									case 1:
										return 1;
									case 2:
										return 2;
									}
									break;
								case 2:
									cout << opponent->getName() << " counter attacks by casting " << opponent->getSpell(e_selection1)->getName() << '\n';
									spellDeclare(opponent->getSpell(e_selection1), opponent);
									spellCast(opponent->getSpell(e_selection1), opponent, playerCharacter, true);
									switch (deathCheck(playerCharacter, opponent)) {
									case 1:
										return 1;
									case 2:
										return 2;
									}
									break;
								case 3:
									cout << opponent->getName() << " counter attacks with " << opponent->getWeapon(e_selection1)->getName() << " and " << opponent->getWeapon(e_selection2)->getName() << '\n';
									weaponDeclare(opponent->getWeapon(e_selection1), opponent->getWeapon(e_selection2), opponent);
									weaponAttack(opponent->getWeapon(e_selection1), opponent->getWeapon(e_selection2), opponent, playerCharacter, true);
									switch (deathCheck(playerCharacter, opponent)) {
									case 1:
										return 1;
									case 2:
										return 2;
									}
									break;
								}
							}
							break;
						}
					}
					else {
						if (playerCharacter->getWeapon(p_selection2)->getNoCounter()) {
							cout << "The effects of " << playerCharacter->getWeapon(p_selection1)->getName() << " are countered, but " << playerCharacter->getWeapon(p_selection2)->getName() << " cannot be countered!\n";
							opponent->addNoCounter(1, playerCharacter->getWeapon(p_selection2)->getName());
							weaponAttack(playerCharacter->getWeapon(p_selection2), playerCharacter, opponent);
							switch (deathCheck(playerCharacter, opponent)) {
							case 1:
								return 1;
							case 2:
								return 2;
							}
							if (playerCharacter->getWeapon(p_selection2)->getNoCounterAttack()) {
								break;
							}
							if (rng(0.f, 1.f) < opponent->getCounterAttackChance()) {
								switch (opponent->chooseAction(&e_selection1, &e_selection2, 3, "", "", firstTurn)) {
								case 1:
									cout << opponent->getName() << " counter attacks with " << opponent->getWeapon(e_selection1)->getName() << '\n';
									weaponDeclare(opponent->getWeapon(e_selection1), opponent);
									weaponAttack(opponent->getWeapon(e_selection1), opponent, playerCharacter, true);
									switch (deathCheck(playerCharacter, opponent)) {
									case 1:
										return 1;
									case 2:
										return 2;
									}
									break;
								case 2:
									cout << opponent->getName() << " counter attacks by casting " << opponent->getSpell(e_selection1)->getName() << '\n';
									spellDeclare(opponent->getSpell(e_selection1), opponent);
									spellCast(opponent->getSpell(e_selection1), opponent, playerCharacter, true);
									switch (deathCheck(playerCharacter, opponent)) {
									case 1:
										return 1;
									case 2:
										return 2;
									}
									break;
								case 3:
									cout << opponent->getName() << " counter attacks with " << opponent->getWeapon(e_selection1)->getName() << " and " << opponent->getWeapon(e_selection2)->getName() << '\n';
									weaponDeclare(opponent->getWeapon(e_selection1), opponent->getWeapon(e_selection2), opponent);
									weaponAttack(opponent->getWeapon(e_selection1), opponent->getWeapon(e_selection2), opponent, playerCharacter, true);
									switch (deathCheck(playerCharacter, opponent)) {
									case 1:
										return 1;
									case 2:
										return 2;
									}
									break;
								}
							}
							break;
						}
						else {
							cout << "The effects of " << playerCharacter->getWeapon(p_selection1)->getName() << " and " << playerCharacter->getWeapon(p_selection2)->getName() << " are countered!\n";
							switch (deathCheck(playerCharacter, opponent)) {
							case 1:
								return 1;
							case 2:
								return 2;
							}
							break;
						}
					}
				}
			}
			weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter->getWeapon(p_selection2), playerCharacter, opponent);
			switch (deathCheck(playerCharacter, opponent)) {
			case 1:
				return 1;
			case 2:
				return 2;
			}
			if (playerCharacter->getWeapon(p_selection1)->getNoCounterAttack() && playerCharacter->getWeapon(p_selection2)->getNoCounterAttack()) {
				break;
			}
			if (rng(0.f, 1.f) < opponent->getCounterAttackChance()) {
				switch (opponent->chooseAction(&e_selection1, &e_selection2, 3, "", "", firstTurn)) {
				case 1:
					cout << opponent->getName() << " counter attacks with " << opponent->getWeapon(e_selection1)->getName() << '\n';
					weaponDeclare(opponent->getWeapon(e_selection1), opponent);
					weaponAttack(opponent->getWeapon(e_selection1), opponent, playerCharacter, true);
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
					break;
				case 2:
					cout << opponent->getName() << " counter attacks by casting " << opponent->getSpell(e_selection1)->getName() << '\n';
					spellDeclare(opponent->getSpell(e_selection1), opponent);
					spellCast(opponent->getSpell(e_selection1), opponent, playerCharacter, true);
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
					break;
				case 3:
					cout << opponent->getName() << " counter attacks with " << opponent->getWeapon(e_selection1)->getName() << " and " << opponent->getWeapon(e_selection2)->getName() << '\n';
					weaponDeclare(opponent->getWeapon(e_selection1), opponent->getWeapon(e_selection2), opponent);
					weaponAttack(opponent->getWeapon(e_selection1), opponent->getWeapon(e_selection2), opponent, playerCharacter, true);
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
					break;
				}
			}
			break;
		}
		//Enemy turn
		opponent->turnStart();
		switch (deathCheck(playerCharacter, opponent)) {
		case 1:
			return 1;
		case 2:
			return 2;
		}
		switch (opponent->chooseAction(&e_selection1, &e_selection2, 0, "", "", firstTurn)) {
		case 0:
			cout << opponent->getName() << " does nothing\n";
			break;
		case 1:
			cout << opponent->getName() << " attacks with " << opponent->getWeapon(e_selection1)->getName() << '\n';
			weaponDeclare(opponent->getWeapon(e_selection1), opponent);
			health = opponent->getHealth();
			if (playerCharacter->chooseAction(&p_selection1, &p_selection2, opponent->getName(), 1, opponent->getWeapon(e_selection1)->getName()) == 2) {
				cout << "You cast " << playerCharacter->getSpell(p_selection1)->getName() << " in response\n";
				spellDeclare(playerCharacter->getSpell(p_selection1), playerCharacter);
				spellCast(playerCharacter->getSpell(p_selection1), playerCharacter, opponent);
				if (playerCharacter->getSpell(p_selection1)->getPropDamage() > 0 || opponent->getHealth() < health) {
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
				}
				if (playerCharacter->getSpell(p_selection1)->getCounterSpell() >= 2) {
					if (opponent->getWeapon(e_selection1)->getNoCounter()) {
						cout << opponent->getWeapon(e_selection1)->getName() << " cannot be countered!\n";
					}
					else {
						cout << "The effects of " << opponent->getWeapon(e_selection1)->getName() << " are countered!\n";
						switch (deathCheck(playerCharacter, opponent)) {
						case 1:
							return 1;
						case 2:
							return 2;
						}
						break;
					}
				}
			}
			weaponAttack(opponent->getWeapon(e_selection1), opponent, playerCharacter);
			switch (deathCheck(playerCharacter, opponent)) {
			case 1:
				return 1;
			case 2:
				return 2;
			}
			if (opponent->getWeapon(e_selection1)->getNoCounterAttack()) {
				break;
			}
			if (rng(0.f, 1.f) < playerCharacter->getCounterAttackChance()) {
				switch (playerCharacter->chooseAction(&p_selection1, &p_selection2, opponent->getName(), 3)) {
				case 1:
					cout << "You counter attack with " << playerCharacter->getWeapon(p_selection1)->getName() << '\n';
					weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter);
					weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter, opponent, true);
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
					break;
				case 2:
					cout << "You counter attack by casting " << playerCharacter->getSpell(p_selection1)->getName() << '\n';
					spellDeclare(playerCharacter->getSpell(p_selection1), playerCharacter);
					spellCast(playerCharacter->getSpell(p_selection1), playerCharacter, opponent, true);
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
					break;
				case 3:
					cout << "You counter attack with " << playerCharacter->getWeapon(p_selection1)->getName() << " and " << playerCharacter->getWeapon(p_selection2)->getName() << '\n';
					weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter->getWeapon(p_selection2), playerCharacter);
					weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter->getWeapon(p_selection2), playerCharacter, opponent, true);
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
					break;
				}
			}
			break;
		case 2:
			cout << opponent->getName() << " casts " << opponent->getSpell(e_selection1)->getName() << '\n';
			spellDeclare(opponent->getSpell(e_selection1), opponent);
			health = opponent->getHealth();
			if (playerCharacter->chooseAction(&p_selection1, &p_selection2, opponent->getName(), 2, opponent->getSpell(e_selection1)->getName()) == 2) {
				cout << "You cast " << playerCharacter->getSpell(p_selection1)->getName() << " in response\n";
				spellDeclare(playerCharacter->getSpell(p_selection1), playerCharacter);
				spellCast(playerCharacter->getSpell(p_selection1), playerCharacter, opponent);
				if (playerCharacter->getSpell(p_selection1)->getPropDamage() > 0 || opponent->getHealth() < health) {
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
				}
				if (playerCharacter->getSpell(p_selection1)->getCounterSpell() == 1 || playerCharacter->getSpell(p_selection1)->getCounterSpell() == 3) {
					if (opponent->getSpell(e_selection1)->getNoCounter()) {
						cout << opponent->getSpell(e_selection1)->getName() << " cannot be countered!\n";
					}
					else {
						cout << "The effects of " << opponent->getSpell(e_selection1)->getName() << " are countered!\n";
						switch (deathCheck(playerCharacter, opponent)) {
						case 1:
							return 1;
						case 2:
							return 2;
						}
						break;
					}
				}
			}
			spellCast(opponent->getSpell(e_selection1), opponent, playerCharacter);
			switch (deathCheck(playerCharacter, opponent)) {
			case 1:
				return 1;
			case 2:
				return 2;
			}
			if (opponent->getSpell(e_selection1)->getCanCounterAttack()) {
				if (rng(0.f, 1.f) < playerCharacter->getCounterAttackChance()) {
					switch (playerCharacter->chooseAction(&p_selection1, &p_selection2, opponent->getName(), 3)) {
					case 1:
						cout << "You counter attack with " << playerCharacter->getWeapon(p_selection1)->getName() << '\n';
						weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter);
						weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter, opponent, true);
						switch (deathCheck(playerCharacter, opponent)) {
						case 1:
							return 1;
						case 2:
							return 2;
						}
						break;
					case 2:
						cout << "You counter attack by casting " << playerCharacter->getSpell(p_selection1)->getName() << '\n';
						spellDeclare(playerCharacter->getSpell(p_selection1), playerCharacter);
						spellCast(playerCharacter->getSpell(p_selection1), playerCharacter, opponent, true);
						switch (deathCheck(playerCharacter, opponent)) {
						case 1:
							return 1;
						case 2:
							return 2;
						}
						break;
					case 3:
						cout << "You counter attack with " << playerCharacter->getWeapon(p_selection1)->getName() << " and " << playerCharacter->getWeapon(p_selection2)->getName() << '\n';
						weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter->getWeapon(p_selection2), playerCharacter);
						weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter->getWeapon(p_selection2), playerCharacter, opponent, true);
						switch (deathCheck(playerCharacter, opponent)) {
						case 1:
							return 1;
						case 2:
							return 2;
						}
						break;
					}
				}
			}
			break;
		case 3:
			cout << opponent->getName() << " attacks with " << opponent->getWeapon(e_selection1)->getName() << " and " << opponent->getWeapon(e_selection2)->getName() << '\n';
			weaponDeclare(opponent->getWeapon(e_selection1), opponent->getWeapon(e_selection2), opponent);
			health = opponent->getHealth();
			if (playerCharacter->chooseAction(&p_selection1, &p_selection2, opponent->getName(), 4, opponent->getWeapon(e_selection1)->getName(), opponent->getWeapon(e_selection2)->getName()) == 2) {
				cout << "You cast " << playerCharacter->getSpell(p_selection1)->getName() << " in response\n";
				spellDeclare(playerCharacter->getSpell(p_selection1), playerCharacter);
				spellCast(playerCharacter->getSpell(p_selection1), playerCharacter, opponent);
				if (playerCharacter->getSpell(p_selection1)->getPropDamage() > 0 || opponent->getHealth() < health) {
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
				}
				if (playerCharacter->getSpell(p_selection1)->getCounterSpell() >= 2) {
					if (opponent->getWeapon(e_selection1)->getNoCounter()) {
						if (opponent->getWeapon(e_selection2)->getNoCounter()) {
							cout << opponent->getWeapon(e_selection1)->getName() << " and " << opponent->getWeapon(e_selection2)->getName() << " cannot be countered!\n";
						}
						else {
							cout << "The effects of " << opponent->getWeapon(e_selection2)->getName() << " are countered, but " << opponent->getWeapon(e_selection1)->getName() << " cannot be countered!\n";
							weaponAttack(opponent->getWeapon(e_selection1), opponent, playerCharacter);
							switch (deathCheck(playerCharacter, opponent)) {
							case 1:
								return 1;
							case 2:
								return 2;
							}
							if (opponent->getWeapon(e_selection1)->getNoCounterAttack()) {
								break;
							}
							if (rng(0.f, 1.f) < playerCharacter->getCounterAttackChance()) {
								switch (playerCharacter->chooseAction(&p_selection1, &p_selection2, opponent->getName(), 3)) {
								case 1:
									cout << "You counter attack with " << playerCharacter->getWeapon(p_selection1)->getName() << '\n';
									weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter);
									weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter, opponent, true);
									switch (deathCheck(playerCharacter, opponent)) {
									case 1:
										return 1;
									case 2:
										return 2;
									}
									break;
								case 2:
									cout << "You counter attack by casting " << playerCharacter->getSpell(p_selection1)->getName() << '\n';
									spellDeclare(playerCharacter->getSpell(p_selection1), playerCharacter);
									spellCast(playerCharacter->getSpell(p_selection1), playerCharacter, opponent, true);
									switch (deathCheck(playerCharacter, opponent)) {
									case 1:
										return 1;
									case 2:
										return 2;
									}
									break;
								case 3:
									cout << "You counter attack with " << playerCharacter->getWeapon(p_selection1)->getName() << " and " << playerCharacter->getWeapon(p_selection2)->getName() << '\n';
									weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter->getWeapon(p_selection2), playerCharacter);
									weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter->getWeapon(p_selection2), playerCharacter, opponent, true);
									switch (deathCheck(playerCharacter, opponent)) {
									case 1:
										return 1;
									case 2:
										return 2;
									}
									break;
								}
							}
							break;
						}
					}
					else {
						if (opponent->getWeapon(e_selection2)->getNoCounter()) {
							cout << "The effects of " << opponent->getWeapon(e_selection1)->getName() << " are countered, but " << opponent->getWeapon(e_selection2)->getName() << " cannot be countered!\n";
							weaponAttack(opponent->getWeapon(e_selection2), opponent, playerCharacter);
							switch (deathCheck(playerCharacter, opponent)) {
							case 1:
								return 1;
							case 2:
								return 2;
							}
							if (opponent->getWeapon(e_selection2)->getNoCounterAttack()) {
								break;
							}
							if (rng(0.f, 1.f) < playerCharacter->getCounterAttackChance()) {
								switch (playerCharacter->chooseAction(&p_selection1, &p_selection2, opponent->getName(), 3)) {
								case 1:
									cout << "You counter attack with " << playerCharacter->getWeapon(p_selection1)->getName() << '\n';
									weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter);
									weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter, opponent, true);
									switch (deathCheck(playerCharacter, opponent)) {
									case 1:
										return 1;
									case 2:
										return 2;
									}
									break;
								case 2:
									cout << "You counter attack by casting " << playerCharacter->getSpell(p_selection1)->getName() << '\n';
									spellDeclare(playerCharacter->getSpell(p_selection1), playerCharacter);
									spellCast(playerCharacter->getSpell(p_selection1), playerCharacter, opponent, true);
									switch (deathCheck(playerCharacter, opponent)) {
									case 1:
										return 1;
									case 2:
										return 2;
									}
									break;
								case 3:
									cout << "You counter attack with " << playerCharacter->getWeapon(p_selection1)->getName() << " and " << playerCharacter->getWeapon(p_selection2)->getName() << '\n';
									weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter->getWeapon(p_selection2), playerCharacter);
									weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter->getWeapon(p_selection2), playerCharacter, opponent, true);
									switch (deathCheck(playerCharacter, opponent)) {
									case 1:
										return 1;
									case 2:
										return 2;
									}
									break;
								}
							}
							break;
						}
						else {
							cout << "The effects of " << opponent->getWeapon(e_selection1)->getName() << " and " << opponent->getWeapon(e_selection2)->getName() << " are countered!\n";
							switch (deathCheck(playerCharacter, opponent)) {
							case 1:
								return 1;
							case 2:
								return 2;
							}
							break;
						}
					}
				}
			}
			weaponAttack(opponent->getWeapon(e_selection1), opponent->getWeapon(e_selection2), opponent, playerCharacter);
			switch (deathCheck(playerCharacter, opponent)) {
			case 1:
				return 1;
			case 2:
				return 2;
			}
			if (opponent->getWeapon(e_selection1)->getNoCounterAttack() && opponent->getWeapon(e_selection2)->getNoCounterAttack()) {
				break;
			}
			if (rng(0.f, 1.f) < playerCharacter->getCounterAttackChance()) {
				switch (playerCharacter->chooseAction(&p_selection1, &p_selection2, opponent->getName(), 3)) {
				case 1:
					cout << "You counter attack with " << playerCharacter->getWeapon(p_selection1)->getName() << '\n';
					weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter);
					weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter, opponent, true);
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
					break;
				case 2:
					cout << "You counter attack by casting " << playerCharacter->getSpell(p_selection1)->getName() << '\n';
					spellDeclare(playerCharacter->getSpell(p_selection1), playerCharacter);
					spellCast(playerCharacter->getSpell(p_selection1), playerCharacter, opponent, true);
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
					break;
				case 3:
					cout << "You counter attack with " << playerCharacter->getWeapon(p_selection1)->getName() << " and " << playerCharacter->getWeapon(p_selection2)->getName() << '\n';
					weaponDeclare(playerCharacter->getWeapon(p_selection1), playerCharacter->getWeapon(p_selection2), playerCharacter);
					weaponAttack(playerCharacter->getWeapon(p_selection1), playerCharacter->getWeapon(p_selection2), playerCharacter, opponent, true);
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
					break;
				}
			}
			break;
		}
		firstTurn = false;
	}
	return 1;
}

void spellCast(spell* magic, player* caster, enemy* target, bool counter) {
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
}

void spellCast(spell* magic, enemy* caster, player* target, bool counter) {
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
			caster->modifyHealth(damageBuffer);
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
			caster->modifyHealth(damageBuffer);
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
			caster->modifyHealth(damageBuffer);
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
			caster->modifyHealth(damageBuffer);
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
			caster->modifyHealth(damageBuffer);
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
			caster->modifyHealth(damageBuffer);
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

void weaponAttack(weapon* weaponry, player* attacker, enemy* target, bool counter) {
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
}

void weaponAttack(weapon* weaponry, enemy* attacker, player* target, bool counter) {
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
			attacker->modifyHealth(damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	damageBuffer = weaponry->getFlatMagicDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage((static_cast<short>((damageBuffer + attacker->getFlatMagicDamageModifier()) * (1 + attacker->getPropMagicDamageModifier()))), 2);
		if (weaponry->getLifelink()) {
			attacker->modifyHealth(damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	damageBuffer = weaponry->getFlatArmourPiercingDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage((static_cast<short>((damageBuffer + attacker->getFlatArmourPiercingDamageModifier()) * (1 + attacker->getPropArmourPiercingDamageModifier()))), 3);
		if (weaponry->getLifelink()) {
			attacker->modifyHealth(damageBuffer);
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
			attacker->modifyHealth(damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	damageBuffer = weaponry->getFlatMagicDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage((static_cast<short>((damageBuffer + attacker->getFlatMagicDamageModifier()) * (1 + attacker->getPropMagicDamageModifier()))), 2);
		if (weaponry->getLifelink()) {
			attacker->modifyHealth(damageBuffer);
		}
	}
	else if (damageBuffer < 0) {
		target->flatDamage(damageBuffer);
	}
	damageBuffer = weaponry->getFlatArmourPiercingDamage();
	if (damageBuffer > 0) {
		damageBuffer = target->flatDamage((static_cast<short>((damageBuffer + attacker->getFlatArmourPiercingDamageModifier()) * (1 + attacker->getPropArmourPiercingDamageModifier()))), 3);
		if (weaponry->getLifelink()) {
			attacker->modifyHealth(damageBuffer);
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

void weaponAttack(weapon* weapon1, weapon* weapon2, player* attacker, enemy* target, bool counter) {
	if (counter) { //If weapon2 hits more times, run the function with them swapped
		if (weapon2->getCounterHits() > weapon1->getCounterHits()) {
			weaponAttack(weapon2, weapon1, attacker, target, counter);
			return;
		}
	}
	else {
		if (weapon2->getHitCount() > weapon1->getHitCount()) {
			weaponAttack(weapon2, weapon1, attacker, target, counter);
			return;
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
}

void weaponAttack(weapon* weapon1, weapon* weapon2, enemy* attacker, player* target, bool counter) {
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

unsigned char deathCheck(player* playerCharacter, enemy* opponent) {
	if (opponent->getHealth() <= 0 && opponent->getDeathSpell()->getReal()) {
		cout << opponent->getName() << " is dead. On death, it casts " << opponent->getDeathSpell()->getName() << '\n';
		spellCast(opponent->getDeathSpell(), playerCharacter);
		if (playerCharacter->getHealth() <= 0) {
			cout << "You are dead\n";
			return 2;
		}
		return 1;
	}
	if (playerCharacter->getHealth() <= 0) {
		cout << "You are dead\n";
		return 2;
	}
	if (opponent->getHealth() <= 0) {
		cout << opponent->getName() << " is dead\n";
		return 1;
	}
	return 0;
}

unsigned char battleMode() {
	bool done = false;
	while (!done) {
		cout << "To enable custom data, enter 1.\nTo play without custom data, enter 0.\n";
		switch (userChoice(0, 1)) {
		case 0:
			g_useCustomData = false;
			break;
		case 1:
			g_useCustomData = true;
			break;
		}
		g_manaName.loadFromFile("MANA");
		g_projName.loadFromFile("PROJECTILE");
		player playerCharacter;
		string blueprintSelection;
		while (true) {
			cout << "Enter the blueprint name of the class you wish to play as:\n";
			getline(cin, blueprintSelection);
			try {
				playerCharacter.loadClass(blueprintSelection);
				break;
			}
			catch (int err) {
				cout << err;
			}
		}
		enemy opponent;
		while (!done) {
			while (!done) {
				cout << "To give yourself a new piece of equipment, enter its blueprint name, prefixed by the first letter of its type and an underscore.\n(h_ for a helmet, t_ for a chestplate, l_ for leggings, f_ for footwear, w_ for a weapon, s_ for a spell)\nIf you wish not to add new equipment, enter EMPTY\n";
				getline(cin, blueprintSelection);
				if (blueprintSelection == "EMPTY") {
					goto endEquip;
				}
				switch (blueprintListSelector(&blueprintSelection)) {
				case 0:
					cout << "No item selected\n";
					break;
				case 1:
				{
					armourHead helmet(blueprintSelection);
					playerCharacter.equip(&helmet);
					break;
				}
				case 2:
				{
					armourTorso chesplate(blueprintSelection);
					playerCharacter.equip(&chesplate);
					break;
				}
				case 3:
				{
					armourLegs greaves(blueprintSelection);
					playerCharacter.equip(&greaves);
					break;
				}
				case 4:
				{
					armourFeet boots(blueprintSelection);
					playerCharacter.equip(&boots);
					break;
				}
				case 5:
				{
					weapon w(blueprintSelection);
					playerCharacter.equip(&w);
					break;
				}
				case 6:
				{
					spell s(blueprintSelection);
					playerCharacter.equip(&s);
					break;
				}
				}
				cout << "To add another item, enter 1.\nTo stop adding items, enter 2.\n";
				if (userChoice(1, 2) == 2) {
					done = true;
				}
			}
			done = false;
		endEquip:
			playerCharacter.calculateModifiers();
			while (!done) {
				cout << "Enter the blueprint name of the enemy you wish to fight:\n";
				getline(cin, blueprintSelection);
				opponent.loadFromFile(blueprintSelection);
				if (opponent.getReal()) {
					done = true;
				}
				else {
					cout << "Failed to load enemy\n";
				}
			}
			done = false;
			cout << opponent.getIntroduction() << '\n';
			if (battleHandler(&playerCharacter, &opponent) == 2) { //Do the fight, do this if the player died
				cout << "To choose a new class, enter 2.\nTo exit to the main menu, enter 0.\n";
				vector<short> options({ 0, 2 });
				switch (userChoice(options)) {
				case 0:
					return 1;
				case 2:
					done = true;
					break;
				}
			}
			else {
				cout << "To fight another enemy, enter 1.\nTo choose a new class, enter 2.\nTo exit to the main menu, enter 0.\n";
				switch (userChoice(0, 2)) {
				case 0:
					return 1;
				case 1:
					playerCharacter.reset();
					break;
				case 2:
					done = true;
					break;
				}
			}
		}
		done = false;
	}
	return 0;
}