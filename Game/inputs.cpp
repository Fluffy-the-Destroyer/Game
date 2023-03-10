#include "inputs.h"
#include <string>
#include "rng.h"
#include "language.h"

//Error codes:
// 1: Bad XML, including premature end of file
// 2: Specified object not found
// 4: unable to open file
// 7: Specified set of values is empty

extern variables g_customVars;

void clearSpace(std::istream* stream) {
	while (stream->peek() == '\t' || stream->peek() == '\n' || stream->peek() == ' ') {
		stream->ignore(1);
	}
}
void clearSpace(std::string* in) {
	while (!in->empty() && (*in)[0] == ' ') {
		in->erase(0, 1);
	}
}

void ignoreLine() {
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
void ignoreLine(std::istream* stream, char end) {
	while (!stream->eof()) {
		switch (stream->peek()) {
		case '\n': //Reached end of line, not in a comment
			stream->ignore(1);
			return;
		case '<': //Might be in a comment, check and if so, skip to end
			stream->ignore(1);
			if (stream->peek() == '!') {
				stream->ignore(1);
				if (stream->peek() == '-') {
					stream->ignore(1);
					if (stream->peek() == '-') {
						stream->ignore(1);
						endComment(stream);
					}
				}
			}
			break;
		default:
			stream->ignore(1);
		}
	}
}

std::string getTag(std::istream* stream) {
	std::string out;
	clearSpace(stream);
	if (stream->peek() != '<') {
		throw 1;
	}
	stream->ignore(1);
	//Check if it's a comment and if so, ignore it
	if (stream->peek() == '!') {
		out += stream->get();
		if (stream->peek() == '-') {
			out += stream->get();
			if (stream->peek() == '-') {
				out += stream->get();
				//We are now in a comment
				endComment(stream); //Move to end of comment
				return getTag(stream);
			}
		}
	}
	getline(*stream, out, '>');
	if (stream->eof()) {
		throw 1;
	}
	return out;
}

int numFromFile(std::istream* stream) {
	std::string buffer1, buffer2;
	while (true) {
		getline(*stream, buffer2, '<');
		buffer1 += buffer2;
		if (stream->peek() == '!') {
			stream->ignore(1);
			if (stream->peek() == '-') {
				stream->ignore(1);
				if (stream->peek() == '-') {
					stream->ignore(1);
					endComment(stream);
					continue;
				}
				else {
					stream->seekg(-3, std::ios_base::cur);
				}
			}
			else {
				stream->seekg(-2, std::ios_base::cur);
			}
		}
		else {
			stream->seekg(-1, std::ios_base::cur);
		}
		break;
	}
	return numFromString(&buffer1);
}
int numFromFile(std::istream* stream, player* playerCharacter) {
	std::string buffer1, buffer2;
	while (true) {
		getline(*stream, buffer2, '<');
		buffer1 += buffer2;
		if (stream->peek() == '!') {
			stream->ignore(1);
			if (stream->peek() == '-') {
				stream->ignore(1);
				if (stream->peek() == '-') {
					stream->ignore(1);
					endComment(stream);
					continue;
				}
				else {
					stream->seekg(-3, std::ios_base::cur);
				}
			}
			else {
				stream->seekg(-2, std::ios_base::cur);
			}
		}
		else {
			stream->seekg(-1, std::ios_base::cur);
		}
		break;
	}
	return numFromString(&buffer1, playerCharacter);
}
float floatFromFile(std::istream* stream) {
	std::string buffer1, buffer2;
	while (true) {
		getline(*stream, buffer2, '<');
		buffer1 += buffer2;
		if (stream->peek() == '!') {
			stream->ignore(1);
			if (stream->peek() == '-') {
				stream->ignore(1);
				if (stream->peek() == '-') {
					stream->ignore(1);
					endComment(stream);
					continue;
				}
				else {
					stream->seekg(-3, std::ios_base::cur);
				}
			}
			else {
				stream->seekg(-2, std::ios_base::cur);
			}
		}
		else {
			stream->seekg(-1, std::ios_base::cur);
		}
		break;
	}
	return floatFromString(&buffer1);
}
std::string stringFromFile(std::istream* stream) {
	std::string buffer1, buffer2;
	while (true) {
		getline(*stream, buffer2, '<');
		buffer1 += buffer2;
		if (stream->peek() == '!') {
			stream->ignore(1);
			if (stream->peek() == '-') {
				stream->ignore(1);
				if (stream->peek() == '-') {
					stream->ignore(1);
					endComment(stream);
					continue;
				}
				else {
					stream->seekg(-3, std::ios_base::cur);
				}
			}
			else {
				stream->seekg(-2, std::ios_base::cur);
			}
		}
		else {
			stream->seekg(-1, std::ios_base::cur);
		}
		break;
	}
	return buffer1;
}

void endComment(std::istream* stream) {
	if (stream->peek() == '-') { //Check for first character being a -
		stream->ignore(1);
		if (stream->peek() == '-') { //Check for empty comment
			stream->ignore(1);
			if (stream->peek() != '>') {
				throw 1;
			}
			stream->ignore(1);
			return;
		}
	}
	while (stream->peek() != '-') { //Ignore until it finds --, the end of the comment
		stream->ignore(std::numeric_limits<std::streamsize>::max(), '-');
		if (stream->eof()) {
			throw 1;
		}
	}
	if (stream->peek() == '-') {
		stream->ignore(1);
	}
	if (stream->peek() != '>') {
		throw 1;
	}
	stream->ignore(1); //Now at end of comment
}

int numFromString(std::string* in) {
	if (in->empty()) {
		return 0;
	}
	int value = 0;
	bool minus = false;
	if ((*in)[0] == '-') {
		minus = true;
		in->erase(0, 1);
	}
	if (in->substr(0, 4) == "rng(") {
		int value2 = 0;
		in->erase(0, 4);
		clearSpace(in);
		value = numFromString(in);
		clearSpace(in);
		if (in->empty() || (*in)[0] != ',') {
			return 0;
		}
		in->erase(0, 1);
		clearSpace(in);
		value2 = numFromString(in);
		clearSpace(in);
		if (in->empty() || (*in)[0] != ')') {
			return 0;
		}
		in->erase(0, 1);
		value = rng(value, value2);
		if (minus) {
			value *= -1;
		}
		return value;
	}
	if (in->substr(0, 2) == "v_") {
		std::string varName = "";
		in->erase(0, 2);
		while (!in->empty() && (*in)[0] != ' ') {
			varName += (*in)[0];
			in->erase(0, 1);
		}
		value = *g_customVars.value(varName);
		if (minus) {
			value *= -1;
		}
		return value;
	}
	while (!in->empty() && (*in)[0] > 47 && (*in)[0] < 58) {
		value *= 10;
		value += (*in)[0] - 48;
		in->erase(0, 1);
	}
	if (minus) {
		value *= -1;
	}
	return value;
}

int numFromString(std::string* in, player* playerCharacter) {
	if (in->empty()) {
		return 0;
	}
	int value = 0;
	bool minus = false;
	if ((*in)[0] == '-') {
		minus = true;
		in->erase(0, 1);
	}
	if (in->substr(0, 4) == "rng(") {
		int value2 = 0;
		in->erase(0, 4);
		clearSpace(in);
		value = numFromString(in, playerCharacter);
		clearSpace(in);
		if (in->empty() || (*in)[0] != ',') {
			return 0;
		}
		in->erase(0, 1);
		clearSpace(in);
		value2 = numFromString(in, playerCharacter);
		clearSpace(in);
		if (in->empty() || (*in)[0] != ')') {
			return 0;
		}
		in->erase(0, 1);
		value = rng(value, value2);
		if (minus) {
			value *= -1;
		}
		return value;
	}
	if (in->substr(0, 2) == "v_") {
		std::string varName = "";
		in->erase(0, 2);
		while (!in->empty() && (*in)[0] != ' ') {
			varName += (*in)[0];
			in->erase(0, 1);
		}
		value = *g_customVars.value(varName);
		if (minus) {
			value *= -1;
		}
		return value;
	}
	if (in->substr(0, 2) == "p_") {
		in->erase(0, 2);
		if (in->substr(0, 6) == "health") {
			in->erase(0, 6);
			value = playerCharacter->getHealth();
		}
		else if (in->substr(0, 9) == "maxHealth") {
			in->erase(0, 9);
			value = playerCharacter->getMaxHealth();
		}
		else if (in->substr(0, 11) == "projectiles") {
			in->erase(0, 11);
			value = playerCharacter->getProjectiles();
		}
		else if (in->substr(0, 4) == "mana") {
			in->erase(0, 4);
			value = playerCharacter->getMana();
		}
		else if (in->substr(0, 7) == "maxMana") {
			in->erase(0, 7);
			value = playerCharacter->getMaxMana();
		}
		else if (in->substr(0, 13) == "turnManaRegen") {
			in->erase(0, 13);
			value = playerCharacter->getTurnManaRegen();
		}
		else if (in->substr(0, 15) == "battleManaRegen") {
			in->erase(0, 15);
			value = playerCharacter->getBattleManaRegen();
		}
		else if (in->substr(0, 12) == "poisonResist") {
			in->erase(0, 12);
			value = static_cast<int>(100 * playerCharacter->getPoisonResist());
		}
		else if (in->substr(0, 11) == "bleedResist") {
			in->erase(0, 11);
			value = static_cast<int>(100 * playerCharacter->getBleedResist());
		}
		else if (in->substr(0, 10) == "constRegen") {
			in->erase(0, 10);
			value = playerCharacter->getConstRegen();
		}
		else if (in->substr(0, 11) == "battleRegen") {
			in->erase(0, 11);
			value = playerCharacter->getBattleRegen();
		}
		else if (in->substr(0, 11) == "weaponSlots") {
			in->erase(0, 11);
			value = playerCharacter->getWeaponSlots();
		}
		else if (in->substr(0, 10) == "spellSlots") {
			in->erase(0, 10);
			value = playerCharacter->getSpellSlots();
		}
		else if (in->substr(0, 10) == "flatArmour") {
			in->erase(0, 10);
			value = playerCharacter->getFlatArmour();
		}
		else if (in->substr(0, 15) == "flatMagicArmour") {
			in->erase(0, 15);
			value = playerCharacter->getFlatMagicArmour();
		}
		else if (in->substr(0, 10) == "propArmour") {
			in->erase(0, 10);
			value = static_cast<int>(100 * playerCharacter->getPropArmour());
		}
		else if (in->substr(0, 15) == "propMagicArmour") {
			in->erase(0, 15);
			value = static_cast<int>(100 * playerCharacter->getFlatMagicArmour());
		}
		else if (in->substr(0, 18) == "flatDamageModifier") {
			in->erase(0, 18);
			value = playerCharacter->getFlatDamageModifier();
		}
		else if (in->substr(0, 18) == "propDamageModifier") {
			in->erase(0, 18);
			value = static_cast<int>(100 * playerCharacter->getPropDamageModifier());
		}
		else if (in->substr(0, 23) == "flatMagicDamageModifier") {
			in->erase(0, 23);
			value = playerCharacter->getFlatMagicDamageModifier();
		}
		else if (in->substr(0, 23) == "propMagicDamageModifier") {
			in->erase(0, 23);
			value = static_cast<int>(100 * playerCharacter->getPropMagicDamageModifier());
		}
		else if (in->substr(0, 32) == "flatArmourPiercingDamageModifier") {
			in->erase(0, 32);
			value = playerCharacter->getFlatArmourPiercingDamageModifier();
		}
		else if (in->substr(0, 32) == "propArmourPiercingDamageModifer") {
			in->erase(0, 32);
			value = static_cast<int>(100 * playerCharacter->getPropArmourPiercingDamageModifier());
		}
		else if (in->substr(0, 11) == "evadeChance") {
			in->erase(0, 11);
			value = static_cast<int>(100 * playerCharacter->getEvadeChance());
		}
		else if (in->substr(0, 19) == "counterAttackChance") {
			in->erase(0, 19);
			value = static_cast<int>(100 * playerCharacter->getCounterAttackChance());
		}
		else if (in->substr(0, 12) == "bonusActions") {
			in->erase(0, 12);
			value = playerCharacter->getBonusActions();
		}
		else if (in->substr(0, 10) == "initiative") {
			in->erase(0, 10);
			value = playerCharacter->getInitiative();
		}
		else if (in->substr(0, 2) == "xp") {
			in->erase(0, 2);
			value = playerCharacter->getXp();
		}
		else if (in->substr(0, 5) == "maxXp") {
			in->erase(0, 5);
			value = playerCharacter->getMaxXp();
		}
		else if (in->substr(0, 5) == "level") {
			in->erase(0, 5);
			value = playerCharacter->getLevel();
		}
		if (minus) {
			value *= -1;
		}
		return value;
	}
	while (!in->empty() && (*in)[0] > 47 && (*in)[0] < 58) {
		value *= 10;
		value += (*in)[0] - 48;
		in->erase(0, 1);
	}
	if (minus) {
		value *= -1;
	}
	return value;
}

float floatFromString(std::string* in) {
	if (in->empty()) {
		return 0;
	}
	float value = 0;
	bool minus = false;
	if ((*in)[0] == '-') {
		minus = true;
		in->erase(0, 1);
	}
	if (in->substr(0, 4) == "rng(") {
		float value2 = 0;
		in->erase(0, 4);
		clearSpace(in);
		value = floatFromString(in);
		clearSpace(in);
		if (in->empty() || (*in)[0] != ',') {
			return 0;
		}
		in->erase(0, 1);
		clearSpace(in);
		value2 = floatFromString(in);
		clearSpace(in);
		if (in->empty() || (*in)[0] != ')') {
			return 0;
		}
		in->erase(0, 1);
		value = rng(value, value2);
		if (minus) {
			value *= -1;
		}
		return value;
	}
	while (!in->empty() && (*in)[0] > 47 && (*in)[0] < 58) {
		value *= 10;
		value += (*in)[0] - 48;
		in->erase(0, 1);
	}
	if (in->empty() || (*in)[0] != 46) {
		if (minus) {
			value *= -1;
		}
		return value;
	}
	in->erase(0, 1);
	short dp = 0;
	while (!in->empty() && (*in)[0] > 47 && (*in)[0] < 58) {
		value *= 10;
		value += (*in)[0] - 48;
		in->erase(0, 1);
		dp++;
	}
	for (short i = 0; i < dp; i++) {
		value /= 10;
	}
	if (minus) {
		value *= -1;
	}
	return value;
}

int userChoice(int lb, int ub) {
	if (ub < lb) {
		throw 7;
	}
	int value = 0;
	while (true) {
		std::cin >> value;
		if (value < lb || value > ub || !std::cin) {
			std::cin.clear();
			ignoreLine();
			std::cout << "Value must be between " << lb << " and " << ub << ". Please re-enter:\n";
			continue;
		}
		ignoreLine();
		return value;
	}
}

short userChoice(std::vector<short> choices) {
	if (choices.empty()) {
		throw 7;
	}
	short value = 0;
	while (true) {
		std::cin >> value;
		if (!std::cin) {
			std::cin.clear();
			ignoreLine();
			std::cout << "Please enter one of the specified values:\n";
			continue;
		}
		for (int i = 0; i < choices.size(); i++) {
			if (value == choices[i]) {
				return value;
			}
		}
		ignoreLine();
		std::cout << "Please enter one of the specified values:\n";
	}
}