#include "language.h"
#include "inputs.h"
#include <iostream>
#include "events.h"
#include "resources.h"
using namespace std;

extern variables g_customVars;
extern resource g_manaName;
extern resource g_projName;
extern bool g_useCustomData;

//Error codes:
// 1: Bad XML
// 4: Unable to open file
// 8: Variable limit reached

void variables::assign(string n, short v) {
	for (short i = 0; i < vars.size(); i++) {
		if (vars[i].name == n) {
			vars[i].value = v;
			return;
		}
	}
	if (vars.size() == SHRT_MAX) { //Only allowed to have 32767 variables, to prevent crashing
		throw 8;
	}
	vars.emplace_back(n, v);
}

short* variables::value(string n) {
	for (short i = 0; i < vars.size(); i++) {
		if (vars[i].name == n) {
			return &(vars[i].value);
		}
	}
	if (vars.size() == SHRT_MAX) {
		throw 8;
	}
	vars.emplace_back(n);
	return &(vars.back().value);
}

variables variables::operator+(const variables& varChanges) {
	for (short i = 0; i < varChanges.vars.size(); i++) {
		*value(varChanges.vars[i].name) += varChanges.vars[i].value;
	}
	return *this;
}

void variables::operator+=(const variables& varChanges) {
	for (short i = 0; i < varChanges.vars.size(); i++) {
		*value(varChanges.vars[i].name) += varChanges.vars[i].value;
	}
}

void variables::reset() {
	vars.resize(0);
}

bool evalCond(string cond, player* playerCharacter) {
	short var1 = 0, var2 = 0;
	char op = '='; //= is ==, < is <, > is >, ! is !=, l is <=, g is >=
	clearSpace(&cond);
	if (cond.empty()) {
		return true;
	}
	if (cond.substr(0, 4) == "true") {
		cond.erase(0, 4);
		clearSpace(&cond);
		if (cond.empty()) {
			return true;
		}
		else {
			return false;
		}
	}
	if ((cond[0] > 47 && cond[0] < 58) || cond[0] == '-' || cond.substr(0, 2) == "v_" || cond.substr(0, 4) == "rng(" || cond.substr(0, 2) == "p_") {
		var1 = numFromString(&cond, playerCharacter);
	}
	else {
		return false;
	}
	clearSpace(&cond);
	if (cond.empty()) {
		return false;
	}
	switch (cond[0]) {
	case '=':
		if (cond.substr(0, 2) != "==") {
			return false;
		}
		cond.erase(0, 2);
		op = '=';
		break;
	case '!':
		if (cond.substr(0, 2) != "!=") {
			return false;
		}
		cond.erase(0, 2);
		op = '!';
		break;
	case '&':
		if (cond.substr(0, 5) == "&lt =") {
			op = 'l';
			cond.erase(0, 5);
		}
		else if (cond.substr(0, 5) == "&gt =") {
			op = 'g';
			cond.erase(0, 5);
		}
		else if (cond.substr(0, 3) == "&lt") {
			op = '<';
			cond.erase(0, 3);
		}
		else if (cond.substr(0, 3) == "&gt") {
			op = '>';
			cond.erase(0, 3);
		}
		else {
			return false;
		}
		break;
	default:
		return false;
	}
	if (cond.empty() || cond[0] != ' ') {
		return false;
	}
	clearSpace(&cond);
	if (cond.empty()) {
		return false;
	}
	if ((cond[0] > 47 && cond[0] < 58) || cond[0] == '-' || cond.substr(0, 2) == "v_" || cond.substr(0, 4) == "rng(" || cond.substr(0, 2) == "p_") {
		var2 = numFromString(&cond, playerCharacter);
	}
	else {
		return false;
	}
	clearSpace(&cond);
	if (!cond.empty()) {
		return false;
	}
	switch (op) {
	case '=':
		return (var1 == var2);
	case '!':
		return (var1 != var2);
	case '<':
		return (var1 < var2);
	case '>':
		return (var1 > var2);
	case 'l':
		return (var1 <= var2);
	case 'g':
		return (var1 >= var2);
	default:
		return false;
	}
}

