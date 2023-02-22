#pragma once
#include <fstream>
#include <iostream>
#include <vector>

void clearSpace(std::istream* stream);
void clearSpace(std::string* in);

void ignoreLine();
void ignoreLine(std::istream* stream, char end = '\n');

std::string getTag(std::istream* stream);

//Moves to end of a comment
void endComment(std::istream* stream);

//Extracts a (decimal) number from the beginning of a string
int numFromString(std::string* in);

//Returns a user selected integer between the specified bounds
int userChoice(int lb, int ub);
//Returns a user selected integer from a given list
short userChoice(std::vector<short> choices);