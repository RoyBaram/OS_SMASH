// smash.cpp

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include "commands.h"
#include "prints.h"
#include "signals.h"

#include <iostream>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

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
	{"jobs", intJobs},
	{"kill", intKill},
	{"fg", intFg},
	{"bg", intBg},
	{"quit", intQuit},
	{"diff", intDiff},
	{"", intDoNothing}
};
string prevWd("");
pid_t smashPID = 0;
pid_t currTaskPid = 0;
volatile sig_atomic_t sentSigint = NOT_SIGNALED;
volatile sig_atomic_t sentSigtstp = NOT_SIGNALED;

/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[])
{
	smashPID = getpid();
	string line;
	string cmd;
	struct sigaction sa_sigint, sa_sigtstp;
	sa_sigint.sa_handler = sigintHandler;
	sa_sigtstp.sa_handler = sigtstpHandler;
	sigaction(SIGINT, &sa_sigint, NULL);
	sigaction(SIGTSTP, &sa_sigtstp, NULL);

	while(true) {
		currTaskPid = getpid();
		cout << "smash > ";
		if (!getline(cin, line)) {
			if (sentSigint == SIGNALED || sentSigtstp == SIGNALED) {
				cin.clear();
				sentSigint = NOT_SIGNALED;
				sentSigtstp = NOT_SIGNALED;
				continue;
			}
		}
		cmd = line;
		Command command;
		parseCmd(cmd, command);

		chkUpdtJoblStatus(jobList);

		executeCommand(command);

		// prints used for debugging
		/*cout << "your command: " << command.getCmd() << " with arguments: [";
		for (const auto& arg : command.getArgs()) {
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