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

void ignoreLine(std::istream* stream) {
	stream->ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //Clear input buffer
}

std::string getTag(std::istream* stream) {
	std::string out;
	clearSpace(stream);
	if (stream->peek() != '<') {
		throw 1;
	}
	stream->ignore(1);
	getline(*stream, out, '>');
	if (stream->eof()) {
		throw 1;
	}
	return out;
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

int userChoice(int lb, int ub) {
	if (ub < lb) {
		throw 7;
	}
	int value = 0;
	while (true) {
		std::cin >> value;
		if (value < lb || value > ub || !std::cin) {
			std::cin.clear();
			ignoreLine(&std::cin);
			std::cout << "Value must be between " << lb << " and " << ub << ". Please re-enter:\n";
			continue;
		}
		ignoreLine(&std::cin);
		return value;
	}
}