unsigned char doLine(ifstream* file, player* playerCharacter) {
	string buffer1 = getTag(file);
	if (buffer1 == "victory/") {
		return 1;
	}
	else if (buffer1 == "defeat/") {
		return 2;
	}
	else if (buffer1 == "text") {
		getline(*file, buffer1, '<');
		file->seekg(-1, ios_base::cur);
		if (getTag(file) != "/text") {
			throw 1;
		}
		cout << buffer1 << '\n';
		ignoreLine(file);
		return 0;
	}
	else if (buffer1.substr(0, 12) == "event load=\"") {
		buffer1.erase(0, 12);
		if (buffer1.empty() || buffer1.back() != '/') {
			throw 1;
		}
		buffer1.pop_back();
		if (buffer1.empty() || buffer1.back() != '\"') {
			throw 1;
		}
		buffer1.pop_back();
		ignoreLine(file);
		if (buffer1 == "EMPTY") {
			return 0;
		}
		Event currentEvent(buffer1);
		if (currentEvent.getEventName() == "EMPTY") {
			return 0;
		}
		unsigned char i = 1;
		while (i == 1) {
			i = currentEvent.eventHandler(playerCharacter);
		}
		return i;
	}
	else if (buffer1.substr(0, 10) == "var name=\"") {
		buffer1.erase(0, 10);
		if (buffer1.empty() || buffer1.back() != '\"') {
			throw 1;
		}
		buffer1.pop_back();
		string buffer2;
		getline(*file, buffer2, '<');
		file->seekg(-1, ios_base::cur);
		if (getTag(file) != "/var") {
			throw 1;
		}
		ignoreLine(file);
		modifyVar(buffer1, buffer2, playerCharacter);
		return 0;
	}
	else if (buffer1.substr(0, 9) == "if cond=\"") {
		buffer1.erase(0, 9);
		if (buffer1.empty() || buffer1.back() != '\"') {
			throw 1;
		}
		buffer1.pop_back();
		ignoreLine(file);
		if (evalCond(buffer1, playerCharacter)) {
			return 0;
		}
		endIf(file, playerCharacter, true);
		return 0;
	}
	else if (buffer1.substr(0, 5) == "else/" || buffer1.substr(0, 14) == "else if cond=\"") {
		ignoreLine(file);
		endIf(file, playerCharacter);
		return 0;
	}
	else if (buffer1 == "if/") {
		ignoreLine(file);
		return 0;
	}
	else if (buffer1.substr(0, 12) == "while cond=\"") {
		buffer1.erase(0, 12);
		if (buffer1.empty() || buffer1.back() != '\"') {
			throw 1;
		}
		buffer1.pop_back();
		ignoreLine(file);
		streampos startPos = file->tellg();
		endWhile(file);
		streampos endPos = file->tellg();
		while (evalCond(buffer1, playerCharacter)) {
			file->seekg(startPos);
			unsigned char i = 0;
			while (i == 0 || i == 5) {
				i = doLine(file, playerCharacter);
			}
			if (i == 1 || i == 2) {
				return i;
			}
			if (i == 3) {
				break;
			}
			if (i == 4) {
				continue;
			}
		}
		file->seekg(endPos);
		return 0;
	}
	else if (buffer1 == "continue/" || buffer1 == "/while") {
		return 4;
	}
	else if (buffer1 == "break/") {
		return 3;
	}
	else if (buffer1 == "save/") {
		ignoreLine(file);
		return 5;
	}
	else {
		throw 1;
	}
}

void endIf(ifstream* file, player* playerCharacter, bool newCond) {
	string buffer = "";
	while (true) {
		buffer = getTag(file);
		if (buffer.substr(0, 7) == "if cond") {
			ignoreLine(file);
			endIf(file, playerCharacter);
			continue;
		}
		else if (newCond && buffer.substr(0, 14) == "else if cond=\"") {
			buffer.erase(0, 14);
			if (buffer.empty() || buffer.back() != '/') {
				throw 1;
			}
			buffer.pop_back();
			if (buffer.empty() || buffer.back() != '\"') {
				throw 1;
			}
			buffer.pop_back();
			if (evalCond(buffer, playerCharacter)) {
				ignoreLine(file);
				return;
			}
		}
		else if (newCond && buffer == "else/") {
			ignoreLine(file);
			return;
		}
		else if (buffer == "/if") {
			ignoreLine(file);
			return;
		}
		ignoreLine(file);
	}
}

void endWhile(ifstream* file) {
	string buffer = "";
	while (true) {
		buffer = getTag(file);
		if (buffer == "/while") {
			ignoreLine(file);
			return;
		}
		else if (buffer.substr(0, 10) == "while cond") {
			ignoreLine(file);
			endWhile(file);
		}
		ignoreLine(file);
	}
}

