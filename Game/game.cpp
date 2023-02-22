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
using namespace std;

resource g_projName, g_manaName; //Loads names for mana and projectiles from misc.xml
variables g_customVars; //Holds variables for branching adventure
bool g_useCustomData = false; //Are we using custom stuff

int main() {
	
	vector<short> choices(1, 0);
	choices.resize(1);

	return 0;
}