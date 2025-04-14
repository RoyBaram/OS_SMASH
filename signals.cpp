// signals.cpp
#include "signals.h"
#include "commands.h"
#include "globals.h"

void sigtstpHandler(int sig) {
	if (sig == SIGTSTP) {
		cout << "smash: caught CTRL+Z" << endl;
		if (currTaskPid == smashPID) {
			sentSigtstp = SIGNALED;
		}
		else {
			cout << "smash: process " << currTaskPid << " was stopped" << endl;
			kill(currTaskPid, SIGTSTP);
		}
	}
}

void sigintHandler(int sig) {
	if (sig == SIGINT) {
		cout << "smash: caught CTRL+C" << endl;
		if (currTaskPid == smashPID) {
			sentSigint = SIGNALED;
		}
		else {
			cout << "smash: process " << currTaskPid << " was killed" << endl;
			kill(currTaskPid, SIGINT);
		}
	}
}