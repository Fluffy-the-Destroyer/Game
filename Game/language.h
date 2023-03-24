#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "adventures.h"

class variable {
private:
	std::string name;
	short value;
	friend class variables;
public:
	variable(std::string n = "", short v = 0) :name(n), value(v) {}
	friend void adventure::save(player*, std::streampos);
};

class variables {
private:
	std::vector<variable> vars;
public:
	//Assigns value v to variable with name n, if it doesn't exist, creates new variable
	void assign(std::string n, short v);
	//Returns pointer to value of variable with name n
	short* value(std::string n);
	//Takes first variables, for each variable in second variables, adds its value to that variable in the first if it exists, otherwise, creates a new variable with that value
	void operator+=(const variables& varChanges);
	//Deletes all the variables
	void reset();
	friend void adventure::save(player*, std::streampos);
	friend uint8_t adventure::loadFromSave();
};

//Interprets cond as a condition
bool evalCond(std::string cond, player* playerCharacter);
//Moves to end of brackets, returns what it skipped
std::string endBracket(std::string* cond);
//Reads and executes a line of code. Returns 1 if victory, 2 if defeat, 3 if break, 4 if continue, 0 if going to next line. 5 if saving, not allowed inside a while loop
uint8_t doLine(std::ifstream* file, player* playerCharacter);
//Moves to end of if statement, or to an else (if) statement
void endIf(std::ifstream* file, player* playerCharacter, bool newCond = false);
//Moves to end of while loop
void endWhile(std::ifstream* file);
//Modifies var according to operation
void modifyVar(std::string var, std::string operation, player* playerCharacter);
//Modifies variable according to operation, has no reference to player
void modifyVar(std::string var, std::string operation);
//Checks if there is a save in specified slot
bool checkSaveSlot(signed char slot);