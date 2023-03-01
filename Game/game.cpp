#include <iostream>
#include <fstream>
#include "player.h"
#include "rng.h"
#include "armour.h"
#include "weapons.h"
#include "inputs.h"
#include "resources.h"
#include "enemies.h"
#include "blueprints.h"
#include "language.h"
#include "events.h"
#include "battle.h"
using namespace std;

resource g_projName, g_manaName; //Loads names for mana and projectiles from misc.xml
variables g_customVars; //Holds variables for branching adventure
bool g_useCustomData = false; //Are we using custom stuff

int main() {
	
	
	
	bool done = false;
	while (!done) {
		cout << "To play adventure mode, enter 1. (Not implemented)\nTo play battle mode, enter 2.\nTo quit, enter 0.\n";
		switch (userChoice(0, 2)) {
		case 2:
			if (battleMode() == 0) {
				done = true;
			}
			break;
		default:
			done = true;
		}
	}
	return 0;
}