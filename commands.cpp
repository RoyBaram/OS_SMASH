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
	for (auto job = jobList.begin(); job != jobList.end(); ) {
		pid_t pid = job->getJobPid();
		int status;
		pid_t res = waitpid(pid, &status, WNOHANG | WUNTRACED | WCONTINUED);

		// process changed status
		if (res == pid) {
			// process is stopped, changes status accordingly
			if (WIFSTOPPED(status)) {
				const_cast<Job&>(*job).setStatus(STOPPED);
				++job;
			}
			else if (WIFCONTINUED(status)) {
				const_cast<Job&>(*job).setStatus(NOSTATUS);
				++job;
			}
			// process finished, no need to keep in jobs list
			else {
				IDs.insertID(job->getJobID());
				job = jobList.erase(job);
			}
		}
		else {
			++job;
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
	vector<char *> argv;
	for (auto& arg : args) {
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
			exit(EXECV);
			break;
		};
	}
	else if (childPid > 0) {
		if (!cmd.isBg()) {
			waitpid(childPid, NULL, 0);
		}
		else {
			int jobId = IDs.getID();
			Job newJob(jobId, cmd);
			newJob.setStatus(BACKGROUND);
			newJob.setJobPid(childPid);
			time_t sTime = time(NULL);
			if (sTime == ((time_t) -1)) {
				perror("smash error: time failed");
				exit(TIME);
			}
			newJob.setJobStart(sTime);
			jobList.insert(newJob);
		}
	}
	else {
		perror("smash error: fork failed");
		exit(FORK);
	}
}



string getcwdStr() {
	char *tmp = getcwd(NULL, 0);
	if (!tmp) {
		perror("smash error: getcwd failed");
		exit(GETCWD);
	}
	string cwd(tmp);
	free(tmp);
	return cwd;
}

