#include "resources.h"
#include <fstream>
#include "inputs.h"
#include <iostream>

//Error codes:
// 1: Bad XML, including premature end of file
// 2: Specified object not found
// 4: unable to open file
// 7: Specified set of values is empty

std::string resource::Singular() {
	if (singular() == "") {
		return singular();
	}
	std::string buffer = singular(); //Create buffer as singular is const
	if (97 <= buffer[0] && buffer[0] <= 122) { //It's a lower case letter
		buffer[0] -= 32; //Will capitalise the first letter
	}
	return buffer;
}

std::string resource::Plural() {
	if (plural() == "") {
		return plural();
	}
	std::string buffer = plural(); //Create buffer as plural is const
	if (97 <= buffer[0] && buffer[0] <= 122) { //It's a lower case letter
		buffer[0] -= 32; //Will capitalise the first letter
	}
	return buffer;
}

void resource::loadFromFile(std::string resource, bool custom) {
	std::ifstream misc;
	std::string buffer = "";
	try {
		if (custom) {
			misc.open("custom\\misc.xml");
		}
		else {
			misc.open("data\\misc.xml");
		}
		if (!misc.is_open()) {
			throw 4;
		}
		ignoreLine(&misc, '<');
		if (custom && misc.eof()) {
			throw 4;
		}
		std::string resourceName = "resourceBlueprint name=\"" + resource + '\"';
		while (buffer != resourceName) {
			buffer = getTag(&misc);
			ignoreLine(&misc);
			if (misc.eof()) {
				throw 2;
			}
		}
		buffer = getTag(&misc);
		while (buffer != "/resourceBlueprint") {
			if (misc.eof()) {
				throw 1;
			}
			if (buffer == "singular") {
				std::getline(misc, s, '<');
			}
			else if (buffer == "plural") {
				std::getline(misc, p, '<');
			}
			else {
				throw 1;
			}
			misc.seekg(-1, std::ios_base::cur);
			if (getTag(&misc) != '/' + buffer) {
				throw 1;
			}
			ignoreLine(&misc);
			if (!misc) {
				throw 1;
			}
			buffer = getTag(&misc);
		}
		misc.close();
	}
	catch (int err) {
		misc.close();
		switch (err) {
		case 1:
			std::cout << "Could not parse blueprint for resource " << resource << '\n';
			break;
		case 2:
			if (custom) {
				loadFromFile(resource, false);
				return;
			}
			std::cout << "Could not find blueprint for resource " << resource << '\n';
			break;
		case 4:
			if (custom) {
				loadFromFile(resource, false);
				return;
			}
			std::cout << "Could not open misc.xml";
			break;
		}
		if (resource == "PROJECTILE") {
			s = "arrow";
			p = "arrows";
		}
		else if (resource == "MANA") {
			s = p = "mana";
		}
	}
}