void modifyVar(string var, string operation, player* playerCharacter) { //= num (=), += num (+), -= num (-), *= num (*), /= num (/), %= num (%), ++, --
	short value;
	signed char op;
	clearSpace(&operation);
	if (operation.empty()) {
		return;
	}
	switch (operation[0]) {
	case '=':
		operation.erase(0, 1);
		if (operation.empty() || operation[0] != ' ') {
			throw 1;
		}
		op = '=';
		break;
	case '+':
		operation.erase(0, 1);
		if (operation.empty()) {
			throw 1;
		}
		switch (operation[0]) {
		case '=':
			operation.erase(0, 1);
			if (operation.empty() || operation[0] != ' ') {
				throw 1;
			}
			op = '+';
			break;
		case '+':
			operation.erase(0, 1);
			while (!operation.empty() && operation[0] == ' ') {
				operation.erase(0, 1);
			}
			if (!operation.empty()) {
				throw 1;
			}
			*g_customVars.value(var) += 1;
			return;
		default:
			throw 1;
		}
		break;
	case '-':
		operation.erase(0, 1);
		if (operation.empty()) {
			throw 1;
		}
		switch (operation[0]) {
		case '=':
			operation.erase(0, 1);
			if (operation.empty() || operation[0] != ' ') {
				throw 1;
			}
			op = '-';
			break;
		case '-':
			operation.erase(0, 1);
			while (!operation.empty() && operation[0] == ' ') {
				operation.erase(0, 1);
			}
			if (!operation.empty()) {
				throw 1;
			}
			*g_customVars.value(var) -= 1;
			return;
		default:
			throw 1;
		}
		break;
	case '*':
		operation.erase(0, 1);
		if (operation.empty() || operation[0] != '=') {
			throw 1;
		}
		operation.erase(0, 1);
		if (operation.empty() || operation[0] != ' ') {
			throw 1;
		}
		op = '*';
		break;
	case '/':
		operation.erase(0, 1);
		if (operation.empty() || operation[0] != '=') {
			throw 1;
		}
		operation.erase(0, 1);
		if (operation.empty() || operation[0] != ' ') {
			throw 1;
		}
		op = '/';
		break;
	case '%':
		operation.erase(0, 1);
		if (operation.empty() || operation[0] != '=') {
			throw 1;
		}
		operation.erase(0, 1);
		if (operation.empty() || operation[0] != ' ') {
			throw 1;
		}
		op = '%';
		break;
	default:
		throw 1;
	}
	clearSpace(&operation);
	if (operation.empty()) {
		throw 1;
	}
	if ((operation[0] > 47 && operation[0] < 58) || operation[0] == '-' || operation.substr(0, 2) == "v_" || operation.substr(0, 4) == "rng(" || operation.substr(0, 2) == "p_") {
		value = numFromString(&operation, playerCharacter);
	}
	else {
		throw 1;
	}
	while (!operation.empty() && operation[0] == ' ') {
		operation.erase(0, 1);
	}
	if (!operation.empty()) {
		throw 1;
	}
	switch (op) {
	case '=':
		g_customVars.assign(var, value);
		return;
	case '+':
		*g_customVars.value(var) += value;
		return;
	case '-':
		*g_customVars.value(var) -= value;
		return;
	case '*':
		*g_customVars.value(var) *= value;
		return;
	case '/':
		*g_customVars.value(var) /= value;
		return;
	case '%':
		*g_customVars.value(var) %= value;
		return;
	default:
		throw 1;
	}
}

