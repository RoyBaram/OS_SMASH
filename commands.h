#ifndef COMMANDS_H
#define COMMANDS_H
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include "prints.h"
#include "signals.h"

#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <sstream>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

using namespace std;

#define CMD_LENGTH_MAX 80
#define ARGS_NUM_MAX 20
#define JOBS_NUM_MAX 100
#define NO_ID -1
#define WAIT_TIME 5
#define NO_ARGS 1
#define CD_ARGS 2
#define FG_ARGS 2
#define QUIT_ARGS 2
#define KILL_ARG 1
#define QUIT_KILL "kill"
#define KILL_ARGS 3
#define EXEC 0
#define NO_EXEC 1
#define ID_ARG 1
#define SIGNUM 2
#define FILE_1 1
#define FILE_2 2
#define DIFF_ARGS 3
#define BAD_FD -1
#define BLOCK_SIZE 256
#define DIFFER "1"
#define IDENT "0"

/*=============================================================================
* error handling - some useful macros and examples of error handling,
* feel free to not use any of this
=============================================================================*/
#define ERROR_EXIT(msg) \
    do { \
        cerr << __FILE__ << ": " << __LINE__ << \
        endl << msg; \
        exit(1); \
    } while(0);

/*=============================================================================
* classes/structs declarations
=============================================================================*/

typedef enum {
    NOSTATUS = 0,
    BACKGROUND,
    STOPPED
} JobStatus;

class Command {
private:
    bool bg;
    string cmd;
    vector<string> args;

public:
    Command(bool bg = false, const string& cmd = "", const vector<string>& args = {})
        : bg(bg), cmd(cmd), args(args) {}

    void setCmd(const string& newCmd) {
        cmd = newCmd;
    }

    string getCmd() const {
        return cmd;
    }

    void setBg(bool bgCond) {
        bg = bgCond;
    }

    bool isBg() const {
        return bg;
    }

    const vector<string>& getArgs() const {
        return args;
    }

    void setArgs(const vector<string>& newArgs) {
        args = newArgs;
    }
};

class Job : public Command {
private:
    int jobID;
    pid_t pid;
    JobStatus status;
    time_t startTime;

public:
    Job() : Command(), jobID(NO_ID), pid(NO_ID), status(NOSTATUS), startTime(0) {}
    Job(int ID) : Command(), jobID(ID), pid(NO_ID), status(NOSTATUS), startTime(0) {}
    Job(int ID, Command cmd) : Command(cmd), jobID(ID), pid(NO_ID), status(NOSTATUS), startTime(0) {}

    int getJobID() const {
        return jobID;
    }

    void setJobID(int newID) {
        jobID = newID;
    }

    JobStatus getStatus() const {
        return status;
    }

    void setStatus(JobStatus newStatus) {
        status = newStatus;
    }

    pid_t getJobPid() const {
        return pid;
    }

    void setJobPid(pid_t newPid) {
        pid = newPid;
    }

    time_t getJobStart() const {
        return startTime;
    }

    void setJobStart(time_t sTime) {
        startTime = sTime;
    }
};

class JobIDs {
private:
    set<int> available;

public:
    JobIDs() {
        for (int i = 0; i < JOBS_NUM_MAX; i++) {
            available.insert(i);
        }
    }

    int getID() {
        int id = *available.begin();
        available.erase(id);
        return id;
    }

    void insertID(int id) {
        available.insert(id);
    }

    bool isEmpty() {
        return available.empty();
    }

    size_t getSize() {
        return available.size();
    }
};

struct JobCompare {
    bool operator()(const Job& a, const Job& b) const {
        return a.getJobID() < b.getJobID();
    }
};

/*=============================================================================
* error definitions, enums
=============================================================================*/
typedef enum  {
	INVALID_COMMAND = 1,
	//feel free to add more values here or delete this
} ParsingError;

typedef enum {
	SMASH_SUCCESS = 0,
	SMASH_QUIT,
	SMASH_FAIL
	//feel free to add more values here or delete this
} CommandResult;

typedef enum {
    EXECV = 1,
    TIME,
    GETCWD,
    FORK,
    WAITPID,
    CLOSE,
    READ
} CmdErr;

/*=============================================================================
* global functions
=============================================================================*/
int parseCmd(const string& line, Command& result);
void chkUpdtJoblStatus(set<Job, JobCompare>& jobList);
string getcwdStr();
bool checkNoArgs(const vector<string>& args);
void runNewProc(
    const Command& cmd,
    unordered_map<string, function<void(const Command&)>>::iterator it,
    int option);
void executeCommand(const Command& cmd);

/*=============================================================================
* internal functions
=============================================================================*/
void intShowpid(const Command& cmd);
void intPwd(const Command& cmd);
void intCd(const Command& cmd);
void intJobs(const Command& cmd);
void intKill(const Command& cmd);
void intFg(const Command& cmd);
void intBg(const Command& cmd);
void intQuit(const Command& cmd);
void intDiff(const Command& cmd);
void intDoNothing(const Command& cmd);

#endif //COMMANDS_H