#include "prints.h"

// C++ implementation for given example
// function for printing errors from internal commands
void perrorSmashInternal(const string cmd, const string msg) {
	cout << "smash error:" 
		 << (cmd.empty() ? "" : cmd)
		 << (cmd.empty() ? "" : ": ")
		 << msg << endl;
}

void perrorSmashExternal(ExternalErr err) {
	cout << "smash: external: ";
	switch(err) {
	case DNE:
		cout << "cannot find program" << endl;
	case INV_CMD:
		cout << "invalid command" << endl;
	default:
		cout << "unexpected error" << endl;
	}
}
