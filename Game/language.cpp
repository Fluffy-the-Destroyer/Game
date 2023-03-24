#include "language.h"
#include "inputs.h"
#include <iostream>
#include "events.h"
#include "resources.h"
#include <thread>
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
	for (unsigned short i = 0; i < vars.size(); i++) {
		if (vars[i].name == n) {
			vars[i].value = v;
			return;
		}
	}
	if (vars.size() == USHRT_MAX) { //Only allowed to have 65535 variables, to prevent crashing
		throw 8;
	}
	vars.emplace_back(n, v);
}

short* variables::value(string n) {
	for (unsigned short i = 0; i < vars.size(); i++) {
		if (vars[i].name == n) {
			return &(vars[i].value);
		}
	}
	if (vars.size() == USHRT_MAX) {
		throw 8;
	}
	vars.emplace_back(n);
	return &(vars.back().value);
}

void variables::operator+=(const variables& varChanges) {
	for (unsigned short i = 0; i < varChanges.vars.size(); i++) {
		*value(varChanges.vars[i].name) += varChanges.vars[i].value;
	}
}

void variables::reset() {
	vars.resize(0);
}

bool evalCond(string cond, player* playerCharacter) {
	clearSpace(&cond); //Get rid of spaces from beginning and end
	while (!cond.empty() && cond.back() == ' ') {
		cond.pop_back();
	}
	if (cond.empty()) {
		throw 1;
	}
	string cond1, cond2 = cond;
	short var1 = 0, var2 = 0;
	char op = '='; //= is ==, < is <, > is >, ! is !=, l is <=, g is >=, & is &&, | is ||
	//Check for whole thing surrounded in brackets
	if (cond2[0] == '(') {
		cond1 = endBracket(&cond2);
		if (cond2.empty()) {
			cond1.erase(0, 1);
			cond1.pop_back();
			return evalCond(cond1, playerCharacter);
		}
		cond1 = "";
		cond2 = cond;
	}
	//Look for ||
	while (true) {
		switch (cond2[0]) {
		case '(':
			cond1 += endBracket(&cond2);
			break;
		case '|':
			if (cond2.substr(0, 2) != "||") {
				throw 1;
			}
			cond2.erase(0, 2);
			return evalCond(cond1, playerCharacter) || evalCond(cond2, playerCharacter);
		default:
			cond1 += cond2[0];
			cond2.erase(0, 1);
		}
		if (cond2.empty()) {
			break;
		}
	}
	cond1 = "";
	cond2 = cond;
	//Look for &&
	while (true) {
		switch (cond2[0]) {
		case '(':
			cond1 += endBracket(&cond2);
			break;
		case '&':
			if (cond2.substr(0, 2) != "&&") {
				throw 1;
			}
			cond2.erase(0, 2);
			return evalCond(cond1, playerCharacter) && evalCond(cond2, playerCharacter);
		default:
			cond1 += cond2[0];
			cond2.erase(0, 1);
		}
		if (cond2.empty()) {
			break;
		}
	}
	cond2 = cond;
	//Check for !
	if (cond2[0] == '!') {
		cond2.erase(0, 1);
		return !evalCond(cond2, playerCharacter);
	}
	if (cond == "true") {
		return true;
	}
	if (cond == "false") {
		return false;
	}
	var1 = numFromString(&cond, playerCharacter);

	return false;
}

string endBracket(string* cond) {
	string cond1 = "(";
	cond->erase(0, 1);
	if (cond->empty()) {
		throw 1;
	}
	while ((*cond)[0] != ')') {
		if ((*cond)[0] == '(') {
			cond1 += endBracket(cond);
		}
		else {
			cond1 += (*cond)[0];
			cond->erase(0, 1);
		}
		if (cond->empty()) {
			throw 1;
		}
	}
	cond1 += ')';
	cond->erase(0, 1);
	return cond1;
}

uint8_t doLine(ifstream* file, player* playerCharacter) {
	string buffer1 = getTag(file);
	if (buffer1 == "victory/") {
		return 1;
	}
	else if (buffer1 == "defeat/") {
		return 2;
	}
	else if (buffer1 == "text") {
		buffer1 = stringFromFile(file);
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
		if (!currentEvent.getReal()) {
			return 0;
		}
		uint8_t i = 1;
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
		buffer2 = stringFromFile(file);
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
		buffer1 = removeEscapes(buffer1);
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
		buffer1 = removeEscapes(buffer1);
		while (evalCond(buffer1, playerCharacter)) {
			file->seekg(startPos);
			uint8_t i = 0;
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
	else if (buffer1 == "sleep") {
		this_thread::sleep_for(chrono::milliseconds(numFromFile(file, playerCharacter)));
		if (getTag(file) != "/sleep") {
			throw 1;
		}
		ignoreLine(file);
		return 0;
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
			buffer = removeEscapes(buffer);
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

void modifyVar(string var, string operation, player* playerCharacter) { //= num (=), += num (+), -= num (-), *= num (*), /= num (/), %= num (%), ++, --, display
	short value;
	char op;
	clearSpace(&operation);
	if (operation.empty()) {
		return;
	}
	if (operation == "display") {
		cout << *g_customVars.value(var) << '\n';
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

void modifyVar(string var, string operation) { //= num (=), += num (+), -= num (-), *= num (*), /= num (/), %= num (%), ++, --
	short value;
	char op;
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
	if ((operation[0] > 47 && operation[0] < 58) || operation[0] == '-' || operation.substr(0, 2) == "v_" || operation.substr(0, 4) == "rng(") {
		value = numFromString(&operation);
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

bool checkSaveSlot(signed char slot) {
	if (slot < 48 || slot > 57) {
		return false;
	}
	ifstream saveFile;
	string savePath = "saves\\sav";
	savePath += slot;
	savePath += ".xml";
	saveFile.open(savePath);
	if (!saveFile.is_open()) { //No save file present
		return false;
	}
	try {
		if (getTag(&saveFile) == "saveGame/") { //Indicates save data
			saveFile.close();
			return true;
		}
	}
	catch (int) {}
	saveFile.close();
	return false;
}