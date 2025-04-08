// commands.cpp
#include "commands.h"
#include "globals.h"



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

void chkUpdtJoblStatus(set<Job, JobCompare>& jobList) {
	for (auto job : jobList) {
		pid_t pid = job.getJobPid();
		int status;
		pid_t res = waitpid(pid, &status, WNOHANG);

		// process exited, no need to keep in jobs list
		if (res == pid) {
			IDs.insertID(job.getJobID);
			jobList.erase(job);
		}

		else if (WIFSTOPPED(status)) {
			job.setStatus(STOPPED);
		}
	}
}

void executeCommand(const Command& cmd) {
	string _cmd = cmd.getCmd();

	auto it = internalCmds.find(_cmd);
	// if cmd is an internal command
	if (it != internalCmds.end()) {
		// if running in background
		if (cmd.isBg()) {
			runNewProc(cmd, it, NO_EXEC);
		}
		// if running in foreground
		else {
			it->second(cmd);
		}
	}
	else {
		runNewProc(cmd, it, EXEC);
	}
}

void runNewProc(
	const Command& cmd,
	unordered_map<string, function<void(const Command&)>>::iterator it,
	int option) {
	vector<string> args = cmd.getArgs();
	vector<string> pureArgs(args.begin() + 1, args.end());
	vector<char *> argv;
	for (auto& arg : pureArgs) {
		argv.push_back(const_cast<char *>(arg.c_str()));
	}
	argv.push_back(nullptr);
	int childPid = fork();
	if (childPid == 0) {
		setpgrp();
		switch(option) {
		case NO_EXEC:
			it->second(cmd);
			break;
		case EXEC:
			execv(cmd.getCmd().c_str(), argv.data());
			perror("smash error: execv failed");
			exit(1);
		};
	}
	else if (childPid > 0) {
		int jobId = IDs.getID();
		Job newJob(jobId, cmd);
		newJob.setStatus(BACKGROUND);
		newJob.setPid(childPid);
		jobList.insert(newJob);
	}
	else {
		perror("smash error: fork failed");
	}
}



string getcwdStr() {
	char *tmp = getcwd(NULL, 0);
	if (!tmp) {
		perror("smash error: getcwd failed");
		exit(1);
	}
	string cwd(tmp);
	free(tmp);
	return cwd;
}

bool chdirAndCheck(const string& cmd, const string& path) {
	status = chdir(path.c_str());
	if (!status) {
		switch(errno) {
		case ENOENT:
			perrorSmashInternal(cmd, "target directory does not exist");
			break;
		case ENOTDIR:
			perrorSmashInternal(cmd, dst + ": not a directory");
			break;
		default:
			perror("smash error: chdir failed");
			exit(1);
		}
		return false;
	}
	return true;
}

bool checkNoArgs(const string& cmd, const vector<string>& args) {
	size_t numArgs = args.size();
	if ((int) numArgs != NO_ARGS) {
		perrorSmashInternal(cmd, "expected 0 arguments");
		return false;
	}
	return true;
}

/*=============================================================================
* implementation of built-in (internal) commands
=============================================================================*/

void intShowpid(const Command& cmd) {
	if (checkNoArgs(cmd.getCmd(), cmd.getArgs())) {
		pid_t pid = cmd.isBg() ? getpid() : getppid();
		cout << "smash pid is " << pid << endl;
	}
}

void intPwd(const Command& cmd) {
	if (checkNoArgs(cmd.getCmd(), cmd.getArgs())) {
		perrorSmashInternal(cmd.getCmd(), "expected 0 arguments");
		string cwd = getcwdStr();
		cout << cwd << endl;
	}
}

void intCd(const Command& cmd) {
	vector<string> args = cmd.getArgs();
	string _cmd = cmd.getCmd();
	string tmpWd = getcwdStr;
	size_t numArgs = args.size();
	if ((int) numArgs != CMD_ARGS) {
		perrorSmashInternal(_cmd, "expected 1 arguments")
	}
	else {
		string path(args[1]);
		if (path == "-") {
			if (prevWd.size() > 0) {
				if (chdirAndCheck(_cmd, prevWd)) {
					prevWd = tmpWd;
				}
			}
			else {
				perrorSmashInternal(_cmd, "old pwd not set");
			}
		}
		else {
			if(chdirAndCheck(_cmd, path)) {
				prevWd = tmpWd;
			}
		}
	}
}