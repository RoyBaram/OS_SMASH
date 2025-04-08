// smash.cpp

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

#include "signals.h"
#include "commands.h"
#include "prints.h"

using namespace std;

/*=============================================================================
* classes/structs declarations
=============================================================================*/

/*=============================================================================
* global variables & data structures
=============================================================================*/
char _line[CMD_LENGTH_MAX];
JobIDs IDs;
set<Job, JobCompare> jobList;
unordered_map<string, function<void(const Command&)>> internalCmds = {
	{"showpid", intShowpid},
	{"pwd", intPwd},
	{"cd", intCd},
	//{"jobs", /**/},
	//{"kill", /**/},
	//{"fg", /**/},
	//{"bg", /**/},
	//{"quit", /**/},
	//{"diff", /**/},
};
string prevWd("");

/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	string line;
	string cmd;

	while(true) {
		cout << "smash > ";
		if (!getline(cin, line)) {
			break;
		}
		cmd = line;
		Command command;
		parseCmd(cmd, command);

		chkUpdtJoblStatus(jobList);

		executeCommand(command);

		// prints used for testing
		/*cout << "your command: " << command.getCmd() << " with arguments: [";
		for (const auto arg : command.getArgs()) {
			cout << arg << ", ";
		}
		string state = command.isBg() ? "background" : "foreground";
		cout << "\b\b]; it is in " << state << endl;*/
		


		// initialize vars for next command
		line.clear();
		cmd.clear();
	}
	return 0;
}