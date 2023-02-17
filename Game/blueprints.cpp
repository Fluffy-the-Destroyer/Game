#include "blueprints.h"
#include <fstream>
#include <iostream>
#include "inputs.h"
#include "rng.h"
using namespace std;

//Error codes:
// 1: Bad XML
// 2: Specified list not found
// 3: Loading empty slot
// 4: Unable to open file
// 5: Empty list

unsigned char blueprintListSelector(string* blueprint, bool custom) {
	ifstream blueprintLists;
	try {
		if (*blueprint == "EMPTY") {
			throw 3;
		}
		switch ((*blueprint)[0]) {
		case 'h':
			blueprint->erase(0, 2);
			if (*blueprint == "EMPTY") {
				return 0;
			}
			return 1;
		case 't':
			blueprint->erase(0, 2);
			if (*blueprint == "EMPTY") {
				return 0;
			}
			return 2;
		case 'l':
			blueprint->erase(0, 2);
			if (*blueprint == "EMPTY") {
				return 0;
			}
			return 3;
		case 'f':
			blueprint->erase(0, 2);
			if (*blueprint == "EMPTY") {
				return 0;
			}
			return 4;
		case 'w':
			blueprint->erase(0, 2);
			if (*blueprint == "EMPTY") {
				return 0;
			}
			return 5;
		case 's':
			blueprint->erase(0, 2);
			if (*blueprint == "EMPTY") {
				return 0;
			}
			return 6;
		case 'i':
			blueprint->erase(0, 2);
			break;
		default:
			throw 1;
		}
		string buffer = "";
		if (*blueprint == "EMPTY") {
			throw 3;
		}
		if (custom) {
			blueprintLists.open("custom\\blueprintLists.xml");
		}
		else {
			blueprintLists.open("data\\blueprintLists.xml");
		}
		if (!blueprintLists.is_open()) {
			throw 4;
		}
		string blueprintName = "blueprintList name=\"" + *blueprint + '\"';
		streampos filePos = 0;
		short listCount = -1;
		while (buffer != blueprintName) {
			buffer = getTag(&blueprintLists);
			ignoreLine(&blueprintLists);
			if (blueprintLists.eof()) {
				throw 2;
			}
		}
		filePos = blueprintLists.tellg();
		do {
			listCount++;
			buffer = getTag(&blueprintLists);
			ignoreLine(&blueprintLists);
		} while (buffer != "/blueprintList");
		blueprintLists.clear();
		if (listCount == 0) {
			throw 5;
		}
		listCount = rng(1, listCount);
		blueprintLists.seekg(filePos);
		for (int i = 1; i < listCount; i++) {
			ignoreLine(&blueprintLists);
		}
		buffer = getTag(&blueprintLists);
		getline(blueprintLists, *blueprint, '<');
		blueprintLists.seekg(-1, ios_base::cur);
		if (getTag(&blueprintLists) != '/' + buffer) {
			throw 1;
		}
		blueprintLists.close();
		if (*blueprint == "EMPTY") {
			return 0;
		}
		else if (buffer == "armourHead") {
			return 1;
		}
		else if (buffer == "armourTorso") {
			return 2;
		}
		else if (buffer == "armourLegs") {
			return 3;
		}
		else if (buffer == "armourFeet") {
			return 4;
		}
		else if (buffer == "weapon") {
			return 5;
		}
		else if (buffer == "spell") {
			return 6;
		}
	}
	catch (int err) {
		blueprintLists.close();
		switch (err) {
		case 1:
			cout << "Unable to parse blueprintList " << *blueprint << '\n';
			return 0;
		case 2:
			if (custom) {
				return blueprintListSelector(blueprint, false);
			}
			cout << "Could not find blueprintList " << *blueprint << '\n';
			return 0;
		case 3:
			return 0;
		case 4:
			if (custom) {
				return blueprintListSelector(blueprint, false);
			}
			cout << "Could not open blueprintLists.xml\n";
			return 0;
		case 5:
			cout << "blueprintList " << *blueprint << " contains no entries\n";
			return 0;
		}
	}
	return 0;
}