bool chdirAndCheck(const string& cmd, const string& path) {
	auto status = chdir(path.c_str());
	if (status == -1) {
		switch(errno) {
		case ENOENT:
			perrorSmashInternal(cmd, "target directory does not exist");
			break;
		case ENOTDIR:
			perrorSmashInternal(cmd, path + ": not a directory");
			break;
		default:
			perror("smash error: chdir failed");	
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

string rebuildCmd(const vector<string>& strs, bool bg) {
	ostringstream oss;
	for (size_t i = 0; i < strs.size(); i++) {
		oss << strs[i];
		if (i < strs.size() - 1) oss << " ";
	}
	if (bg) {
		oss << " &";
	}
	return oss.str();
}

void safeClose(int fd) {
	if (close(fd) != 0) {
		perror("smash error: close failed");
		exit(CLOSE);
	}
}

bool checkNDir(int fd) {
	struct stat st;
	// use fstat() to get info on file descriptor
	if(fstat(fd, &st) != 0) {
		perror("smash error: fstat failed");
		return false;
	}
	if (S_ISDIR(st.st_mode)) {
		perrorSmashInternal("diff", "paths are not files");
		return false;
	}
	return true;
}

bool openFileAndCheck(string path, int *retFd) {
	int fd = open(path.c_str(), O_RDONLY);
	bool retval = true;
	*retFd = BAD_FD;
	// couldn't open file in read-only mode
	if (fd == BAD_FD) {
		switch (errno) {
		case ENOENT:
			perrorSmashInternal("diff", "expected valid paths for files");
			retval = false;
			break;
		default:
			perror("smash error: open failed");
			break;
		}
	}
	// opened file in read-only mode
	else {
		if (!checkNDir(fd)) {
			retval = false;
			safeClose(fd);
		}
		else {
			*retFd = fd;
		}
	}

	return retval;
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
		string cwd = getcwdStr();
		cout << cwd << endl;
	}
}

void intCd(const Command& cmd) {
	vector<string> args = cmd.getArgs();
	string _cmd = cmd.getCmd();
	string tmpWd = getcwdStr();
	size_t numArgs = args.size();
	if ((int) numArgs != CD_ARGS) {
		perrorSmashInternal(_cmd, "expected 1 arguments");
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

void intJobs(const Command& cmd) {
	int id;
	string _cmd;
	pid_t pid;
	time_t elpased;
	time_t now = time(NULL);
	if (cmd.getArgs().size() != NO_ARGS) {
		perrorSmashInternal(cmd.getCmd(), "expected 0 arguments");
	}
	else {
		if (now == ((time_t) -1)) {
			perror("smash error: time failed");
			exit(TIME);
		}
		JobStatus status;
		for (auto job : jobList) {
			id = job.getJobID();
			_cmd = rebuildCmd(job.getArgs(), job.isBg());
			pid = job.getJobPid();
			elpased = difftime(now, job.getJobStart());
			status = job.getStatus();
			cout << "[" << id << "] " << _cmd << ": ";
			cout << pid << " " << elpased << " secs ";
			cout << (status == STOPPED ? "(stopped)" : "");
			cout << endl;
		}
	}
}

void intKill(const Command& cmd) {
	int signum, id;
	string _cmd = cmd.getCmd();
	vector<string> args = cmd.getArgs();
	pid_t pid;
	if (args.size() != KILL_ARGS) {
		perrorSmashInternal(_cmd, "invalid arguments");
	}
	else {
		try {
			id = stoi(args[ID_ARG]);
			signum = stoi(args[SIGNUM]);
		} catch(const exception& ex1) {
			perrorSmashInternal(_cmd, "invalid arguments");
			return;
		}
		Job tmp(id);
		auto job = jobList.find(tmp);
		// there is a job with desired ID in the jobs list
		if (job != jobList.end()) {
			pid = job->getJobPid();
			if (kill(pid, signum) != 0) {
				switch(errno) {
				case EINVAL:
					perrorSmashInternal(_cmd, "invalid arguments");
					break;
				default:
					return;
				}
			}
			else {
				cout << "signal " << signum << " was sent to pid " << pid << endl;
			}
		}
		else {
			string msg = "job id " + to_string(id) + " does not exist";
			perrorSmashInternal(_cmd, msg);
		}
	}
}

void intFg(const Command& cmd) {
	int id;
	string _cmd;
	vector<string> args = cmd.getArgs();
	pid_t pid;
	int nargs = args.size();
	set<Job, JobCompare>::iterator job;
	// if no arguments were given to fg
	if (nargs == NO_ARGS) {
		// the list is empty
		if (jobList.size() == 0) {
			perrorSmashInternal(_cmd, "jobs list is empty");
			return;
		}
		// the list has at least one job
		else {
			job = prev(jobList.end());
			id = job->getJobID();
			pid = job->getJobPid();
		}
	}
	// arguments were given
	else if (nargs == FG_ARGS) {
		try {
			id = stoi(args[ID_ARG]);
		} catch(const exception& ex) {
			perrorSmashInternal(_cmd, "invalid arguments");
			return;
		}
		Job tmp(id);
		job = jobList.find(tmp);
		if (job == jobList.end()) {
			string msg = "job id " + to_string(id) + " does not exist";
			perrorSmashInternal(cmd.getCmd(), msg);
			return;
		}
	}
	else {
		perrorSmashInternal(cmd.getCmd(), "invalid arguments");
		return;
	}

	pid = job->getJobPid();
	if (job->getStatus() == STOPPED) {
				kill(pid, SIGCONT);
	}
	_cmd = rebuildCmd(job->getArgs(), job->isBg());
	cout << "[" << id << "] " << _cmd << endl;
	if (waitpid(pid, NULL, 0) == -1) {
		perror("smash error: waitpid failed");
		exit(WAITPID);
	}
	jobList.erase(job);
}

void intBg(const Command& cmd) {
	int id;
	string _cmd;
	vector<string> args = cmd.getArgs();
	pid_t pid;
	int nargs = args.size();
	set<Job, JobCompare>::iterator job;
	// if no arguments were given to bg
	if (nargs == NO_ARGS) {
		// the list is empty
		if (jobList.size() == 0) {
			perrorSmashInternal(_cmd, "there are no stopped jobs to resume");
			return;
		}
		// the list has at least one job
		else {
			auto it = prev(jobList.end());
			while(true) {
				if (it->getStatus() == STOPPED) {
					job = it;
					id = job->getJobID();
					pid = job->getJobPid();
					break;
				}

				// got to the first element, and found no stopped jobs
				if (it == jobList.begin()) {
					perrorSmashInternal(_cmd, "there are no stopped jobs to resume");
					return;
				}

				it--;
			}
		}
	}
	// arguments were given
	else if (nargs == FG_ARGS) {
		try {
			id = stoi(args[ID_ARG]);
		} catch(const exception& ex) {
			perrorSmashInternal(_cmd, "invalid arguments");
			return;
		}
		Job tmp(id);
		job = jobList.find(tmp);
		if (job == jobList.end()) {
			string msg = "job id " + to_string(id) + " does not exist";
			perrorSmashInternal(cmd.getCmd(), msg);
			return;
		}
	}
	// invalid arguments were given
	else {
		perrorSmashInternal(cmd.getCmd(), "invalid arguments");
		return;
	}

	pid = job->getJobPid();
	// if given job ID is a stopped job
	if (job->getStatus() == STOPPED) {
				kill(pid, SIGCONT);
				const_cast<Job&>(*job).setStatus(NOSTATUS);
	}
	// if given job ID is not stopped
	else {
		string msg = "job id " + to_string(id) + " is already in background";
		perrorSmashInternal(cmd.getCmd(), msg);
		return;
	}
	_cmd = rebuildCmd(job->getArgs(), job->isBg());
	cout << _cmd << ": " << pid << endl;
}

void intQuit(const Command& cmd) {
	int id;
	pid_t pid;
	vector<string> args = cmd.getArgs();
	int nargs = args.size();
	// to many arguments
	if (nargs > QUIT_ARGS) {
		perrorSmashInternal(cmd.getCmd(), "expected 0 or 1 arguments");
		return;
	}
	// no arguments
	else if (nargs == NO_ARGS) {
		exit(SMASH_SUCCESS);
	}
	// additional argument given
	else if(nargs == QUIT_ARGS) {
		// not "kill" argument
		if (args[KILL_ARG] != QUIT_KILL) {
			perrorSmashInternal(cmd.getCmd(), "unexpected arguments");
			return;
		}
		for (auto job = jobList.begin(); job != jobList.end(); ) {
			id = job->getJobID();
			pid = job->getJobPid();
			int status;
			cout << "[" << id << "] " << rebuildCmd(job->getArgs(), job->isBg());
			cout << " - sending SIGTERM... ";
			cout << flush;
			kill(pid, SIGTERM);
			sleep(WAIT_TIME);
			int waitedpid = waitpid(pid, &status, WNOHANG);
			if (waitedpid && WIFSIGNALED(status)) {
				cout << "done" << endl;
			}
			else {
				cout << "sending SIGKILL...";
				kill(pid, SIGKILL);
				cout << "done" << endl;
			}
			job++;
		}
		exit(SMASH_SUCCESS);
	}
}

void intDiff(const Command& cmd) {
	vector<string> args = cmd.getArgs();
	string output = IDENT;
	int nargs = args.size();
	if (nargs != DIFF_ARGS) {
		perrorSmashInternal(cmd.getCmd(), "expected 2 arguments");
	}
	else {
		int fd1, fd2;
		ssize_t read1, read2;
		char block1[BLOCK_SIZE], block2[BLOCK_SIZE];
		if (openFileAndCheck(args[FILE_1], &fd1)) {
			if (openFileAndCheck(args[FILE_2], &fd2)) {
				do {
					read1 = read(fd1, block1, (size_t) BLOCK_SIZE);
					read2 = read(fd2, block2, (size_t) BLOCK_SIZE);
					if ((int) read1 < 0 or (int) read2 < 0) {
						perror("smash error: read failed");
						safeClose(fd1);
						safeClose(fd2);
						exit(READ);
					}
					else if (read1 != read2) {
						output = DIFFER;
					}
					for (int byte = 0; byte < read1; byte++) {
						if (block1[byte] != block2[byte]) {
							output = DIFFER;
						}
					}
				}
				while ((read1 > 0 && read2 > 0) && (output != DIFFER));
				cout << output << endl;
				safeClose(fd1);
				safeClose(fd2);
			}
		}
	}
}