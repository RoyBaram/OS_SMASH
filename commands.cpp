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
	istringstream iss(line);
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

void checkJoblStatus(set<Job, JobCompare>& jobList) {
	for (const auto job : jobList) {
		pid_t pid = job.getJobPid();
		int status;
		pid_t res = waitpid(pid, &status, WNOHANG | WUNTRACED);

		// process exited, no need to keep in jobs list
		if (res == pid) {
			jobList.erase(job);
		}
	}
}

