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
#include "adventures.h"
#include <iomanip>
using namespace std;

resource g_projName, g_manaName; //Loads names for mana and projectiles from misc.xml
variables g_customVars; //Holds variables for branching adventure
bool g_useCustomData; //Are we using custom stuff

/*Error codes:
	0: Unspecified, no further error messages should be shown
	1: Bad XML
	2: Unable to find specified item
	3: Loading empty slot
	4: Could not open file
	5: Empty list
	6: Accessing slot out of range
	7: Attempting to choose from empty set
	8: Variable limit reached
*/

int main() {
	
	

	bool done = false;
	while (!done) {
		cout << "To play adventure mode, enter 1.\nTo play battle mode, enter 2.\nTo quit, enter 0.\n";
		switch (userChoice(0, 2)) {
		case 1:
			if (adventureMode() == 0) {
				done = true;
			}
			break;
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