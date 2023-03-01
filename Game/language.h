#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "player.h"

class variable {
private:
	std::string name;
	short value;
	friend class variables;
public:
	variable(std::string n = "", short v = 0) :name(n), value(v) {}
	friend void save(std::ifstream* file, player* playerCharacter, std::string filePath, unsigned char slot);
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
	variables operator+(const variables& varChanges);
	void operator+=(const variables& varChanges);
	//Deletes all the variables
	void reset();
	friend void save(std::ifstream* file, player* playerCharacter, std::string filePath, unsigned char slot);
};

//Interprets cond as a condition
bool evalCond(std::string cond, player* playerCharacter);
//Reads and executes a line of code. Returns 1 if victory, 2 if defeat, 3 if break, 4 if continue, 0 if going to next line. 5 if saving, not allowed inside a while loop
unsigned char doLine(std::ifstream* file, player* playerCharacter);
//Moves to end of if statement, or to an else (if) statement
void endIf(std::ifstream* file, player* playerCharacter, bool newCond = false);
//Moves to end of while loop
void endWhile(std::ifstream* file);
//Modifies var according to operation
void modifyVar(std::string var, std::string operation, player* playerCharacter);
//Saves the game
void save(std::ifstream* file, player* playerCharacter, std::string filePath, unsigned char slot);