#pragma once
#include "player.h"
#include "enemies.h"
#include "spells.h"
#include "weapons.h"
#define POISON_MULTIPLIER 1 //Multiplier for poison effect
#define BLEED_MULTIPLIER 1
#define REGEN_MULTIPLIER 1

//Handles battle, returns 1 if enemy killed, returns 2 if player killed. firstGo=1 is player goes first, firstGo=-1 is enemy goes first, firstGo=0 is use initiative
unsigned char battleHandler(player* playerCharacter, enemy* opponent, signed char firstGo = 0);
//Spell cast by player on enemy, returns 1 if target is dead, 2 if caster is dead
void spellCast(spell* magic, player* caster, enemy* target, bool counter = false);
//Individual hit
void spellHit(spell* magic, player* caster, enemy* target);
//Applies casting costs
void spellDeclare(spell* magic, player* caster);
//Spell cast by enemy on player, returns 1 if target is dead, 2 if caster is dead
void spellCast(spell* magic, enemy* caster, player* target, bool counter = false);
//Individual hit
void spellHit(spell* magic, enemy* caster, player* target);
//Applies casting costs
void spellDeclare(spell* magic, enemy* caster);
//Spell cast on player by nobody (e.g. in an event), returns 1 if target is dead
unsigned char spellCast(spell* magic, player* target);
//Individual hit
void spellHit(spell* magic, player* target);
//Weapon attack by player on enemy, returns 1 if target is dead, 2 if caster is dead
void weaponAttack(weapon* weaponry, player* attacker, enemy* target, bool counter = false);
//Individual hit
void weaponHit(weapon* weaponry, player* attacker, enemy* target);
//Applies casting costs
void weaponDeclare(weapon* weaponry, player* attacker);
//Weapon attack by enemy on player, returns 1 if target is dead, 2 if caster is dead
void weaponAttack(weapon* weaponry, enemy* attacker, player* target, bool counter = false);
//Individual hit
void weaponHit(weapon* weaponry, enemy* attacker, player* target);
//Applies costs
void weaponDeclare(weapon* weaponry, enemy* attacker);
//Weapon attack by player on enemy with dual weapons, returns 1 if target is dead, 2 if attacker is dead
void weaponAttack(weapon* weapon1, weapon* weapon2, player* attacker, enemy* target, bool counter = false);
//Applies costs
void weaponDeclare(weapon* weapon1, weapon* weapon2, player* attacker);
//Dual weapon attack by enemy on player
void weaponAttack(weapon* weapon1, weapon* weapon2, enemy* attacker, player* target, bool counter = false);
//Applies costs
void weaponDeclare(weapon* weapon1, weapon* weapon2, enemy* attacker);
//Recalculates player modifiers, resets status effects/DOT and spell cooldowns, intended for end of battle
void resetPlayer(player* playerCharacter);
//If enemy is dead and has a death spell, casts it, then returns 2 if player is dead, then returns 1 if enemy is dead, then returns 0
unsigned char deathCheck(player* playerCharacter, enemy* opponent);
//Battle mode. Returns 0 if quitting, 1 if returning to main menu
unsigned char battleMode();