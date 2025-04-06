// commands.cpp
#include "commands.h"

// C++ implementation for given example
// function for printing errors from internal commands
void perrorSmash(const string cmd, const string msg) {
	cerr << "smash error:" 
		 << (cmd.empty() ? "" : cmd)
		 << (cmd.empty() ? "" : ": ")
		 << msg << endl;
}

int parseCmd(string line, CmdArgs& result) {
	isstringstream iss(line);
	string token;
	vector<string> tokens;

	for (iss >> token && tokens.size() < ARGS_NUM_MAX) {
		tokens.push_back(token);
	}

	if (tokens.empty()) {
		return INVALID_COMMAND;
	}

	result.cmd = tokens[0];
	result.args.assign(tokens.begin(), tokens.end());

	return 0;
}