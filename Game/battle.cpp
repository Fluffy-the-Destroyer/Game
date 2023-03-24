#include "battle.h"
#include "rng.h"
#include <iostream>
#include "resources.h"
#include "inputs.h"
#include "armour.h"
#include "blueprints.h"
#include <thread>
using namespace std;

extern resource g_manaName, g_projName;
extern bool g_useCustomData;

//Error codes:
// 6: Trying to access slot which is out of range

uint8_t battleHandler(player* playerCharacter, enemy* opponent, int8_t firstGo) {
	//Check if enemy dies immediately
	switch (deathCheck(playerCharacter, opponent)) {
	case 1:
		return 1;
	case 2:
		return 2;
	}
	playerCharacter->resetBonusActions();
	opponent->resetBonusActions();
	uint8_t p_selection1 = 0, p_selection2 = 0, e_selection1 = 0, e_selection2 = 0; //For holding weapon/spell selection and action choice
	short health; //For holding combatants' health
	bool firstTurn = true; //Is it the first turn
	//Determine who goes first
	if (firstGo == 1) {}
	else if (firstGo == -1) {
		cout << opponent->getName() << " goes first\n";
		this_thread::sleep_for(chrono::milliseconds(500));
		goto enemyTurn;
	}
	else {
		short p_initiative = playerCharacter->rollInitiative();
		short e_initiative = opponent->rollInitiative();
		if (e_initiative > p_initiative) { //If enemy rolled higher, skip player's first turn
			cout << opponent->getName() << " goes first\n";
			this_thread::sleep_for(chrono::milliseconds(500));
			goto enemyTurn;
		}
		else if (p_initiative == e_initiative) { //If rolled the same, pick randomly
			if (rng(1, 2) == 1) {
				cout << opponent->getName() << " goes first\n";
				this_thread::sleep_for(chrono::milliseconds(500));
				goto enemyTurn;
			}
		}
	}
	cout << "You go first\n";
	this_thread::sleep_for(chrono::milliseconds(500));
	while (true) { //Turn cycle loop
	//playerTurn:
		this_thread::sleep_for(chrono::milliseconds(500));
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
			this_thread::sleep_for(chrono::milliseconds(500));
			if (opponent->chooseAction(&e_selection1, &e_selection2, 1, playerCharacter->getWeapon(p_selection1)->getName(), "", firstTurn) == 2) { //Get enemy action choice
				cout << opponent->getName() << " casts " << opponent->getSpell(e_selection1)->getName() << " in response\n";
				spellDeclare(opponent->getSpell(e_selection1), opponent);
				this_thread::sleep_for(chrono::milliseconds(500));
				spellCast(opponent->getSpell(e_selection1), opponent, playerCharacter);
				if (opponent->getSpell(e_selection1)->getPropDamage() > 0 || playerCharacter->getHealth() < health) { //Only run death check if enemy's spell was damaging
					switch (deathCheck(playerCharacter, opponent)) {
					case 1:
						return 1;
					case 2:
						return 2;
					}
				}
				if (opponent->getSpell(e_selection1)->getCounterSpell() >= 2) { //Check counterSpell
					if (playerCharacter->getWeapon(p_selection1)->getNoCounter()) {
						cout << playerCharacter->getWeapon(p_selection1)->getName() << " cannot be countered!\n";
						opponent->addNoCounter(1, playerCharacter->getWeapon(p_selection1)->getName()); //Tell enemy it cannot be countered
					}
					else {
						cout << "The effects of " << playerCharacter->getWeapon(p_selection1)->getName() << " are countered\n";
						switch (deathCheck(playerCharacter, opponent)) {
						case 1:
							return 1;
						case 2:
							return 2;
						}
						break;
					}
					this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
			this_thread::sleep_for(chrono::milliseconds(500));
			if (opponent->chooseAction(&e_selection1, &e_selection2, 2, playerCharacter->getSpell(p_selection1)->getName(), "", firstTurn) == 2) {
				cout << opponent->getName() << " casts " << opponent->getSpell(e_selection1)->getName() << " in response\n";
				spellDeclare(opponent->getSpell(e_selection1), opponent);
				this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
						this_thread::sleep_for(chrono::milliseconds(500));
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
						this_thread::sleep_for(chrono::milliseconds(500));
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
						this_thread::sleep_for(chrono::milliseconds(500));
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
			this_thread::sleep_for(chrono::milliseconds(500));
			if (opponent->chooseAction(&e_selection1, &e_selection2, 4, playerCharacter->getWeapon(p_selection1)->getName(), playerCharacter->getWeapon(p_selection2)->getName(), firstTurn) == 2) {
				cout << opponent->getName() << " casts " << opponent->getSpell(e_selection1)->getName() << " in response\n";
				spellDeclare(opponent->getSpell(e_selection1), opponent);
				this_thread::sleep_for(chrono::milliseconds(500));
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
							this_thread::sleep_for(chrono::milliseconds(500));
						}
						else {
							cout << "The effects of " << playerCharacter->getWeapon(p_selection2)->getName() << " are countered, but " << playerCharacter->getWeapon(p_selection1)->getName() << " cannot be countered!\n";
							this_thread::sleep_for(chrono::milliseconds(500));
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
									this_thread::sleep_for(chrono::milliseconds(500));
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
									this_thread::sleep_for(chrono::milliseconds(500));
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
									this_thread::sleep_for(chrono::milliseconds(500));
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
							this_thread::sleep_for(chrono::milliseconds(500));
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
									this_thread::sleep_for(chrono::milliseconds(500));
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
									this_thread::sleep_for(chrono::milliseconds(500));
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
									this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
	enemyTurn:
		this_thread::sleep_for(chrono::milliseconds(500));
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
			this_thread::sleep_for(chrono::milliseconds(500));
			if (playerCharacter->chooseAction(&p_selection1, &p_selection2, opponent->getName(), 1, opponent->getWeapon(e_selection1)->getName()) == 2) {
				cout << "You cast " << playerCharacter->getSpell(p_selection1)->getName() << " in response\n";
				spellDeclare(playerCharacter->getSpell(p_selection1), playerCharacter);
				this_thread::sleep_for(chrono::milliseconds(500));
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
						this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
			this_thread::sleep_for(chrono::milliseconds(500));
			if (playerCharacter->chooseAction(&p_selection1, &p_selection2, opponent->getName(), 2, opponent->getSpell(e_selection1)->getName()) == 2) {
				cout << "You cast " << playerCharacter->getSpell(p_selection1)->getName() << " in response\n";
				spellDeclare(playerCharacter->getSpell(p_selection1), playerCharacter);
				this_thread::sleep_for(chrono::milliseconds(500));
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
						this_thread::sleep_for(chrono::milliseconds(500));
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
						this_thread::sleep_for(chrono::milliseconds(500));
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
						this_thread::sleep_for(chrono::milliseconds(500));
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
						this_thread::sleep_for(chrono::milliseconds(500));
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
			this_thread::sleep_for(chrono::milliseconds(500));
			if (playerCharacter->chooseAction(&p_selection1, &p_selection2, opponent->getName(), 4, opponent->getWeapon(e_selection1)->getName(), opponent->getWeapon(e_selection2)->getName()) == 2) {
				cout << "You cast " << playerCharacter->getSpell(p_selection1)->getName() << " in response\n";
				spellDeclare(playerCharacter->getSpell(p_selection1), playerCharacter);
				this_thread::sleep_for(chrono::milliseconds(500));
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
							this_thread::sleep_for(chrono::milliseconds(500));
						}
						else {
							cout << "The effects of " << opponent->getWeapon(e_selection2)->getName() << " are countered, but " << opponent->getWeapon(e_selection1)->getName() << " cannot be countered!\n";
							this_thread::sleep_for(chrono::milliseconds(500));
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
									this_thread::sleep_for(chrono::milliseconds(500));
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
									this_thread::sleep_for(chrono::milliseconds(500));
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
									this_thread::sleep_for(chrono::milliseconds(500));
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
							this_thread::sleep_for(chrono::milliseconds(500));
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
									this_thread::sleep_for(chrono::milliseconds(500));
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
									this_thread::sleep_for(chrono::milliseconds(500));
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
									this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
					this_thread::sleep_for(chrono::milliseconds(500));
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
	if (magic->getEffectType() / 10 == 1 || magic->getEffectType() / 10 == 2 || magic->getEffectType() % 10 == 1 || magic->getEffectType() % 10 == 2) {
		cout << "Caster effects:\n";
	}
	caster->propDamage(magic->getPropSelfDamage());
	if (magic->getPropSelfDamage() > 0) {
		cout << -100 * magic->getPropSelfDamage() << "% health\n";
	}
	else if (magic->getPropSelfDamage() < 0) {
		cout << -100 * magic->getPropSelfDamage() << "% of health recovered\n";
	}
	if (magic->getEffectType() / 10 == 1 || magic->getEffectType() / 10 == 2) {
		short healthLoss = caster->flatDamage(magic->getFlatSelfDamage(), magic->getFlatSelfMagicDamage(), magic->getFlatSelfArmourPiercingDamage(), magic->getSelfOverHeal());
		if (healthLoss > 0) {
			cout << healthLoss << " damage\n";
		}
		else if (healthLoss < 0) {
			cout << -healthLoss << " healing\n";
		}
		else {
			cout << "No damage\n";
		}
	}
	if (magic->getEffectType() % 10 == 1 || magic->getEffectType() % 10 == 2) { //Spell affects caster
		cout << showpos;
		if (magic->getSelfPoison() != 0 || magic->getSelfBleed() != 0 || magic->getTempRegenSelf() != 0) {
			if (magic->getSelfPoison() != 0) {
				if (caster->modifyPoison(magic->getSelfPoison())) {
					cout << magic->getSelfPoison() << " poison, ";
				}
				else {
					cout << "Poison resisted, ";
				}
			}
			if (magic->getSelfBleed() != 0) {
				if (caster->modifyBleed(magic->getSelfBleed())) {
					cout << magic->getSelfBleed() << " bleed, ";
				}
				else {
					cout << "Bleed resisted, ";
				}
			}
			if (magic->getTempRegenSelf() != 0) {
				caster->modifyTempRegen(magic->getTempRegenSelf());
				cout << magic->getTempRegenSelf() << " regeneration, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getPoisonResistModifier() != 0 || magic->getBleedResistModifier() != 0) {
			if (magic->getPoisonResistModifier() != 0) {
				caster->modifyPoisonResist(magic->getPoisonResistModifier());
				cout << magic->getPoisonResistModifier() << " poison resist, ";
			}
			if (magic->getBleedResistModifier() != 0) {
				caster->modifyBleedResist(magic->getBleedResistModifier());
				cout << magic->getBleedResistModifier() << " bleed resist, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getMaxHealthModifier() != 0 || magic->getTurnRegenModifier() != 0 || magic->getBattleRegenModifier() != 0) {
			if (magic->getMaxHealthModifier() != 0) {
				caster->modifyMaxHealth(magic->getMaxHealthModifier());
				cout << magic->getMaxHealthModifier() << " max health, ";
			}
			if (magic->getTurnRegenModifier() != 0) {
				caster->modifyTurnRegen(magic->getTurnRegenModifier());
				cout << magic->getTurnRegenModifier() << " health per turn, ";
			}
			if (magic->getBattleRegenModifier() != 0) {
				caster->modifyBattleRegen(magic->getBattleRegenModifier());
				cout << magic->getBattleRegenModifier() << " health at end of battle, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getMaxManaModifier() != 0 || magic->getTurnManaRegenModifier() != 0 || magic->getBattleManaRegenModifier() != 0) {
			if (magic->getMaxManaModifier() != 0) {
				caster->modifyMaxMana(magic->getMaxManaModifier());
				cout << magic->getMaxManaModifier() << " max " << g_manaName.plural() << ", ";
			}
			if (magic->getTurnManaRegenModifier() == 1 || magic->getTurnManaRegenModifier() == -1) {
				caster->modifyTurnManaRegen(magic->getTurnManaRegenModifier());
				cout << magic->getTurnManaRegenModifier() << ' ' << g_manaName.singular() << " per turn, ";
			}
			else if (magic->getTurnManaRegenModifier() != 0) {
				caster->modifyTurnManaRegen(magic->getTurnManaRegenModifier());
				cout << magic->getTurnManaRegenModifier() << ' ' << g_manaName.plural() << " per turn, ";
			}
			if (magic->getBattleManaRegenModifier() == 1 || magic->getBattleManaRegenModifier() == -1) {
				caster->modifyBattleManaRegen(magic->getBattleManaRegenModifier());
				cout << magic->getBattleManaRegenModifier() << ' ' << g_manaName.singular() << " at end of battle, ";
			}
			else if (magic->getBattleManaRegenModifier() != 0) {
				caster->modifyBattleManaRegen(magic->getBattleManaRegenModifier());
				cout << magic->getBattleManaRegenModifier() << ' ' << g_manaName.plural() << " at end of battle, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getFlatArmourModifier() != 0 || magic->getFlatMagicArmourModifier() != 0) {
			if (magic->getFlatArmourModifier() != 0) {
				caster->modifyFlatArmour(magic->getFlatArmourModifier());
				cout << magic->getFlatArmourModifier() << " physical armour, ";
			}
			if (magic->getFlatMagicArmourModifier() != 0) {
				caster->modifyFlatMagicArmour(magic->getFlatMagicArmourModifier());
				cout << magic->getFlatMagicArmourModifier() << " magic armour, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getPropArmourModifier() != 0 || magic->getPropMagicArmourModifier() != 0) {
			if (magic->getPropArmourModifier() != 0) {
				caster->modifyPropArmour(magic->getPropArmourModifier());
				cout << 100 * magic->getPropArmourModifier() << "% physical damage received, ";
			}
			if (magic->getPropMagicArmourModifier() != 0) {
				caster->modifyPropMagicArmour(magic->getPropMagicArmourModifier());
				cout << 100 * magic->getPropMagicArmourModifier() << "% magic damage received, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getFlatDamageModifier() != 0 || magic->getFlatMagicDamageModifier() != 0 || magic->getFlatArmourPiercingDamageModifier() != 0) {
			if (magic->getFlatDamageModifier() != 0) {
				caster->modifyFlatDamageModifier(magic->getFlatDamageModifier());
				cout << magic->getFlatDamageModifier() << " physical damage dealt, ";
			}
			if (magic->getFlatMagicDamageModifier() != 0) {
				caster->modifyFlatMagicDamageModifier(magic->getFlatMagicDamageModifier());
				cout << magic->getFlatMagicDamageModifier() << " magic damage dealt, ";
			}
			if (magic->getFlatArmourPiercingDamageModifier() != 0) {
				caster->modifyFlatArmourPiercingDamageModifier(magic->getFlatArmourPiercingDamageModifier());
				cout << magic->getFlatArmourPiercingDamageModifier() << " armour piercing damage dealt, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getPropDamageModifier() != 0 || magic->getPropMagicDamageModifier() != 0 || magic->getPropArmourPiercingDamageModifier() != 0) {
			if (magic->getPropDamageModifier() != 0) {
				caster->modifyPropDamageModifier(magic->getPropDamageModifier());
				cout << 100 * magic->getPropDamageModifier() << "% physical damage dealt, ";
			}
			if (magic->getPropMagicDamageModifier() != 0) {
				caster->modifyPropMagicDamageModifier(magic->getPropMagicDamageModifier());
				cout << 100 * magic->getPropMagicDamageModifier() << "% magic damage dealt, ";
			}
			if (magic->getPropArmourPiercingDamageModifier() != 0) {
				caster->modifyPropArmourPiercingDamageModifier(magic->getPropArmourPiercingDamageModifier());
				cout << 100 * magic->getPropArmourPiercingDamageModifier() << "% armour piercing damage dealt, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getEvadeChanceModifier() != 0) {
			caster->modifyEvadeChance(magic->getEvadeChanceModifier());
			cout << 100 * magic->getEvadeChanceModifier() << "% evade chance\n";
		}
		if (magic->getCounterAttackChanceModifier() != 0) {
			caster->modifyCounterAttackChance(magic->getCounterAttackChanceModifier());
			cout << 100 * magic->getCounterAttackChanceModifier() << "% counter attack chance\n";
		}
		if (magic->getBonusActionsModifier() != 0) {
			caster->modifyBonusActions(magic->getBonusActionsModifier());
			cout << magic->getBonusActionsModifier() << " bonus actions\n";
		}
	}
	cout << noshowpos;
	this_thread::sleep_for(chrono::milliseconds(500));
	if (magic->getEffectType() % 10 < 2 && magic->getEffectType() / 10 < 2) {
		return;
	}
	uint8_t hits;
	if (counter) {
		hits = magic->getCounterHits();
	}
	else {
		hits = magic->getHitCount();
	}
	for (uint8_t i = 0; i < hits; i++) {
		spellHit(magic, caster, target);
	}
	this_thread::sleep_for(chrono::milliseconds(400));
}

void spellCast(spell* magic, enemy* caster, player* target, bool counter) {
	if (magic->getEffectType() / 10 == 1 || magic->getEffectType() / 10 == 2 || magic->getEffectType() % 10 == 1 || magic->getEffectType() % 10 == 2) {
		cout << "Caster effects:\n";
	}
	caster->propDamage(magic->getPropSelfDamage());
	if (magic->getPropSelfDamage() > 0) {
		cout << -100 * magic->getPropSelfDamage() << "% health\n";
	}
	else if (magic->getPropSelfDamage() < 0) {
		cout << -100 * magic->getPropSelfDamage() << "% of health recovered\n";
	}
	if (magic->getEffectType() / 10 == 1 || magic->getEffectType() / 10 == 2) {
		short healthLoss = caster->flatDamage(magic->getFlatSelfDamage(), magic->getFlatSelfMagicDamage(), magic->getFlatSelfArmourPiercingDamage(), magic->getSelfOverHeal());
		if (healthLoss > 0) {
			cout << healthLoss << " damage\n";
		}
		else if (healthLoss < 0) {
			cout << -healthLoss << " healing\n";
		}
		else {
			cout << "No damage\n";
		}
	}
	if (magic->getEffectType() % 10 == 1 || magic->getEffectType() % 10 == 2) { //Spell affects caster
		cout << showpos;
		if (magic->getSelfPoison() != 0 || magic->getSelfBleed() != 0 || magic->getTempRegenSelf() != 0) {
			if (magic->getSelfPoison() != 0) {
				if (caster->modifyPoison(magic->getSelfPoison())) {
					cout << magic->getSelfPoison() << " poison, ";
				}
				else {
					cout << "Poison resisted, ";
				}
			}
			if (magic->getSelfBleed() != 0) {
				if (caster->modifyBleed(magic->getSelfBleed())) {
					cout << magic->getSelfBleed() << " bleed, ";
				}
				else {
					cout << "Bleed resisted, ";
				}
			}
			if (magic->getTempRegenSelf() != 0) {
				caster->modifyTempRegen(magic->getTempRegenSelf());
				cout << magic->getTempRegenSelf() << " regeneration, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getPoisonResistModifier() != 0 || magic->getBleedResistModifier() != 0) {
			if (magic->getPoisonResistModifier() != 0) {
				caster->modifyPoisonResist(magic->getPoisonResistModifier());
				cout << magic->getPoisonResistModifier() << " poison resist, ";
			}
			if (magic->getBleedResistModifier() != 0) {
				caster->modifyBleedResist(magic->getBleedResistModifier());
				cout << magic->getBleedResistModifier() << " bleed resist, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getMaxHealthModifier() != 0 || magic->getTurnRegenModifier() != 0) {
			if (magic->getMaxHealthModifier() != 0) {
				caster->modifyMaxHealth(magic->getMaxHealthModifier());
				cout << magic->getMaxHealthModifier() << " max health, ";
			}
			if (magic->getTurnRegenModifier() != 0) {
				caster->modifyTurnRegen(magic->getTurnRegenModifier());
				cout << magic->getTurnRegenModifier() << " health per turn, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getMaxManaModifier() != 0 || magic->getTurnManaRegenModifier() != 0) {
			if (magic->getMaxManaModifier() != 0) {
				caster->modifyMaxMana(magic->getMaxManaModifier());
				cout << magic->getMaxManaModifier() << " max " << g_manaName.plural() << ", ";
			}
			if (magic->getTurnManaRegenModifier() == 1 || magic->getTurnManaRegenModifier() == -1) {
				caster->modifyTurnManaRegen(magic->getTurnManaRegenModifier());
				cout << magic->getTurnManaRegenModifier() << ' ' << g_manaName.singular() << " per turn, ";
			}
			else if (magic->getTurnManaRegenModifier() != 0) {
				caster->modifyTurnManaRegen(magic->getTurnManaRegenModifier());
				cout << magic->getTurnManaRegenModifier() << ' ' << g_manaName.plural() << " per turn, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getFlatArmourModifier() != 0 || magic->getFlatMagicArmourModifier() != 0) {
			if (magic->getFlatArmourModifier() != 0) {
				caster->modifyFlatArmour(magic->getFlatArmourModifier());
				cout << magic->getFlatArmourModifier() << " physical armour, ";
			}
			if (magic->getFlatMagicArmourModifier() != 0) {
				caster->modifyFlatMagicArmour(magic->getFlatMagicArmourModifier());
				cout << magic->getFlatMagicArmourModifier() << " magic armour, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getPropArmourModifier() != 0 || magic->getPropMagicArmourModifier() != 0) {
			if (magic->getPropArmourModifier() != 0) {
				caster->modifyPropArmour(magic->getPropArmourModifier());
				cout << 100 * magic->getPropArmourModifier() << "% physical damage received, ";
			}
			if (magic->getPropMagicArmourModifier() != 0) {
				caster->modifyPropMagicArmour(magic->getPropMagicArmourModifier());
				cout << 100 * magic->getPropMagicArmourModifier() << "% magic damage received, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getFlatDamageModifier() != 0 || magic->getFlatMagicDamageModifier() != 0 || magic->getFlatArmourPiercingDamageModifier() != 0) {
			if (magic->getFlatDamageModifier() != 0) {
				caster->modifyFlatDamageModifier(magic->getFlatDamageModifier());
				cout << magic->getFlatDamageModifier() << " physical damage dealt, ";
			}
			if (magic->getFlatMagicDamageModifier() != 0) {
				caster->modifyFlatMagicDamageModifier(magic->getFlatMagicDamageModifier());
				cout << magic->getFlatMagicDamageModifier() << " magic damage dealt, ";
			}
			if (magic->getFlatArmourPiercingDamageModifier() != 0) {
				caster->modifyFlatArmourPiercingDamageModifier(magic->getFlatArmourPiercingDamageModifier());
				cout << magic->getFlatArmourPiercingDamageModifier() << " armour piercing damage dealt, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getPropDamageModifier() != 0 || magic->getPropMagicDamageModifier() != 0 || magic->getPropArmourPiercingDamageModifier() != 0) {
			if (magic->getPropDamageModifier() != 0) {
				caster->modifyPropDamageModifier(magic->getPropDamageModifier());
				cout << 100 * magic->getPropDamageModifier() << "% physical damage dealt, ";
			}
			if (magic->getPropMagicDamageModifier() != 0) {
				caster->modifyPropMagicDamageModifier(magic->getPropMagicDamageModifier());
				cout << 100 * magic->getPropMagicDamageModifier() << "% magic damage dealt, ";
			}
			if (magic->getPropArmourPiercingDamageModifier() != 0) {
				caster->modifyPropArmourPiercingDamageModifier(magic->getPropArmourPiercingDamageModifier());
				cout << 100 * magic->getPropArmourPiercingDamageModifier() << "% armour piercing damage dealt, ";
			}
			cout << "\b\b\n";
		}
		if (magic->getEvadeChanceModifier() != 0) {
			caster->modifyEvadeChance(magic->getEvadeChanceModifier());
			cout << 100 * magic->getEvadeChanceModifier() << "% evade chance\n";
		}
		if (magic->getCounterAttackChanceModifier() != 0) {
			caster->modifyCounterAttackChance(magic->getCounterAttackChanceModifier());
			cout << 100 * magic->getCounterAttackChanceModifier() << "% counter attack chance\n";
		}
		if (magic->getBonusActionsModifier() != 0) {
			caster->modifyBonusActions(magic->getBonusActionsModifier());
			cout << magic->getBonusActionsModifier() << " bonus actions\n";
		}
	}
	cout << noshowpos;
	this_thread::sleep_for(chrono::milliseconds(500));
	if (magic->getEffectType() % 10 < 2 && magic->getEffectType() / 10 < 2) {
		return;
	}
	cout << "Target effects:\n";
	uint8_t hits;
	if (counter) {
		hits = magic->getCounterHits();
	}
	else {
		hits = magic->getHitCount();
	}
	for (uint8_t i = 0; i < hits; i++) {
		spellHit(magic, caster, target);
	}
	this_thread::sleep_for(chrono::milliseconds(400));
}

uint8_t spellCast(spell* magic, player* target) {
	if (magic->getEffectType() % 10 < 2 && magic->getEffectType() / 10 < 2 && magic->getInitiativeModifier() == 0) {
		return 0;
	}
	uint8_t hits = magic->getHitCount();
	for (uint8_t i = 0; i < hits; i++) {
		spellHit(magic, target);
	}
	this_thread::sleep_for(chrono::milliseconds(400));
	if (target->getHealth() <= 0) {
		return 1;
	}
	return 0;
}

void spellHit(spell* magic, player* caster, enemy* target) {
	if (!magic->getNoEvade() && rng(0.f, 1.f) < target->getEvadeChance()) {
		cout << "Evade!\n";
		this_thread::sleep_for(chrono::milliseconds(100));
		return;
	}
	cout << "Hit!\n";
	target->propDamage(magic->getPropDamage());
	if (magic->getPropDamage() > 0) {
		cout << -100 * magic->getPropDamage() << "% health\n";
	}
	else if (magic->getPropDamage() < 0) {
		cout << -100 * magic->getPropDamage() << "% of health recovered\n";
	}
	if (magic->getEffectType() / 10 > 1) {
		short healthSteal = max((short)0, target->getHealth());
		short p = magic->getFlatDamage(), m = magic->getFlatMagicDamage(), a = magic->getFlatArmourPiercingDamage();
		caster->applyDamageModifiers(&p, &m, &a);
		short healthLoss = target->flatDamage(p, m, a, magic->getTargetOverHeal());
		if (healthLoss > 0) {
			cout << healthLoss << " damage";
			if (magic->getLifeLink()) {
				healthSteal = min(healthSteal, healthLoss);
				if (healthSteal > 0) {
					cout << " (caster is healed for " << healthSteal << " by lifeLink)";
					caster->modifyHealth(healthSteal);
				}
			}
			cout << '\n';
		}
		else if (healthLoss < 0) {
			cout << -healthLoss << " healing\n";
		}
		else {
			cout << "No damage\n";
		}
	}
	if (magic->getEffectType() % 10 < 2) {
		this_thread::sleep_for(chrono::milliseconds(100));
		return;
	}
	cout << showpos;
	if (magic->getPoison() != 0 || magic->getBleed() != 0 || magic->getTempRegen() != 0) {
		if (magic->getPoison() != 0) {
			if (target->modifyPoison(magic->getPoison())) {
				cout << magic->getPoison() << " poison, ";
			}
			else {
				cout << "Poison resisted, ";
			}
		}
		if (magic->getBleed() != 0) {
			if (target->modifyBleed(magic->getBleed())) {
				cout << magic->getBleed() << " bleed, ";
			}
			else {
				cout << "Bleed resisted, ";
			}
		}
		if (magic->getTempRegen() != 0) {
			target->modifyTempRegen(magic->getTempRegen());
			cout << magic->getTempRegen() << " regeneration, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getMaxHealthModifierEnemy() != 0 || magic->getTurnRegenModifierEnemy() != 0) {
		if (magic->getMaxHealthModifierEnemy() != 0) {
			target->modifyMaxHealth(magic->getMaxHealthModifierEnemy());
			cout << magic->getMaxHealthModifierEnemy() << " max health, ";
		}
		if (magic->getTurnRegenModifierEnemy() != 0) {
			target->modifyTurnRegen(magic->getTurnRegenModifierEnemy());
			cout << magic->getTurnRegenModifierEnemy() << " health per turn, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getMaxManaModifierEnemy() != 0 || magic->getManaChangeEnemy() != 0 || magic->getTurnManaRegenModifierEnemy() != 0) {
		if (magic->getMaxManaModifierEnemy() != 0) {
			target->modifyMaxMana(magic->getMaxManaModifierEnemy());
			cout << magic->getMaxManaModifierEnemy() << " max " << g_manaName.plural() << ", ";
		}
		if (magic->getManaChangeEnemy() == 1 || magic->getManaChangeEnemy() == -1) {
			target->modifyMana(magic->getManaChangeEnemy());
			cout << magic->getManaChangeEnemy() << ' ' << g_manaName.singular() << ", ";
		}
		else if (magic->getManaChangeEnemy() != 0) {
			target->modifyMana(magic->getManaChangeEnemy());
			cout << magic->getManaChangeEnemy() << ' ' << g_manaName.plural() << ", ";
		}
		if (magic->getTurnManaRegenModifierEnemy() == 1 || magic->getTurnManaRegenModifierEnemy() == -1) {
			target->modifyTurnManaRegen(magic->getTurnManaRegenModifierEnemy());
			cout << magic->getTurnManaRegenModifierEnemy() << ' ' << g_manaName.singular() << " per turn, ";
		}
		else if (magic->getTurnManaRegenModifierEnemy() != 0) {
			target->modifyTurnManaRegen(magic->getTurnManaRegenModifierEnemy());
			cout << magic->getTurnManaRegenModifierEnemy() << ' ' << g_manaName.plural() << " per turn, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getPoisonResistModifierEnemy() != 0 || magic->getBleedResistModifierEnemy() != 0) {
		if (magic->getPoisonResistModifierEnemy() != 0) {
			target->modifyPoisonResist(magic->getPoisonResistModifierEnemy());
			cout << 100 * magic->getPoisonResistModifierEnemy() << "% poison resist, ";
		}
		if (magic->getBleedResistModifierEnemy() != 0) {
			target->modifyBleedResist(magic->getBleedResistModifierEnemy());
			cout << 100 * magic->getBleedResistModifierEnemy() << "% bleed resist, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getFlatArmourModifierEnemy() != 0 || magic->getFlatMagicArmourModifierEnemy() != 0) {
		if (magic->getFlatArmourModifierEnemy() != 0) {
			target->modifyFlatArmour(magic->getFlatArmourModifierEnemy());
			cout << magic->getFlatArmourModifierEnemy() << " physical armour, ";
		}
		if (magic->getFlatMagicArmourModifierEnemy() != 0) {
			target->modifyFlatMagicArmour(magic->getFlatMagicArmourModifierEnemy());
			cout << magic->getFlatMagicArmourModifierEnemy() << " magic armour, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getPropArmourModifierEnemy() != 0 || magic->getPropMagicArmourModifierEnemy() != 0) {
		if (magic->getPropArmourModifierEnemy() != 0) {
			target->modifyPropArmour(magic->getPropArmourModifierEnemy());
			cout << 100 * magic->getPropArmourModifierEnemy() << "% physical damage received, ";
		}
		if (magic->getPropMagicArmourModifierEnemy() != 0) {
			target->modifyPropMagicArmour(magic->getPropMagicArmourModifierEnemy());
			cout << 100 * magic->getPropMagicArmourModifierEnemy() << "% magic damage received, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getFlatDamageModifierEnemy() != 0 || magic->getFlatMagicDamageModifierEnemy() != 0 || magic->getFlatArmourPiercingDamageModifierEnemy() != 0) {
		if (magic->getFlatDamageModifierEnemy() != 0) {
			target->modifyFlatDamageModifier(magic->getFlatDamageModifierEnemy());
			cout << magic->getFlatDamageModifierEnemy() << " physical damage dealt, ";
		}
		if (magic->getFlatMagicDamageModifierEnemy() != 0) {
			target->modifyFlatMagicDamageModifier(magic->getFlatMagicDamageModifierEnemy());
			cout << magic->getFlatMagicDamageModifierEnemy() << " magic damage dealt, ";
		}
		if (magic->getFlatArmourPiercingDamageModifierEnemy() != 0) {
			target->modifyFlatArmourPiercingDamageModifier(magic->getFlatArmourPiercingDamageModifierEnemy());
			cout << magic->getFlatArmourPiercingDamageModifierEnemy() << " armour piercing damage dealt, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getPropDamageModifierEnemy() != 0 || magic->getPropMagicDamageModifierEnemy() != 0 || magic->getPropArmourPiercingDamageModifierEnemy() != 0) {
		if (magic->getPropDamageModifierEnemy() != 0) {
			target->modifyPropDamageModifier(magic->getPropDamageModifierEnemy());
			cout << 100 * magic->getPropDamageModifierEnemy() << "% physical damage dealt, ";
		}
		if (magic->getPropMagicDamageModifierEnemy() != 0) {
			target->modifyPropMagicDamageModifier(magic->getPropMagicDamageModifierEnemy());
			cout << 100 * magic->getPropMagicDamageModifierEnemy() << "% magic damage dealt, ";
		}
		if (magic->getPropArmourPiercingDamageModifierEnemy() != 0) {
			target->modifyPropArmourPiercingDamageModifier(magic->getPropArmourPiercingDamageModifierEnemy());
			cout << 100 * magic->getPropArmourPiercingDamageModifierEnemy() << "% armour piercing damage dealt, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getEvadeChanceModifierEnemy() != 0 || magic->getBonusActionsModifierEnemy() != 0 || magic->getCounterAttackChanceModifierEnemy() != 0) {
		if (magic->getEvadeChanceModifierEnemy() != 0) {
			target->modifyEvadeChance(magic->getEvadeChanceModifierEnemy());
			cout << 100 * magic->getEvadeChanceModifierEnemy() << "% evade chance, ";
		}
		if (magic->getCounterAttackChanceModifierEnemy() != 0) {
			target->modifyCounterAttackChance(magic->getCounterAttackChanceModifierEnemy());
			cout << 100 * magic->getCounterAttackChanceModifierEnemy() << "% counter attack chance, ";
		}
		if (magic->getBonusActionsModifierEnemy() != 0) {
			target->modifyBonusActions(magic->getBonusActionsModifierEnemy());
			cout << magic->getBonusActionsModifierEnemy() << " bonus actions, ";
		}
		cout << "\b\b\n";
	}
	cout << noshowpos;
	this_thread::sleep_for(chrono::milliseconds(100));
}

void spellHit(spell* magic, enemy* caster, player* target) {
	if (!magic->getNoEvade() && rng(0.f, 1.f) < target->getEvadeChance()) {
		cout << "Evade!\n";
		this_thread::sleep_for(chrono::milliseconds(100));
		return;
	}
	cout << "Hit!\n";
	target->propDamage(magic->getPropDamage());
	if (magic->getPropDamage() > 0) {
		cout << -100 * magic->getPropDamage() << "% health\n";
	}
	else if (magic->getPropDamage() < 0) {
		cout << -100 * magic->getPropDamage() << "% of health recovered\n";
	}
	if (magic->getEffectType() / 10 > 1) {
		short healthSteal = max((short)0, target->getHealth());
		short p = magic->getFlatDamage(), m = magic->getFlatMagicDamage(), a = magic->getFlatArmourPiercingDamage();
		caster->applyDamageModifiers(&p, &m, &a);
		short healthLoss = target->flatDamage(p, m, a, magic->getTargetOverHeal());
		if (healthLoss > 0) {
			cout << healthLoss << " damage";
			if (magic->getLifeLink()) {
				healthSteal = min(healthSteal, healthLoss);
				if (healthSteal > 0) {
					cout << " (caster is healed for " << healthSteal << " by lifeLink)";
					caster->modifyHealth(healthSteal);
				}
			}
			cout << '\n';
		}
		else if (healthLoss < 0) {
			cout << -healthLoss << " healing\n";
		}
		else {
			cout << "No damage\n";
		}
	}
	if (magic->getEffectType() % 10 < 2) {
		this_thread::sleep_for(chrono::milliseconds(100));
		return;
	}
	cout << showpos;
	if (magic->getPoison() != 0 || magic->getBleed() != 0 || magic->getTempRegen() != 0) {
		if (magic->getPoison() != 0) {
			if (target->modifyPoison(magic->getPoison())) {
				cout << magic->getPoison() << " poison, ";
			}
			else {
				cout << "Poison resisted, ";
			}
		}
		if (magic->getBleed() != 0) {
			if (target->modifyBleed(magic->getBleed())) {
				cout << magic->getBleed() << " bleed, ";
			}
			else {
				cout << "Bleed resisted, ";
			}
		}
		if (magic->getTempRegen() != 0) {
			target->modifyTempRegen(magic->getTempRegen());
			cout << magic->getTempRegen() << " regeneration, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getMaxHealthModifierEnemy() != 0 || magic->getTurnRegenModifierEnemy() != 0 || magic->getBattleRegenModifierEnemy() != 0) {
		if (magic->getMaxHealthModifierEnemy() != 0) {
			target->modifyMaxHealth(magic->getMaxHealthModifierEnemy());
			cout << magic->getMaxHealthModifierEnemy() << " max health, ";
		}
		if (magic->getTurnRegenModifierEnemy() != 0) {
			target->modifyTurnRegen(magic->getTurnRegenModifierEnemy());
			cout << magic->getTurnRegenModifierEnemy() << " health per turn, ";
		}
		if (magic->getBattleRegenModifierEnemy() != 0) {
			target->modifyBattleRegen(magic->getBattleRegenModifierEnemy());
			cout << magic->getBattleRegenModifierEnemy() << " health at end of battle, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getMaxManaModifierEnemy() != 0 || magic->getManaChangeEnemy() != 0 || magic->getTurnManaRegenModifierEnemy() != 0 || magic->getBattleManaRegenModifierEnemy() != 0) {
		if (magic->getMaxManaModifierEnemy() != 0) {
			target->modifyMaxMana(magic->getMaxManaModifierEnemy());
			cout << magic->getMaxManaModifierEnemy() << " max " << g_manaName.plural() << ", ";
		}
		if (magic->getManaChangeEnemy() == 1 || magic->getManaChangeEnemy() == -1) {
			target->modifyMana(magic->getManaChangeEnemy());
			cout << magic->getManaChangeEnemy() << ' ' << g_manaName.singular() << ", ";
		}
		else if (magic->getManaChangeEnemy() != 0) {
			target->modifyMana(magic->getManaChangeEnemy());
			cout << magic->getManaChangeEnemy() << ' ' << g_manaName.plural() << ", ";
		}
		if (magic->getTurnManaRegenModifierEnemy() == 1 || magic->getTurnManaRegenModifierEnemy() == -1) {
			target->modifyTurnManaRegen(magic->getTurnManaRegenModifierEnemy());
			cout << magic->getTurnManaRegenModifierEnemy() << ' ' << g_manaName.singular() << " per turn, ";
		}
		else if (magic->getTurnManaRegenModifierEnemy() != 0) {
			target->modifyTurnManaRegen(magic->getTurnManaRegenModifierEnemy());
			cout << magic->getTurnManaRegenModifierEnemy() << ' ' << g_manaName.plural() << " per turn, ";
		}
		if (magic->getBattleManaRegenModifierEnemy() == 1 || magic->getBattleManaRegenModifierEnemy() == -1) {
			target->modifyBattleManaRegen(magic->getBattleManaRegenModifierEnemy());
			cout << magic->getBattleManaRegenModifierEnemy() << ' ' << g_manaName.singular() << " at end of battle, ";
		}
		else if (magic->getBattleManaRegenModifierEnemy() != 0) {
			target->modifyBattleManaRegen(magic->getBattleManaRegenModifierEnemy());
			cout << magic->getBattleManaRegenModifierEnemy() << ' ' << g_manaName.plural() << " at end of battle, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getPoisonResistModifierEnemy() != 0 || magic->getBleedResistModifierEnemy() != 0) {
		if (magic->getPoisonResistModifierEnemy() != 0) {
			target->modifyPoisonResist(magic->getPoisonResistModifierEnemy());
			cout << 100 * magic->getPoisonResistModifierEnemy() << "% poison resist, ";
		}
		if (magic->getBleedResistModifierEnemy() != 0) {
			target->modifyBleedResist(magic->getBleedResistModifierEnemy());
			cout << 100 * magic->getBleedResistModifierEnemy() << "% bleed resist, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getFlatArmourModifierEnemy() != 0 || magic->getFlatMagicArmourModifierEnemy() != 0) {
		if (magic->getFlatArmourModifierEnemy() != 0) {
			target->modifyFlatArmour(magic->getFlatArmourModifierEnemy());
			cout << magic->getFlatArmourModifierEnemy() << " physical armour, ";
		}
		if (magic->getFlatMagicArmourModifierEnemy() != 0) {
			target->modifyFlatMagicArmour(magic->getFlatMagicArmourModifierEnemy());
			cout << magic->getFlatMagicArmourModifierEnemy() << " magic armour, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getPropArmourModifierEnemy() != 0 || magic->getPropMagicArmourModifierEnemy() != 0) {
		if (magic->getPropArmourModifierEnemy() != 0) {
			target->modifyPropArmour(magic->getPropArmourModifierEnemy());
			cout << 100 * magic->getPropArmourModifierEnemy() << "% physical damage received, ";
		}
		if (magic->getPropMagicArmourModifierEnemy() != 0) {
			target->modifyPropMagicArmour(magic->getPropMagicArmourModifierEnemy());
			cout << 100 * magic->getPropMagicArmourModifierEnemy() << "% magic damage received, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getFlatDamageModifierEnemy() != 0 || magic->getFlatMagicDamageModifierEnemy() != 0 || magic->getFlatArmourPiercingDamageModifierEnemy() != 0) {
		if (magic->getFlatDamageModifierEnemy() != 0) {
			target->modifyFlatDamageModifier(magic->getFlatDamageModifierEnemy());
			cout << magic->getFlatDamageModifierEnemy() << " physical damage dealt, ";
		}
		if (magic->getFlatMagicDamageModifierEnemy() != 0) {
			target->modifyFlatMagicDamageModifier(magic->getFlatMagicDamageModifierEnemy());
			cout << magic->getFlatMagicDamageModifierEnemy() << " magic damage dealt, ";
		}
		if (magic->getFlatArmourPiercingDamageModifierEnemy() != 0) {
			target->modifyFlatArmourPiercingDamageModifier(magic->getFlatArmourPiercingDamageModifierEnemy());
			cout << magic->getFlatArmourPiercingDamageModifierEnemy() << " armour piercing damage dealt, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getPropDamageModifierEnemy() != 0 || magic->getPropMagicDamageModifierEnemy() != 0 || magic->getPropArmourPiercingDamageModifierEnemy() != 0) {
		if (magic->getPropDamageModifierEnemy() != 0) {
			target->modifyPropDamageModifier(magic->getPropDamageModifierEnemy());
			cout << 100 * magic->getPropDamageModifierEnemy() << "% physical damage dealt, ";
		}
		if (magic->getPropMagicDamageModifierEnemy() != 0) {
			target->modifyPropMagicDamageModifier(magic->getPropMagicDamageModifierEnemy());
			cout << 100 * magic->getPropMagicDamageModifierEnemy() << "% magic damage dealt, ";
		}
		if (magic->getPropArmourPiercingDamageModifierEnemy() != 0) {
			target->modifyPropArmourPiercingDamageModifier(magic->getPropArmourPiercingDamageModifierEnemy());
			cout << 100 * magic->getPropArmourPiercingDamageModifierEnemy() << "% armour piercing damage dealt, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getEvadeChanceModifierEnemy() != 0 || magic->getBonusActionsModifierEnemy() != 0 || magic->getCounterAttackChanceModifierEnemy() != 0) {
		if (magic->getEvadeChanceModifierEnemy() != 0) {
			target->modifyEvadeChance(magic->getEvadeChanceModifierEnemy());
			cout << 100 * magic->getEvadeChanceModifierEnemy() << "% evade chance, ";
		}
		if (magic->getCounterAttackChanceModifierEnemy() != 0) {
			target->modifyCounterAttackChance(magic->getCounterAttackChanceModifierEnemy());
			cout << 100 * magic->getCounterAttackChanceModifierEnemy() << "% counter attack chance, ";
		}
		if (magic->getBonusActionsModifierEnemy() != 0) {
			target->modifyBonusActions(magic->getBonusActionsModifierEnemy());
			cout << magic->getBonusActionsModifierEnemy() << " bonus actions, ";
		}
		cout << "\b\b\n";
	}
	cout << noshowpos;
	this_thread::sleep_for(chrono::milliseconds(100));
}

void spellHit(spell* magic, player* target) {
	if (!magic->getNoEvade() && rng(0.f, 1.f) < target->getEvadeChance()) {
		cout << "Evade!\n";
		this_thread::sleep_for(chrono::milliseconds(100));
		return;
	}
	cout << "Hit!\n";
	target->propDamage(magic->getPropDamage());
	if (magic->getPropDamage() > 0) {
		cout << -100 * magic->getPropDamage() << "% health\n";
	}
	else if (magic->getPropDamage() < 0) {
		cout << -100 * magic->getPropDamage() << "% of health recovered\n";
	}
	if (magic->getEffectType() / 10 > 1) {
		short healthLoss = target->flatDamage(magic->getFlatDamage(), magic->getFlatMagicDamage(), magic->getFlatArmourPiercingDamage(), magic->getTargetOverHeal());
		if (healthLoss > 0) {
			cout << healthLoss << " damage";
			cout << '\n';
		}
		else if (healthLoss < 0) {
			cout << -healthLoss << " healing\n";
		}
		else {
			cout << "No damage\n";
		}
	}
	if (magic->getEffectType() % 10 < 2) {
		this_thread::sleep_for(chrono::milliseconds(100));
		return;
	}
	cout << showpos;
	if (magic->getPoison() != 0 || magic->getBleed() != 0 || magic->getTempRegen() != 0) {
		if (magic->getPoison() != 0) {
			if (target->modifyPoison(magic->getPoison())) {
				cout << magic->getPoison() << " poison, ";
			}
			else {
				cout << "Poison resisted, ";
			}
		}
		if (magic->getBleed() != 0) {
			if (target->modifyBleed(magic->getBleed())) {
				cout << magic->getBleed() << " bleed, ";
			}
			else {
				cout << "Bleed resisted, ";
			}
		}
		if (magic->getTempRegen() != 0) {
			target->modifyTempRegen(magic->getTempRegen());
			cout << magic->getTempRegen() << " regeneration, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getMaxHealthModifierEnemy() != 0 || magic->getTurnRegenModifierEnemy() != 0 || magic->getBattleRegenModifierEnemy() != 0) {
		if (magic->getMaxHealthModifierEnemy() != 0) {
			target->modifyMaxHealth(magic->getMaxHealthModifierEnemy());
			cout << magic->getMaxHealthModifierEnemy() << " max health, ";
		}
		if (magic->getTurnRegenModifierEnemy() != 0) {
			target->modifyTurnRegen(magic->getTurnRegenModifierEnemy());
			cout << magic->getTurnRegenModifierEnemy() << " health per turn, ";
		}
		if (magic->getBattleRegenModifierEnemy() != 0) {
			target->modifyBattleRegen(magic->getBattleRegenModifierEnemy());
			cout << magic->getBattleRegenModifierEnemy() << " health at end of battle, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getMaxManaModifierEnemy() != 0 || magic->getManaChangeEnemy() != 0 || magic->getTurnManaRegenModifierEnemy() != 0 || magic->getBattleManaRegenModifierEnemy() != 0) {
		if (magic->getMaxManaModifierEnemy() != 0) {
			target->modifyMaxMana(magic->getMaxManaModifierEnemy());
			cout << magic->getMaxManaModifierEnemy() << " max " << g_manaName.plural() << ", ";
		}
		if (magic->getManaChangeEnemy() == 1 || magic->getManaChangeEnemy() == -1) {
			target->modifyMana(magic->getManaChangeEnemy());
			cout << magic->getManaChangeEnemy() << ' ' << g_manaName.singular() << ", ";
		}
		else if (magic->getManaChangeEnemy() != 0) {
			target->modifyMana(magic->getManaChangeEnemy());
			cout << magic->getManaChangeEnemy() << ' ' << g_manaName.plural() << ", ";
		}
		if (magic->getTurnManaRegenModifierEnemy() == 1 || magic->getTurnManaRegenModifierEnemy() == -1) {
			target->modifyTurnManaRegen(magic->getTurnManaRegenModifierEnemy());
			cout << magic->getTurnManaRegenModifierEnemy() << ' ' << g_manaName.singular() << " per turn, ";
		}
		else if (magic->getTurnManaRegenModifierEnemy() != 0) {
			target->modifyTurnManaRegen(magic->getTurnManaRegenModifierEnemy());
			cout << magic->getTurnManaRegenModifierEnemy() << ' ' << g_manaName.plural() << " per turn, ";
		}
		if (magic->getBattleManaRegenModifierEnemy() == 1 || magic->getBattleManaRegenModifierEnemy() == -1) {
			target->modifyBattleManaRegen(magic->getBattleManaRegenModifierEnemy());
			cout << magic->getBattleManaRegenModifierEnemy() << ' ' << g_manaName.singular() << " at end of battle, ";
		}
		else if (magic->getBattleManaRegenModifierEnemy() != 0) {
			target->modifyBattleManaRegen(magic->getBattleManaRegenModifierEnemy());
			cout << magic->getBattleManaRegenModifierEnemy() << ' ' << g_manaName.plural() << " at end of battle, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getPoisonResistModifierEnemy() != 0 || magic->getBleedResistModifierEnemy() != 0) {
		if (magic->getPoisonResistModifierEnemy() != 0) {
			target->modifyPoisonResist(magic->getPoisonResistModifierEnemy());
			cout << 100 * magic->getPoisonResistModifierEnemy() << "% poison resist, ";
		}
		if (magic->getBleedResistModifierEnemy() != 0) {
			target->modifyBleedResist(magic->getBleedResistModifierEnemy());
			cout << 100 * magic->getBleedResistModifierEnemy() << "% bleed resist, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getFlatArmourModifierEnemy() != 0 || magic->getFlatMagicArmourModifierEnemy() != 0) {
		if (magic->getFlatArmourModifierEnemy() != 0) {
			target->modifyFlatArmour(magic->getFlatArmourModifierEnemy());
			cout << magic->getFlatArmourModifierEnemy() << " physical armour, ";
		}
		if (magic->getFlatMagicArmourModifierEnemy() != 0) {
			target->modifyFlatMagicArmour(magic->getFlatMagicArmourModifierEnemy());
			cout << magic->getFlatMagicArmourModifierEnemy() << " magic armour, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getPropArmourModifierEnemy() != 0 || magic->getPropMagicArmourModifierEnemy() != 0) {
		if (magic->getPropArmourModifierEnemy() != 0) {
			target->modifyPropArmour(magic->getPropArmourModifierEnemy());
			cout << 100 * magic->getPropArmourModifierEnemy() << "% physical damage received, ";
		}
		if (magic->getPropMagicArmourModifierEnemy() != 0) {
			target->modifyPropMagicArmour(magic->getPropMagicArmourModifierEnemy());
			cout << 100 * magic->getPropMagicArmourModifierEnemy() << "% magic damage received, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getFlatDamageModifierEnemy() != 0 || magic->getFlatMagicDamageModifierEnemy() != 0 || magic->getFlatArmourPiercingDamageModifierEnemy() != 0) {
		if (magic->getFlatDamageModifierEnemy() != 0) {
			target->modifyFlatDamageModifier(magic->getFlatDamageModifierEnemy());
			cout << magic->getFlatDamageModifierEnemy() << " physical damage dealt, ";
		}
		if (magic->getFlatMagicDamageModifierEnemy() != 0) {
			target->modifyFlatMagicDamageModifier(magic->getFlatMagicDamageModifierEnemy());
			cout << magic->getFlatMagicDamageModifierEnemy() << " magic damage dealt, ";
		}
		if (magic->getFlatArmourPiercingDamageModifierEnemy() != 0) {
			target->modifyFlatArmourPiercingDamageModifier(magic->getFlatArmourPiercingDamageModifierEnemy());
			cout << magic->getFlatArmourPiercingDamageModifierEnemy() << " armour piercing damage dealt, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getPropDamageModifierEnemy() != 0 || magic->getPropMagicDamageModifierEnemy() != 0 || magic->getPropArmourPiercingDamageModifierEnemy() != 0) {
		if (magic->getPropDamageModifierEnemy() != 0) {
			target->modifyPropDamageModifier(magic->getPropDamageModifierEnemy());
			cout << 100 * magic->getPropDamageModifierEnemy() << "% physical damage dealt, ";
		}
		if (magic->getPropMagicDamageModifierEnemy() != 0) {
			target->modifyPropMagicDamageModifier(magic->getPropMagicDamageModifierEnemy());
			cout << 100 * magic->getPropMagicDamageModifierEnemy() << "% magic damage dealt, ";
		}
		if (magic->getPropArmourPiercingDamageModifierEnemy() != 0) {
			target->modifyPropArmourPiercingDamageModifier(magic->getPropArmourPiercingDamageModifierEnemy());
			cout << 100 * magic->getPropArmourPiercingDamageModifierEnemy() << "% armour piercing damage dealt, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getEvadeChanceModifierEnemy() != 0 || magic->getBonusActionsModifierEnemy() != 0 || magic->getCounterAttackChanceModifierEnemy() != 0) {
		if (magic->getEvadeChanceModifierEnemy() != 0) {
			target->modifyEvadeChance(magic->getEvadeChanceModifierEnemy());
			cout << 100 * magic->getEvadeChanceModifierEnemy() << "% evade chance, ";
		}
		if (magic->getCounterAttackChanceModifierEnemy() != 0) {
			target->modifyCounterAttackChance(magic->getCounterAttackChanceModifierEnemy());
			cout << 100 * magic->getCounterAttackChanceModifierEnemy() << "% counter attack chance, ";
		}
		if (magic->getBonusActionsModifierEnemy() != 0) {
			target->modifyBonusActions(magic->getBonusActionsModifierEnemy());
			cout << magic->getBonusActionsModifierEnemy() << " bonus actions, ";
		}
		cout << "\b\b\n";
	}
	if (magic->getInitiativeModifier() != 0) {
		cout << magic->getInitiativeModifier() << " speed\n";
	}
	cout << noshowpos;
	this_thread::sleep_for(chrono::milliseconds(100));
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
	if (weaponry->getEffectType() / 10 == 1 || weaponry->getEffectType() / 10 == 2 || weaponry->getEffectType() % 10 == 1 || weaponry->getEffectType() % 10 == 2) {
		cout << "Attacker effects:\n";
	}
	attacker->propDamage(weaponry->getPropSelfDamage());
	if (weaponry->getPropSelfDamage() > 0) {
		cout << -100 * weaponry->getPropSelfDamage() << "% health\n";
	}
	else if (weaponry->getPropSelfDamage() < 0) {
		cout << -100 * weaponry->getPropSelfDamage() << "% of health recovered\n";
	}
	if (weaponry->getEffectType() / 10 == 1 || weaponry->getEffectType() / 10 == 2) {
		short healthLoss = attacker->flatDamage(weaponry->getFlatSelfDamage(), weaponry->getFlatSelfMagicDamage(), weaponry->getFlatArmourPiercingDamage(), weaponry->getSelfOverHeal());
		if (healthLoss > 0) {
			cout << healthLoss << " damage\n";
		}
		else if (healthLoss < 0) {
			cout << -healthLoss << " healing\n";
		}
		else {
			cout << "No damage\n";
		}
	}
	if (weaponry->getSelfPoison() != 0 || weaponry->getSelfBleed() != 0) {
		cout << showpos;
		if (weaponry->getSelfPoison() != 0) {
			if (attacker->modifyPoison(weaponry->getSelfPoison())) {
				cout << +weaponry->getSelfPoison() << " poison, ";
			}
			else {
				cout << "Poison resisted, ";
			}
		}
		if (weaponry->getSelfBleed() != 0) {
			if (attacker->modifyBleed(weaponry->getSelfBleed())) {
				cout << +weaponry->getSelfBleed() << " bleed, ";
			}
			else {
				cout << "Bleed resisted, ";
			}
		}
		cout << "\b\b\n";
	}
	cout << noshowpos;
	this_thread::sleep_for(chrono::milliseconds(500));
	if (weaponry->getEffectType() / 10 < 2 && weaponry->getEffectType() % 10 < 2) {
		return;
	}
	uint8_t hits;
	if (counter) {
		hits = weaponry->getCounterHits();
	}
	else {
		hits = weaponry->getHitCount();
	}
	for (uint8_t i = 0; i < hits; i++) {
		weaponHit(weaponry, attacker, target);
	}
	this_thread::sleep_for(chrono::milliseconds(400));
}

void weaponAttack(weapon* weaponry, enemy* attacker, player* target, bool counter) {
	if (weaponry->getEffectType() / 10 == 1 || weaponry->getEffectType() / 10 == 2 || weaponry->getEffectType() % 10 == 1 || weaponry->getEffectType() % 10 == 2) {
		cout << "Attacker effects:\n";
	}
	attacker->propDamage(weaponry->getPropSelfDamage());
	if (weaponry->getPropSelfDamage() > 0) {
		cout << -100 * weaponry->getPropSelfDamage() << "% health\n";
	}
	else if (weaponry->getPropSelfDamage() < 0) {
		cout << -100 * weaponry->getPropSelfDamage() << "% of health recovered\n";
	}
	if (weaponry->getEffectType() / 10 == 1 || weaponry->getEffectType() / 10 == 2) {
		short healthLoss = attacker->flatDamage(weaponry->getFlatSelfDamage(), weaponry->getFlatSelfMagicDamage(), weaponry->getFlatArmourPiercingDamage(), weaponry->getSelfOverHeal());
		if (healthLoss > 0) {
			cout << healthLoss << " damage\n";
		}
		else if (healthLoss < 0) {
			cout << -healthLoss << " healing\n";
		}
		else {
			cout << "No damage\n";
		}
	}
	if (weaponry->getSelfPoison() != 0 || weaponry->getSelfBleed() != 0) {
		cout << showpos;
		if (weaponry->getSelfPoison() != 0) {
			if (attacker->modifyPoison(weaponry->getSelfPoison())) {
				cout << +weaponry->getSelfPoison() << " poison, ";
			}
			else {
				cout << "Poison resisted, ";
			}
		}
		if (weaponry->getSelfBleed() != 0) {
			if (attacker->modifyBleed(weaponry->getSelfBleed())) {
				cout << +weaponry->getSelfBleed() << " bleed, ";
			}
			else {
				cout << "Bleed resisted, ";
			}
		}
		cout << "\b\b\n";
	}
	cout << noshowpos;
	this_thread::sleep_for(chrono::milliseconds(500));
	if (weaponry->getEffectType() / 10 < 2 && weaponry->getEffectType() % 10 < 2) {
		return;
	}
	uint8_t hits;
	if (counter) {
		hits = weaponry->getCounterHits();
	}
	else {
		hits = weaponry->getHitCount();
	}
	for (uint8_t i = 0; i < hits; i++) {
		weaponHit(weaponry, attacker, target);
	}
	this_thread::sleep_for(chrono::milliseconds(400));
}

void weaponHit(weapon* weaponry, player* attacker, enemy* target) {
	if (!weaponry->getNoEvade() && rng(0.f, 1.f) < target->getEvadeChance()) {
		cout << "Evade!\n";
		this_thread::sleep_for(chrono::milliseconds(100));
		return;
	}
	cout << "Hit!\n";
	target->propDamage(weaponry->getPropDamage());
	if (weaponry->getPropDamage() > 0) {
		cout << -100 * weaponry->getPropDamage() << "% health\n";
	}
	else if (weaponry->getPropDamage() < 0) {
		cout << -100 * weaponry->getPropDamage() << "% of health recovered\n";
	}
	if (weaponry->getEffectType() / 10 > 1) {
		short healthSteal = max((short)0, target->getHealth());
		short p = weaponry->getFlatDamage(), m = weaponry->getFlatMagicDamage(), a = weaponry->getFlatArmourPiercingDamage();
		attacker->applyDamageModifiers(&p, &m, &a);
		short healthLoss = target->flatDamage(p, m, a, weaponry->getTargetOverHeal());
		if (healthLoss > 0) {
			cout << healthLoss << " damage";
			if (weaponry->getLifeLink()) {
				healthSteal = min(healthSteal, healthLoss);
				if (healthSteal > 0) {
					cout << " (attacker is healed for " << healthSteal << " by lifeLink)";
					attacker->modifyHealth(healthSteal);
				}
			}
			cout << '\n';
		}
		else if (healthLoss < 0) {
			cout << -healthLoss << " healing\n";
		}
		else {
			cout << "No damage\n";
		}
	}
	if (weaponry->getPoison() > 0 || weaponry->getBleed() > 0) {
		cout << showpos;
		if (weaponry->getPoison() > 0) {
			if (target->modifyPoison(weaponry->getPoison())) {
				cout << +weaponry->getPoison() << " poison, ";
			}
			else {
				cout << "Poison resisted, ";
			}
		}
		if (weaponry->getBleed() > 0) {
			if (target->modifyBleed(weaponry->getBleed())) {
				cout << +weaponry->getBleed() << " bleed, ";
			}
			else {
				cout << "Bleed resisted, ";
			}
		}
		cout << "\b\b\n" << noshowpos;
	}
	this_thread::sleep_for(chrono::milliseconds(100));
}

void weaponHit(weapon* weaponry, enemy* attacker, player* target) {
	if (!weaponry->getNoEvade() && rng(0.f, 1.f) < target->getEvadeChance()) {
		cout << "Evade!\n";
		this_thread::sleep_for(chrono::milliseconds(100));
		return;
	}
	cout << "Hit!\n";
	target->propDamage(weaponry->getPropDamage());
	if (weaponry->getPropDamage() > 0) {
		cout << -100 * weaponry->getPropDamage() << "% health\n";
	}
	else if (weaponry->getPropDamage() < 0) {
		cout << -100 * weaponry->getPropDamage() << "% of health recovered\n";
	}
	if (weaponry->getEffectType() / 10 > 1) {
		short healthSteal = max((short)0, target->getHealth());
		short p = weaponry->getFlatDamage(), m = weaponry->getFlatMagicDamage(), a = weaponry->getFlatArmourPiercingDamage();
		attacker->applyDamageModifiers(&p, &m, &a);
		short healthLoss = target->flatDamage(p, m, a, weaponry->getTargetOverHeal());
		if (healthLoss > 0) {
			cout << healthLoss << " damage";
			if (weaponry->getLifeLink()) {
				healthSteal = min(healthSteal, healthLoss);
				if (healthSteal > 0) {
					cout << " (attacker is healed for " << healthSteal << " by lifeLink)";
					attacker->modifyHealth(healthSteal);
				}
			}
			cout << '\n';
		}
		else if (healthLoss < 0) {
			cout << -healthLoss << " healing\n";
		}
		else {
			cout << "No damage\n";
		}
	}
	if (weaponry->getPoison() > 0 || weaponry->getBleed() > 0) {
		cout << showpos;
		if (weaponry->getPoison() > 0) {
			if (target->modifyPoison(weaponry->getPoison())) {
				cout << +weaponry->getPoison() << " poison, ";
			}
			else {
				cout << "Poison resisted, ";
			}
		}
		if (weaponry->getBleed() > 0) {
			if (target->modifyBleed(weaponry->getBleed())) {
				cout << +weaponry->getBleed() << " bleed, ";
			}
			else {
				cout << "Bleed resisted, ";
			}
		}
		cout << "\b\b\n" << noshowpos;
	}
	this_thread::sleep_for(chrono::milliseconds(100));
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
	uint8_t spellSlots = playerCharacter->getSpellSlots();
	try {
		for (uint8_t i = 0; i < spellSlots; i++) {
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
	if (weapon1->getEffectType() / 10 == 1 || weapon1->getEffectType() / 10 == 2 || weapon1->getEffectType() % 10 == 1 || weapon1->getEffectType() % 10 == 2 || weapon2->getEffectType() / 10 == 1 || weapon2->getEffectType() / 10 == 2 || weapon2->getEffectType() % 10 == 1 || weapon2->getEffectType() % 10 == 2) {
		cout << "Attacker effects:\n";
	}
	if (weapon1->getPropSelfDamage() > 0) {
		if (weapon2->getPropSelfDamage() > 0) {
			float totProp = weapon1->getPropSelfDamage() + weapon2->getPropSelfDamage() - (weapon1->getPropSelfDamage() * weapon2->getPropSelfDamage());
			attacker->propDamage(totProp);
			cout << -100 * totProp << "% health\n";
		}
		else if (weapon2->getPropSelfDamage() < 0) {
			attacker->propDamage(weapon1->getPropSelfDamage());
			attacker->propDamage(weapon2->getPropSelfDamage());
			cout << -100 * weapon1->getPropSelfDamage() << "% health, then " << -100 * weapon2->getPropSelfDamage() << "% of health recovered\n";
		}
		else {
			attacker->propDamage(weapon1->getPropSelfDamage());
			cout << -100 * weapon1->getPropSelfDamage() << "% health\n";
		}
	}
	else if (weapon1->getPropSelfDamage() < 0) {
		if (weapon2->getPropSelfDamage() > 0) {
			attacker->propDamage(weapon2->getPropSelfDamage());
			attacker->propDamage(weapon1->getPropSelfDamage());
			cout << -100 * weapon2->getPropSelfDamage() << "% health, then " << -100 * weapon1->getPropSelfDamage() << "% of health recovered\n";
		}
		else if (weapon2->getPropSelfDamage() < 0) {
			float totProp = max(-1.f, weapon1->getPropSelfDamage() + weapon2->getPropSelfDamage());
			attacker->propDamage(totProp);
			cout << -100 * totProp << "% of health recovered\n";
		}
		else {
			attacker->propDamage(weapon1->getPropSelfDamage());
			cout << -100 * weapon1->getPropSelfDamage() << "% of health recovered\n";
		}
	}
	else {
		if (weapon2->getPropSelfDamage() > 0) {
			attacker->propDamage(weapon2->getPropSelfDamage());
			cout << -100 * weapon2->getPropSelfDamage() << "% health\n";
		}
		else if (weapon2->getPropSelfDamage() < 0) {
			attacker->propDamage(weapon2->getPropSelfDamage());
			cout << -100 * weapon2->getPropSelfDamage() << "% of health recovered\n";
		}
	}
	if (weapon1->getEffectType() / 10 == 1 || weapon1->getEffectType() / 10 == 2) {
		if (weapon2->getEffectType() / 10 == 1 || weapon2->getEffectType() / 10 == 2) {
			if (weapon1->getSelfOverHeal()) {
				long healthLoss = attacker->flatDamage(weapon2->getFlatSelfDamage(), weapon2->getFlatSelfMagicDamage(), weapon2->getFlatSelfArmourPiercingDamage(), weapon2->getSelfOverHeal());
				healthLoss += attacker->flatDamage(weapon1->getFlatSelfDamage(), weapon1->getFlatSelfMagicDamage(), weapon1->getFlatSelfArmourPiercingDamage(), true);
				if (healthLoss > 0) {
					cout << healthLoss << " damage\n";
				}
				else if (healthLoss < 0) {
					cout << -healthLoss << " healing\n";
				}
				else {
					cout << "No damage\n";
				}
			}
			else {
				long healthLoss = attacker->flatDamage(weapon1->getFlatSelfDamage(), weapon1->getFlatSelfMagicDamage(), weapon1->getFlatSelfArmourPiercingDamage());
				healthLoss += attacker->flatDamage(weapon2->getFlatSelfDamage(), weapon2->getFlatSelfMagicDamage(), weapon2->getFlatSelfArmourPiercingDamage(), weapon2->getSelfOverHeal());
				if (healthLoss > 0) {
					cout << healthLoss << " damage\n";
				}
				else if (healthLoss < 0) {
					cout << -healthLoss << " healing\n";
				}
				else {
					cout << "No damage\n";
				}
			}
		}
		else {
			short healthLoss = attacker->flatDamage(weapon1->getFlatSelfDamage(), weapon1->getFlatSelfMagicDamage(), weapon1->getFlatSelfArmourPiercingDamage(), weapon1->getSelfOverHeal());
			if (healthLoss > 0) {
				cout << healthLoss << " damage\n";
			}
			else if (healthLoss < 0) {
				cout << -healthLoss << " healing\n";
			}
			else {
				cout << "No damage\n";
			}
		}
	}
	else if (weapon2->getEffectType() / 10 == 1 || weapon2->getEffectType() / 10 == 2) {
		short healthLoss = attacker->flatDamage(weapon2->getFlatSelfDamage(), weapon2->getFlatSelfMagicDamage(), weapon2->getFlatSelfArmourPiercingDamage(), weapon2->getSelfOverHeal());
		if (healthLoss > 0) {
			cout << healthLoss << " damage\n";
		}
		else if (healthLoss < 0) {
			cout << -healthLoss << " healing\n";
		}
		else {
			cout << "No damage\n";
		}
	}
	if (weapon1->getSelfPoison() + weapon2->getSelfPoison() > 0 || weapon1->getSelfBleed() + weapon2->getSelfBleed() > 0) {
		cout << showpos;
		if (weapon1->getSelfPoison() + weapon2->getSelfPoison() > 0) {
			if (attacker->modifyPoison(weapon1->getSelfPoison() + weapon2->getSelfPoison())) {
				cout << weapon1->getSelfPoison() + weapon2->getSelfPoison() << " poison, ";
			}
			else {
				cout << "Poison resisted, ";
			}
		}
		if (weapon1->getSelfBleed() + weapon2->getSelfBleed() > 0) {
			if (attacker->modifyBleed(weapon1->getSelfBleed() + weapon2->getSelfBleed())) {
				cout << weapon1->getSelfBleed() + weapon2->getSelfBleed() << " bleed, ";
			}
			else {
				cout << "Bleed resisted, ";
			}
		}
		cout << "\b\b\n" << noshowpos;
	}
	this_thread::sleep_for(chrono::milliseconds(500));
	if (weapon1->getEffectType() / 10 < 2 && weapon1->getEffectType() % 10 < 2 && weapon2->getEffectType() / 10 < 2 && weapon2->getEffectType() % 10 < 2) {
		return;
	}
	cout << "Target effects:\n";
	uint8_t hits1, hits2;
	if (counter) {
		hits1 = weapon1->getCounterHits();
		hits2 = weapon2->getCounterHits();
	}
	else {
		hits1 = weapon1->getHitCount();
		hits2 = weapon2->getHitCount();
	}
	for (uint8_t i = 0; i < hits1; i++) {
		weaponHit(weapon1, attacker, target);
		if (i < hits2) {
			weaponHit(weapon2, attacker, target);
		}
	}
	this_thread::sleep_for(chrono::milliseconds(400));
}

void weaponAttack(weapon* weapon1, weapon* weapon2, enemy* attacker, player* target, bool counter) {
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
	if (weapon1->getEffectType() / 10 == 1 || weapon1->getEffectType() / 10 == 2 || weapon1->getEffectType() % 10 == 1 || weapon1->getEffectType() % 10 == 2 || weapon2->getEffectType() / 10 == 1 || weapon2->getEffectType() / 10 == 2 || weapon2->getEffectType() % 10 == 1 || weapon2->getEffectType() % 10 == 2) {
		cout << "Attacker effects:\n";
	}
	if (weapon1->getPropSelfDamage() > 0) {
		if (weapon2->getPropSelfDamage() > 0) {
			float totProp = weapon1->getPropSelfDamage() + weapon2->getPropSelfDamage() - (weapon1->getPropSelfDamage() * weapon2->getPropSelfDamage());
			attacker->propDamage(totProp);
			cout << -100 * totProp << "% health\n";
		}
		else if (weapon2->getPropSelfDamage() < 0) {
			attacker->propDamage(weapon1->getPropSelfDamage());
			attacker->propDamage(weapon2->getPropSelfDamage());
			cout << -100 * weapon1->getPropSelfDamage() << "% health, then " << -100 * weapon2->getPropSelfDamage() << "% of health recovered\n";
		}
		else {
			attacker->propDamage(weapon1->getPropSelfDamage());
			cout << -100 * weapon1->getPropSelfDamage() << "% health\n";
		}
	}
	else if (weapon1->getPropSelfDamage() < 0) {
		if (weapon2->getPropSelfDamage() > 0) {
			attacker->propDamage(weapon2->getPropSelfDamage());
			attacker->propDamage(weapon1->getPropSelfDamage());
			cout << -100 * weapon2->getPropSelfDamage() << "% health, then " << -100 * weapon1->getPropSelfDamage() << "% of health recovered\n";
		}
		else if (weapon2->getPropSelfDamage() < 0) {
			float totProp = max(-1.f, weapon1->getPropSelfDamage() + weapon2->getPropSelfDamage());
			attacker->propDamage(totProp);
			cout << -100 * totProp << "% of health recovered\n";
		}
		else {
			attacker->propDamage(weapon1->getPropSelfDamage());
			cout << -100 * weapon1->getPropSelfDamage() << "% of health recovered\n";
		}
	}
	else {
		if (weapon2->getPropSelfDamage() > 0) {
			attacker->propDamage(weapon2->getPropSelfDamage());
			cout << -100 * weapon2->getPropSelfDamage() << "% health\n";
		}
		else if (weapon2->getPropSelfDamage() < 0) {
			attacker->propDamage(weapon2->getPropSelfDamage());
			cout << -100 * weapon2->getPropSelfDamage() << "% of health recovered\n";
		}
	}
	if (weapon1->getEffectType() / 10 == 1 || weapon1->getEffectType() / 10 == 2) {
		if (weapon2->getEffectType() / 10 == 1 || weapon2->getEffectType() / 10 == 2) {
			if (weapon1->getSelfOverHeal()) {
				long healthLoss = attacker->flatDamage(weapon2->getFlatSelfDamage(), weapon2->getFlatSelfMagicDamage(), weapon2->getFlatSelfArmourPiercingDamage(), weapon2->getSelfOverHeal());
				healthLoss += attacker->flatDamage(weapon1->getFlatSelfDamage(), weapon1->getFlatSelfMagicDamage(), weapon1->getFlatSelfArmourPiercingDamage(), true);
				if (healthLoss > 0) {
					cout << healthLoss << " damage\n";
				}
				else if (healthLoss < 0) {
					cout << -healthLoss << " healing\n";
				}
				else {
					cout << "No damage\n";
				}
			}
			else {
				long healthLoss = attacker->flatDamage(weapon1->getFlatSelfDamage(), weapon1->getFlatSelfMagicDamage(), weapon1->getFlatSelfArmourPiercingDamage());
				healthLoss += attacker->flatDamage(weapon2->getFlatSelfDamage(), weapon2->getFlatSelfMagicDamage(), weapon2->getFlatSelfArmourPiercingDamage(), weapon2->getSelfOverHeal());
				if (healthLoss > 0) {
					cout << healthLoss << " damage\n";
				}
				else if (healthLoss < 0) {
					cout << -healthLoss << " healing\n";
				}
				else {
					cout << "No damage\n";
				}
			}
		}
		else {
			short healthLoss = attacker->flatDamage(weapon1->getFlatSelfDamage(), weapon1->getFlatSelfMagicDamage(), weapon1->getFlatSelfArmourPiercingDamage(), weapon1->getSelfOverHeal());
			if (healthLoss > 0) {
				cout << healthLoss << " damage\n";
			}
			else if (healthLoss < 0) {
				cout << -healthLoss << " healing\n";
			}
			else {
				cout << "No damage\n";
			}
		}
	}
	else if (weapon2->getEffectType() / 10 == 1 || weapon2->getEffectType() / 10 == 2) {
		short healthLoss = attacker->flatDamage(weapon2->getFlatSelfDamage(), weapon2->getFlatSelfMagicDamage(), weapon2->getFlatSelfArmourPiercingDamage(), weapon2->getSelfOverHeal());
		if (healthLoss > 0) {
			cout << healthLoss << " damage\n";
		}
		else if (healthLoss < 0) {
			cout << -healthLoss << " healing\n";
		}
		else {
			cout << "No damage\n";
		}
	}
	if (weapon1->getSelfPoison() + weapon2->getSelfPoison() > 0 || weapon1->getSelfBleed() + weapon2->getSelfBleed() > 0) {
		cout << showpos;
		if (weapon1->getSelfPoison() + weapon2->getSelfPoison() > 0) {
			if (attacker->modifyPoison(weapon1->getSelfPoison() + weapon2->getSelfPoison())) {
				cout << weapon1->getSelfPoison() + weapon2->getSelfPoison() << " poison, ";
			}
			else {
				cout << "Poison resisted, ";
			}
		}
		if (weapon1->getSelfBleed() + weapon2->getSelfBleed() > 0) {
			if (attacker->modifyBleed(weapon1->getSelfBleed() + weapon2->getSelfBleed())) {
				cout << weapon1->getSelfBleed() + weapon2->getSelfBleed() << " bleed, ";
			}
			else {
				cout << "Bleed resisted, ";
			}
		}
		cout << "\b\b\n" << noshowpos;
	}
	this_thread::sleep_for(chrono::milliseconds(500));
	if (weapon1->getEffectType() / 10 < 2 && weapon1->getEffectType() % 10 < 2 && weapon2->getEffectType() / 10 < 2 && weapon2->getEffectType() % 10 < 2) {
		return;
	}
	cout << "Target effects:\n";
	uint8_t hits1, hits2;
	if (counter) {
		hits1 = weapon1->getCounterHits();
		hits2 = weapon2->getCounterHits();
	}
	else {
		hits1 = weapon1->getHitCount();
		hits2 = weapon2->getHitCount();
	}
	for (uint8_t i = 0; i < hits1; i++) {
		weaponHit(weapon1, attacker, target);
		if (i < hits2) {
			weaponHit(weapon2, attacker, target);
		}
	}
	this_thread::sleep_for(chrono::milliseconds(400));
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

uint8_t deathCheck(player* playerCharacter, enemy* opponent) {
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

uint8_t battleMode() {
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
				cout << "To give yourself a new piece of equipment, enter its blueprint name, prefixed by the first letter of its type and an underscore.\n(h_ for a helmet, t_ for a chest plate, l_ for leggings, f_ for footwear, w_ for a weapon, s_ for a spell)\nIf you wish not to add new equipment, enter EMPTY\n";
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
					armourTorso chestPlate(blueprintSelection);
					playerCharacter.equip(&chestPlate);
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
				cout << showpos << opponent.getXp() << " experience\n" << noshowpos;
				playerCharacter.giveXp(opponent.getXp());
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