void save(ifstream* file, player* playerCharacter, string filePath, unsigned char slot) {
	if (slot < 48 || slot>57) {
		return;
	}
	string savePath = "saves\\sav";
	savePath += slot;
	savePath += ".xml";
	if (playerCharacter->weaponSlots != playerCharacter->weapons.size()) {
		cout << "Save failed, size of weapons vector does not match number of weapon slots\n";
		return;
	}
	if (playerCharacter->spellSlots != playerCharacter->spells.size()) {
		cout << "Save failed, size of spells vector does not match number of spell slots\n";
		return;
	}
	try {
		ofstream saveFile(savePath, ofstream::trunc);
		if (!saveFile.is_open()) {
			throw 4;
		}
		//Adventure file
		saveFile << "<filePath pos=\"" << file->tellg() << "\">" << filePath << "</filePath>\n";
		//Resources
		saveFile << "<resource type=\"PROJECTILE\">\n";
			saveFile << "\t<singular>" << g_projName.singular() << "</singular>\n";
			saveFile << "\t<plural>" << g_projName.plural() << "</plural>\n";
		saveFile << "</resource>\n";
		saveFile << "<resource type=\"MANA\">\n";
			saveFile << "\t<singular>" << g_manaName.singular() << "</singular>\n";
			saveFile << "\t<plural>" << g_manaName.plural() << "</plural>\n";
		saveFile << "</resource>\n\n";
		//Variables
		saveFile << "<variables>\n";
		for (short i = 0; i < g_customVars.vars.size(); i++) {
				saveFile << "\t<var name=\"" << g_customVars.vars[i].name << "\">" << g_customVars.vars[i].value << "</var>\n";
		}
		saveFile << "</variables>\n\n";
		//Custom data
		saveFile << "<customData>" << g_useCustomData << "</customData>\n\n";
		//Player
		saveFile << "<player>\n";
			saveFile << "\t<health>" << playerCharacter->health << "</health>\n";
			saveFile << "\t<maxHealthBase>" << playerCharacter->maxHealthBase << "</maxHealthBase>\n";
			saveFile << "\t<projectiles>" << playerCharacter->projectiles << "</projectiles>\n";
			saveFile << "\t<mana>" << playerCharacter->mana << "</mana>\n";
			saveFile << "\t<maxManaBase>" << playerCharacter->maxManaBase << "</maxManaBase>\n";
			saveFile << "\t<turnManaRegenBase>" << playerCharacter->turnManaRegenBase << "</turnManaRegenBase>\n";
			saveFile << "\t<battleManaRegenBase>" << playerCharacter->battleManaRegenBase << "</battleManaRegenBase>\n";
			saveFile << "\t<poison>" << +playerCharacter->poison << "</poison>\n";
			saveFile << "\t<poisonResistBase>" << playerCharacter->poisonResistBase << "</poisonResistBase>\n";
			saveFile << "\t<bleed>" << +playerCharacter->bleed << "</bleed>\n";
			saveFile << "\t<bleedResistBase>" << playerCharacter->bleedResistBase << "</bleedResistBase>\n";
			saveFile << "\t<tempRegen>" << +playerCharacter->tempRegen << "</tempRegen>\n";
			saveFile << "\t<constRegenBase>" << playerCharacter->constRegenBase << "</constRegenBase>\n";
			saveFile << "\t<battleRegenBase>" << playerCharacter->battleRegenBase << "</battleRegenBase>\n";
			saveFile << "\t<weaponSlots>" << +playerCharacter->weaponSlots << "</weaponSlots>\n";
			saveFile << "\t<weapons>\n";
			for (unsigned char i = 0; i < playerCharacter->weaponSlots; i++) {
					saveFile << "\t\t<weapon>" << playerCharacter->weapons[i].getWeaponName() << "</weapon>\n";
			}
			saveFile << "\t</weapons>\n";
			saveFile << "\t<spellSlots>" << +playerCharacter->spellSlots << "</spellSlots>\n";
			saveFile << "\t<spells>\n";
			for (unsigned char i = 0; i < playerCharacter->spellSlots; i++) {
					saveFile << "\t\t<spell>" << playerCharacter->spells[i].getSpellName() << "</spell>\n";
			}
			saveFile << "\t</spells>\n";
			saveFile << "\t<flatArmourBase>" << playerCharacter->flatArmourBase << "</flatArmourBase>\n";
			saveFile << "\t<propArmourBase>" << playerCharacter->propArmourBase << "</propArmourBase>\n";
			saveFile << "\t<flatMagicArmourBase>" << playerCharacter->flatMagicArmourBase << "</flatMagicArmourBase>\n";
			saveFile << "\t<propMagicArmourBase>" << playerCharacter->propMagicArmourBase << "</propMagicArmourBase>\n";
			saveFile << "\t<armourHead>" << playerCharacter->helmet.getArmourName() << "</armourHead>\n";
			saveFile << "\t<armourTorso>" << playerCharacter->chestplate.getArmourName() << "</armourTorso>\n";
			saveFile << "\t<armourLegs>" << playerCharacter->greaves.getArmourName() << "</armourLegs>\n";
			saveFile << "\t<armourFeet>" << playerCharacter->boots.getArmourName() << "</armourFeet>\n";
			saveFile << "\t<flatDamageModifierBase>" << playerCharacter->flatDamageModifierBase << "</flatDamageModifierBase>\n";
			saveFile << "\t<propDamageModifierBase>" << playerCharacter->propDamageModifierBase << "</propDamageModifierBase>\n";
			saveFile << "\t<flatMagicDamageModifierBase>" << playerCharacter->flatMagicDamageModifierBase << "</flatMagicDamageModifierBase>\n";
			saveFile << "\t<propMagicDamageModifierBase>" << playerCharacter->propMagicDamageModifierBase << "</propMagicDamageModifierBase>\n";
			saveFile << "\t<flatArmourPiercingDamageModifierBase>" << playerCharacter->flatArmourPiercingDamageModifierBase << "</flatArmourPiercingDamageModifierBase>\n";
			saveFile << "\t<propArmourPiercingDamageModifierBase>" << playerCharacter->propArmourPiercingDamageModifierBase << "</propArmourPiercingDamageModifierBase>\n";
			saveFile << "\t<evadeChanceBase>" << playerCharacter->evadeChanceBase << "</evadeChanceBase>\n";
			saveFile << "\t<counterAttackChanceBase>" << playerCharacter->counterAttackChanceBase << "</counterAttackChanceBase>\n";
			saveFile << "\t<bonusActionsBase>" << playerCharacter->bonusActionsBase << "</bonusActionsBase>\n";
		saveFile << "</player>";
		saveFile.close();
		cout << "Game saved\n";
	}
	catch (int) {
		cout << "Save failed, could not open save file\n";
	}
}