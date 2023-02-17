#pragma once
#include <string>

extern bool g_useCustomData;

//Picks a random item from the list named blueprint. Sets blueprint to that items blueprint name, returns the type of item. 1-4 are armour pieces, 5 is a weapon, 6 is a spell. Returns 0 if no item is chosen
unsigned char blueprintListSelector(std::string* blueprint, bool custom = g_useCustomData);