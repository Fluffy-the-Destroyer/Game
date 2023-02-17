#pragma once
#include <fstream>
#include <iostream>

void clearSpace(std::istream* stream);
void clearSpace(std::string* in);

void ignoreLine(std::istream* stream);

std::string getTag(std::istream* stream);

//Extracts a (decimal) number from the beginning of a string
int numFromString(std::string* in);

//Returns a user selected integer between the specified bounds
int userChoice(int lb, int ub);