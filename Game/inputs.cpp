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