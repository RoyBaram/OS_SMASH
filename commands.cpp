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

int parseCmd(const string& line, Command& result) {
	isstringstream iss(line);
	string token;
	vector<string> tokens;
	bool bg = false;

	while (iss >> token && tokens.size() < ARGS_NUM_MAX) {
		if (token == "&") {
			bg = true;
			continue;
		}
		tokens.push_back(token);
	}

	if (tokens.empty()) {
		return INVALID_COMMAND;
	}

	result.setCmd(tokens[0]);
	result.setBg(bg);
	result.setArgs(tokens);

	return 0;
}

