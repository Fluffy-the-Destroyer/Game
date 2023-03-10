#pragma once
#include <fstream>
#include <iostream>
#include <vector>
#include "player.h"

void clearSpace(std::istream* stream);
void clearSpace(std::string* in);

void ignoreLine();
void ignoreLine(std::istream* stream, char end = '\n');

std::string getTag(std::istream* stream);
int numFromFile(std::istream* stream);
int numFromFile(std::istream* stream, player* playerCharacter);
float floatFromFile(std::istream* stream);
std::string stringFromFile(std::istream* stream);

//Moves to end of a comment
void endComment(std::istream* stream);

//Extracts a (decimal) integer from the beginning of a string
int numFromString(std::string* in);
int numFromString(std::string* in, player* playerCharacter);

//Extracts a float from a string
float floatFromString(std::string* in);

//Returns a user selected integer between the specified bounds
int userChoice(int lb, int ub);
//Returns a user selected integer from a given list
short userChoice(std::vector<short> choices);